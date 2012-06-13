/* bdd.c
 * Written by Daniel Kudrow, 05/29/12
 * Last updated 05/29/12
 *
 * binary decision diagram
 * this file contains methods that traverse a CFG and encode it as a boolean
 * logic formula in a CUDD BDD structure
 */

#include <string.h>
#include <stdlib.h>

#include "cfg.h"
#include "bdd.h"

/* functions for symbol table management */

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
    sym = sym->next;                   /* the BDD if found */
  }
  return -1;                           /* if not found, return -1*/
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
  while (next) {                       /* populate symbol table */
    if (!symtab_insert(new_symtab, next->name)) {
      printf("Error: variable '%s' declared twice/n", next->name);
      exit(1);
    }
    next = next->children[NEXT_DECL];
  }
  return new_symtab;                   /* return filled symbol table */
}

/* index the variables in the boolean encoding of the program */
pos* pos_init(ast_node* program) {
  int n, i, j, k=0;
  pos* new_pos = malloc(sizeof(pos));  /* fill current and next state symtabs */
  for (new_pos->pc_size=0, n=program->id; n; n/=2, ++new_pos->pc_size);
  new_pos->vars = symtab_init(program);
  new_pos->vars_ = symtab_init(program);
  ast_node* next = program->children[PROC_LIST_HEAD];
  for (new_pos->num_procs = 0; next; new_pos->num_procs++)
    next = next->children[NEXT_PROC];  /* determine the number of processes */
  next = program->children[DECL_LIST_HEAD];
  for (new_pos->num_vars = 0; next; new_pos->num_vars++)
    next = next->children[NEXT_DECL];  /* determine the number of global vars */
  new_pos->pc = malloc(sizeof(int*) * new_pos->num_procs);
  for (i=0; i<new_pos->num_procs; i++) {
    new_pos->pc[i] = malloc(sizeof(int) * new_pos->pc_size);
    for (j=0; j<new_pos->pc_size; j++)
      new_pos->pc[i][j] = 2 * new_pos->num_vars + k++;
  }
  new_pos->pc_ = malloc(sizeof(int*) * new_pos->num_procs);
  for (i=0; i<new_pos->num_procs; i++) {
    new_pos->pc_[i] = malloc(sizeof(int) * new_pos->pc_size);
    for (j=0; j<new_pos->pc_size; j++)
      new_pos->pc_[i][j] = 2 * new_pos->num_vars + k++;
  }
  return new_pos;
}

/* methods for converting a CFG into a BDD */

/* encode the program counter in an array of boolean variables */
DdNode* bdd_encode_pc(DdManager* m, DdNode* node, int** pc_array, int pc_size,
                      int proc, int pc) {
  int i;
  DdNode* tmp, *var;
  DdNode* ret = Cudd_ReadOne(m);
  Cudd_Ref(ret);
  for (i=0; i<pc_size; i++) {          /* iterate for each bit in pc_size */
    var = Cudd_bddIthVar(m, pc_array[proc][i]);
    if (pc % 2)                        /* encode true bit */
      tmp = Cudd_bddAnd(m, var, ret);
    else                               /* encode false bit */
      tmp = Cudd_bddAnd(m, Cudd_Not(var), ret);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, ret);       /* release the old bdd each time a new */
    ret = tmp;                         /* one is made */
    pc /= 2;
  }
  tmp = Cudd_bddAnd(m, ret, node);     /* add encoding to existing edge */
  Cudd_Ref(tmp);
  Cudd_RecursiveDeref(m, ret);
  Cudd_RecursiveDeref(m, node); 
  return tmp;                          /* return updated edge */
}

