#include "9cc.h"

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
void debug_token(Token *token) {
  int i = 0;
  for (Token *tk = token; tk; tk = tk->next) {
    printf("=== TOKEN %d ===\n", i++);
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
    printf("\n");
  }
  exit(1);
}

void debug_node(Node *node) {
  printf("== NODE == \n");

  printf("kind: ");
  switch (node->kind) {
    case ND_ADD:
      printf("ND_ADD\n");
      break;
    case ND_SUB:
      printf("ND_SUB\n");
      break;
    case ND_MUL:
      printf("ND_MUL\n");
      break;
    case ND_DIV:
      printf("ND_DIV\n");
      break;
    case ND_EQ:
      printf("ND_EQ\n");
      break;
    case ND_NE:
      printf("ND_NE\n");
      break;
    case ND_LAT:
      printf("ND_LAT\n");
      break;
    case ND_LET:
      printf("ND_LET\n");
      break;
    case ND_LAE:
      printf("ND_LAE\n");
      break;
    case ND_LEE:
      printf("ND_LEE\n");
      break;
    case ND_ASSIGN:
      printf("ND_ASSIGN\n");
      break;
    case ND_RETURN:
      printf("ND_RETURN\n");
      break;
    case ND_IF:
      printf("ND_IF\n");
      break;
    case ND_FOR:
      printf("ND_FOR\n");
      break;
    case ND_WHILE:
      printf("ND_WHILE\n");
      break;
    case ND_BLOCK:
      printf("ND_BLOCK\n");
      break;
    case ND_FUNCALL:
      printf("ND_FUNCALL\n");
      break;
    case ND_EXPR_STMT:
      printf("ND_EXPR_STMT\n");
      break;
    case ND_VAR:
      printf("ND_VAR\n");
      break;
    case ND_NUM:
      printf("ND_NUM\n");
      printf("    val: %ld\n", node->val);
      break;
    case ND_DEREF:
      printf("ND_DEREF\n");
      break;
    case ND_ADDR:
      printf("ND_ADDR\n");
      break;
    default:
      printf("\nDebugError: Unknown node kind\n");
      return;
  }

  printf("type: ");
  switch (node->type->kind) {
    case TY_INT:
      printf("TY_INT\n");
      return;
    case TY_PTR:
      printf("TY_PTR\n");
      return;
    case TY_ARRAY:
      printf("TY_ARRAY\n");
      return;
    default:
      printf("\nDebugError: Unknown node type\n");
      return;
  }
}

void debug_nodes(Node *node) {
  int i = 0;
  for (Node *nd = node; nd; nd = nd->lhs) {
    debug_node(nd);
  }
}

void debug_func(Function *func) {
  int i = 0;
  for (Function *fn = func; fn; fn = fn->next) {
    printf("===== FUNCTION %d =====\n", i++);
    printf("name: \"%s\"\n", fn->name);
    printf("stack_size: %d\n", fn->stack_size);
    printf("nodes:\n");
    debug_nodes(fn->node);
    printf("\n");
  }
  exit(1);
}
