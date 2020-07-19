#include "9cc.h"

static Token *current_token;

// Scope for local, global variables, typedefs
// or enum constants.
typedef struct VarScope VarScope;
struct VarScope {
  VarScope *next;
  char *name;
  int depth;

  Var *var;
  Type *type_def;
  Type *enum_ty;
  int enum_val;
};

// Scopes for struct.
typedef struct TagScope TagScope;
struct TagScope {
  TagScope *next;
  char *name;
  int depth;
  Type *ty;
};

// Variable attributes such as typedef or extern.
typedef struct {
  bool is_typedef;
  bool is_static;
} VarAttr;

static Var *locals;
static Var *globals;

// C has two block scope;
// one is for variables/typedefs and
// the other is for struct tags.
static VarScope *var_scope;
static TagScope *tag_scope;

// scope_depth is incremented at "{" and decremented at "}".
static int scope_depth;

// Find variable or typedef by name.
static VarScope *find_var(Token *tok) {
  for (VarScope *sc = var_scope; sc; sc = sc->next)
    if (strlen(sc->name) == tok->len &&
        !strncmp(tok->loc, sc->name, tok->len))
      return sc;

  return NULL;
}

static TagScope *find_tag(Token *tag) {
  for (TagScope *sc = tag_scope; sc; sc = sc->next)
    if (strlen(sc->name) == tag->len &&
        !strncmp(sc->name, tag->loc, tag->len))
      return sc;

  return NULL;
}

static Type *find_typedef(Token *tok) {
  if (tok->kind != TK_IDENT)
    return NULL;

  VarScope *sc = find_var(tok);
  if (sc)
    return sc->type_def;
  else
    return NULL;
}

static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_unary_node(NodeKind kind, Node *lhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  return node;
}

static Node *new_num_node(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static VarScope *push_scope(char *name) {
  VarScope *sc = calloc(1, sizeof(VarScope));
  sc->next = var_scope;
  sc->name = name;
  sc->depth = scope_depth;
  var_scope = sc;
  return sc;
}

static Var *new_lvar(Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  char *name = strndup(ty->name->loc, ty->name->len);
  var->name = name;
  var->next = locals;
  var->ty = ty;
  var->is_local = true;
  locals = var;
  VarScope *sc = push_scope(name);
  sc->var = var;
  return var;
}

static Var *new_gvar(char *name, Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = globals;
  var->ty = ty;
  var->is_local = false;
  globals = var;
  VarScope *sc = push_scope(name);
  sc->var = var;
  return var;
}

static void push_tag_scope(Token *tag, Type *ty) {
  TagScope *sc = calloc(1, sizeof(TagScope));
  sc->next = tag_scope;
  sc->name = strndup(tag->loc, tag->len);
  sc->depth = scope_depth;
  sc->ty = ty;
  tag_scope = sc;
}

static int get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error_at(tok->loc, "expected a number");
  return tok->val;
}

static bool equal(Token *tok, char *op) {
  return strlen(op) == tok->len &&
         !strncmp(tok->loc, op, tok->len);
}

// 次のtokenが期待しているcharのとき、tokenを1つ進めて
// trueを返す。それ以外はfalseを返す。
static bool consume(char *op) {
  if (!equal(current_token, op))
    return false;
  current_token = current_token->next;
  return true;
}

static void skip(char *op) {
  if (consume(op))
    return;
  else
    error_at(current_token->loc, "Not '%s'", op);
}

// Points to node representing a switch if we are parsing
// a switch statement. Otherwise, NULL.
static Node *current_switch;

static Node *new_add_node(Node *lhs, Node *rhs);
static Node *new_sub_node(Node *lhs, Node *rhs);

static Program *program();
static Var *global_var();
static Function *funcdef();
static Type *typespec(VarAttr *attr);
static Type *struct_decl();
static Type *enum_specifier();
static Type *func_params(Type *ty);
static Type *declarator(Type *type);
static Type *type_suffix(Type *type);
static Node *compound_stmt();
static Node *declaration();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *logor();
static Node *logand();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *bitand();
static Node *unary();
static Node *postfix();
static Node *primary();

