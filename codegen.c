#include "9cc.h"

static int top;
static int labelseq = 1;

static char *reg(int idx) {
  char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
  if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
    error("register out of range: %d", idx);
  return r[idx];
}

// Pushes the given node's address to the stack.
static void gen_var(Node *node) {
  if (node->kind != ND_VAR)
    error("expected a variable");

  printf("  lea %s, [rbp-%d]\n", reg(top++), node->var->offset);
}

static void gen(Node *node) {
  switch (node->kind) {
    case ND_IF: {
      int seq = labelseq++;
      if (node->els) {
        gen(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.else.%d\n", seq);
        gen(node->then);
        printf("  je  .L.end.%d\n", seq);
        printf(".L.else.%d:\n", seq);
        gen(node->els);
        printf(".L.end.%d:\n", seq);
      } else {
        gen(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
        gen(node->then);
        printf(".L.end.%d:\n", seq);
      }
      return;
    }
    case ND_FOR: {
      int seq = labelseq++;
      if (node->init)
        gen(node->init);
      printf(".L.begin.%d:\n", seq);
      if (node->cond) {
        gen(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
      }
      gen(node->then);
      if (node->inc)
        gen(node->inc);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_WHILE: {
      int seq = labelseq++;
      printf(".L.begin.%d:\n", seq);
      if (node->cond) {
        gen(node->cond);
        printf("  cmp %s, 0\n", reg(--top));
        printf("  je  .L.end.%d\n", seq);
      }
      gen(node->then);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next)
        gen(n);
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  mov rax, %s\n", reg(--top));
      printf("  jmp .L.return\n");
      return;
    case ND_NUM:
      printf("  mov %s, %ld\n", reg(top++), node->val);
      return;
    case ND_VAR:
      gen_var(node);
      // Fetch value of variable by its address,
      // and load it onto register.
      printf("  mov %s, [%s]\n", reg(top - 1), reg(top - 1));
      return;
    case ND_ASSIGN:
      gen(node->rhs);
      gen_var(node->lhs);
      printf("  mov [%s], %s\n", reg(top - 1), reg(top - 2));
      // Right-hand value remains on top of reg.
      top--;
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

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

void codegen(Function *prog) {
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // Prologue
  // r12-15 are callee-saved registers.
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);
  printf("  mov [rbp-8], r12\n");
  printf("  mov [rbp-16], r13\n");
  printf("  mov [rbp-24], r14\n");
  printf("  mov [rbp-32], r15\n");

  for (Node *n = prog->node; n; n = n->next) {
    gen(n);
    // FIXME
    // assert(top == 0);
  }

  // Epilogue
  printf(".L.return:\n");
  printf("  mov r12, [rbp-8]\n");
  printf("  mov r13, [rbp-16]\n");
  printf("  mov r14, [rbp-24]\n");
  printf("  mov r15, [rbp-32]\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
