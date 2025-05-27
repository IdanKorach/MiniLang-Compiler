#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic_analysis.h"
#include <stdio.h>

// Forward declarations
struct node;
struct scope;

// Global counters for temp variables and labels
extern int temp_counter;
extern int label_counter;

// Main 3AC generation function
void generate_3ac(struct node* ast_root, struct scope* global_scope);

// Helper functions for different AST node types
void generate_function(struct node* func);
void generate_function_body(struct node* body);
void generate_statement(struct node* stmt);
void generate_statements(struct node* stmts);
void generate_init_statement(struct node* init);
void generate_assign_statement(struct node* assign);

// Expression generation functions
char* generate_expression(struct node* expr);
char* generate_binary_operation(struct node* expr);

// Logical expression generation functions
char* generate_logical_and(struct node* expr);
char* generate_logical_or(struct node* expr);
char* generate_logical_not(struct node* expr);

// Helper functions for temp variables and labels (NEW)
char* new_temp();
char* new_label();
void reset_counters();  // Reset counters for each function

#endif // CODEGEN_H