/*
 * Production rules:
 *   program = (funcdef | global_var)*
 *   funcdef = typespec func_name "(" func_params ")" "{" compound_stmt "}"
 *   typespec = "void" | "_Bool" | "char" | "int"
 *            | struct_decl | enum_specifier
 *            | ("typedef" typespec) | typedef-name
 *   struct_decl = "struct" ident? ("{" struct_members "}")?
 *   enum_specifier = "enum" "{" enum_list "}"
 *   enum_list = ident ("," ident)*
 *   func_params = typespec declarator ("," typespec declarator)*
 *   declarator = "*"* ident type_suffix
 *   type_suffix = "[" num "]" type_suffix
 *               | "(" func_params ")"
 *               | ε
 *   compound_stmt = (declaration | stmt)*
 *   declaration = typespec (declarator ("=" expr)?)? ";"
 *   stmt = "return" expr ";"
 *        | "if" "(" expr ")" stmt ("else" stmt)?
 *        | "switch" "(" expr ")" stmt
 *        | "case" num ":" stmt
 *        | "for" "(" (expr_stmt? ";" | declaration) expr? ";" expr_stmt? ")" stmt
 *        | "while" "(" expr ")" stmt
 *        | "break" ";"
 *        | "continue" ";"
 *        | "{" compound_stmt "}"
 *        | expr_stmt ";"
 *   expr_stmt = expr
 *   expr = assign
 *   assign = logor (assign_op assign)?
 *   assign_op = "=" | "+=" | "-=" | "*=" | "/="
 *   logor = logand ("||" logand)*
 *   logand = equality ("&&" equality)*
 *   equality = relational ("==" relational | "!=" relational)*
 *   relational = add ("<" add | "<=" add | ">=" add | ">" add)*
 *   add = mul ("+" mul | "-" mul)*
 *   mul = bitand ("*" bitand | "/" bitand)*
 *   bitand = unary ("&" unary)*
 *   unary = ("+" | "-" | "*" | "&" | "~") unary
 *         | ("++" | "--") unary
 *         | "sizeof" unary
 *         | postfix
 *   postfix = primary (("[" expr "]")* | "." ident | "->" indent | "++" | "--")?
 *   primary = "(" "{" compound_stmt "}" ")"
 *           | "(" expr ")"
 *           | ident ("(" func_args? ")")?
 *           | str
 *           | num
 */

