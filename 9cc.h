#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * tokenize.c
 */
typedef enum {
  TK_RESERVED, // Keywords or orerators
  TK_IDENT,    // Identifiers
  TK_NUM,      // Numeric literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token
typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  long val;  // If TK_NUM, number value
  char *str; // Rested string
  int len;   // Token length
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
  int offset; // Offset from RBForP
};

typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LAT,       // >
  ND_LET,       // <
  ND_LAE,       // >=
  ND_LEE,       // <=
  ND_ASSIGN,    // =
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for"
  ND_WHILE,     // "while"
  ND_BLOCK,     // Block "{...}"
  ND_FUNCALL,   // Function call
  ND_EXPR_STMT, // Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // Integer
} NodeKind;

// AST Node
typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;

  Node *lhs;  // Left-hand side
  Node *rhs;  // Right-hand side

  Var *var;       // For ND_VAR
  long val;       // For ND_NUM
  Node *body;     // For ND_BLOCK
  char *funcname; // For ND_FUNCALL

  // For control flows
  Node *init;
  Node *cond;
  Node *inc;
  Node *then;
  Node *els;
};

typedef struct Function Function;
struct Function {
  Function *next;
  char *name;
  Node *node;
  Var *locals;
  int stack_size;
};

Function *parse(Token *token);

/*
 * codegen.c
 */
void codegen(Function *prog);

// Debugger
void debug_token(Token *token);
void debug_func(Function *func);
void debug_node(Node *node);
