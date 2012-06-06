/* bdd.h
 * Written by Daniel Kudrow, 05/29/12
 * Last updated 05/29/12
 *
 * binary decision diagram
 */

#ifndef BDD_H
#define BDD_H

#include "ast.h"

#define TAB_SIZE 101

/* link for chained hash table */
typedef struct _symbol {
  char* name;
  int var;
  struct _symbol* next;
} symbol;

/* record to keep track of variable positions in the BDD */
typedef struct {
  int num_vars;
  int num_procs;
  int pc_size;
  symbol** vars;                       /* global variables */
  symbol** vars_;                      /* next state global variables */
  int** pc;                            /* program counters */
  int** pc_;                           /* next state program counters */
} pos;

pos* pos_init(ast_node* program);

#endif /* BDD_H */ 