// program = (funcdef | global_var)*
static Program *program() {
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (current_token->kind != TK_EOF) {
    Token *tmp = current_token;
    VarAttr attr = {};
    Type *base_ty = typespec(&attr);
    Type *ty = declarator(base_ty);

    // Typedef
    if (attr.is_typedef) {
      char *typedef_name = strndup(ty->name->loc, ty->name->len);
      VarScope *sc = push_scope(typedef_name);
      sc->type_def = ty;
      skip(";");
      continue;
    }

    // Function declaration
    if (ty->kind == TY_FUNC && consume(";"))
      continue;

    current_token = tmp;

    // Function
    if (ty->kind == TY_FUNC) {
      cur = cur->next = funcdef();
      continue;
    }

    // Gloval variable
    global_var();
  }

  // Assign offsets to local variables.
  for (Function *fn = head.next; fn; fn = fn->next) {
    int offset = 32; // 32 for callee-saved registers
    for (Var *var = fn->locals; var; var = var->next) {
      offset += var->ty->size;
      var->offset = offset;
    }
    fn->stack_size = align_to(offset, 16);
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->funcs = head.next;
  return prog;
}

// global_var = typespec declarator ";"
static Var *global_var() {
  Type *base_ty = typespec(NULL);
  Type *ty = declarator(base_ty);
  Var *var = new_gvar(strndup(ty->name->loc, ty->name->len), ty);
  skip(";");
  return var;
}

// func_params = typespec declarator ("," typespec declarator)*
static Type *func_params(Type *ty) {
  Type head = {};
  Type *cur = &head;

  while (!equal(current_token, ")")) {
    if (cur != &head)
      skip(",");
    Type *base_ty = typespec(NULL);
    Type *param_ty = declarator(base_ty);
    new_lvar(param_ty);
    cur = cur->next = param_ty;
  }

  ty = func_type(ty);
  ty->params = head.next;
  return ty;
}

// typespec = "void" | "_Bool" | "char" | "int"
//          | struct_decl | enum_specifier
//          | ("typedef" typespec) | ("static" typespec)
//          | typedef-name
static Type *typespec(VarAttr *attr) {
  if (consume("void"))
    return ty_void;

  if (consume("_Bool"))
    return ty_bool;

  if (consume("char"))
    return ty_char;

  if (consume("int"))
    return ty_int;

  if (equal(current_token, "struct"))
    return struct_decl();

  if (equal(current_token, "enum"))
    return enum_specifier();

  if (equal(current_token, "typedef") || equal(current_token, "static")) {
    if (!attr)
      error_at(current_token->loc, "storage class specifier is not allowed in this context");

    if (equal(current_token, "typedef"))
      attr->is_typedef = true;
    else
      attr->is_static = true;

    if (attr->is_typedef + attr->is_static > 1)
      error_at(current_token->loc, "typedef and static may not be used together");

    current_token = current_token->next;
    return typespec(attr);
  }

  // Typedef name
  Type *ty = find_typedef(current_token);
  current_token = current_token->next;
  if (ty)
    return ty;

  error_at(current_token->loc, "typename expected");
}

// struct_members = (typespec declarator ";")*
static Member *struct_members() {
  Member head = {};
  Member *cur = &head;

  while (!equal(current_token, "}")) {
    Type *base_ty = typespec(NULL);
    Type *ty = declarator(base_ty);

    Member *mem = calloc(1, sizeof(Member));
    mem->ty = ty;
    mem->name = ty->name;
    cur = cur->next = mem;

    skip(";");
  }

  return head.next;
}

// struct_decl = "struct" ident? ("{" struct_members "}")?
static Type *struct_decl() {
  skip("struct");

  Token *tag = NULL;
  if (current_token->kind == TK_IDENT) {
    tag = current_token;
    current_token = current_token->next;
  }

  if (tag && !equal(current_token, "{")) {
    TagScope *sc = find_tag(tag);
    if (!sc)
      error_at(tag->loc, "unknown struct type");
    return sc->ty;
  }

  skip("{");

  // Construct a struct object.
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_STRUCT;
  ty->members = struct_members();

  // Assign offsets to members.
  int offset = 0;
  for (Member *mem = ty->members; mem; mem = mem->next) {
    offset = align_to(offset, mem->ty->align);
    mem->offset = offset;
    offset += mem->ty->size;

    if (ty->align < mem->ty->align)
      ty->align = mem->ty->align;
  }
  ty->size = align_to(offset, ty->align);

  skip("}");

  // Register the struct type if a name was given.
  if (tag)
    push_tag_scope(tag, ty);

  return ty;
}

// enum_specifier = "enum" "{" enum_list "}"
// enum_list = ident ("," ident)*
static Type *enum_specifier() {
  Type *ty = enum_type();

  skip("enum");
  skip("{");

  // Read an enum-list.
  int i = 0;
  while (!equal(current_token, "}")) {
    if (i != 0)
      skip(",");

    if (current_token->kind != TK_IDENT)
      error_at(current_token->loc, "expected ident for enum list");

    char *name = strndup(current_token->loc, current_token->len);
    current_token = current_token->next;

    VarScope *sc = push_scope(name);
    sc->enum_ty = ty;
    sc->enum_val = i;

    i++;
  }

  skip("}");

  return ty;
}

// declarator = "*"* ident type_suffix
static Type *declarator(Type *ty) {
  while (consume("*"))
    ty = pointer_to(ty);

  if (current_token->kind != TK_IDENT)
    error_at(current_token->loc, "expected a variable name");

  Token *name_tok = current_token;
  current_token = current_token->next;

  ty = type_suffix(ty);
  ty->name = name_tok;
  return ty;
}

// type_suffix = "[" num "]" type_suffix
//             | "(" func_params ")"
//             | ε
static Type *type_suffix(Type *ty) {
  if (consume("[")) {
    if (current_token->kind != TK_NUM)
      error_at(current_token->loc, "expected a number");
    int len = current_token->val;
    current_token = current_token->next;
    skip("]");
    ty = type_suffix(ty);
    return array_of(ty, len);
  }

  if (consume("(")) {
    ty = func_params(ty);
    skip(")");
    return ty;
  }

  return ty;
}

// "{" (expr ("," expr)*)? "}"
static Node *array_initializer(Node *var_node) {
  if (var_node->var->ty->kind != TY_ARRAY)
    error_at(current_token->loc, "an array initializer for non array type variable");

  skip("{");

  Node head = {};
  Node *cur = &head;
  int i = 0;
  while (!equal(current_token, "}")) {
    if (i != 0)
      skip(",");

    // Buid nodes representing `*(a + 2) = expr;`
    Node *deref_node = new_unary_node(
      ND_DEREF,
      new_add_node(var_node, new_num_node(i))
    );
    Node *assign_node = new_binary_node(ND_ASSIGN, deref_node, expr());
    cur = cur->next = new_unary_node(ND_EXPR_STMT, assign_node);
    i++;
  }

  if (var_node->var->ty->array_len != i)
    error_at(current_token->loc, "wrong number of array length");

  skip("}");
  return head.next;
}

// declaration = typespec (declarator ("=" expr)?)? ";"
static Node *declaration() {
  VarAttr attr = {};
  Type *base_ty = typespec(&attr);

  Node *node = new_node(ND_BLOCK);
  node->body = NULL;

  if (consume(";"))
    return node;

  Type *ty = declarator(base_ty);
  if (ty->kind == TY_VOID)
    error_at(current_token->loc, "variable declared void");

  if (attr.is_typedef) {
    char *name = strndup(ty->name->loc, ty->name->len);
    VarScope *sc = push_scope(name);
    sc->type_def = ty;
    skip(";");
    return node;
  }

  Var *var = new_lvar(ty);

  if (!equal(current_token, "=")) {
    skip(";");
    return node;
  }

  skip("=");
  Node *var_node = new_node(ND_VAR);
  var_node->var = var;

  if (equal(current_token, "{")) {
    // linked list of assign statements
    node->body = array_initializer(var_node);
  } else {
    Node *assign_node = new_binary_node(ND_ASSIGN, var_node, expr());
    node->body = new_unary_node(ND_EXPR_STMT, assign_node);
  }
  skip(";");
  return node;
}

static void enter_scope() {
  scope_depth++;
}

static void leave_scope() {
  scope_depth--;

  while (var_scope && var_scope->depth > scope_depth)
    var_scope = var_scope->next;

  while (tag_scope && tag_scope->depth > scope_depth)
    tag_scope = tag_scope->next;
}

static bool is_typename(Token *tok) {
  static char *kw[] = {"void", "_Bool", "char", "int", "struct", "typedef", "enum", "static"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
        return true;

  return find_typedef(tok);
}

// compound_stmt = (declaration | stmt)*
static Node *compound_stmt() {
  Node head = {};
  Node *cur = &head;

  enter_scope();

  while (!equal(current_token, "}")) {
    if (is_typename(current_token))
      cur = cur->next = declaration();
    else
      cur = cur->next = stmt();
    add_type(cur);
  }

  leave_scope();

  return head.next;
}

// funcdef = typespec func_name "(" func_params ")" "{" compound_stmt "}"
static Function *funcdef() {
  locals = NULL;
  Function *fn = calloc(1, sizeof(Function));

  VarAttr attr = {};
  Type *base_ty = typespec(&attr);

  fn->name = strndup(current_token->loc, current_token->len);
  fn->is_static = attr.is_static;
  current_token = current_token->next;

  enter_scope();

  // Params
  skip("(");
  func_params(base_ty);
  fn->params = locals;
  skip(")");

  // Body
  skip("{");
  Node *block_node = new_node(ND_BLOCK);
  block_node->body = compound_stmt();
  fn->node = block_node;
  fn->locals = locals;
  skip("}");

  leave_scope();

  return fn;
}

// expr_stmt = expr
static Node *expr_stmt() {
  return new_unary_node(ND_EXPR_STMT, expr());
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "switch" "(" expr ")" stmt
//      | "case" num ":" stmt
//      | "default" ":" stmt
//      | "for" "(" (expr_stmt? ";" | declaration) expr? ";" expr_stmt? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "break" ";"
//      | "continue" ";"
//      | "{" compound_stmt "}"
//      | expr_stmt ";"
static Node *stmt() {
  if (consume("return")) {
    Node *node = new_unary_node(ND_RETURN, expr());
    skip(";");
    return node;
  }

  if (consume("if")) {
    Node *node = new_node(ND_IF);
    skip("(");
    node->cond = expr();
    skip(")");
    node->then = stmt();
    if (consume("else"))
      node->els = stmt();
    return node;
  }

  if (consume("switch")) {
    Node *node = new_node(ND_SWITCH);
    skip("(");
    node->cond = expr();
    skip(")");

    Node *outside_sw = current_switch;
    current_switch = node;
    node->then = stmt();
    current_switch = outside_sw;
    return node;
  }

  if (consume("case")) {
    if (!current_switch)
      error_at(current_token->loc, "stray case");

    if (current_token->kind != TK_NUM)
      error_at(current_token->loc, "expected number");
    int val = current_token->val;
    current_token = current_token->next;

    Node *node = new_node(ND_CASE);
    node->val = val;
    skip(":");
    node->lhs = stmt();
    node->case_next = current_switch->case_next;
    current_switch->case_next = node;
    return node;
  }

  if (consume("default")) {
    if (!current_switch)
      error_at(current_token->loc, "stray default");

    if (current_switch->default_case)
      error_at(current_token->loc, "duplicated default");

    Node *node = new_node(ND_CASE);
    skip(":");
    node->lhs = stmt();
    current_switch->default_case = node;
    return node;
  }

  if (consume("for")) {
    Node *node = new_node(ND_FOR);
    skip("(");

    enter_scope();

    if (is_typename(current_token)) {
      node->init = declaration();
    } else {
      if (!equal(current_token, ";"))
        node->init = expr_stmt();
      skip(";");
    }

    if (!equal(current_token, ";"))
      node->cond = expr();
    skip(";");

    if (!equal(current_token, ")"))
      node->inc = expr_stmt();
    skip(")");

    node->then = stmt();

    leave_scope();
    return node;
  }

  if (consume("while")) {
    Node *node = new_node(ND_WHILE);
    skip("(");
    node->cond = expr();
    skip(")");
    node->then = stmt();
    return node;
  }

  if (consume("break")) {
    skip(";");
    return new_node(ND_BREAK);
  }

  if (consume("continue")) {
    skip(";");
    return new_node(ND_CONTINUE);
  }

  if (consume("{")) {
    Node *node = new_node(ND_BLOCK);
    node->body = compound_stmt();
    skip("}");
    return node;
  }

  Node *node = expr_stmt();
  skip(";");
  return node;
}

// expr = assign
static Node *expr() {
  return assign();
}

// assign = logor (assign_op assign)?
// assign_op = "=" | "+=" | "-=" | "*=" | "/="
static Node *assign() {
  Node *node = logor();

  if (consume("="))
    node = new_binary_node(ND_ASSIGN, node, assign());
  else if (consume("+="))
    node = new_binary_node(
      ND_ASSIGN,
      node,
      new_add_node(node, assign())
    );
  else if (consume("-="))
    node = new_binary_node(
      ND_ASSIGN,
      node,
      new_sub_node(node, assign())
    );
  else if (consume("*="))
    node = new_binary_node(
      ND_ASSIGN,
      node,
      new_binary_node(ND_MUL, node, assign())
    );
  else if (consume("/="))
    node = new_binary_node(
      ND_ASSIGN,
      node,
      new_binary_node(ND_DIV, node, assign())
    );

  return node;
}

// logor = logand ("||" logand)*
static Node *logor() {
  Node *node = logand();

  while (equal(current_token, "||")) {
    skip("||");
    node = new_binary_node(ND_LOGOR, node, logand());
  }

  return node;
}

// logand = equality ("&&" equality)*
static Node *logand() {
  Node *node = equality();

  while (equal(current_token, "&&")) {
    skip("&&");
    node = new_binary_node(ND_LOGAND, node, equality());
  }

  return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">=" add | ">" add)*
static Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_binary_node(ND_LET, node, relational());
    else if (consume(">"))
      node = new_binary_node(ND_LAT, node, relational());
    else if (consume("<="))
      node = new_binary_node(ND_LEE, node, relational());
    else if (consume(">="))
      node = new_binary_node(ND_LAE, node, relational());
    else
      return node;
  }
}

static Node *new_add_node(Node *lhs, Node *rhs) {
  add_type(lhs);
  add_type(rhs);

  // num + num
  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary_node(ND_ADD, lhs, rhs);

  if (lhs->ty->base && rhs->ty->base)
    error(current_token->loc, "invalid operands");

  // ptr + num
  return new_binary_node(
    ND_ADD,
    lhs,
    new_binary_node(
      ND_MUL,
      rhs,
      new_num_node(lhs->ty->base->size)
    )
  );
}

static Node *new_sub_node(Node *lhs, Node *rhs) {
  add_type(lhs);
  add_type(rhs);

  // num - num
  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary_node(ND_SUB, lhs, rhs);

  // ptr - num
  if (lhs->ty->kind == TY_PTR && is_integer(rhs->ty)) {
    return new_binary_node(
        ND_SUB,
        lhs,
        new_binary_node(
          ND_MUL,
          rhs,
          new_num_node(lhs->ty->base->size)
        )
    );
  }

  // ptr - ptr
  if (lhs->ty->kind == TY_PTR && rhs->ty->kind == TY_PTR) {
    return new_binary_node(
        ND_DIV,
        new_binary_node(ND_SUB, lhs, rhs),
        new_num_node(lhs->ty->base->size)
    );
  }

  error_at(current_token->loc, "invalid operand of \"-\"");
}

// add = mul ("+" mul | "-" mul)*
static Node *add(){
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_add_node(node, mul());
    else if (consume("-"))
      node = new_sub_node(node, mul());
    else
      return node;
  }
}

