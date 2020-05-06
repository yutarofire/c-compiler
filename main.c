#include "chibicc.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: wrong number of arguments", argv[0]);

  Token *token = tokenize(argv[1]);
  Node *node = parse(token);
  codegen(node);

  return 0; 
}

/*
 * for debug

  // token debug
  printf("==========\n");
  printf("token debug\n");
  printf("==========\n");
  for (int i = 0; i < 100; i++) {
    printf("str: \"%s\"\n", token->str);
    printf("val: %ld\n", token->val);
    printf("len: %d\n", token->len);
    printf("kind: ");
    switch (token->kind) {
      case TK_RESERVED:
        printf("TK_RESERVED\n");
        break;
      case TK_NUM:
        printf("TK_NUM\n");
        break;
      case TK_EOF:
        printf("TK_EOF\n");
        break;
    }
    token = token->next;
    printf("----------\n");
  }
  printf("==========\n");

 */
