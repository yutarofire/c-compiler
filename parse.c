#include "9cc.h"

static Token *current_token;
static Var *locals;
static Var *globals;

static Program *program();
static Var *global_var();
static Function *funcdef();
static Type *typespec();
static void func_params();
static Node *declarator(Type *type);
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
 *   program = (global_var | funcdef)*
 *   funcdef = typespec func_name "(" func_params ")" "{" compound_stmt "}"
 *   typespec = "int"
 *   func_params = typespec declarator ("," typespec declarator)*
 *   declarator = "*"* ident ("[" num "]")?
 *   compound_stmt = (declaration | stmt)*
 *   declaration = typespec declarator ("=" expr)? ";"
 *   stmt = "return" expr ";"
 *        | "if" "(" expr ")" stmt ("else" stmt)?
 *        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *        | "while" "(" expr ")" stmt
 *        | "{" stmt* "}"
 *        | expr ";"
 *   expr = assign
 *   assign = equality ("=" assign)?
 *   equality = relational ("==" relational | "!=" relational)*
 *   relational = add ("<" add | "<=" add | ">=" add | ">" add)*
 *   add = mul ("+" mul | "-" mul)*
 *   mul = unary ("*" unary | "/" unary)*
 *   unary = ("+" | "-" | "*" | "&") unary
 *         | "sizeof" unary
 *         | postfix
 *   postfix = primary ("[" expr "]")*
 *   primary = "(" "{" compound_stmt "}" ")"
 *           | "(" expr ")"
 *           | ident ("(" func_args? ")")?
 *           | str
 *           | num
 */

// Find local variable by name.
static Var *find_var(Token *token) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == token->len &&
        !strncmp(token->str, var->name, token->len))
      return var;

  for (Var *var = globals; var; var = var->next)
    if (strlen(var->name) == token->len &&
        !strncmp(token->str, var->name, token->len))
      return var;

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

static Node *new_num_node(long val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Var *new_lvar(char *name, Type *type) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = locals;
  var->type = type;
  var->is_local = true;
  locals = var;
  return var;
}

static Var *new_gvar(char *name, Type *type) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = globals;
  var->type = type;
  var->is_local = false;
  globals = var;
  return var;
}

static int get_number(Token *token) {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  return token->val;
}

static bool equal(Token *token, char *op) {
  return strlen(op) == token->len &&
         !strncmp(token->str, op, token->len);
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
    error_at(current_token->str, "Not '%s'", op);
}

static int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

