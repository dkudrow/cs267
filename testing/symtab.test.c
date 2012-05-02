/* symtab.test.c
 * Written by Daniel Kudrow, 05/01/12
 * Last updated 05/01/12
 *
 * symbol table test program
 */

#include <stdio.h>
#include "../symtab.h"

int main() {
  char* names[] = { "j", "elw", "var3", "var4", "var5", "var6", "var7" };
  symbol** symtab = symbol_table_init();
  int i;
  for (i=0; i<7; i++)
    symbol_table_insert(symtab, names[i]);

  for (i=0; i<7; i++)
    printf("%s: %i\n",names[i], symbol_table_lookup(symtab, names[i]));
  printf("var%i: %i\n",8 , symbol_table_lookup(symtab, "var8"));
  printf("var%i: %i\n",9 , symbol_table_lookup(symtab, "var9"));

//   char* s = malloc(65), *c;
//   while (s = fgets(s, 64, stdin)) {
//     c = strchr(s, '\n');
//     c = '\0';
//     printf("%s: %i\n", s, hash(s));
//   }

  return 0;
}