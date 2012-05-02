/* symtab.h
 * Written by Daniel Kudrow, 05/01/12
 * Last updated 05/01/12
 *
 * symbol table
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdlib.h>
#include <string.h>

#define TAB_SIZE  101                  /* size of hash table */

/* djb2 hash function */
int hash(char* s) {
  unsigned long h = 5381;
  char c;
  while (c = *s++)
    h = (h << 5) + h + c;
  return h % TAB_SIZE;
}

/* link for separately chained hash table */
typedef struct _symbol {
  char* name;
  struct _symbol* next;
} symbol;

/* create and initialize a new symbol table */
symbol** symbol_table_init() {
  return calloc(TAB_SIZE, sizeof(symbol*));
}

/* lookup a name in a symbol table*/
int symbol_table_lookup(symbol** symbol_table, char* name) {
  int h = hash(name);
  symbol* sym = symbol_table[h];
  while (sym) {
    if (!strcmp(sym->name, name))
      return 1;
    sym = sym->next;
  }
  return 0;
}

/* add a symbol to a symbol table */
int symbol_table_insert(symbol** symbol_table, char* name) {
  int h = hash(name);
  symbol* sym = symbol_table[h];
  while (sym) {
    if (!strcmp(sym->name, name))
      return 0;
    sym = sym->next;
  }
  sym = malloc(sizeof(symbol));
  sym->name = strdup(name);
  if (symbol_table[h]) {
    sym->next = symbol_table[h];
    symbol_table[h] = sym;
  } else
    symbol_table[h] = sym;
  return 1;
} 

#endif /* SYMTAB_H */