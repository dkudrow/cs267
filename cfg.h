/* cfg.h
 * Written by Daniel Kudrow, 05/09/12
 * Last updated 05/17/12
 *
 * conrtol flow graph
 * this files contains a structures and function prototypes to aid in
 * generating CFG's
 */

#ifndef CFG_H
#define CFG_H

#include "ast.h"
/* stdio.h is included in ast.h so it is not included explicitly here */

/* macros for identifying successors in different CFG node types */
#define CONTINUE  0
#define CFG_IF    1
#define CFG_ELSE  0
#define CFG_WHILE 1
#define CFG_AWAIT 1

/* CFG nodes wrap AST nodes */
typedef struct _cfg_node {
  ast_node* node;
  struct _cfg_node* succ[2];
  struct _cfg_node* next_proc;
} cfg_node;  

/* CFG methods */
void cfg_draw_graph(cfg_node* host, FILE* out);
cfg_node* cfg_init(ast_node* program);

#endif /* CFG_H */