// program = (global_var | funcdef)*
static Program *program() {
  Function head = {};
  Function *cur = &head;
  globals = NULL;

  while (current_token->kind != TK_EOF) {
    Token *tmp = current_token;
    Type *ty = typespec();
    declarator(ty);
    bool is_func = equal(current_token, "(");
    current_token = tmp;

    // Function
    if (is_func) {
      cur = cur->next = funcdef();
      continue;
    }

    // Gloval variable
    Var *gvar = global_var();
  }

  // Assign offsets to local variables.
  for (Function *fn = head.next; fn; fn = fn->next) {
    int offset = 32; // 32 for callee-saved registers
    for (Var *var = fn->locals; var; var = var->next) {
      offset += var->type->size;
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
  Node *node = declarator(base_ty);
  new_gvar(node->var->name, node->var->type);
  skip(";");
}

// func_params = typespec declarator ("," typespec declarator)*
static void func_params() {
  int i = 0;
  while (!equal(current_token, ")")) {
    if (i != 0)
      skip(",");
    Type *type = typespec();
    declarator(type);
    i++;
  }
}

// typespec = "int"
static Type *typespec() {
  if (consume("int"))
    return type_int;

  if (consume("char"))
    return type_char;

  error_at(current_token->str, "invalid type");
}

// declarator = "*"* ident ("[" num "]")?
static Node *declarator(Type *type) {
  while (consume("*"))
    type = pointer_to(type);

  if (current_token->kind != TK_IDENT)
    error_at(current_token->str, "expected a variable name");

  char *name = strndup(current_token->str, current_token->len);
  current_token = current_token->next;

  if (equal(current_token, "[")) {
    skip("[");
    if (current_token->kind != TK_NUM)
      error_at(current_token->str, "expected a number");
    type = array_of(type, current_token->val);
    current_token = current_token->next;
    skip("]");
  }

  Var *var = new_lvar(name, type);
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

// declaration = typespec declarator ("=" expr)? ";"
static Node *declaration() {
  Type *type = typespec();
  Node *var_node = declarator(type);

  if (consume("=")) {
    Node *assign_node = new_binary_node(ND_ASSIGN, var_node, expr());
    skip(";");
    return new_unary_node(ND_EXPR_STMT, assign_node);
  }

  skip(";");
  return new_unary_node(ND_EXPR_STMT, var_node);
}

// compound_stmt = (declaration | stmt)*
static Node *compound_stmt() {
  Node head = {};
  Node *cur = &head;
  while (!equal(current_token, "}")) {
    if (equal(current_token, "int") ||
        equal(current_token, "char"))
      cur = cur->next = declaration();
    else
      cur = cur->next = stmt();
    add_type(cur);
  }
  return head.next;
}

// funcdef = typespec func_name "(" func_params ")" "{" compound_stmt "}"
static Function *funcdef() {
  locals = NULL;
  Function *fn = calloc(1, sizeof(Function));

  typespec();
  fn->name = strndup(current_token->str, current_token->len);
  current_token = current_token->next;

  // Params
  skip("(");
  func_params();
  fn->params = locals;
  skip(")");

  // Body
  skip("{");
  Node *block_node = new_node(ND_BLOCK);
  block_node->body = compound_stmt();
  fn->node = block_node;
  fn->locals = locals;
  skip("}");

  return fn;
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" stmt* "}"
//      | expr ";"
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
      node->init = declaration();
    else
      skip(";");

    if (!equal(current_token, ";"))
      node->cond = expr();
    skip(";");

    if (!equal(current_token, ")"))
      node->inc = new_unary_node(ND_EXPR_STMT, expr());
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

  Node *node = new_unary_node(ND_EXPR_STMT, expr());
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
  if (is_integer(lhs->type) && is_integer(rhs->type))
    return new_binary_node(ND_ADD, lhs, rhs);

  if (lhs->type->base && rhs->type->base)
    error(current_token->str, "invalid operands");

  // ptr + num
  return new_binary_node(
    ND_ADD,
    lhs,
    new_binary_node(
      ND_MUL,
      rhs,
      new_num_node(lhs->type->base->size)
    )
  );
}

static Node *new_sub_node(Node *lhs, Node *rhs) {
  add_type(lhs);
  add_type(rhs);

  // num - num
  if (is_integer(lhs->type) && is_integer(rhs->type))
    return new_binary_node(ND_SUB, lhs, rhs);

  // ptr - num
  if (lhs->type->kind == TY_PTR && is_integer(rhs->type)) {
    return new_binary_node(
        ND_SUB,
        lhs,
        new_binary_node(
          ND_MUL,
          rhs,
          new_num_node(lhs->type->base->size)
        )
    );
  }

  // ptr - ptr
  if (lhs->type->kind == TY_PTR && rhs->type->kind == TY_PTR) {
    return new_binary_node(
        ND_DIV,
        new_binary_node(ND_SUB, lhs, rhs),
        new_num_node(8)
    );
  }

  error_at(current_token->str, "invalid operand of \"-\"");
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
    return new_num_node(node->type->size);
  }

  return postfix();
}

// postfix = primary ("[" expr "]")*
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
  Type *ty = array_of(type_char, tok->cont_len);
  Var *var = new_gvar(new_gvar_name(), ty);
  var->init_data = tok->contents;
  return var;
}

//   primary = "(" "{" compound_stmt "}" ")"
//           | "(" expr ")"
//           | ident args?
//           | str
//           | num
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
      funcall_node->funcname = strndup(current_token->str, current_token->len);
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
      error_at(current_token->str, "undefined variable");
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

  error_at(current_token->str, "unexpected token");
}

Program *parse(Token *token) {
  current_token = token;
  Program *prog = program();

  if (current_token->kind != TK_EOF)
    error_at(current_token->str, "extra token");

  return prog;
}
