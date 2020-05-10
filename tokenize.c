#include "9cc.h"

static char *user_input;

// Reports an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
static void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

// 新しいtokenを生成して、cur(rent) tokenのnextに繋げる。
// 新しいtokenを返す。
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

static bool starts_with(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

static bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || c == '_';
}

// tokenのlinked listを構築する。
Token *tokenize(char *p) {
  user_input = p;

  // Dummy head token
  Token head = {};
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // Keywords
    if (starts_with(p, "return") && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    if (starts_with(p, "if") && !is_alnum(p[2])) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (starts_with(p, "else") && !is_alnum(p[4])) {
      cur = new_token(TK_RESERVED, cur, p, 4);
      p += 4;
      continue;
    }

    if (starts_with(p, "for") && !is_alnum(p[3])) {
      cur = new_token(TK_RESERVED, cur, p, 3);
      p += 3;
      continue;
    }

    if (starts_with(p, "while") && !is_alnum(p[5])) {
      cur = new_token(TK_RESERVED, cur, p, 5);
      p += 5;
      continue;
    }

    // Identifier
    if (isalpha(*p)) {
      char *q = p++;
      while (isalnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }

    // Multi-letter operators
    if (starts_with(p, "==") || starts_with(p, "!=") ||
        starts_with(p, ">=") || starts_with(p, "<=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // Single-letter operators
    if (starts_with(p, "+") || starts_with(p, "-") ||
        starts_with(p, "*") || starts_with(p, "/") ||
        starts_with(p, "<") || starts_with(p, ">") ||
        starts_with(p, "(") || starts_with(p, ")") ||
        starts_with(p, "=") || starts_with(p, ";")) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    error_at(p, "Invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
