#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic_analysis.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
struct node;
struct scope;

// ============================================================================
// GLOBAL STATE MANAGEMENT
// ============================================================================
// Global counters for temp variables and labels
extern int temp_counter;
extern int label_counter;

// Helper functions for temp variables and labels
char* new_temp(void);
char* new_label(void);
void reset_counters(void);  // Reset counters for each function

// ============================================================================
// MAIN 3AC GENERATION ENTRY POINT
// ============================================================================
void generate_3ac(struct node* ast_root, struct scope* global_scope);

// ============================================================================
// FUNCTION-LEVEL CODE GENERATION
// ============================================================================
void process_ast_functions(struct node* node);
void generate_function(struct node* func);
void generate_function_body(struct node* body);

// Stack size calculation
int calculate_function_stack_size(struct node* func);
struct node* find_params_node(struct node* func);
struct node* search_for_params(struct node* node);
int calculate_params_size(struct node* params_node);
int calculate_param_types_size(struct node* node);
int count_parameters_under_type(struct node* type_node);
int count_param_names(struct node* node);
int get_type_from_string(char* type_str);
int get_type_size(int type);
int is_valid_param_name(char* token);

// ============================================================================
// STATEMENT GENERATION
// ============================================================================
void generate_statement(struct node* stmt);
void generate_statements(struct node* stmts);

// Variable declarations and assignments
void generate_init_statement(struct node* init);
void generate_assign_statement(struct node* assign);
void generate_multiple_assignment(struct node* multi_assign_node);
void handle_declaration_statement(struct node* declare_node);

// Comma declaration support
void generate_comma_declaration(struct node* declare_node);
void generate_variable_list_assignments(struct node* var_list);
int has_init_var_nodes(struct node* var_list);

// Function calls and returns
void generate_function_call_statement(struct node* call_stmt);
void generate_return_statement(struct node* return_node);

// ============================================================================
// CONTROL FLOW GENERATION
// ============================================================================
// Basic control structures
void generate_simple_if(struct node* if_node);
void generate_if_else(struct node* if_else_node);
void generate_while_statement(struct node* while_node);

// Complex if-elif-else chains
void generate_if_elif(struct node* if_elif_node);
void generate_if_elif_else(struct node* if_elif_else_node);
void generate_if_elif_with_final_else(struct node* if_elif_node, char* else_label, char* end_label);

// Elif chain processing helpers
void process_if_body_and_elif_chain(struct node* sequence, char* elif_start_label, char* end_label);
void process_elif_chain(struct node* elif_sequence, char* current_label, char* end_label);
void process_if_body_and_elif_with_final_else(struct node* sequence, char* elif_start_label, char* else_label, char* end_label);
void process_elif_chain_with_else_destination(struct node* elif_sequence, char* current_label, char* else_label, char* end_label);

// Individual elif handlers
void generate_single_elif(struct node* elif_node, char* end_label);
void generate_single_elif_with_next(struct node* elif_node, char* next_label, char* end_label);
void generate_single_elif_with_else_fallback(struct node* elif_node, char* else_label, char* end_label);

// ============================================================================
// EXPRESSION GENERATION
// ============================================================================
// Main expression handler
char* generate_expression(struct node* expr);

// Arithmetic and comparison operations
char* generate_binary_operation(struct node* expr);

// Logical operations (with short-circuit evaluation)
char* generate_logical_and(struct node* expr);
char* generate_logical_or(struct node* expr);
char* generate_logical_not(struct node* expr);

// Function call expressions
char* generate_function_call_expression(struct node* call_expr);

// String operations
char* generate_string_index(struct node* index_node);
char* generate_string_slice(struct node* slice_node);
char* generate_string_slice_step(struct node* slice_step_node);

// ============================================================================
// FUNCTION ARGUMENT HANDLING
// ============================================================================
void generate_function_arguments(struct node* call_node, int* arg_count, int* total_bytes);
void process_call_arguments(struct node* node, int* arg_count, int* total_bytes, int skip_first);
void generate_push_param(struct node* arg_node, int* total_bytes);
char* generate_argument_value(struct node* arg_node);
int is_argument_node(struct node* node);

#endif // CODEGEN_H