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

// Token
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
// Local variable
typedef struct Var Var;
struct Var {
  Var *next;
  char *name;
  int offset;
};

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
  ND_RETURN, // "return"
  ND_IF,     // "if"
  ND_FOR,    // "for"
  ND_WHILE,  // "while"
  ND_LVAR,   // local variable
  ND_NUM,    // integer
  ND_BLOCK,  // block
} NodeKind;

// AST Node
typedef struct Node Node;
struct Node {
  NodeKind kind;

  Node *lhs; // left-hand side
  Node *rhs; // right-hand side

  // used for control flows
  Node *init;
  Node *cond;
  Node *inc;
  Node *then;
  Node *els;

  Var *var;  // used for ND_LVAR
  long val;  // used for ND_NUM

  // used for ND_BLOCK
  Node *body;
  Node *next;
};

Node **parse(Token *token);

/*
 * codegen.c
 */
void codegen(Node **node);
