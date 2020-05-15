#include "9cc.h"

static void debug_token(Token *token);

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: wrong number of arguments", argv[0]);

  Token *token = tokenize(argv[1]);
  Node **code = parse(token);
  codegen(code);

  return 0; 
}

/*
 * for debugging
 */
static void debug_token(Token *token) {
  printf("=== TOKEN DEBUG ===\n");
  for (int i = 0; i < 100; i++) {
    printf("kind: ");
    switch (token->kind) {
      case TK_RESERVED:
        printf("TK_RESERVED");
        break;
      case TK_IDENT:
        printf("TK_IDENT");
        break;
      case TK_NUM:
        printf("TK_NUM");
        break;
      case TK_EOF:
        printf("TK_EOF");
        break;
      default:
        printf("ERROR");
        break;
    }
    printf("\n");
    printf("str: \"%s\"\n", token->str);
    printf("val: %ld\n", token->val);
    printf("len: %d\n", token->len);
    token = token->next;
    if (token->kind == TK_EOF)
      break;
    printf("----------\n");
  }
  printf("==========\n");
  exit(1);
}
