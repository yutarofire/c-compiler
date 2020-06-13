#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Type Type;

/*
 * tokenize.c
 */
typedef enum {
  TK_RESERVED, // Keywords or orerators
  TK_IDENT,    // Identifiers
  TK_NUM,      // Numeric literals
  TK_STR,      // String literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token
typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;

  long val;  // For TK_NUM, number value
  char *loc; // Token location
  int len;   // Token length

  // String literal
  char *contents; // including terminating '\0'
  int cont_len;   // length
};

Token *tokenize_file(char *filename);

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
  Type *type;
  bool is_local;

  // Local variable
  int offset;

  // Global variable
  char *init_data;
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
  ND_DEREF,     // unary *
  ND_ADDR,      // unary &
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for"
  ND_WHILE,     // "while"
  ND_BLOCK,     // Block "{...}"
  ND_FUNCALL,   // Function call
  ND_EXPR_STMT, // Expression statement
  ND_STMT_EXPR, // Statement expression
  ND_VAR,       // Variable
  ND_NUM,       // Integer
} NodeKind;

// AST Node
typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Type *type;

  Node *lhs; // Left-hand side
  Node *rhs; // Right-hand side

  Var *var;  // For ND_VAR
  long val;  // For ND_NUM

  // Block or statement-expression
  Node *body;

  // Function call
  char *funcname;
  Node *args;

  // Control flows
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
  Var *params;

  Node *node;
  Var *locals;
  int stack_size;
};

typedef struct {
  Var *globals;
  Function *funcs;
} Program;

Program *parse(Token *tok);

/*
 * type.c
 */
typedef enum {
  TY_INT,
  TY_CHAR,
  TY_PTR,
  TY_ARRAY,
} TypeKind;

struct Type {
  TypeKind kind;
  int size; // sizeof() value
  Token *name;

  // Pointer or Array
  Type *base;

  // TY_ARRAY
  int array_len;
};

extern Type *type_int;
extern Type *type_char;
Type *pointer_to(Type *base);
Type *array_of(Type *base, int len);
bool is_integer(Type *type);
void add_type(Node *node);

/*
 * codegen.c
 */
void codegen(Program *prog);

// Debugger
void debug_token(Token *token);
void debug_func(Function *func);
void debug_node(Node *node);
