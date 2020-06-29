#include "9cc.h"

typedef struct VarScope VarScope;
struct VarScope {
  VarScope *next;
  int depth;
  Var *var;
};

typedef struct TagScope TagScope;
struct TagScope {
  TagScope *next;
  char *name;
  int depth;
  Type *ty;
};

static Token *current_token;

static Var *locals;
static Var *globals;

// C has two block scope;
// one is for variables and the other is for struct tags.
static VarScope *var_scope;
static TagScope *tag_scope;

// scope_depth is incremented at "{" and decremented at "}".
static int scope_depth;

// Find local variable by name.
static Var *find_var(Token *tok) {
  for (VarScope *sc = var_scope; sc; sc = sc->next)
    if (strlen(sc->var->name) == tok->len &&
        !strncmp(tok->loc, sc->var->name, tok->len))
      return sc->var;

  for (Var *var = globals; var; var = var->next)
    if (strlen(var->name) == tok->len &&
        !strncmp(tok->loc, var->name, tok->len))
      return var;

  return NULL;
}

static TagScope *find_tag(Token *tag) {
  for (TagScope *sc = tag_scope; sc; sc = sc->next)
    if (strlen(sc->name) == tag->len &&
        !strncmp(sc->name, tag->loc, tag->len))
      return sc;

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

static void push_scope(Var *var) {
  VarScope *sc = calloc(1, sizeof(VarScope));
  sc->next = var_scope;
  sc->depth = scope_depth;
  sc->var = var;
  var_scope = sc;
}

static Var *new_lvar(Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = strndup(ty->name->loc, ty->name->len);
  var->next = locals;
  var->ty = ty;
  var->is_local = true;
  push_scope(var);
  locals = var;
  return var;
}

static Var *new_gvar(char *name, Type *ty) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = globals;
  var->ty = ty;
  var->is_local = false;
  globals = var;
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

static Program *program();
static Var *global_var();
static Function *funcdef();
static Type *typespec();
static Type *struct_decl();
static Type *func_params(Type *ty);
static Type *declarator(Type *type);
static Type *type_suffix(Type *type);
static Node *compound_stmt();
static Node *declaration();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *postfix();
static Node *primary();

/*
 * Production rules:
 *   program = (funcdef | global_var)*
 *   funcdef = typespec func_name "(" func_params ")" "{" compound_stmt "}"
 *   typespec = "int" | "char" | struct_decl
 *   struct_decl = "struct" ident? ("{" struct_members "}")?
 *   func_params = typespec declarator ("," typespec declarator)*
 *   declarator = "*"* ident type_suffix
 *   type_suffix = "[" num "]"
 *               | "(" func_params ")"
 *               | ε
 *   compound_stmt = (declaration | stmt)*
 *   declaration = typespec declarator ("=" expr)? ";"
 *   stmt = "return" expr ";"
 *        | "if" "(" expr ")" stmt ("else" stmt)?
 *        | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
 *        | "while" "(" expr ")" stmt
 *        | "{" stmt* "}"
 *        | expr_stmt ";"
 *   expr_stmt = expr
 *   expr = assign
 *   assign = equality ("=" assign)?
 *   equality = relational ("==" relational | "!=" relational)*
 *   relational = add ("<" add | "<=" add | ">=" add | ">" add)*
 *   add = mul ("+" mul | "-" mul)*
 *   mul = unary ("*" unary | "/" unary)*
 *   unary = ("+" | "-" | "*" | "&") unary
 *         | "sizeof" unary
 *         | postfix
 *   postfix = primary ("[" expr "]" | "." ident | "->" indent)?
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
    Type *base_ty = typespec();
    Type *ty = declarator(base_ty);

    // Function declaration
    if (ty->kind == TY_FUNC && consume(";")) {
      continue;
    }

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
  Type *base_ty = typespec();
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
    Type *base_ty = typespec();
    Type *param_ty = declarator(base_ty);
    new_lvar(param_ty);
    cur = cur->next = param_ty;
  }

  ty = func_type(ty);
  ty->params = head.next;
  return ty;
}

// typespec = "void" | "char" | "int"
//          | struct_decl
static Type *typespec() {
  if (consume("void"))
    return ty_void;

  if (consume("char"))
    return ty_char;

  if (consume("int"))
    return ty_int;

  if (equal(current_token, "struct"))
    return struct_decl();

  error_at(current_token->loc, "typename expected");
}

// struct_members = (typespec declarator ";")*
static Member *struct_members() {
  Member head = {};
  Member *cur = &head;

  while (!equal(current_token, "}")) {
    Type *base_ty = typespec();
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

// type_suffix = "[" num "]"
//             | "(" func_params ")"
//             | ε
static Type *type_suffix(Type *ty) {
  if (consume("[")) {
    if (current_token->kind != TK_NUM)
      error_at(current_token->loc, "expected a number");
    ty = array_of(ty, current_token->val);
    current_token = current_token->next;
    skip("]");
    return ty;
  }

  if (consume("(")) {
    ty = func_params(ty);
    skip(")");
    return ty;
  }

  return ty;
}

// declaration = typespec (declarator ("=" expr)?)? ";"
static Node *declaration() {
  Type *base_ty = typespec();

  Node *node = new_node(ND_BLOCK);
  node->body = NULL;

  if (consume(";"))
    return node;

  Type *ty = declarator(base_ty);
  if (ty->kind == TY_VOID)
    error_at(current_token->loc, "variable declared void");

  Var *var = new_lvar(ty);

  if (!equal(current_token, "=")) {
    skip(";");
    return node;
  }

  skip("=");
  Node *var_node = new_node(ND_VAR);
  var_node->var = var;
  Node *assign_node = new_binary_node(ND_ASSIGN, var_node, expr());
  node->body = new_unary_node(ND_EXPR_STMT, assign_node);
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
  static char *kw[] = {"void", "char", "int", "struct"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
        return true;

  return false;
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

  Type *base_ty = typespec();

  fn->name = strndup(current_token->loc, current_token->len);
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
//      | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" stmt* "}"
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

  if (consume("for")) {
    Node *node = new_node(ND_FOR);
    skip("(");

    if (!equal(current_token, ";"))
      node->init = expr_stmt();
    skip(";");

    if (!equal(current_token, ";"))
      node->cond = expr();
    skip(";");

    if (!equal(current_token, ")"))
      node->inc = expr_stmt();
    skip(")");

    node->then = stmt();
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

// assign = equality ("=" assign)?
static Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary_node(ND_ASSIGN, node, assign());
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

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-" | "*" | "&") unary
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

// postfix = primary ("[" expr "]" | "." ident | "->" indent)?
static Node *postfix() {
  Node *node = primary();

  if (consume("[")) {
    Node *idx = expr();
    node = new_unary_node(
      ND_DEREF,
      new_add_node(node, idx)
    );
    skip("]");
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

static char *new_gvar_name(void) {
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

    // Variable
    Var *var = find_var(current_token);
    if (!var)
      error_at(current_token->loc, "undefined variable");
    Node *node = new_node(ND_VAR);
    node->var = var;
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
