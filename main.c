#include "9cc.h"

static void debug_token(Token *token);
static void debug_func(Function *func);
// TODO: debug_node

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: wrong number of arguments", argv[0]);

  Token *token = tokenize(argv[1]);
  Function *prog = parse(token);
  codegen(prog);

  return 0; 
}

/*
 * for debugging
 */
static void debug_token(Token *token) {
  printf("=== TOKEN DEBUG ===\n");
  for (Token *tk = token; tk; tk = tk->next) {
    printf("kind: ");
    switch (tk->kind) {
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
    printf("str: \"%s\"\n", tk->str);
    printf("val: %ld\n", tk->val);
    printf("len: %d\n", tk->len);
    printf("----------\n");
  }
  exit(1);
}

static void debug_func(Function *func) {
  printf("=== FUNCTION DEBUG ===\n");
  for (Function *fn = func; fn; fn = fn->next) {
    printf("name: \"%s\"\n", fn->name);
    printf("stack_size: %d\n", fn->stack_size);
    printf("----------\n");
  }
  exit(1);
}
