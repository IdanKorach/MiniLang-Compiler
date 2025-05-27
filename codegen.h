#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic_analysis.h"
#include <stdio.h>

struct node;
struct scope;

// Main 3AC generation function
void generate_3ac(struct node* ast_root, struct scope* global_scope);

// Helper functions for different AST node types
void generate_function(struct node* func);
void generate_function_body(struct node* body);
void generate_statement(struct node* stmt);
void generate_init_statement(struct node* init);
void generate_assign_statement(struct node* assign);

#endif // CODEGEN_H