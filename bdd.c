/* bdd.c
 * Written by Daniel Kudrow, 05/29/12
 * Last updated 05/29/12
 *
 * binary decision diagram
 */

#include <string.h>
#include <stdlib.h>
/* #include <stdio.h> */

#include "cfg.h"
#include "bdd.h"
#include "cudd.h"
#include "util.h"

/* symbol table management */

/* djb2 hash function */
int hash(char* s) {
  unsigned long h = 5381;
  char c;
  while ((c = *s++))
    h = (h << 5) + h + c;
  return h % TAB_SIZE;
}

/* lookup a name in a symbol table*/
int symtab_lookup(symbol** symbol_table, char* name) {
  int h = hash(name);
  symbol* sym = symbol_table[h];
  while (sym) {
    if (!strcmp(sym->name, name))
      return sym->var;                 /* return the variable's position in */
    sym = sym->next;                   /* the BDD */
  }
  return -1;
}

/* add a symbol to a symbol table */
int symtab_insert(symbol** symbol_table, char* name) {
  static int var_count = 0;            /* this numerical id will be used to */
  int h = hash(name);                  /* reference the variable in the BDD */
  symbol* sym = symbol_table[h];
  while (sym) {                        /* check if name exists in table */
    if (!strcmp(sym->name, name))
      return 0;                        /* unsuccesful return */
    sym = sym->next;
  }
  sym = malloc(sizeof(symbol));        /* if not, create a new symbol */
  sym->name = strdup(name);
  sym->var = var_count++;              /* increment the global variable count */
  if (symbol_table[h]) {
    sym->next = symbol_table[h];
    symbol_table[h] = sym;
  } else
    symbol_table[h] = sym;
  return 1;                            /* succesful return */
}

/* create and initialize a new symbol table */
symbol** symtab_init(ast_node* program) {
  symbol** new_symtab = calloc(TAB_SIZE, sizeof(symbol*));
  ast_node* next = program->children[DECL_LIST_HEAD];
  while (next) {
    if (!symtab_insert(new_symtab, next->name))
      /* error */;
    next = next->children[NEXT_DECL];
  }
  return new_symtab;
}

/* index the variables in the boolean encoding of the program */
pos* pos_init(ast_node* program) {
  int n, i, j, k=0;
  pos* new_pos = malloc(sizeof(pos)); /* fill current and next state symtabs */
  for (new_pos->pc_size=0, n=program->id; n; n/=2, ++new_pos->pc_size);
  new_pos->vars = symtab_init(program);
  new_pos->vars_ = symtab_init(program);
  ast_node* next = program->children[PROC_LIST_HEAD];
  for (new_pos->num_procs = 0; next; new_pos->num_procs++)
    next = next->children[NEXT_PROC];
  next = program->children[DECL_LIST_HEAD];
  for (new_pos->num_vars = 0; next; new_pos->num_vars++)
    next = next->children[NEXT_DECL];    
  new_pos->pc = malloc(sizeof(int*) * new_pos->num_procs);
  for (i=0; i<new_pos->num_procs; i++) {
    new_pos->pc[i] = malloc(sizeof(int) * new_pos->pc_size);
    for (j=0; j<new_pos->pc_size; j++) {
      new_pos->pc[i][j] = 2 * new_pos->num_vars + k++;
//       printf("pc[%i][%i] = %i\n", i, j, 2 * new_pos->num_vars + k-1);
    }
  }
  new_pos->pc_ = malloc(sizeof(int*) * new_pos->num_procs);
  for (i=0; i<new_pos->num_procs; i++) {
    new_pos->pc_[i] = malloc(sizeof(int) * new_pos->pc_size);
    for (j=0; j<new_pos->pc_size; j++) {
      new_pos->pc_[i][j] = 2 * new_pos->num_vars + k++;
//       printf("pc_[%i][%i] = %i\n", i, j, 2 * new_pos->num_vars + k-1);
    }
  }
  return new_pos;
}

