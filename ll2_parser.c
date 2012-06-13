/* parser.c
 * Written by Daniel Kudrow, 04/15/12
 * Last updated 05/17/12
 *
 * lexer and parser
 * this file contains the lexer and parser responsible for generating the AST
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ast.h"

/* define terminal tokens */
typedef enum {
  T_UNDEF = -1, T_EOF, T_SEMI, T_COLON, T_CONCUR, T_ID, T_SKIP, T_IF,
  T_THEN, T_ELSE, T_WHILE, T_DO, T_AWAIT, T_ASSIGN, T_L_PAREN, T_R_PAREN,
  T_L_BRACE, T_R_BRACE, T_L_BRACK, T_R_BRACK, T_TRUE, T_FALSE, T_NOT, T_AND,
  T_OR, T_IMPL, T_EQ, T_AF, T_EF, T_EG, T_AG
} token_t;

/* return string representation of token */
char* token_to_string(token_t token) {
  switch (token) {
    case T_UNDEF:
      return "'undefined'";
    case T_EOF:
      return "'EOF'";
    case T_SEMI:
      return "';'";
    case T_COLON:
      return "':'";
    case T_CONCUR:
      return "'||'";
    case T_ID:
      return "'ID'";
    case T_SKIP:
      return "'skip'";
    case T_IF:
      return "'if'";
    case T_THEN:
      return "'then'";
    case T_ELSE:
      return "'then'";
    case T_WHILE:
      return "'while'";
    case T_DO:
      return "'do'";
    case T_AWAIT:
      return "'await'";
    case T_ASSIGN:
      return "':='";
    case T_L_PAREN:
      return "'('";
    case T_R_PAREN:
      return "')'";
    case T_L_BRACE:
      return "'{'";
    case T_R_BRACE:
      return "'}'";
    case T_L_BRACK:
      return "'['";
    case T_R_BRACK:
      return "']'";
    case T_TRUE:
      return "'true'";
    case T_FALSE:
      return "'false'";
    case T_NOT:
      return "'!'";
    case T_AND:
      return "'&'";
    case T_OR:
      return "'|'";
    case T_IMPL:
      return "'=>'";
    case T_EQ:
      return "'='";
    case T_AF:
      return "'AF'";
    case T_EF:
      return "'EF'";
    case T_AG:
      return "'AG'";
    case T_EG:
      return "'EG'";
  }
}

/* array of reserved names */
static int num_rids = 13;
static struct {
  char* name;
  token_t type;
} rids[] = {
  {"true", T_TRUE}, {"false", T_FALSE}, {"skip", T_SKIP}, {"if", T_IF},
  {"then", T_THEN}, {"else", T_ELSE}, {"while", T_WHILE}, {"do", T_DO},
  {"await", T_AWAIT}, {"AF", T_AF}, {"EF", T_EF}, {"AG", T_AG}, {"EG", T_EG}
};

/* global variables */
static int lineno = 1;                 /* current line */
static int charno = 1;                 /* current char */
static char buffer[65] = "";           /* buffer for lexeme */
static char lexeme[65];                /* current lexeme */
static token_t lookahead_1;            /* first lookahead */
static token_t lookahead_2;            /* second lookahead */
static int stat_count = 1;             /* statement counter */

/* wrap fgetc and increment charno */
char fgetc_(FILE* in) {
  ++charno;
  return fgetc(in);
}

/* wrap ungetc and decrement charno */
void ungetc_(char c, FILE* in) {
  if (ungetc(c, in) > 0)
    --charno;
}

/* return next token from input */
int get_next_token(FILE* in) {
  char current;                        /* current character */
  int i, lexlen = 0;                   /* length of lexeme */

  while (isspace(current = fgetc_(in)))/* skip whitespace */
    if (current == '\n') { ++lineno; charno = 1; }

  while (current == '#') {             /* skip comments */
    while ((current = fgetc_(in)) != '\n');
    ++lineno; charno = 1;
    while (isspace(current = fgetc_(in)))
      if (current == '\n') { ++lineno; charno = 1; }
  }

  if (isalpha(current)) {              /* tokenize names */
    do {
      if (lexlen < 65)                 /* lexeme holds first 64 chars */
        buffer[lexlen++] = current;    /* construct lexeme */
    } while (isalnum(current = fgetc_(in)));
    ungetc_(current, in);              /* ungetc_ current */
    buffer[lexlen] = '\0';             /* terminate string */
    for (i=0; i<num_rids; i++) {       /* check for reserved names */
      if (!strcmp(buffer, rids[i].name))
        return rids[i].type;
    }
    return T_ID;
  }

  switch (current) {                   /* tokenize smbolic operators */
    case ':':                          /* ambiguous symbols */
      if ((current = fgetc_(in)) == '=')
        return T_ASSIGN;
      ungetc_(current, in);
      return T_COLON;
    case '=':
      if ((current = fgetc_(in)) == '>')
        return T_IMPL;
      ungetc_(current, in);
      return T_EQ;
    case '|':
      if ((current = fgetc_(in)) == '|')
        return T_CONCUR;
      ungetc_(current, in);
      return T_OR;
    case ';':                          /* unique symbols */
      return T_SEMI;
    case '!':
      return T_NOT;
    case '&':
      return T_AND;
    case '(':
      return T_L_PAREN;
    case ')':
      return T_R_PAREN;
    case '{':
      return T_L_BRACE;
    case '}':
      return T_R_BRACE;
    case '[':
      return T_L_BRACK;
    case ']':
      return T_R_BRACK;
    case EOF:
      return T_EOF;
    default:                           /* unrecognized symbol */
      fprintf(stderr, "Scan error on line %i:%i invalid character '%c'\n",
lineno, charno, current);
      exit(1);
  }
}

