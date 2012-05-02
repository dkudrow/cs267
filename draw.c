/* draw.c
 * Written by Daniel Kudrow, 04/30/12
 * Last updated 04/30/12
 *
 * visual representation of abstrat syntzx tree
 */

#include <stdio.h>

#include "ast.h"

/* traverse an AST and output GraphViz dot code for each node */
void ast_draw_node(ast_node* host, int parent, FILE* out) {
  if (!host) return;                   /* return at list terminators */
  static int node_count = 0;           /* global node count */
  int current = node_count++;          /* id of current node */
  switch (host->tag) {
    case _prog:                        /* visit program node */
      fprintf(out, "%i [label=Program];\n", current);
      ast_draw_node(host->children[DECL_LIST_HEAD], current, out);
      ast_draw_node(host->children[PROC_LIST_HEAD], current, out);
      break;
    case _decl:                        /* visit declaration node */
      fprintf(out, "%i [label=%s];\n", current, host->name);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[NEXT_DECL], parent, out);
      break;
    case _proc:                        /* visit process node */
      fprintf(out, "%i [label=Process];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[BLOCK], current, out);
      ast_draw_node(host->children[NEXT_PROC], parent, out);
      break;
    case _block:                       /* visit block node */
      ast_draw_node(host->children[STAT_LIST_HEAD], parent, out);
      break;
    case _assign_stat:                 /* visit assignment statement node */
      fprintf(out, "%i [label=\"Assign: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      fprintf(out, "%i [label=%s];\n", node_count, host->name);
      fprintf(out, "%i -> %i;\n", current, node_count++);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _skip_stat:                   /* visit skip statement node */
      fprintf(out, "%i [label=\"Skip: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _if_then_stat:                /* visit if-then statement node */
      fprintf(out, "%i [label=\"If-Then: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[NEXT_STAT], current, out);
      break;
    case _if_else_stat:                /* visit if-then-else statement node */
      fprintf(out, "%i [label=\"If-Then: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[BLOCK2], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _while_stat:                  /* visit while statement node */
      fprintf(out, "%i [label=\"While: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _await_stat:                  /* visit await statement node */
      fprintf(out, "%i [label=\"Await: %s\"];\n", current,
              (host->label?host->label:""));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _id_expr:                     /* visit id expression node */
      fprintf(out, "%i [label=ID];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      fprintf(out, "%i [label=%s];\n", node_count, host->name);
      fprintf(out, "%i -> %i;\n", current, node_count++);
      break;
    case _lit_expr:                    /* visit literal expression node */
      fprintf(out, "%i [label=Literal];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      fprintf(out, "%i [label=%i];\n", node_count, host->val);
      fprintf(out, "%i -> %i;\n", current, node_count++);
      break;
    case _not_expr:                    /* visit not expression node */
      fprintf(out, "%i [label=Not];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR1], current, out);
      break;
    case _and_expr:                    /* visit and expression node */
      fprintf(out, "%i [label=And];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR1], current, out);
      ast_draw_node(host->children[EXPR2], current, out);
      break;
    case _or_expr:                     /* visit or expression node */
      fprintf(out, "%i [label=Or];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR1], current, out);
      ast_draw_node(host->children[EXPR2], current, out);
      break;
    case _eq_expr:                     /* visit equality expression node */
      fprintf(out, "%i [label=Equals];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR1], current, out);
      ast_draw_node(host->children[EXPR2], current, out);
      break;
    case _impl_expr:                   /* visit implication expression node */
      fprintf(out, "%i [label=Implies];\n", current);
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR1], current, out);
      ast_draw_node(host->children[EXPR2], current, out);
      break;
  }
}

/* initiate traversal of AST */
void ast_draw_tree(ast_node* tree, FILE* out) {
  fprintf(out, "digraph {\n");
  ast_draw_node(tree, 0, out);
  fprintf(out, "}\n");
}
