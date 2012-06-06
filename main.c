/* main.c
 * Written by Daniel Kudrow, 05/17/12
 * Last updated 05/17/12
 *
 * main
 */

#include <stdio.h>

#include "cfg.h"
#include "bdd.h"
#include "cudd.h"
#include "util.h"

DdNode* encode_prog(DdManager* m, pos* postab, cfg_node* proc_list_head);

int main() {
  ast_node* tree = parse();
  pos* p = pos_init(tree);
//   ast_draw_tree(tree, stdout); 
  cfg_node* cfg = cfg_init(tree);
//   cfg_draw_graph(cfg, stdout);
  DdManager* m;
  m = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
//   TEST expressions
//   DdNode* a = bdd_expr(m, p->symtab, tree->children[PROC_LIST_HEAD]->children[BLOCK]->children[STAT_LIST_HEAD]->children[EXPR]);

//   TEST pc encoding  
//   DdNode* a = bdd_encode_pc(m, Cudd_ReadOne(m), p->pc_, p->pc_size, 0, 0);
//   DdNode* b = bdd_encode_pc(m, Cudd_ReadOne(m), p->pc_, p->pc_size, 0, 1);
//   DdNode* c = bdd_encode_pc(m, Cudd_ReadOne(m), p->pc_, p->pc_size, 0, 2);
//   DdNode* d = bdd_encode_pc(m, Cudd_ReadOne(m), p->pc_, p->pc_size, 0, 3);

//   DdNode* pc1 = Cudd_bddIthVar(m, 0);
//   DdNode* pc2 = Cudd_bddIthVar(m, 1);
//   DdNode* pc3 = Cudd_bddIthVar(m, 2);
//   DdNode* a = Cudd_bddAnd(m, Cudd_Not(pc1), Cudd_Not(pc2));
//   a = Cudd_bddAnd(m, a, Cudd_Not(pc3));
//   DdNode* b = Cudd_bddAnd(m, pc1, Cudd_Not(pc2));
//   b = Cudd_bddAnd(m, b, Cudd_Not(pc3));
//   DdNode* c = Cudd_bddAnd(m, Cudd_Not(pc1), pc2);
//   c = Cudd_bddAnd(m, c, Cudd_Not(pc3));
//   DdNode* d = Cudd_bddAnd(m, pc1, pc2);
//   d = Cudd_bddAnd(m, d, Cudd_Not(pc3));
//   char* R_names[] = {"1", "2", "3"};
//   char* O_names[] = {"a", "b", "c", "d"};  
  
//   DdNode* dump_array[] = {a, b, c, d};
//   char* R_names[] = {"a", "b", "c", "a'", "b'", "c'", "0", "1", "2", "0'", "1'", "2'"};
//   char* O_names[] = {"a", "b", "c", "d"};

//   TEST same
//   DdNode* a = bdd_same(m, p, 0, -1);

//   DdNode* var1 = Cudd_bddIthVar(m, 0);
//   DdNode* var2 = Cudd_bddIthVar(m, 1);
//   DdNode* var3 = Cudd_bddIthVar(m, 2);
//   DdNode* var4 = Cudd_bddIthVar(m, 3);
//   
//   DdNode* a = Cudd_bddXnor(m, var1, var2);
//   DdNode* b = Cudd_bddXnor(m, var3, var4);
//   DdNode* c = Cudd_bddAnd(m, a, b);
  
//   char* R_names[] = {"var1", "var2", "var3", "var4"};
//   char* O_names[] = {"a", "b", "c"}; 
  
//   TEST all
  DdNode* a = encode_prog(m, p, cfg);
  char* O_names[] = {"a"};
  char* R_names[] = {"a", "a'", "0", "1", "0'", "1'"};
//   char* R_names[] = {"a", "b", "turn", "a'", "b'", "turn'", "pc1_0", "pc1_1",
//                      "pc1_2", "pc2_0", "pc2_1", "pc2_2", "pc1_0'", "pc1_1'",
//                      "pc1_2'", "pc2_0'", "pc2_1'", "pc2_2'"};
  DdNode* dump_array[] = {a};
  Cudd_DumpDot(m, 1, dump_array, R_names, O_names, stdout);
  return 0;
}