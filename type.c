#include "9cc.h"

Type *type_int = &(Type){TY_INT, 8};

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->size = 8;
  ty->base = base;
  return ty;
}

Type *array_of(Type *base, int len) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->size = base->size * len;
  ty->base = base;
  ty->array_len = len;
  return ty;
}

void add_type(Node *node) {
  if (!node || node->type)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);

  for (Node *n = node->body; n; n = n->next)
    add_type(n);
  for (Node *n = node->args; n; n = n->next)
    add_type(n);

  switch (node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_ASSIGN:
      node->type = node->lhs->type;
      return;
    case ND_EQ:
    case ND_NE:
    case ND_LAT:
    case ND_LET:
    case ND_LAE:
    case ND_LEE:
    case ND_NUM:
    case ND_FUNCALL:
      node->type = type_int;
      return;
    case ND_VAR:
      node->type = node->var->type;
      return;
    case ND_ADDR:
      if (node->lhs->type->kind == TY_ARRAY)
        node->type = pointer_to(node->lhs->type->base);
      else
        node->type = pointer_to(node->lhs->type);
      return;
    case ND_DEREF:
      node->type = node->lhs->type->base;
      return;
  }
}
