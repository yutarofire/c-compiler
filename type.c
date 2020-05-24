#include "9cc.h"

void add_type(Node *node) {
  if (!node || node->type)
    return;

  add_type(node->lhs);
  add_type(node->rhs);

  switch (node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV: {
      node->type = node->lhs->type;
      return;
    }
    case ND_FUNCALL:
    case ND_VAR:
    case ND_NUM: {
      Type *type = calloc(1, sizeof(Type));
      type->kind = TY_INT;
      node->type = type;
      return;
    }
    case ND_ADDR: {
      Type *type = calloc(1, sizeof(Type));
      type->kind = TY_PTR;
      type->base = node->lhs->type;
      node->type = type;
      return;
    }
  }

  error("invalid type");
}
