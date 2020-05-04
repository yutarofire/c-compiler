#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * tokenize.c
 */
typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

Token *tokenize(char *p);

/*
 * parse.c
 */
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LAT, // >
  ND_LET, // <
  ND_LEE, // <=
  ND_LAE, // >=
  ND_NUM, // number
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs; // left-hand side
  Node *rhs; // right-hand side
  int val;
};

Node *parse(Token *token);

/*
 * codegen.c
 */
void codegen(Node *node);