/* recursively convert an AST expression into a BDD */
DdNode* bdd_expr(DdManager* m, symbol** symtab, ast_node* expr) {
  int i;
  DdNode* ret, *tmp1, *tmp2;
  switch (expr->tag) {
    case _id_expr:                     /* simply return that variable's BDD */
      if ((i = symtab_lookup(symtab, expr->name)) < 0) {
        printf("Error: variable '%s' not found in symbol table\n", expr->name);
        exit(1);
      }  
      ret = Cudd_bddIthVar(m, symtab_lookup(symtab, expr->name));
      break;
    case _lit_expr:                    /* return a BDD 1 or 0 */
      ret = expr->val ? Cudd_ReadOne(m) : Cudd_ReadLogicZero(m);
      break;
    case _not_expr:                    /* return complement BDD */
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      ret = Cudd_Not(tmp1);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      break;
    case _and_expr:                    /* return logical AND */
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddAnd(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _or_expr:                     /* return logical OR */
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddOr(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _eq_expr:                     /* check for equality, return 1 or 0 */
      tmp1 = bdd_expr(m, symtab, expr->children[EXPR1]);
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);      
      if (Cudd_EquivDC(m, tmp1, tmp2, Cudd_ReadLogicZero(m)))
        ret = Cudd_ReadOne(m);
      else 
        ret = Cudd_ReadLogicZero(m);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    case _impl_expr:                   /* return logical implication (!a|b) */
      tmp1 = Cudd_Not(bdd_expr(m, symtab, expr->children[EXPR1]));
      tmp2 = bdd_expr(m, symtab, expr->children[EXPR2]);
      ret = Cudd_bddOr(m, tmp1, tmp2);
      Cudd_Ref(ret);
      Cudd_RecursiveDeref(m, tmp1);
      Cudd_RecursiveDeref(m, tmp2);
      break;
    default:
      printf("Error: invalid expression\n");
      exit(1);
  }
  return ret; 
}

/* preserve the state of unaffected variables */
DdNode* bdd_same(DdManager* m, pos* postab, int var, int proc) {
  DdNode* ret = Cudd_ReadOne(m);
  DdNode* var1, *var2, *xnor, *tmp;    /* XNOR gate simulates equality */
  Cudd_Ref(ret);
  int i, j;
  for (i=0; i<postab->num_procs; i++) {/* iterate through all processes */
    if (i == proc) continue;           /* skip pc of current process */
    for (j=0; j<postab->pc_size; j++) {/* iterate through all bits */
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
  for (i=0; i<postab->num_vars; i++) { /* iterate through all globals */
    if (i == var) continue;            /* skip assignment variable */
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
/* formula for assignment: pc ^ var' <=> expr ^ pc' ^ same(pc, var) */
DdNode* bdd_mk_assign(DdManager *m, pos* postab, cfg_node* host, int proc) {
  DdNode* tmp1 = Cudd_bddIthVar(m, symtab_lookup(postab->vars_, host->node->name));
  DdNode* tmp2 = bdd_expr(m, postab->vars, host->node->children[EXPR]);
  DdNode* ret = Cudd_bddXnor(m, tmp1, tmp2);
  Cudd_Ref(ret);                       /* encode assignment with XNOR */
  Cudd_RecursiveDeref(m, tmp2);
  ret = bdd_encode_pc(m, ret, postab->pc, postab->pc_size, proc,
                      host->node->id); /* encode program counter */
  tmp1 = bdd_same(m, postab, symtab_lookup(postab->vars, host->node->name),
                  proc);               /* encode unchanging variables */
  tmp2 = ret;
  ret = Cudd_bddAnd(m, tmp1, tmp2);
  Cudd_Ref(ret);
  Cudd_RecursiveDeref(m, tmp1);
  Cudd_RecursiveDeref(m, tmp2);
  return ret;
}

/* create an edge from a skip statement */
/* formula for skip: pc ^ pc' ^ same(pc) */
DdNode* bdd_mk_skip(DdManager *m, pos *postab, int host_id, int proc) {
  DdNode* ret = bdd_same(m, postab, -1, proc);
  ret = bdd_encode_pc(m, ret, postab->pc, postab->pc_size, proc, host_id);
  return ret;
}


/* traverse a CFG and encode each edge as a BDD */
DdNode* bdd_convert_cfg(DdManager *m, pos* postab, cfg_node* host, int proc,
                        DdNode* current, DdNode* R, int prev) {
  DdNode* branch;                      /* holds branch conditions */
  DdNode* tmp1, *tmp2;                 /* temporary vars for cleanup */
  current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                          host->node->id);
  tmp1 = Cudd_bddOr(m, R, current);    /* add the completed edge to the */
  Cudd_Ref(tmp1);                      /* transistion system by disjuncting */
  Cudd_RecursiveDeref(m, R);
  Cudd_RecursiveDeref(m, current);
  R = tmp1;  
  switch (host->node->tag) {
    case _assign_stat:                 /* assignment and skip statements */
    case _skip_stat:                   /* form very similar edges */
      if (host->node->tag == _assign_stat)
        current = bdd_mk_assign(m, postab, host, proc);
      else if (host->node->tag == _skip_stat)
        current = bdd_mk_skip(m, postab, host->node->id,  proc);
      break;
    case _if_then_stat:                /* if a branch is encountered, */
    case _if_else_stat:                /* add the branch condition to the  */
    case _while_stat:                  /* edge's encoding and continue */
      current = bdd_mk_skip(m, postab, host->node->id,  proc);
      tmp1 = bdd_expr(m, postab->vars, host->node->children[EXPR]);
      branch = Cudd_bddAnd(m, tmp1, current);
      Cudd_Ref(branch);                /* conjunct branch condition to branch */
      tmp2 = Cudd_bddAnd(m, Cudd_Not(tmp1), current);
      Cudd_Ref(tmp2);                  /* conjuct complement of the branch */
      Cudd_RecursiveDeref(m, tmp1);    /* condition to current */
      Cudd_RecursiveDeref(m, current);
      current = tmp2;
      if (host->node->id < prev)       /* terminate after back edge */
        return R;
      else {                           /* traverse the 'true' branch */
        tmp1 = bdd_convert_cfg(m, postab, host->succ[CFG_IF | CFG_WHILE], proc,
                              branch, R, prev);
        Cudd_RecursiveDeref(m, R);
      }
      break;
    case _await_stat:                  /* await is both a branch and a skip */
      tmp1 = bdd_mk_skip(m, postab, host->node->id,  proc);
      tmp2 = bdd_expr(m, postab->vars, host->node->children[EXPR]);      
      current = Cudd_bddAnd(m, tmp1, tmp2);
      Cudd_Ref(current);               /* conjunct branch condition to current */
      branch = Cudd_bddAnd(m, tmp1, Cudd_Not(tmp2));
      Cudd_Ref(branch);                /* conjuct complement of the branch */
      Cudd_RecursiveDeref(m, tmp1);    /* condition to branch */
      Cudd_RecursiveDeref(m, tmp2);    /* branch is a self-referential edge */
      branch = bdd_encode_pc(m, branch, postab->pc_, postab->pc_size, proc,
                              host->node->id);
      tmp1 = Cudd_bddOr(m, R, branch);
      Cudd_Ref(tmp1);
      Cudd_RecursiveDeref(m, R);
      Cudd_RecursiveDeref(m, branch);  /* continue edge continues to next node */
      break;
    default:
      printf("Error: invalid statement in CFG\n");
      exit(1);
  }
  if (host->succ[CONTINUE])  {         /* if the node has a successor */
    R = bdd_convert_cfg(m, postab, host->succ[CONTINUE], proc, current,
                        tmp1, host->node->id);
    Cudd_RecursiveDeref(m, tmp1);      /* initiate new edge and call again */
  } else {                             /* if the node terminates a branch */
    current = bdd_encode_pc(m, current, postab->pc_, postab->pc_size, proc,
                            host->node->id);
    R = Cudd_bddOr(m, tmp1, current);
    Cudd_Ref(R);                       /* terminate node with an infinite loop */
    Cudd_RecursiveDeref(m, tmp1);
    Cudd_RecursiveDeref(m, current);
  }
  return R;                            /* return complete transition relation */
}

/* wrap the CFG to BDD conversion */
DdNode* encode_prog(DdManager* m, pos* postab, cfg_node* proc_list_head) {
  DdNode* tmp, *proc, *init;
  DdNode* R = Cudd_ReadLogicZero(m);
  Cudd_Ref(R);
  cfg_node* next = proc_list_head;
  int proc_no = 0;                     /* all process begin with an 'init' */
  while (next) {                       /* skip node that has id = 0 */
//     init = Cudd_ReadOne(m);
//     init = bdd_encode_pc(m, init, postab->pc, postab->pc_size, proc_no, 0);
    init = bdd_mk_skip(m, postab, 0, proc_no);
    proc = bdd_convert_cfg(m, postab, proc_list_head, proc_no, init, R, 0);
    tmp = Cudd_bddOr(m, R, proc);      /* add process to transition relation */
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, R);
    Cudd_RecursiveDeref(m, proc);
    R = tmp;
    next = next->next_proc;            /* next process */
    ++proc_no;
  }
  return R;                            /* return complete transition relation */
}

DdNode* compute_EX(DdManager* m, pos* postab, DdNode* R, DdNode* p) {
  DdNode* tmp;
  /* rename variables */
  int perm[2*(postab->num_procs*postab->pc_size+postab->num_vars)];
  int i, j, k;
  for (i=0; i<postab->num_vars; i++) {
    perm[i] = i+postab->num_vars;
    perm[i+postab->num_vars] = i;
  }
  for (j=0; j<postab->num_procs; j++) {
    for (k=0; k<postab->pc_size; k++) {
      perm[postab->num_vars+i] = postab->pc_[j][k];
      perm[postab->num_vars+postab->num_procs*postab->pc_size+i++] = postab->pc[j][k];
    }
  }
  p = Cudd_bddPermute(m, p, perm);
  /* construct cube of next state variables */
  DdNode* cube = Cudd_ReadOne(m);
  for (i=0; i<postab->num_vars; i++) {
    tmp = Cudd_bddAnd(m, cube, Cudd_bddIthVar(m, postab->num_vars+i));
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, cube);
    cube = tmp;
  }
  for (i=0; i<postab->num_procs*postab->pc_size; i++) {
    tmp = Cudd_bddAnd(m, cube, Cudd_bddIthVar(m, 2*postab->num_vars+postab->num_procs*postab->pc_size+i));
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m, cube);
    cube = tmp;
  }
  DdNode* EX_p = Cudd_bddAndAbstract(m, R, p, cube);
  Cudd_Ref(EX_p);
  Cudd_RecursiveDeref(m, p);
  Cudd_RecursiveDeref(m, cube);
  return EX_p;
}

DdNode* compute_EF(DdManager* m, pos* postab, DdNode* R, DdNode* p) {
  DdNode* last = Cudd_ReadOne(m);
  DdNode* current = p;
    while (!Cudd_EquivDC(m, last, current, Cudd_ReadLogicZero(m))) {
//       printf("iterating\n");
    last = Cudd_bddAnd(m, current, Cudd_ReadOne(m));
    current = Cudd_bddOr(m, p, compute_EX(m, postab, R, current));
  }
  return current;
}