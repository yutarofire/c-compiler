#include "9cc.h"

static char *user_input;
static char *filename;

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
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  int pos = loc - line + indent;
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
static Token *new_token(TokenKind kind, Token *cur, char *loc, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = loc;
  tok->len = len;
  cur->next = tok;
  return tok;
}

static bool starts_with(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || c == '_';
}

static char *keywords[] = {"return", "if", "else", "for", "while", "sizeof", "int", "char"};

static char read_escaped_char(char *p) {
  switch (*p) {
    case 't': return '\t';
    case 'n': return '\n';
    case 'r': return '\r';
    default: return *p;
  }
}

// tokenのlinked listを構築する。
Token *tokenize(char *p) {
  user_input = p;

  // Dummy token
  Token head = {};
  Token *cur = &head;

  while (*p) {
    // Skip line comment
    if (starts_with(p, "//")) {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    // Skip block comment
    if (starts_with(p, "/*")) {
      p += 2;
      while (!starts_with(p, "*/")) {
        p++;
        if (!*p)
          error("unclosed block comment");
      }
      p += 2;
      continue;
    }

    // Skip whitespace characters.
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

    // String literal
    if (*p == '"') {
      char *start = p;

      p++; // beginning '"'

      // Find the closing double-quote.
      char *end = p;
      for (; *end != '"'; end++) {
        if (*end == '\0')
          error_at(end, "unclosed string literal");
        if (*end == '\\')
          end++;
      }

      int buf_size = end - p + 1; // Including terminating '\0'
      char *buf = malloc(buf_size);

      int len = 0;
      while (*p != '"') {
        if (*p == '\\') {
          buf[len++] = read_escaped_char(p + 1);
          p += 2;
        } else {
          buf[len++] = *p++;
        }
      }
      buf[len++] = '\0';

      p++; // terminating '"'

      cur = new_token(TK_STR, cur, start, p - start);
      cur->contents = buf;
      cur->cont_len = len;
      continue;
    }

    // Keywords
    bool is_kw_tokenized = false;
    for (int i=0; i < (sizeof(keywords)/sizeof(keywords[0])); i++) {
      char *kw = keywords[i];
      int len = strlen(kw);
      if (!starts_with(p, kw) || is_alnum(p[len]))
        continue;

      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      is_kw_tokenized = true;
      break;
    }
    if (is_kw_tokenized)
      continue;

    // Identifier
    if (is_alpha(*p)) {
      char *q = p++;
      while (is_alnum(*p))
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
        starts_with(p, "=") || starts_with(p, ";") ||
        starts_with(p, "{") || starts_with(p, "}") ||
        starts_with(p, "*") || starts_with(p, "&") ||
        starts_with(p, "[") || starts_with(p, "]") ||
        starts_with(p, ",")) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    error_at(p, "Invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

static char *read_file(char *path) {
  filename = path;

  FILE *fp;

  if (strcmp(path, "-") == 0) {
    fp = stdin;
  } else {
    fp = fopen(path, "r");
    if (!fp)
      error("cannot open %s: %s", path, strerror(errno));
  }

  int buflen = 4096;
  int nread = 0;
  char *buf = calloc(1, buflen);

  for (;;) {
    int end = buflen - 2; // extra 2 bytes for the trailing "\n\0"
    int n = fread(buf + nread, 1, end - nread, fp);
    if (n == 0)
      break;
    nread += n;
    if (nread == end) {
      buflen *= 2;
      buf = realloc(buf, buflen);
    }
  }

  if (fp != stdin)
    fclose(fp);

  if (nread == 0 || buf[nread - 1] != '\n')
    buf[nread++] = '\n';
  buf[nread++] = '\0';

  return buf;
}

Token *tokenize_file(char *path) {
  return tokenize(read_file(path));
}
