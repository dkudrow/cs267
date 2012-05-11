/* cfg.c
 * Written by Daniel Kudrow, 05/09/12
 * Last updated 05/09/12
 *
 * conrtol flow graph
 */

#include <stdlib.h>

#include "cfg.h"

cfg_node* ast_to_cfg(ast_node* host, cfg_node* succ);

cfg_node* cfg_visit_block(ast_node* host, cfg_node* succ) {
  if (!host->children[NEXT_STAT])
    return ast_to_cfg(host, succ);
  cfg_node* new_cfg_node = cfg_visit_block(host->children[NEXT_STAT], succ);
  return ast_to_cfg(host, new_cfg_node);
}

cfg_node* cfg_mk_assign_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->block_id = id;
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

cfg_node* cfg_mk_skip_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->block_id = id;
  return new_cfg_node;
}

cfg_node* cfg_mk_if_then_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->block_id = id;
  new_cfg_node->succ[CFG_IF] = ast_to_cfg(host->children[BLOCK1], succ);
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

cfg_node* cfg_mk_if_else_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->block_id = id;
  new_cfg_node->succ[CFG_IF] = ast_to_cfg(host->children[BLOCK1], succ);
  new_cfg_node->succ[CFG_ELSE] = ast_to_cfg(host->children[BLOCK2], succ);
  return new_cfg_node;
}

cfg_node* cfg_mk_while_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->block_id = id;
  new_cfg_node->succ[CFG_WHILE] = ast_to_cfg(host->children[BLOCK1], new_cfg_node);
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

cfg_node* cfg_mk_await_stat(ast_node* host, cfg_node* succ, int id) {
  cfg_node* new_cfg_node = malloc(sizeof(cfg_node));
  new_cfg_node->node = host;
  new_cfg_node->block_id = id;
  new_cfg_node->succ[CFG_AWAIT] = new_cfg_node;
  new_cfg_node->succ[CONTINUE] = succ;
  return new_cfg_node;
}

/* visit an AST sub-tree and return the converted CFG */
cfg_node* ast_to_cfg(ast_node* host, cfg_node* succ) {
  static int block_count = 0;
  if (!host) return succ;
  switch (host->tag) {
    case _block:
      return cfg_visit_block(host->children[STAT_LIST_HEAD], succ);
    case _assign_stat:
      return cfg_mk_assign_stat(host, succ, block_count++);
    case _skip_stat:
      return cfg_mk_skip_stat(host, succ, block_count++);
    case _if_then_stat:
      return cfg_mk_if_then_stat(host, succ, block_count++);
    case _if_else_stat:
      return cfg_mk_if_else_stat(host, succ, block_count++);
    case _while_stat:
      return cfg_mk_while_stat(host, succ, block_count++);
    case _await_stat:
      return cfg_mk_await_stat(host, succ, block_count++);
    default:
    puts("DANGER WILL ROBINSON\n");
    return NULL;
  }
}

cfg_node* cfg_init(ast_node* program) {
  if (!program) return NULL;
  ast_node* next = program->children[PROC_LIST_HEAD];
  cfg_node* proc = ast_to_cfg(next->children[BLOCK], NULL);
  next = next->children[NEXT_PROC];
  while (next) {
    proc->next_proc = ast_to_cfg(next->children[BLOCK], NULL);
    proc = proc->next_proc;
    next = next->children[NEXT_PROC];
  }
  return proc;
}