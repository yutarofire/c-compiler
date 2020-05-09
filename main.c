#include "9cc.h"

static void debug_token(Token *token);
static void debug_node(Node *node);

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: wrong number of arguments", argv[0]);

  Token *token = tokenize(argv[1]);
  Node **code = parse(token);

  /*
   * generate assembly code
   */
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; code[i]; i++) {
    codegen(code[i]);

    // 式の評価結果がスタックに残っている
    // はずなので、popしておく
    printf("  pop rax\n");
  }

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");

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

static void debug_node(Node *node) {
  printf("=== NODE DEBUG ===\n");
  for (int i = 0; i < 100; i++) {
    printf("kind: ");
    switch (node->kind) {
      case ND_ADD:
        printf("+");
        break;
      case ND_SUB:
        printf("-");
        break;
      case ND_MUL:
        printf("*");
        break;
      case ND_DIV:
        printf("/");
        break;
      case ND_EQ:
        printf("==");
        break;
      case ND_NE:
        printf("!=");
        break;
      case ND_LAT:
        printf(">");
        break;
      case ND_LET:
        printf("<");
        break;
      case ND_LAE:
        printf(">=");
        break;
      case ND_LEE:
        printf("<=");
        break;
      case ND_ASSIGN:
        printf("=");
        break;
      case ND_LVAR:
        printf("local var");
        break;
      case ND_NUM:
        printf("number");
        break;
      default:
        printf("ERROR");
        break;
    }
    printf("\n");
    printf("val: %ld\n", node->val);
    printf("----------\n");
    node = node->lhs;
  }
  printf("==========\n");
  exit(1);
}
