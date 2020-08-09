#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2)
    error("%s: wrong number of arguments", argv[0]);

  Token *tok = tokenize_file(argv[1]);
  Program *prog = parse(tok);
  codegen(prog);

  return 0; 
}