/* encode the program counter in an array of boolean variables */
DdNode* bdd_encode_pc(DdManager* m, DdNode* node, int** pc_array, int pc_size,
                      int proc, int pc) {
  int i;
  DdNode* tmp, *var;
  DdNode* ret = Cudd_ReadOne(m);
  Cudd_Ref(ret);
  for (i=0; i<pc_size; i++) {
    var = Cudd_bddIthVar(m, pc_array[proc][i]);
    if (pc % 2)                      /* encode true bit */
      tmp = Cudd_bddAnd(m, var, ret);
    else                               /* encode false bit */
      tmp = Cudd_bddAnd(m, Cudd_Not(var), ret);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, ret);       /* make sure to release the old bdd */
    ret = tmp;                         /* each time a new one is made */
    pc /= 2;
  }
  tmp = Cudd_bddAnd(m, ret, node);
  Cudd_Ref(tmp);
  Cudd_RecursiveDeref(m, ret);
  Cudd_RecursiveDeref(m, node); 
  return tmp;
}

/* recursively convert an AST expression into a BDD */
DdNode* bdd_expr(DdManager* m, symbol** symtab, ast_node* expr) {
  DdNode* ret, *tmp1, *tmp2;
  switch (expr->tag) {
    case _id_expr:
      ret = Cudd_bddIthVar(m, symtab_lookup(symtab, expr->name));
      break;
    case _lit_expr:
      ret = expr->val ? Cudd_ReadOne(m) : Cudd_ReadLogicZero(m);
      break;
    case _not_expr:
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      ret = Cudd_Not(tmp1);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      break;
    case _and_expr:
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddAnd(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _or_expr:
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddOr(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _eq_expr:
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);      
      if (Cudd_EquivDC(m, tmp1, tmp2, Cudd_ReadLogicZero(m)))
        ret = Cudd_ReadOne(m);
      else 
        ret = Cudd_ReadLogicZero(m);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _impl_expr:
      tmp1 = Cudd_Not(bdd_expr(m, symtab, expr->children[EXPR1]));
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddOr(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    default:
      /* error */ ;
  }
  return ret; 
}

/* preserve the state of unaffected variables */
DdNode* bdd_same(DdManager* m, pos* postab, int var, int proc) {
  DdNode* ret = Cudd_ReadOne(m);
  DdNode* var1, *var2, *xnor, *tmp;
  Cudd_Ref(ret);
  int i, j;
  for (i=0; i<postab->num_procs; i++) {
    if (i == proc) continue;
    for (j=0; j<postab->pc_size; j++) {
      var1 = Cudd_bddIthVar(m, postab->pc[i][j]);
      var2 = Cudd_bddIthVar(m, postab->pc_[i][j]);
      xnor = Cudd_bddXnor(m, var1, var2);
      Cudd_Ref(xnor);
      tmp = Cudd_bddAnd(m, ret, xnor);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(m, xnor);
      Cudd_RecursiveDeref(m, ret);
      ret = tmp;
    }
  }
  for (i=0; i<postab->num_vars; i++) {
    if (i == var) continue;
    var1 = Cudd_bddIthVar(m, i);
    var2 = Cudd_bddIthVar(m, i+postab->num_vars);
    xnor = Cudd_bddXnor(m, var1, var2);
    Cudd_Ref(xnor);
    tmp = Cudd_bddAnd(m, ret, xnor);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, xnor);
    Cudd_RecursiveDeref(m, ret);
    ret = tmp;
  }
  return ret;
}

/* create an edge from an assignment statement */
DdNode* bdd_mk_assign(DdManager *m, pos* postab, cfg_node* host, int proc) {
  DdNode* tmp1 = Cudd_bddIthVar(m, symtab_lookup(postab->vars_, host->node->name));
  DdNode* tmp2 = bdd_expr(m, postab->vars, host->node->children[EXPR]);
  DdNode* ret = Cudd_bddXnor(m, tmp1, tmp2);
  Cudd_Ref(ret);
  Cudd_RecursiveDeref(m, tmp2);
  ret = bdd_encode_pc(m, ret, postab->pc, postab->pc_size, proc,
                      host->node->id);
  tmp1 = bdd_same(m, postab, symtab_lookup(postab->vars_, host->node->name),
                  proc);
  tmp2 = ret;
  ret = Cudd_bddAnd(m, tmp1, tmp2);
  Cudd_Ref(ret);
  Cudd_RecursiveDeref(m, tmp1);
  Cudd_RecursiveDeref(m, tmp2);
  return ret;
}

/* create an edge from a skip statement */
DdNode* bdd_skip(DdManager *m, pos *postab, int host_id, int proc) {
  DdNode* ret = bdd_same(m, postab, -1, proc);
  ret = bdd_encode_pc(m, ret, postab->pc, postab->pc_size, proc, host_id);
  return ret;
}

// /* traverse a CFG and encode each edge as a BDD */
// DdNode* bdd_convert_cfg(DdManager *m, pos* postab, cfg_node* host, int proc,
//                         DdNode* current, DdNode* R, int prev) {
//   DdNode* branch;                      /* holds the new branch condition */
//   DdNode* tmp;                         /* temp variable to aid in cleanup */
//   switch (host->node->tag) {           /* only action statements can */
//     case _assign_stat:                 /* terminate an edge */
//       current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                               host->node->id);
//       tmp = Cudd_bddOr(m, R, current);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(m, R);
//       Cudd_RecursiveDeref(m, current);
//       R = tmp;
//       if (prev < 0) return R;          /* ecnountered end of while loop */
//       current = bdd_mk_assign(m, postab, host, proc);
//       if (host->succ[CONTINUE])  {     /* check for terminal node */
//         tmp = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current, R,
//                             host->node->id);
//         Cudd_RecursiveDeref(m, R);
//         R = tmp;
//       } else {                          /* if so, repeat forever */
//         current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                                 host->node->id);
//         tmp = Cudd_bddOr(m, R, current);
//         Cudd_Ref(tmp);
//         Cudd_RecursiveDeref(m, R);
//         Cudd_RecursiveDeref(m, current);
//         R = tmp;
//       }
//       return R;
//     case _skip_stat:
//       current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                               host->node->id);
//       tmp = Cudd_bddOr(m, R, current);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(m, R);
//       Cudd_RecursiveDeref(m, current);
//       R = tmp;
//       if (prev < 0) return R;
//       current = bdd_skip(m, postab, host,  proc);
//       if (host->succ[CONTINUE]) {      /* check for terminal node */
//         tmp = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current, R,
//                               host->node->id);
//         Cudd_RecursiveDeref(m, R);
//         R = tmp;
//       } else {                         /* if so, repeat forever */
//         current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                                 host->node->id);
//         tmp = Cudd_bddOr(m, R, current);
//         Cudd_Ref(tmp);
//         Cudd_RecursiveDeref(m, R);
//         Cudd_RecursiveDeref(m, current);
//         tmp = R;
//       }     
//       return R;
//     case _if_then_stat:                /* traverse branches until an action */
//       tmp = bdd_expr(m, postab->vars, host->node->children[EXPR]);
//       branch = Cudd_bddAnd(m, tmp, current);
//       current = Cudd_bddAnd(m, Cudd_Not(tmp), current);
//       Cudd_Ref(branch);
//       Cudd_RecursiveDeref(m, tmp);
//       tmp = bdd_convert_cfg(m, postab, host->succ[CFG_IF], proc, branch, R,
//                           prev);
//       Cudd_RecursiveDeref(m, R);
//       R = tmp;
//       tmp = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current, R,
//                           prev);
//       Cudd_RecursiveDeref(m, R);
//       R = tmp;
//       return R;
//     case _if_else_stat:
//       tmp = bdd_expr(m, postab->vars, host->node->children[EXPR]);
//       branch = Cudd_bddAnd(m, tmp, current);
//       Cudd_Ref(branch);
//       Cudd_RecursiveDeref(m, tmp);
//       tmp = bdd_convert_cfg(m, postab, host->succ[CFG_IF], proc, branch, R,
//                           prev);
//       Cudd_RecursiveDeref(m, R);
//       R = tmp;
//       tmp = bdd_convert_cfg(m, postab, host->succ[CFG_ELSE], proc, current, R,
//                           prev);
//       Cudd_RecursiveDeref(m, R);
//       R = tmp;
//       return R;
//     case _while_stat:
//       tmp = bdd_expr(m, postab->vars, host->node->children[EXPR]);
//       branch = Cudd_bddAnd(m, tmp, current);
//       Cudd_Ref(branch);
//       Cudd_RecursiveDeref(m, tmp);
//       if (host->node->id < prev)
//         tmp = bdd_convert_cfg(m, postab, host->succ[CFG_WHILE], proc, branch, R,
//                             -1);
//       else 
//         tmp = bdd_convert_cfg(m, postab, host->succ[CFG_WHILE], proc, branch, R,
//                             prev);
//       Cudd_RecursiveDeref(m, R);
//       R = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current, tmp,
//                           prev);
//       Cudd_RecursiveDeref(m, tmp);
//       return R;
//     case _await_stat:  
//       current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                               host->node->id);
//       tmp = Cudd_bddOr(m, R, current);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(m, R);
//       Cudd_RecursiveDeref(m, current);
//       R = tmp;
//       current = bdd_skip(m, postab, host,  proc);
//       tmp = bdd_expr(m, postab->vars, host->node->children[EXPR]);
//       branch = Cudd_bddAnd(m, branch, current);
//       Cudd_Ref(branch);
//       Cudd_RecursiveDeref(m, tmp);
//       branch = bdd_encode_pc(m, branch, postab->pc_, postab->pc_size, proc,
//                              host->node->id);
//       tmp = R;
//       R = Cudd_bddOr(m, R, branch);
//       Cudd_Ref(R);
//       Cudd_RecursiveDeref(m, branch);
//       Cudd_RecursiveDeref(m, tmp);
//       if (host->succ[CONTINUE]) {      /* check for terminal node */
//         tmp = R;
//         Cudd_RecursiveDeref(m, tmp);
//         R = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current, R,
//                             prev);
//       } else {                         /* if so, repeat forever */
//         current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
//                                 host->node->id);
//         tmp = R;
//         R = Cudd_bddOr(m, R, current);
//         Cudd_Ref(R);
//         Cudd_RecursiveDeref(m, current);
//         Cudd_RecursiveDeref(m, tmp);
//       }     
//       return R;    
//     default:
//       /* error */;
//   }
//   return R;
// }

/* traverse a CFG and encode each edge as a BDD */
DdNode* bdd_convert_cfg(DdManager *m, pos* postab, cfg_node* host, int proc,
                        DdNode* current, DdNode* R, int prev) {
  DdNode* branch;                      /* holds branch conditions */
  DdNode* tmp1, *tmp2;                 /* temporary vars for cleanup */
  switch (host->node->tag) {
    case _assign_stat:                 /* assignment and skip statements */
    case _skip_stat:                   /* form very similar edges */
      current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                              host->node->id);
      tmp1 = Cudd_bddOr(m, R, current); /* add the completed edge to the */
      Cudd_Ref(tmp1);                   /* transistion system by disjuncting */
      Cudd_RecursiveDeref(m, R);
      Cudd_RecursiveDeref(m, current);
      if (prev < 0) return tmp1;        /* terminate in case of back edge */
      if (host->node->tag == _assign_stat)
        current = bdd_mk_assign(m, postab, host, proc);
      else if (host->node->tag == _skip_stat)
        current = bdd_skip(m, postab, host->node->id,  proc);
      if (host->succ[CONTINUE])  {     /* the node has a successor */
        R = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current,
                            tmp1, host->node->id);
        Cudd_RecursiveDeref(m, tmp1);  /* initiate new edge and call again */
      } else {                         /* otherwise stay forever */
        current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                                host->node->id);
        R = Cudd_bddOr(m, tmp1, current);
        Cudd_Ref(R);
        Cudd_RecursiveDeref(m, tmp1);
        Cudd_RecursiveDeref(m, current);
      }
      return R;
    case _if_then_stat:                /* traverse branches and keep track */
    case _if_else_stat:                /* of branch conditions until action */
    case _while_stat:
      tmp1 = bdd_expr(m, postab->vars, host->node->children[EXPR]);
      branch = Cudd_bddAnd(m, tmp1, current);
      Cudd_Ref(branch);                /* conjunct branch condition to branch */
      tmp2 = Cudd_bddAnd(m, Cudd_Not(tmp1), current);
      Cudd_Ref(tmp2);                  /* conjuct complement of branch */
      Cudd_RecursiveDeref(m, tmp1);    /* to current */
      Cudd_RecursiveDeref(m, current);
      current = tmp2;
      if (host->node->id < prev)       /* terminate after back edge */
        tmp1 = bdd_convert_cfg(m, postab, host->succ[CFG_IF | CFG_WHILE], proc,
                              branch, R, -1);
      else
        tmp1 = bdd_convert_cfg(m, postab, host->succ[CFG_IF | CFG_WHILE], proc,
                              branch, R, prev);
      Cudd_RecursiveDeref(m, R);
      R = tmp1;
      if (host->succ[CONTINUE]) {
        tmp1 = bdd_convert_cfg(m, postab, host->succ[CONTINUE | CFG_ELSE], proc,
                            current, R, prev);
        Cudd_RecursiveDeref(m, R);
        R = tmp1;
      }
      return R;
    case _await_stat:  
      current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                              host->node->id);
      tmp1 = Cudd_bddOr(m, R, current);
      Cudd_Ref(tmp1);
      Cudd_RecursiveDeref(m, R);
      Cudd_RecursiveDeref(m, current);
      R = tmp1;
      
      tmp1 = bdd_skip(m, postab, host->node->id,  proc);
      tmp2 = bdd_expr(m, postab->vars, host->node->children[EXPR]);      
      branch = Cudd_bddAnd(m, tmp1, tmp2);
      Cudd_Ref(branch);                /* conjunct branch condition to branch */
      current = Cudd_bddAnd(m, Cudd_Not(tmp1), tmp2);
      Cudd_Ref(current);               /* conjuct complement of branch */
      Cudd_RecursiveDeref(m, tmp1);    /* to current */
      Cudd_RecursiveDeref(m, tmp2);
      /* complete self referential edge current */
      current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                              host->node->id);
      tmp1 = Cudd_bddOr(m, R, current);
      Cudd_Ref(tmp1);
      Cudd_RecursiveDeref(m, R);
      Cudd_RecursiveDeref(m, current);
      /* complete continue edge branch */
      if (host->succ[CONTINUE])  {     /* the node has a successor */
        R = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, branch, tmp1,
                            host->node->id);
        Cudd_RecursiveDeref(m, tmp1);  /* initiate new edge and call again */
      } else {                         /* otherwise stay forever */
        branch = bdd_encode_pc(m, branch, postab->pc_, postab->pc_size, proc,
                                host->node->id);
        R = Cudd_bddOr(m, tmp1, branch);
        Cudd_Ref(R);
        Cudd_RecursiveDeref(m, tmp1);
        Cudd_RecursiveDeref(m, branch);
      }
      return R; 
    default:
      /* error */;
  }
  return R;
}

/* wrap the CFG to BDD conversion */
DdNode* encode_prog(DdManager* m, pos* postab, cfg_node* proc_list_head) {
  DdNode* tmp, *proc, *init;
  DdNode* R = Cudd_ReadLogicZero(m);
  Cudd_Ref(R);
  cfg_node* next = proc_list_head;
  int proc_no = 0;
  while (next) {
    init = bdd_skip(m, postab, 0, proc_no);
    proc = bdd_convert_cfg(m, postab, proc_list_head, proc_no, init, R, 0);
    tmp = Cudd_bddOr(m, R, proc);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, R);
    Cudd_RecursiveDeref(m, proc);
    R = tmp;
    next = next->next_proc;
    ++proc_no;
  }
  return R;
}