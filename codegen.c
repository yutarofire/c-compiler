#include "9cc.h"

static int top;
static int labelseq = 1;
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static Function *current_func;

static char *reg(int idx) {
  char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

// Load the value from where the stack top is pointing to.
static void load(Type *ty) {
  if (ty->kind == TY_ARRAY)
    return;

  if (ty->size == 1)
    printf("  movsx %s, byte ptr [%s]\n", reg(top - 1), reg(top - 1));
  else
    printf("  mov %s, [%s]\n", reg(top - 1), reg(top - 1));
}

static void store(Type *ty) {
  if (ty->size == 1)
    printf("  mov [%s], %sb\n", reg(top - 1), reg(top - 2));
  else
    printf("  mov [%s], %s\n", reg(top - 1), reg(top - 2));

  top--;
}

static void gen_expr();

// Pushes the given node's address to the stack.
static void gen_var(Node *node) {
  switch (node->kind) {
    case ND_VAR:
      if (node->var->is_local)
        printf("  lea %s, [rbp-%d]\n", reg(top++), node->var->offset);
      else
        printf("  mov %s, offset %s\n", reg(top++), node->var->name);
      return;
    case ND_DEREF:
      gen_expr(node->lhs);
      return;
  }

  error("expected a variable or dereferencer");
}

static void gen_expr(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  mov %s, %ld\n", reg(top++), node->val);
      return;
    case ND_VAR:
      gen_var(node);
      load(node->type);
      return;
    case ND_DEREF:
      gen_expr(node->lhs);
      load(node->type);
      return;
    case ND_ADDR:
      gen_var(node->lhs);
      return;
    case ND_ASSIGN:
      gen_expr(node->rhs);
      gen_var(node->lhs);
      store(node->type);
      return;
    case ND_FUNCALL: {
      int i = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        gen_expr(arg);
        printf("  mov %s, %s\n", argreg64[i++], reg(--top));
      }

      printf("  push r10\n");
      printf("  push r11\n");
      printf("  mov rax, 0\n");
      printf("  call %s\n", node->funcname);
      printf("  pop r11\n");
      printf("  pop r10\n");
      printf("  mov %s, rax\n", reg(top++));
      return;
    }
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

  char *rd = reg(top - 2); // left-hand value
  char *rs = reg(top - 1); // right-hand value
  top--;

  switch (node->kind) {
    case ND_ADD:
      printf("  add %s, %s\n", rd, rs);
      return;
    case ND_SUB:
      printf("  sub %s, %s\n", rd, rs);
      return;
    case ND_MUL:
      printf("  imul %s, %s\n", rd, rs);
      return;
    case ND_DIV:
      printf("  mov rax, %s\n", rd);
      printf("  cqo\n");
      printf("  idiv %s\n", rs);
      printf("  mov %s, rax\n", rd);
      return;
    case ND_EQ:
      printf("  cmp %s, %s\n", rd, rs);
      printf("  sete al\n");
      printf("  movzx %s, al\n", rd);
      return;
    case ND_NE:
      printf("  cmp %s, %s\n", rd, rs);
      printf("  setne al\n");
      printf("  movzx %s, al\n", rd);
      return;
    case ND_LAT:
      printf("  cmp %s, %s\n", rs, rd);
      printf("  setl al\n");
      printf("  movzx %s, al\n", rd);
      return;
    case ND_LET:
      printf("  cmp %s, %s\n", rd, rs);
      printf("  setl al\n");
      printf("  movzx %s, al\n", rd);
      return;
    case ND_LAE:
      printf("  cmp %s, %s\n", rs, rd);
      printf("  setle al\n");
      printf("  movzx %s, al\n", rd);
      return;
    case ND_LEE:
      printf("  cmp %s, %s\n", rd, rs);
      printf("  setle al\n");
      printf("  movzx %s, al\n", rd);
      return;
    default:
      error("invalid expression");
  }
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
    case ND_IF: {
      int seq = labelseq++;
      if (node->els) {
        gen_expr(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.else.%d\n", seq);
        gen_stmt(node->then);
        printf("  je  .L.end.%d\n", seq);
        printf(".L.else.%d:\n", seq);
        gen_stmt(node->els);
        printf(".L.end.%d:\n", seq);
      } else {
        gen_expr(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
        gen_stmt(node->then);
        printf(".L.end.%d:\n", seq);
      }
      return;
    }
    case ND_FOR: {
      int seq = labelseq++;
      if (node->init)
        gen_stmt(node->init);
      printf(".L.begin.%d:\n", seq);
      if (node->cond) {
        gen_expr(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
      }
      gen_stmt(node->then);
      if (node->inc)
        gen_stmt(node->inc);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_WHILE: {
      int seq = labelseq++;
      printf(".L.begin.%d:\n", seq);
      if (node->cond) {
        gen_expr(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
      }
      gen_stmt(node->then);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next)
        gen_stmt(n);
      return;
    case ND_RETURN:
      gen_expr(node->lhs);
      printf("  mov rax, %s\n", reg(--top));
      printf("  jmp .L.return.%s\n", current_func->name);
      return;
    case ND_EXPR_STMT:
      gen_expr(node->lhs);
      top--;
      return;
    default:
      error("invalid statement");
  }
}

static void emit_text(Function *funcs) {
  printf(".text\n");

  for (Function *fn = funcs; fn; fn = fn->next) {
    printf(".globl %s\n", fn->name);
    printf("%s:\n", fn->name);

    current_func = fn;

    // Prologue
    // r12-15 are callee-saved registers.
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);
    printf("  mov [rbp-8], r12\n");
    printf("  mov [rbp-16], r13\n");
    printf("  mov [rbp-24], r14\n");
    printf("  mov [rbp-32], r15\n");

    // Save arguments to the stack
    int i = 0;
    for (Var *param = fn->params; param; param = param->next)
      i++;
    for (Var *param = fn->params; param; param = param->next) {
      if (param->type->size == 1)
        printf("  mov [rbp-%d], %s\n", param->offset, argreg8[--i]);
      else
        printf("  mov [rbp-%d], %s\n", param->offset, argreg64[--i]);
    }

    // Emit code
    for (Node *n = fn->node; n; n = n->next) {
      gen_stmt(n);
      assert(top == 0);
    }

    // Epilogue
    printf(".L.return.%s:\n", fn->name);
    printf("  mov r12, [rbp-8]\n");
    printf("  mov r13, [rbp-16]\n");
    printf("  mov r14, [rbp-24]\n");
    printf("  mov r15, [rbp-32]\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

static void emit_data(Var *globals) {
  printf(".data\n");

  for (Var *gvar = globals; gvar; gvar = gvar->next) {
    printf("%s:\n", gvar->name);

    if (!gvar->init_data) {
      printf("  .zero %d\n", gvar->type->size);
      continue;
    }

    for (int i = 0; i < gvar->type->size; i++)
      printf("  .byte %d\n", gvar->init_data[i]);
  }
}

void codegen(Program *prog) {
  printf(".intel_syntax noprefix\n");
  emit_data(prog->globals);
  emit_text(prog->funcs);
}
