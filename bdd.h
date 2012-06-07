/* bdd.h
 * Written by Daniel Kudrow, 05/29/12
 * Last updated 05/29/12
 *
 * binary decision diagram
 * this file contains structures and function prototypes used in encoding a
 * CFG as a boolean formula in a CUDD BDD structure
 */

#ifndef BDD_H
#define BDD_H

#include "ast.h"
#include "cudd.h"
#include "util.h"

#define TAB_SIZE 101

/* link for chained hash table */
typedef struct _symbol {
  char* name;
  int var;
  struct _symbol* next;
} symbol;

/* record to keep track of variable positions in the BDD */
typedef struct {
  int num_vars;                        /* number of global variables */
  int num_procs;                       /* number of processes */
  int pc_size;                         /* bit length of program counter */
  symbol** vars;                       /* global variables */
  symbol** vars_;                      /* next state global variables */
  int** pc;                            /* program counters */
  int** pc_;                           /* next state program counters */
} pos;

/* BDD methods */
pos* pos_init(ast_node* program);
DdNode* encode_prog(DdManager* m, pos* postab, cfg_node* proc_list_head);

#endif /* BDD_H */ 