#include "9cc.h"

char *user_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 新しいtokenを生成して、cur(rent) tokenのnextに繋げる。
// 新しいtokenを返す。
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// tokenのlinked listを構築する。
Token *tokenize(char *p) {
  user_input = p;

  // ダミーのtokenを先頭に持ってくる。
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "==", 2) == 0 ||
        strncmp(p, "!=", 2) == 0 ||
        strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' ||
        *p == '*' || *p == '/' ||
        *p == '<' || *p == '>' ||
        *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "Cannot tokenize");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
