#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "lang.h"
#include "L1.h"

unsigned int get_next_block_id(unsigned int * p_bbcnt) {  
  unsigned int bbcnt = * p_bbcnt;
  (* p_bbcnt) ++;
  return bbcnt;
}

unsigned int get_next_order_id(unsigned int * p_ocnt) {
  unsigned int ocnt = * p_ocnt;
  (* p_ocnt) ++;
  return ocnt;
}

void reverse_order_id(unsigned int * p_ocnt) {
  (* p_ocnt) --;
}

unsigned int block_number_calculate(struct L1_cmd_listbox * cs, enum L1_BasicBlockType t) {
  int flag = 1;
  unsigned int res = 1;
  struct L1_cmd_list * p = cs -> head;
  while (p) {
    switch (p -> head -> t) {
    case L1_T_IF:
      res += block_number_calculate(p -> head -> d.IF.left, L1_T_UJMP) +
	     block_number_calculate(p -> head -> d.IF.right, L1_T_UJMP);
      if (p -> tail || t == L1_T_CJMP) {
	res ++;
      }
      break;
    case L1_T_WHILE:
      res += block_number_calculate(p -> head -> d.WHILE.pre, L1_T_CJMP) +
	     block_number_calculate(p -> head -> d.WHILE.body, L1_T_CJMP) - flag;
      if (p -> tail || t == L1_T_CJMP) {
	res ++;
      }
      break;
    default:
      ;
    }
    p = p -> tail;
    flag = 0;
  }
  return res;
}

void basic_block_gen(struct L1_cmd_list * p,
		     unsigned int id,
		     struct L1_basic_block_end * end_info,
		     struct L1_basic_blocks * bbs,
		     unsigned int * p_bbcnt,
		     unsigned int * p_ocnt) {
  int flag = 1;
  struct L1_basic_block * bb = (bbs -> l) + id;
  bbs -> order[get_next_order_id(p_ocnt)] = id;
  struct L1_cmd_list * * ptr_p = & (bb -> head);
  * ptr_p = p;
  while (p) {
    switch (p -> head -> t) {
    case L1_T_IF:
      if (p -> tail || end_info -> t == L1_T_CJMP) {
	unsigned int id_then = get_next_block_id(p_bbcnt);
        unsigned int id_else = get_next_block_id(p_bbcnt);
        unsigned int id_next = get_next_block_id(p_bbcnt);
        basic_block_gen(p -> head -> d.IF.left -> head, id_then, L1_TUJmp(id_next), bbs, p_bbcnt, p_ocnt);
        basic_block_gen(p -> head -> d.IF.right -> head, id_else, L1_TUJmp(id_next), bbs, p_bbcnt, p_ocnt);
	basic_block_gen(p -> tail, id_next, end_info, bbs, p_bbcnt, p_ocnt);
	* ptr_p = NULL;
	L1_set_BBEnd(bb, L1_TCJmp(& (p -> head -> d.IF.cond), id_then, id_else));
	return;
      }
      else {
	unsigned int id_then = get_next_block_id(p_bbcnt);
        unsigned int id_else = get_next_block_id(p_bbcnt);
        basic_block_gen(p -> head -> d.IF.left -> head, id_then, end_info, bbs, p_bbcnt, p_ocnt);
        basic_block_gen(p -> head -> d.IF.right -> head, id_else, L1_BBEndDup(end_info), bbs, p_bbcnt, p_ocnt);
	* ptr_p = NULL;
	L1_set_BBEnd(bb, L1_TCJmp(& (p -> head -> d.IF.cond), id_then, id_else));
	return;
      }
    case L1_T_WHILE:
      if (p -> tail || end_info -> t == L1_T_CJMP) {
	unsigned int id_pre;
	if (flag) {
	  id_pre = id;
	  reverse_order_id(p_ocnt);
	}
	else {
	  id_pre = get_next_block_id(p_bbcnt);
	}
        unsigned int id_body = get_next_block_id(p_bbcnt);
        unsigned int id_next = get_next_block_id(p_bbcnt);
        basic_block_gen(p -> head -> d.WHILE.pre -> head, id_pre, L1_TCJmp(& (p -> head -> d.WHILE.cond), id_body, id_next), bbs, p_bbcnt, p_ocnt);
	basic_block_gen(p -> head -> d.WHILE.body -> head, id_body, L1_TUJmp(id_pre), bbs, p_bbcnt, p_ocnt);
	basic_block_gen(p -> tail, id_next, end_info, bbs, p_bbcnt, p_ocnt);
	if (! flag) {
	  * ptr_p = NULL;
	  L1_set_BBEnd(bb, L1_TUJmp(id_pre));
	}
	return;
      }
      else {
	unsigned int id_pre;
	if (flag) {
	  id_pre = id;
	  reverse_order_id(p_ocnt);
	}
	else {
	  id_pre = get_next_block_id(p_bbcnt);
	}
        unsigned int id_body = get_next_block_id(p_bbcnt);
	unsigned int id_next = end_info -> dst1;
        basic_block_gen(p -> head -> d.WHILE.pre -> head, id_pre, L1_TCJmp(& (p -> head -> d.WHILE.cond), id_body, id_next), bbs, p_bbcnt, p_ocnt);
	basic_block_gen(p -> head -> d.WHILE.body -> head, id_body, L1_TUJmp(id_pre), bbs, p_bbcnt, p_ocnt);
	if (! flag) {
	  * ptr_p = NULL;
	  L1_set_BBEnd(bb, L1_TUJmp(id_pre));
	}
	return;
      }
    default:
      ptr_p = & (p -> tail);
      p = * ptr_p;
      flag = 0;
    }
  }
  L1_set_BBEnd(bb, end_info);
}

struct L1_basic_blocks * BB_gen(struct L1_cmd_listbox * cs) {
  struct L1_basic_blocks * res =
    L1_BasicBlocks_Init(block_number_calculate(cs, L1_T_UJMP));
  unsigned int bbcnt = 0;
  unsigned int ocnt = 0;
  basic_block_gen(cs -> head, get_next_block_id(& bbcnt), L1_TUJmp(res -> n), res, & bbcnt, & ocnt);
  return res;
}