// mul = bitand ("*" bitand | "/" bitand)*
static Node *mul() {
  Node *node = bitand();

  for (;;) {
    if (consume("*"))
      node = new_binary_node(ND_MUL, node, bitand());
    else if (consume("/"))
      node = new_binary_node(ND_DIV, node, bitand());
    else
      return node;
  }
}

// bitand = unary ("&" unary)*
static Node *bitand() {
  Node *node = unary();

  while (equal(current_token, "&")) {
    skip("&");
    node = new_binary_node(ND_BITAND, node, unary());
  }

  return node;
}

// unary = ("+" | "-" | "*" | "&" | "~") unary
//       | ("++" | "--") unary
//       | "sizeof" unary
//       | postfix
static Node *unary() {
  if (consume("+"))
    return unary();

  if (consume("-"))
    return new_binary_node(ND_SUB, new_num_node(0), unary());

  if (consume("*"))
    return new_unary_node(ND_DEREF, unary());

  if (consume("&"))
    return new_unary_node(ND_ADDR, unary());

  if (consume("~"))
    return new_unary_node(ND_BITNOT, unary());

  if (consume("++")) {
    Node *node = unary();
    return new_binary_node(
      ND_ASSIGN,
      node,
      new_add_node(node, new_num_node(1))
    );
  }

  if (consume("--")) {
    Node *node = unary();
    return new_binary_node(
      ND_ASSIGN,
      node,
      new_sub_node(node, new_num_node(1))
    );
  }

  if (consume("sizeof")) {
    Node *node = unary();
    add_type(node);
    return new_num_node(node->ty->size);
  }

  return postfix();
}

