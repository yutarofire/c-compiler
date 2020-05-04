#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    // FIXME
    fprintf(stderr, "Wrong number of args\n");
    return 1;
  }

  Token *token = tokenize(argv[1]);
  Node *node = parse(token);
  codegen(node);

  return 0; 
}

/*
 * for debug

  // token debug
  printf("==========\n");
  for (int i = 0; i < 100; i++) {
    printf("str: \"%s\"\n", token->str);
    printf("val: %d\n", token->val);
    printf("len: %d\n", token->len);
    token = token->next;
    printf("----------\n");
  }
  printf("==========\n");

 */
