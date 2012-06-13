/* main.c
 * Written by Daniel Kudrow, 05/17/12
 * Last updated 05/17/12
 *
 * main
 */

#include <stdio.h>

#include "cfg.h"
#include "bdd.h"

int main() {
  ast_node* tree = parse();
  pos* pt = pos_init(tree);
//   ast_draw_tree(tree, stdout); 
  cfg_node* cfg = cfg_init(tree);
//   cfg_draw_graph(cfg, stdout);
  
  DdManager* m;
  m = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  
//   DdNode* var1 = Cudd_ReadOne(m);
//   DdNode* var2 = Cudd_ReadOne(m);
//   DdNode* var3 = Cudd_ReadOne(m);
//   DdNode* var4 = Cudd_ReadOne(m);
//   var1 = bdd_encode_pc(m, var1, pt->pc, pt->pc_size, 0, 0);
//   var2 = bdd_encode_pc(m, var2, pt->pc_, pt->pc_size, 0, 1);
//   var3 = bdd_encode_pc(m, var3, pt->pc, pt->pc_size, 0, 1);
//   var4 = bdd_encode_pc(m, var4, pt->pc_, pt->pc_size, 0, 1);
//   DdNode* both1 = Cudd_bddAnd(m, var1, var2);
//   DdNode* both2 = Cudd_bddAnd(m, var3, var4);
//   DdNode* both = Cudd_bddOr(m, both1, both2);
  

  DdNode* R = encode_prog(m, pt, cfg);
  DdNode* p = bdd_expr(m, pt->vars, tree->children[SPEC_LIST_HEAD]->children[EXPR]);
  
  DdNode* ex_p = compute_EX(m, pt, R, p);
  
//   /* TEST */
//   DdNode* testprog = encode_prog(m, p, cfg);
  char* O_names[] = {"EX(p)"};
  
  /* test.verf */
  char* R_names[] = {"a",/* "b",*/ "a'",/* "b'",*/ 
                     "pc1_0", "pc1_1", "pc2_0", "pc2_1",
                     "pc1'_0", "pc1'_1", "pc2'_0", "pc2'_1"};
  
  /* test.mutex.verf */
//   char* R_names[] = {"a", "b", "turn", "a'", "b'", "turn'", "pc1_0", "pc1_1",
//                      "pc1_2", "pc2_0", "pc2_1", "pc2_2", "pc1_0'", "pc1_1'",
//                      "pc1_2'", "pc2_0'", "pc2_1'", "pc2_2'"};

  DdNode* dump_array[] = {ex_p};
  Cudd_DumpDot(m, 1, dump_array, R_names, O_names, stdout);
  return 0;
}