static Member *get_struct_member(Type *ty) {
  for (Member *mem = ty->members; mem; mem = mem->next)
    if (mem->name->len == current_token->len &&
        !strncmp(mem->name->loc, current_token->loc, mem->name->len))
      return mem;

  error_at(current_token->loc, "no such member");
}

static Node *struct_ref(Node *lhs) {
  add_type(lhs);
  if (lhs->ty->kind != TY_STRUCT)
    error_at(current_token->loc, "not a struct");

  Node *node = new_unary_node(ND_MEMBER, lhs);
  Member *mem = get_struct_member(lhs->ty);
  node->member = mem;
  return node;
}

// Convert A++ to `({ A = A + 1; A - 1; })`
static Node *new_inc(Node *node) {
  Node *stmt_expr = new_node(ND_STMT_EXPR);

  Node *body = new_unary_node(
    ND_EXPR_STMT,
    new_binary_node(
      ND_ASSIGN,
      node,
      new_add_node(node, new_num_node(1))
    )
  );
  body->next = new_unary_node(
    ND_EXPR_STMT,
    new_sub_node(node, new_num_node(1))
  );

  stmt_expr->body = body;
  return stmt_expr;
}

// Convert A-- to `({ A = A - 1; A + 1; })`
static Node *new_dec(Node *node) {
  Node *stmt_expr = new_node(ND_STMT_EXPR);

  Node *body = new_unary_node(
    ND_EXPR_STMT,
    new_binary_node(
      ND_ASSIGN,
      node,
      new_sub_node(node, new_num_node(1))
    )
  );
  body->next = new_unary_node(
    ND_EXPR_STMT,
    new_add_node(node, new_num_node(1))
  );

  stmt_expr->body = body;
  return stmt_expr;
}