/* a match returns true and consumes the current token */
/* a mismatch returns false and leaves the current token unchanged */
int match(token_t first, token_t second) {
  if (lookahead_1 == first && second == T_UNDEF) {
    strncpy(lexeme, buffer, 64);       /* save lexeme before advancing lexer */
    lookahead_1 = lookahead_2;
    lookahead_2 = get_next_token(stdin);
    return 1;
  } else if (lookahead_1 == first && lookahead_2 == second) {
    strncpy(lexeme, buffer, 64);       /* save lexeme before advancing lexer */
    lookahead_1 = get_next_token(stdin);
    lookahead_2 = get_next_token(stdin);
    return 1;
  } else
  return 0;
}

/* print error message and exit */
void syntax_error(const char* e) {
  fprintf(stderr, "Syntax error on line %i:%i ", lineno, charno);
  fprintf(stderr, "found %s but expected ", token_to_string(lookahead_1));
  fprintf(stderr, "%s\n", e);
  exit(1);
}

/* a match consumes the next token */
/* a mismatch raises an error */
void expect(token_t expected) {
  if (lookahead_1 == expected) {
    lookahead_1 = lookahead_2;
    lookahead_2 = get_next_token(stdin);
    return;
  } else
    syntax_error(token_to_string(expected));
}

/* forward declarations for parser */
void parse_program(ast_node* program);
void parse_specification(ast_node* program);
void parse_declarations(ast_node* program);
void parse_processes(ast_node* program);
void parse_process(ast_node* block);
ast_node* parse_block();
void parse_statements(ast_node* block);
int parse_lstatement(ast_node* block);
ast_node* parse_expression();
ast_node* parse_expression_(ast_node* lhs);
ast_node* parse_base();

/* program -> declaration* processes */
void parse_program(ast_node* program) {
  parse_specification(program);
  parse_declarations(program);
  parse_processes(program);
}

void parse_specification(ast_node* program) {
  ast_node* expr;
  while (match(T_L_BRACK, T_UNDEF)) {
    expr = parse_expression();
    ast_push_spec(program, expr);
    expect(T_R_BRACK);
  }
}

/* Declaration -> 'id' ';' */
void parse_declarations(ast_node* program) {
  while (match(T_ID, T_SEMI))
    ast_push_decl(program, strdup(lexeme));
}

/* processes -> process ['||' process] */
void parse_processes(ast_node* program) {
  ast_node* block;
  do {
    block = ast_push_proc(program);
    parse_process(block);
    program->id = (program->id < stat_count ? stat_count : program->id);
    stat_count = 1;
  } while (match(T_CONCUR, T_UNDEF));
  expect(T_EOF);
}

/* process -> statements */
void parse_process(ast_node* block) {
  parse_statements(block);
}

/* block -> lstatement | '{' statements '}' */
ast_node* parse_block() {
  ast_node* block = ast_block_init();
  if (match(T_L_BRACE, T_UNDEF)) {
    parse_statements(block);
    expect(T_R_BRACE);
  } else if (parse_lstatement(block));
  else
    syntax_error("'ID', 'skip', 'if', 'while' or 'await'");
  return block;
}

/* statements -> lstatement [statements] */
void parse_statements(ast_node* block) {
  if (parse_lstatement(block))
    while (parse_lstatement(block));
  else
    syntax_error("'ID', 'skip', 'if', 'while' or 'await'");
}

