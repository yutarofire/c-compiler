#include "9cc.h"

Type *type_int = &(Type){TY_INT};

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->base = base;
  return ty;
}

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
    case ND_NUM:
      node->type = type_int;
      return;
    case ND_ADDR: {
      node->type = pointer_to(node->lhs->type);
      return;
    }
  }

  error("invalid type");
}
