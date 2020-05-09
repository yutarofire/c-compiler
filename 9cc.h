#define _POSIX_C_SOURCE 200809L
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
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数
  TK_EOF,      // EOF
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  long val;
  char *str;
  int len;
};

Token *tokenize(char *input);

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

/*
 * parse.c
 */
typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LAT,    // >
  ND_LET,    // <
  ND_LAE,    // >=
  ND_LEE,    // <=
  ND_ASSIGN, // =
  ND_LVAR,   // local variable
  ND_NUM,    // integer
} NodeKind;

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int offset;
};

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *lhs;  // left-hand side
  Node *rhs;  // right-hand side
  LVar *lvar; // used for ND_LVAR
  long val;   // used for ND_NUM
};

Node **parse(Token *token);

/*
 * codegen.c
 */
void codegen(Node *node);
