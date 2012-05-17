/* draw.c
 * Written by Daniel Kudrow, 04/30/12
 * Last updated 05/17/12
 *
 * visual representation of abstrat syntzx tree
 */

#include <stdio.h>

#include "cfg.h"

/* ast.h is included in cfg.h so it is not explicitly included here */

/* if a node has a label, return it as a string */
#define AST_LABEL(host) (host->label?host->label:"")
#define CFG_LABEL(host) (host->node->label?host->node->label:"")

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
      fprintf(out, "%i [label=\"Assign: %s\"];\n", current, AST_LABEL(host));
      fprintf(out, "%i -> %i;\n", parent, current);
      fprintf(out, "%i [label=%s];\n", node_count, host->name);
      fprintf(out, "%i -> %i;\n", current, node_count++);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _skip_stat:                   /* visit skip statement node */
      fprintf(out, "%i [label=\"Skip: %s\"];\n", current, AST_LABEL(host));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _if_then_stat:                /* visit if-then statement node */
      fprintf(out, "%i [label=\"If-Then: %s\"];\n", current, AST_LABEL(host));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[NEXT_STAT], current, out);
      break;
    case _if_else_stat:                /* visit if-then-else statement node */
      fprintf(out, "%i [label=\"If-Then: %s\"];\n", current, AST_LABEL(host));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[BLOCK2], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _while_stat:                  /* visit while statement node */
      fprintf(out, "%i [label=\"While: %s\"];\n", current, AST_LABEL(host));
      fprintf(out, "%i -> %i;\n", parent, current);
      ast_draw_node(host->children[EXPR], current, out);
      ast_draw_node(host->children[BLOCK1], current, out);
      ast_draw_node(host->children[NEXT_STAT], parent, out);
      break;
    case _await_stat:                  /* visit await statement node */
      fprintf(out, "%i [label=\"Await: %s\"];\n", current, AST_LABEL(host));
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


/* output string representation of AST statement node */
void cfg_to_string(cfg_node* host, FILE* out) {
  switch(host->node->tag) {
    case _assign_stat:
      fprintf(out, "\"ASSIGN\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
    case _skip_stat:
      fprintf(out, "\"SKIP\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
    case _if_then_stat:
      fprintf(out, "\"IF-THEN\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
    case _if_else_stat:
      fprintf(out, "\"IF-ELSE\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
    case _while_stat:
      fprintf(out, "\"WHILE\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
    case _await_stat:
      fprintf(out, "\"AWAIT\\n%s\\npc: %i", CFG_LABEL(host), host->node->id);
      break;
  }
}

/* draw CFG node and descend into succesors */
void cfg_draw_node(cfg_node* host, FILE* out, int proc, int parent) {
  if (!host) return;
  fprintf(out, "%i%i [label=", proc, host->node->id);
  cfg_to_string(host, out);
  fprintf(out, "\"];\n");
  if (parent >= 0)                     /* root node has no parent */
    fprintf(out, "%i%i -> %i%i;\n", proc, parent, proc, host->node->id);
  /* the following tests prevent cfg_draw_node from moving backwards */
  /* through the control flow */
  if (host->succ[0] && host->node->id >= host->succ[0]->node->id)
    fprintf(out, "%i%i -> %i%i;\n", proc, host->node->id, proc, host->succ[0]->node->id);
  else
    cfg_draw_node(host->succ[0], out, proc, host->node->id);
  if (host->succ[1] && host->node->id >= host->succ[1]->node->id)
    fprintf(out, "%i%i -> %i%i;\n", proc, host->node->id, proc, host->succ[1]->node->id);
  else
    cfg_draw_node(host->succ[1], out, proc, host->node->id);
}

/* initiate traversal of CFG */
void cfg_draw_graph(cfg_node* host, FILE* out) {
  cfg_node* next = host;
  int proc = 1; 
  fprintf(out, "digraph {\n");
  while (next) {
    cfg_draw_node(next, stdout, proc++, -1);
    next = next->next_proc;
  }
  fprintf(out, "}\n");
}