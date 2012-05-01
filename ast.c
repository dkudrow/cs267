/* ast.c
 * Written by Daniel Kudrow, 04/30/12
<<<<<<< HEAD
 * Last updated 05/17/12
=======
 * Last updated 05/01/12
>>>>>>> added comments
 *
 * abstract syntax tree
 * this file contains methods used by the parser to build an AST
 */

# include <stdlib.h>

#include "ast.h"

<<<<<<< HEAD
/* create a new AST */
/* returns a pointer to an empty program node */
=======
/* create and initialize new AST */
/* return root (empty program node) */
>>>>>>> added comments
ast_node* ast_init() {
  ast_node* new_prog_node = malloc(sizeof(ast_node));
  new_prog_node->tag = _prog;
  new_prog_node->children[DECL_LIST_HEAD] = NULL;
  new_prog_node->children[DECL_LIST_TAIL] = NULL;
  new_prog_node->children[PROC_LIST_HEAD] = NULL;
  new_prog_node->children[PROC_LIST_HEAD] = NULL;
  return new_prog_node;
}

<<<<<<< HEAD
/* push a new specification to an existing program node */
/* returns a pointer to the new specification node's expression child */
void ast_push_spec(ast_node* prog_node, ast_node* expr) {
  ast_node* new_spec_node = malloc(sizeof(ast_node));
  new_spec_node->tag = _spec;
  new_spec_node->children[EXPR] = expr;
  new_spec_node->children[NEXT_SPEC] = NULL;
  if (prog_node->children[SPEC_LIST_TAIL])
    prog_node->children[SPEC_LIST_TAIL]->children[NEXT_SPEC] = new_spec_node;
  else
    prog_node->children[SPEC_LIST_HEAD] = new_spec_node;
  prog_node->children[SPEC_LIST_TAIL] = new_spec_node;
}

/* push a new declaration node to an existing program node */
=======
/* push new declaration node to AST */
>>>>>>> added comments
void ast_push_decl(ast_node* prog_node, char* name) {
  ast_node* new_decl_node = malloc(sizeof(ast_node));
  new_decl_node->tag = _decl;
  new_decl_node->name = name;
  new_decl_node->children[NEXT_DECL] = NULL;
  if (prog_node->children[DECL_LIST_TAIL])
    prog_node->children[DECL_LIST_TAIL]->children[NEXT_DECL] = new_decl_node;
  else
    prog_node->children[DECL_LIST_HEAD] = new_decl_node;
  prog_node->children[DECL_LIST_TAIL] = new_decl_node;
}

<<<<<<< HEAD
/* create a new block node */
/* returns a pointer to an empty block node */
=======
/* create and initialize new block node */
/* return pointer to empty block node */
>>>>>>> added comments
ast_node* ast_block_init() {
  ast_node* new_block_node = malloc(sizeof(ast_node));
  new_block_node->tag = _block;
  new_block_node->children[STAT_LIST_HEAD] = NULL;
  new_block_node->children[STAT_LIST_TAIL] = NULL;
  return new_block_node;
}

<<<<<<< HEAD
/* push a new process to an existing program node */
/* returns a pointer to the new process node's block child */
=======
/* push new process node to AST */
/* return pointer to process's block child */
>>>>>>> added comments
ast_node* ast_push_proc(ast_node* prog_node) {
  ast_node* new_proc_node = malloc(sizeof(ast_node));
  ast_node* new_block_node = ast_block_init();
  new_proc_node->tag = _proc;
  new_proc_node->children[BLOCK] = new_block_node;
  new_proc_node->children[NEXT_PROC] = NULL;
  if (prog_node->children[PROC_LIST_TAIL])
    prog_node->children[PROC_LIST_TAIL]->children[NEXT_PROC] = new_proc_node;
  else
    prog_node->children[PROC_LIST_HEAD] = new_proc_node;
  prog_node->children[PROC_LIST_TAIL] = new_proc_node;
  return new_block_node;
}

