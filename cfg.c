/* cfg.c
 * Written by Daniel Kudrow, 05/09/12
 * Last updated 05/29/12
 *
 * conrtol flow graph
 * this file contains functions that superimpose a set of edges onto an AST
 * to create a CFG
 */

#include <stdlib.h>

#include "cfg.h"

/* forward declaration */
cfg_node* ast_to_cfg(ast_node* host, cfg_node* succ);

/* traverse a function block */
cfg_node* cfg_visit_block(ast_node* host, cfg_node* succ) {
  if (!host->children[NEXT_STAT])
    return ast_to_cfg(host, succ);
  cfg_node* new_cfg_node = cfg_visit_block(host->children[NEXT_STAT], succ);
  return ast_to_cfg(host, new_cfg_node);
}

/* make an assignment statement node */
cfg_node* cfg_mk_assign_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* make a skip statement node */
cfg_node* cfg_mk_skip_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* make an if-then statement node */
cfg_node* cfg_mk_if_then_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CFG_IF] = ast_to_cfg(host->children[BLOCK1], succ);
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* make an if-then-else statement node */
cfg_node* cfg_mk_if_else_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CFG_IF] = ast_to_cfg(host->children[BLOCK1], succ);
  new_cfg_node->succ[CFG_ELSE] = ast_to_cfg(host->children[BLOCK2], succ);
  return new_cfg_node;
}

/* make a while statement node */
cfg_node* cfg_mk_while_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CFG_WHILE] = ast_to_cfg(host->children[BLOCK1], new_cfg_node);
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* make an await statement node */
cfg_node* cfg_mk_await_stat(ast_node* host, cfg_node* succ) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->succ[CFG_AWAIT] = new_cfg_node;
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* visit an AST sub-tree and return the converted CFG */
cfg_node* ast_to_cfg(ast_node* host, cfg_node* succ) {
  if (!host) return succ;
  switch (host->tag) {
    case _block:
      return cfg_visit_block(host->children[STAT_LIST_HEAD], succ);
    case _assign_stat:
      return cfg_mk_assign_stat(host, succ);
    case _skip_stat:
      return cfg_mk_skip_stat(host, succ);
    case _if_then_stat:
      return cfg_mk_if_then_stat(host, succ);
    case _if_else_stat:
      return cfg_mk_if_else_stat(host, succ);
    case _while_stat:
      return cfg_mk_while_stat(host, succ);
    case _await_stat:
      return cfg_mk_await_stat(host, succ);
  }
}

/* wrap AST to CFG conversion */ 
cfg_node* cfg_init(ast_node* program) {
  if (!program) return NULL;
  ast_node* ast_next_proc = program->children[PROC_LIST_HEAD];
  cfg_node* cfg_list_head = ast_to_cfg(ast_next_proc->children[BLOCK], NULL);
  cfg_node* cfg_next_proc = cfg_list_head;
  ast_next_proc = ast_next_proc->children[NEXT_PROC];
  while (ast_next_proc) {
    cfg_next_proc->next_proc = ast_to_cfg(ast_next_proc->children[BLOCK], NULL);
    cfg_next_proc = cfg_next_proc->next_proc;
    ast_next_proc = ast_next_proc->children[NEXT_PROC];
  }
  return cfg_list_head;
}