// postfix = primary (("[" expr "]")* | "." ident | "->" indent | "++" | "--")
static Node *postfix() {
  Node *node = primary();

  if (equal(current_token, "[")) {
    while (equal(current_token, "[")) {
      skip("[");
      Node *idx = expr();
      node = new_unary_node(
        ND_DEREF,
        new_add_node(node, idx)
      );
      skip("]");
    }
  }

  if (consume(".")) {
    node = struct_ref(node);
    current_token = current_token->next;
  }

  if (consume("->")) {
    // x->y is short for (*x).y
    node = new_unary_node(ND_DEREF, node);
    node = struct_ref(node);
    current_token = current_token->next;
  }

  if (consume("++"))
    node = new_inc(node);

  if (consume("--"))
    node = new_dec(node);

  return node;
}

static Node *func_args() {
  Node head = {};
  Node *cur = &head;

  while (!equal(current_token, ")")) {
    if (cur != &head)
      skip(",");
    cur = cur->next = expr();
  }

  return head.next;
}

static char *new_gvar_name() {
  static int cnt = 0;
  char *buf = malloc(20);
  sprintf(buf, ".L.data.%d", cnt++);
  return buf;
}

static Var *new_string_literal(Token *tok) {
  Type *ty = array_of(ty_char, tok->cont_len);
  Var *var = new_gvar(new_gvar_name(), ty);
  var->init_data = tok->contents;
  return var;
}