/* lstatement -> ['ID' ':'] statement */
int parse_lstatement(ast_node* block) {
  int current_id = stat_count++;       /* store ID before parsing a block */
  char* label = (char*)NULL;
  ast_node* expr;
  ast_node* block1, *block2;
  if (match(T_ID, T_COLON))            /* statement has a label */
    label = strdup(lexeme);
  if (match(T_ID, T_ASSIGN)) {         /* parse assignment statement */
    char* name = strdup(lexeme);       /* save lexeme before advancing */
    expr = parse_expression();
    expect(T_SEMI);
    ast_push_assign_stat(block, label, name, expr, current_id, lineno);
    return 1;
  } else if (match(T_SKIP, T_UNDEF)) { /* parse skip statement */
    expect(T_SEMI);
    ast_push_skip_stat(block, label, current_id, lineno);
    return 1;
  } else if (match(T_IF, T_UNDEF)) {   /* parse if-then-else statement */
    expect(T_L_PAREN);
    expr = parse_expression();
    expect(T_R_PAREN);
    expect(T_THEN);
    block1 = parse_block();
    if (match(T_ELSE, T_UNDEF)) {      /* create else node */
      block2 = parse_block();
      ast_push_if_else_stat(block, label, expr, block1, block2, current_id, lineno);
    } else
      ast_push_if_then_stat(block, label, expr, block1, current_id, lineno);
    return 1;
  } else if (match(T_WHILE, T_UNDEF)) {/* parse while statement */
    expect(T_L_PAREN);
    expr = parse_expression();
    expect(T_R_PAREN);
    expect(T_DO);
    block1 = parse_block();
    ast_push_while_stat(block, label, expr, block1, current_id, lineno);
    return 1;
  } else if (match(T_AWAIT, T_UNDEF)) {/* parse await statement */
    expect(T_L_PAREN);
    expr = parse_expression();
    expect(T_R_PAREN);
    expect(T_SEMI);
    ast_push_await_stat(block, label, expr, current_id, lineno);
    return 1;
  } else {
    --stat_count;                      /* backtrack statement counter */
    return 0;
  }
}

/* expression -> base expression_ */
ast_node* parse_expression() {
  ast_node* lhs = parse_base();
  return parse_expression_(lhs);
}

/* expression -> op expression expression_ */
ast_node* parse_expression_(ast_node* lhs) {
  if (match(T_AND, T_UNDEF)) {         /* parse binary and expression */
    ast_node* rhs = parse_expression();
    return ast_push_and_expr(lhs, rhs, lineno);
  } else if (match(T_OR, T_UNDEF)) {   /* parse binary or expression */
    ast_node* rhs = parse_expression();
    return ast_push_or_expr(lhs, rhs, lineno);
  } else if (match(T_EQ, T_UNDEF)) {   /* parse binary equality expression */
    ast_node* rhs = parse_expression();
    return ast_push_eq_expr(lhs, rhs, lineno);
  } else if (match(T_IMPL, T_UNDEF)) { /* parse binary implies expression */
    ast_node* rhs = parse_expression();
    return ast_push_impl_expr(lhs, rhs, lineno);
  }
  return lhs;
}

/* base -> 'id' | 'true' | 'false' | '(' expression ')' | '!' base */
ast_node* parse_base() {
  if (match(T_ID, T_UNDEF))            /* parse variable instance */
    return ast_push_id_expr(strdup(lexeme), lineno);
  else if (match(T_TRUE, T_UNDEF))     /* parse literal */
    return ast_push_lit_expr(1, lineno);
  else if (match(T_FALSE, T_UNDEF))
    return ast_push_lit_expr(0, lineno);
  else if (match(T_L_PAREN, T_UNDEF)) {/* parse parenthetical */
    ast_node* expr = parse_expression();
    expect(T_R_PAREN);
    return expr;
  } else if (match(T_NOT, T_UNDEF)) {  /* parse unary not */
    ast_node* expr = parse_base();
    return ast_push_not_expr(expr, lineno);
  } else if (match(T_AF, T_UNDEF)) {  /* parse CTL AF */
    ast_node* expr = parse_base();
    return ast_push_AF_expr(expr, lineno);
  } else if (match(T_EF, T_UNDEF)) {  /* parse CTL EF */
    ast_node* expr = parse_base();
    return ast_push_EF_expr(expr, lineno);
  } else if (match(T_AG, T_UNDEF)) {  /* parse CTL AG */
    ast_node* expr = parse_base();
    return ast_push_AG_expr(expr, lineno);
  } else if (match(T_EG, T_UNDEF)) {  /* parse CTL EG */
    ast_node* expr = parse_base();
    return ast_push_EG_expr(expr, lineno);
  }
  else
    syntax_error("'ID', '(', '!', 'true' or 'false'");
}

/* wrap parse routines */
ast_node* parse() {
  ast_node* tree = ast_init();
  lookahead_1 = get_next_token(stdin);
  lookahead_2 = get_next_token(stdin);
  parse_program(tree);
  return tree;
}