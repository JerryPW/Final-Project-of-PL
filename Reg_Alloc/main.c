#include <stdio.h>
#include "lang.h"
#include "L1.h"
#include "lexer.h"
#include "parser.h"

extern struct cmd * root;
int yyparse();
struct L1_cmd_listbox * TAC_gen(struct cmd * c);  // L1.h
struct L1_basic_blocks * BB_gen(struct L1_cmd_listbox * l); // L1.h

int main(int argc, char **argv) {
    yyin = fopen(argv[1], "rb");
    yyparse();
    fclose(yyin);
    print_cmd(root); // lang.c
    printf("\n");
    struct L1_cmd_listbox * l = TAC_gen(root);
    print_L1_cmd_listbox(l); // L1.c
    struct L1_basic_blocks * bbs = BB_gen(l);
    freopen("BasicBlock.txt", "w", stdout);
    print_L1_basic_blocks(bbs); // L1.c   
}