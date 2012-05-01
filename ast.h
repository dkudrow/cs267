/* ast.h
 * Written by Daniel Kudrow, 04/30/12
 * Last updated 05/29/12
 *
 * abstract syntax tree
 * this file defines the structures and functions needed by the parser to 
 * construct an AST
 */

#ifndef AST_H
#define AST_H

<<<<<<< HEAD
#include <stdio.h>

/* the following macros specify the order of children in each type of node */
/* the children are indexed by 'node->children[MACRO]' */
=======
/* the folowing macros specify how the children of different flavors of */
/* ast_node node are arranged in the children array */
>>>>>>> added comments

/* program node children */
#define DECL_LIST_HEAD 0
#define DECL_LIST_TAIL 1
#define PROC_LIST_HEAD 2
#define PROC_LIST_TAIL 3
#define SPEC_LIST_HEAD 4
#define SPEC_LIST_TAIL 5

/* specification node children */
#define NEXT_SPEC      0
#define EXPR           1

/* declaration node children */
#define NEXT_DECL      0

/* block node children */
#define STAT_LIST_HEAD 0
#define STAT_LIST_TAIL 1

/* process node children */
#define NEXT_PROC      0
#define BLOCK          1

/* statement node children */
#define NEXT_STAT      0
#define EXPR           1
#define BLOCK1         2
#define BLOCK2         3

/* expression node children */
#define EXPR1          0
#define EXPR2          1

<<<<<<< HEAD
/* generic AST node type */
typedef struct _ast_node {
  /* the tag member sepcifies what type of AST node is being referenced */
=======
/* node type for AST */
typedef struct _ast_node {
  /* the tag field defines the node type */
>>>>>>> added comments
  enum {
    _undef = 0, _prog, _spec, _decl, _proc, _block, _assign_stat, _skip_stat,
    _if_then_stat, _if_else_stat, _while_stat, _await_stat, _id_expr,
    _lit_expr, _not_expr, _and_expr, _or_expr, _eq_expr, _impl_expr, _AF_expr,
    _EF_expr, _AG_expr, _EG_expr
  } tag;

  /* generic members common to all AST nodes */
  char* label;                         /* the statement's label */
  int lineno;                          /* line number in program */
  char* name;                          /* the assignment variable */
  int id;                              /* the statements pc value */
  int val;                             /* the value of a literal */
  struct _ast_node* children[6];       /* the next statement */
} ast_node;                            /* in program nodes, the if field */
                                       /* contains the highest pc value */

/* AST methods */
<<<<<<< HEAD
void ast_draw_tree(ast_node* tree, FILE* out);
ast_node* parse();
=======
>>>>>>> added comments
ast_node* ast_init();
ast_node* ast_block_init();
void ast_push_spec(ast_node* prog_node, ast_node* expr);
void ast_push_decl(ast_node* prog_node, char* name);
ast_node* ast_push_proc(ast_node* prog_node);
ast_node* ast_stat_init(ast_node* block, char* label, int id, int lineno);
void ast_push_assign_stat(ast_node* block, char* label, char* name,
                          ast_node* expr, int id, int lineno);
void ast_push_skip_stat(ast_node* block, char* label, int id, int lineno);
void ast_push_if_then_stat(ast_node* block, char* label, ast_node* expr,
                           ast_node* block1, int id, int lineno);
void ast_push_if_else_stat(ast_node* block, char* label, ast_node* expr,
                           ast_node* block1, ast_node* block2, int id, int lineno);
void ast_push_while_stat(ast_node* block, char* label, ast_node* expr,
                         ast_node* block1, int id, int lineno);  
void ast_push_await_stat(ast_node* block, char* label, ast_node* expr, int id,
                         int lineno);
ast_node* ast_push_id_expr(char* name, int lineno);
ast_node* ast_push_lit_expr(int val, int lineno);
ast_node* ast_push_not_expr(ast_node* expr1, int lineno);
ast_node* ast_push_and_expr(ast_node* expr1, ast_node* expr2, int lineno);
ast_node* ast_push_or_expr(ast_node* expr1, ast_node* expr2, int lineno);
ast_node* ast_push_eq_expr(ast_node* expr1, ast_node* expr2, int lineno);
ast_node* ast_push_impl_expr(ast_node* expr1, ast_node* expr, int lineno2);
ast_node* ast_push_AF_expr(ast_node* expr1, int lineno);
ast_node* ast_push_EF_expr(ast_node* expr1, int lineno);
ast_node* ast_push_AG_expr(ast_node* expr1, int lineno);
ast_node* ast_push_EG_expr(ast_node* expr1, int lineno);

#endif /* AST_H */