<<<<<<< HEAD
/* create a new statement node and push it onto a block node */
/* returns a pointer to the new statement node */
ast_node* ast_stat_init(ast_node* block, char* label, int id, int lineno) {
=======
/* create and initialize new statement node */
/* return pointer to new empty statement node */
ast_node* ast_stat_init(ast_node* block, char* label) {
>>>>>>> added comments
  ast_node* new_stat_node = malloc(sizeof(ast_node));
  new_stat_node->label = label;
  new_stat_node->lineno = lineno;
  new_stat_node->id = id;
  new_stat_node->children[NEXT_STAT] = NULL;
  if (block->children[STAT_LIST_TAIL])
    block->children[STAT_LIST_TAIL]->children[NEXT_STAT] = new_stat_node;
  else
    block->children[STAT_LIST_HEAD] = new_stat_node;
  block->children[STAT_LIST_TAIL] = new_stat_node;
  return new_stat_node;
}

<<<<<<< HEAD
/* push an assigment statement onto a block node */
=======
/* push new assignment statement to AST */
>>>>>>> added comments
void ast_push_assign_stat(ast_node* block, char* label, char* name,
                          ast_node* expr, int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
  new_stat_node->tag = _assign_stat;
  new_stat_node->name = name;
  new_stat_node->children[EXPR] = expr;
}

<<<<<<< HEAD
/* push a skip statement onto a block node */
void ast_push_skip_stat(ast_node* block, char* label, int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
  new_stat_node->tag = _skip_stat;
}

/* push an if-then statement onto a block node */
=======
/* push new skip statement to AST */
void ast_push_skip_stat(ast_node* block, char* label) {
  ast_node* new_stat_node = ast_stat_init(block, label);
  new_stat_node->tag = _skip_stat;
}

/* push new if-then statement to AST */
>>>>>>> added comments
void ast_push_if_then_stat(ast_node* block, char* label, ast_node* expr,
                           ast_node* block1, int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
  new_stat_node->tag = _if_then_stat;
  new_stat_node->children[EXPR] = expr;
  new_stat_node->children[BLOCK1] = block1;
}

<<<<<<< HEAD
/* push an if-then-else statement onto a block node */
=======
/* push new if-then-else statement to AST */
>>>>>>> added comments
void ast_push_if_else_stat(ast_node* block, char* label, ast_node* expr,
                           ast_node* block1, ast_node* block2, int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
  new_stat_node->tag = _if_else_stat;
  new_stat_node->children[EXPR] = expr;
  new_stat_node->children[BLOCK1] = block1;
  new_stat_node->children[BLOCK2] = block2;
}

<<<<<<< HEAD
/* push a while statement onto a block node */
=======
/* push new while statement to AST */
>>>>>>> added comments
void ast_push_while_stat(ast_node* block, char* label, ast_node* expr,
                         ast_node* block1, int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
  new_stat_node->tag = _while_stat;
  new_stat_node->children[EXPR] = expr;
  new_stat_node->children[BLOCK1] = block1;
}

<<<<<<< HEAD
/* push an await statement onto a block node */
void ast_push_await_stat(ast_node* block, char* label, ast_node* expr,
                         int id, int lineno) {
  ast_node* new_stat_node = ast_stat_init(block, label, id, lineno);
=======
/* push new await statement to AST */
void ast_push_await_stat(ast_node* block, char* label, ast_node* expr) {
  ast_node* new_stat_node = ast_stat_init(block, label);
>>>>>>> added comments
  new_stat_node->tag = _await_stat;
  new_stat_node->children[EXPR] = expr;
}

<<<<<<< HEAD
/* create a new id expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_id_expr(char* name, int lineno) {
=======
/* push new id expression to AST */
ast_node* ast_push_id_expr(char* name) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _id_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->name = name;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new literal expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_lit_expr(int val, int lineno) {
=======
/* push new literal expression to AST */
ast_node* ast_push_lit_expr(int val) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _lit_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->val = val;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new not expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_not_expr(ast_node* expr1, int lineno) {
=======
/* push new not expression to AST */
ast_node* ast_push_not_expr(ast_node* expr1) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _not_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new and expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_and_expr(ast_node* expr1, ast_node* expr2, int lineno) {
=======
/* push new and expression to AST */
ast_node* ast_push_and_expr(ast_node* expr1, ast_node* expr2) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _and_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  new_expr_node->children[EXPR2] = expr2;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new or expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_or_expr(ast_node* expr1, ast_node* expr2, int lineno) {
=======
/* push new or expression to AST */
ast_node* ast_push_or_expr(ast_node* expr1, ast_node* expr2) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _or_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  new_expr_node->children[EXPR2] = expr2;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new equality expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_eq_expr(ast_node* expr1, ast_node* expr2, int lineno) {
=======
/* push new equality expression to AST */
ast_node* ast_push_eq_expr(ast_node* expr1, ast_node* expr2) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _eq_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  new_expr_node->children[EXPR2] = expr2;
  return new_expr_node;
}

<<<<<<< HEAD
/* create a new implication expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_impl_expr(ast_node* expr1, ast_node* expr2, int lineno) {
=======
/* push new implication expression to AST */
ast_node* ast_push_impl_expr(ast_node* expr1, ast_node* expr2) {
>>>>>>> added comments
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _impl_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  new_expr_node->children[EXPR2] = expr2;
  return new_expr_node;
}

/* create a new AF expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_AF_expr(ast_node* expr1, int lineno) {
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _AF_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  return new_expr_node;
}

/* create a new EF expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_EF_expr(ast_node* expr1, int lineno) {
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _EF_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  return new_expr_node;
}

/* create a new AG expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_AG_expr(ast_node* expr1, int lineno) {
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _AG_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  return new_expr_node;
}

/* create a new EG expression */
/* returns a pointer to the new (orphan) expression node */
ast_node* ast_push_EG_expr(ast_node* expr1, int lineno) {
  ast_node* new_expr_node = malloc(sizeof(ast_node));
  new_expr_node->tag = _EG_expr;
  new_expr_node->lineno = lineno;
  new_expr_node->children[EXPR1] = expr1;
  return new_expr_node;
}