// primary = "(" "{" compound_stmt "}" ")"
//         | "(" expr ")"
//         | ident args?
//         | str
//         | num
// args = "(" func_args? ")"
static Node *primary() {
  if (equal(current_token, "(") && equal(current_token->next, "{")) {
    current_token = current_token->next->next;
    Node *node = new_node(ND_STMT_EXPR);
    node->body = compound_stmt();
    skip("}");
    skip(")");
    return node;
  }

  if (consume("(")) {
    Node *node = expr();
    skip(")");
    return node;
  }

  if (current_token->kind == TK_IDENT) {
    // Function call
    if (equal(current_token->next, "(")) {
      Node *funcall_node = new_node(ND_FUNCALL);
      funcall_node->funcname = strndup(current_token->loc, current_token->len);
      current_token = current_token->next;
      skip("(");

      Node *args = func_args();
      funcall_node->args = args;

      skip(")");
      return funcall_node;
    }

    // Variable or enum constant
    VarScope *sc = find_var(current_token);
    if (!sc || (!sc->var && !sc->enum_ty))
      error_at(current_token->loc, "undefined variable");

    Node *node;
    if (sc->var)
      node = new_node(ND_VAR);
    else
      node = new_num_node(sc->enum_val);

    node->var = sc->var;
    current_token = current_token->next;
    return node;
  }

  if (current_token->kind == TK_STR) {
    Var *var = new_string_literal(current_token);
    current_token = current_token->next;
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
  }

  if (current_token->kind == TK_NUM) {
    Node *node = new_num_node(get_number(current_token));
    current_token = current_token->next;
    return node;
  }

  error_at(current_token->loc, "unexpected token");
}

Program *parse(Token *tok) {
  current_token = tok;
  Program *prog = program();

  if (current_token->kind != TK_EOF)
    error_at(current_token->loc, "extra token");

  return prog;
}
