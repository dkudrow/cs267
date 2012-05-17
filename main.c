/* main.c
 * Written by Daniel Kudrow, 05/17/12
 * Last updated 05/17/12
 *
 * main
 */

#include <stdio.h>

#include "cfg.h"

int main() {
  ast_node* tree = parse();
  cfg_node* cfg = cfg_init(tree);
  cfg_draw_graph(cfg, stdout);
  /* ast_draw_tree(tree, stdout); */
  return 0;
}