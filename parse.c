#include "9cc.h"

static Token *currentToken;
static Var *locals;

static Node *program();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

// Find local variable by name.
static Var *find_var(Token *token) {
  for (Var *var = locals; var; var = var->next)
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

static Node *new_num_node(long val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Node *new_ident_node(Var *var) {
  Node *node = new_node(ND_LVAR);
  node->var = var;
  return node;
}

static Var *new_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = locals;
  // FIXME
  if (locals)
    var->offset = locals->offset + 8;
  else
    var->offset = 0;
  locals = var;
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
  if (!equal(currentToken, op))
    return false;
  currentToken = currentToken->next;
  return true;
}

static void skip(char *op) {
  if (consume(op))
    return;
  else
    error_at(currentToken->str, "Not '%s'", op);
}

// program = stmt*
static Node *program() {
  Node head = {};
  Node *cur = &head;

  while (currentToken->kind != TK_EOF)
    cur = cur->next = stmt();

  return head.next;
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "while" "(" expr ")" stmt
//      | "{" stmt* "}"
//      | expr ";"
static Node *stmt() {
  if (consume("return")) {
    Node *node = new_node(ND_RETURN);
    node->lhs = expr();
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

    if (!equal(currentToken, ";"))
      node->init = expr();
    skip(";");

    if (!equal(currentToken, ";"))
      node->cond = expr();
    skip(";");

    if (!equal(currentToken, ")"))
      node->inc = expr();
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
    Node head = {};
    Node *cur = &head;

    while (!consume("}"))
      cur = cur->next = stmt();

    Node *node = new_node(ND_BLOCK);
    node->body = head.next;
    return node;
  }

  Node *node = expr();
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

// add = mul ("+" mul | "-" mul)*
static Node *add(){
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary_node(ND_SUB, node, mul());
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

// unary = ("+" | "-")? primary
static Node *unary() {
  if (consume("+"))
    return primary();

  if (consume("-"))
    return new_binary_node(ND_SUB, new_num_node(0), primary());

  return primary();
}

// primary = num | ident | "(" expr ")"
static Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    skip(")");
    return node;
  }

  if (currentToken->kind == TK_NUM) {
    Node *node = new_num_node(get_number(currentToken));
    currentToken = currentToken->next;
    return node;
  }

  if (currentToken->kind == TK_IDENT) {
    Var *var = find_var(currentToken);
    if (!var)
      var = new_var(strndup(currentToken->str, currentToken->len));
    Node *node = new_ident_node(var);
    currentToken = currentToken->next;
    return node;
  }
}

Function *parse(Token *token) {
  currentToken = token;
  Node *node = program();

  if (currentToken->kind != TK_EOF)
    error_at(currentToken->str, "extra token");

  Function *prog = calloc(1, sizeof(Function));
  prog->node = node;
  prog->locals = locals;
  return prog;
}
