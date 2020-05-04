#include "9cc.h"

// FIXME
Token *tok;

/*
 * tok processors
 */
// 次のtokが期待しているcharのとき、tokを1つ進めて
// trueを返す。それ以外はfalseを返す。
bool consume(char *op) {
  if (tok->kind != TK_RESERVED ||
      strlen(op) != tok->len ||
      memcmp(tok->str, op, tok->len))
    return false;
  tok = tok->next;
  return true;
}

// 次のtokが期待しているcharのとき、tokを1つ進める。
// それ以外はエラーを報告する。
void expect(char *op) {
  if (tok->kind != TK_RESERVED ||
      strlen(op) != tok->len ||
      memcmp(tok->str, op, tok->len))
    error_at(tok->str, "Not '%c'", op);
  tok = tok->next;
}

// 次のtokが数値のとき、tokを1つ進めてその数値を返す。
// それ以外はエラーを報告する。
int expect_number() {
  if (tok->kind != TK_NUM)
    error_at(tok->str, "Not number");
  int val = tok->val;
  tok = tok->next;
  return val;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// production rules
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr() {
  return equality();
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">=" add | ">" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LET, node, relational());
    else if (consume("<="))
      node = new_node(ND_LEE, node, relational());
    else if (consume(">"))
      node = new_node(ND_LAT, node, relational());
    else if (consume(">="))
      node = new_node(ND_LAE, node, relational());
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add(){
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume("+"))
    return primary();
  else if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}

// primary = num | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}

Node *parse(Token *token) {
  tok = token;
  Node *node = expr();

  if (tok->kind != TK_EOF)
    printf("error!!"); // FIXME

  return node;
}
