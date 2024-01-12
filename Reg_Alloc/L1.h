#ifndef L1_H_INCLUDED
#define L1_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "lang.h"

enum L1_ConstOrVar {
  L1_T_CONST = 0,
  L1_T_VAR
};

enum L1_ExprType {
  L1_T_CONST_OR_VAR = 0,
  L1_T_BINOP,
  L1_T_UNOP,
  L1_T_DEREF,
  L1_T_MALLOC,
  L1_T_RI,
  L1_T_RC
};

enum L1_CmdType {
  L1_T_ASGN_VAR = 0,
  L1_T_ASGN_MEM,
  L1_T_IF,
  L1_T_WHILE,
  L1_T_WI,
  L1_T_WC
};

enum L1_BasicBlockType {
  L1_T_UJMP = 0,
  L1_T_CJMP
};

struct L1_const_or_var {
  enum L1_ConstOrVar t;
  union {
    struct {unsigned int value; } CONST;
    struct {unsigned int name; } VAR;
  } d;
};

struct L1_expr {
  enum L1_ExprType t;
  union {
    struct {struct L1_const_or_var cv; } CONST_OR_VAR;
    struct {enum BinOpType op; struct L1_const_or_var cv1; struct L1_const_or_var cv2; } BINOP;
    struct {enum UnOpType op; struct L1_const_or_var cv; } UNOP;
    struct {struct L1_const_or_var cv; } DEREF;
    struct {struct L1_const_or_var cv; } MALLOC;
    struct {void * none; } RI;
    struct {void * none; } RC;
  } d;
};

struct L1_cmd_listbox;

struct L1_cmd {
  enum L1_CmdType t;
  union {
    struct {unsigned int name; struct L1_expr right; } ASGN_VAR;
    struct {unsigned int name; struct L1_const_or_var cv; } ASGN_MEM;
    struct {struct L1_expr cond; struct L1_cmd_listbox * left; struct L1_cmd_listbox * right; } IF;
    struct {struct L1_cmd_listbox * pre; struct L1_expr cond; struct L1_cmd_listbox * body; } WHILE;
    struct {struct L1_const_or_var cv; } WI;
    struct {struct L1_const_or_var cv; } WC;
  } d;
};

struct L1_cmd_list {
  struct L1_cmd * head;
  struct L1_cmd_list * tail;
};

struct L1_cmd_listbox {
  struct L1_cmd_list * head;
  struct L1_cmd_list * * tail;
};

struct L1_basic_block_end {
  enum L1_BasicBlockType t;
  struct L1_expr cond;
  unsigned int dst1;
  unsigned int dst2;
};

struct L1_basic_block {
  struct L1_cmd_list * head;
  struct L1_basic_block_end end_info;
};

struct L1_basic_blocks {
  struct L1_basic_block * l;
  unsigned int * order;
  unsigned int n;
};

struct L1_const_or_var * L1_ConstOrVar_from_Expr(struct L1_expr * arg);
struct L1_const_or_var * L1_TConst(unsigned int value);
struct L1_const_or_var * L1_TVar(unsigned int name);
struct L1_expr * L1_TConstOrVar(struct L1_const_or_var * cv);
struct L1_expr * L1_TBinop(enum BinOpType op, struct L1_const_or_var * cv1, struct L1_const_or_var * cv2);
struct L1_expr * L1_TUnop(enum UnOpType op, struct L1_const_or_var * cv);
struct L1_expr * L1_TDeref(struct L1_const_or_var * cv);
struct L1_expr * L1_TMalloc(struct L1_const_or_var * cv);
struct L1_expr * L1_TReadInt();
struct L1_expr * L1_TReadChar();
struct L1_cmd * L1_TAsgnVar(unsigned name, struct L1_expr * right);
struct L1_cmd * L1_TAsgnMem(unsigned name, struct L1_const_or_var * cv);
struct L1_cmd * L1_TIf(struct L1_expr * cond, struct L1_cmd_listbox * left, struct L1_cmd_listbox * right);
struct L1_cmd * L1_TWhile(struct L1_cmd_listbox * pre, struct L1_expr * cond, struct L1_cmd_listbox * body);
struct L1_cmd * L1_TWriteInt(struct L1_const_or_var * cv);
struct L1_cmd * L1_TWriteChar(struct L1_const_or_var * cv);
struct L1_cmd_listbox * L1_CmdListBox_Empty();
struct L1_cmd_listbox * L1_CmdListBox_Single(struct L1_cmd * c);
void L1_CmdListBox_Add1(struct L1_cmd_listbox * cs, struct L1_cmd * c);
void L1_CmdListBox_Append(struct L1_cmd_listbox * cs, struct L1_cmd_listbox * cs0);

struct L1_basic_block_end * L1_TUJmp(unsigned int dst);
struct L1_basic_block_end * L1_TCJmp(struct L1_expr * cond, unsigned int dst1, unsigned int dst2);
struct L1_basic_block_end * L1_BBEndDup(struct L1_basic_block_end * end_info);
void L1_set_BBEnd(struct L1_basic_block * bb, struct L1_basic_block_end * end_info);
struct L1_basic_blocks * L1_BasicBlocks_Init(unsigned int n);

void print_L1_const_or_var(struct L1_const_or_var * e);
void print_L1_expr(struct L1_expr * e);
void print_L1_cmd(struct L1_cmd * c);
void print_L1_cmd_list(struct L1_cmd_list * c);
void print_L1_cmd_listbox(struct L1_cmd_listbox * c);
void print_L1_basic_block_end(struct L1_basic_block_end * end_info);
void print_L1_basic_block(struct L1_basic_block * bb);
void print_L1_basic_blocks(struct L1_basic_blocks * bbs);


#endif // L1_H_INCLUDED
