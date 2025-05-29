#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Forward declarations
struct node;
struct scope;

// AST node structure
typedef struct node {
    char *token;
    struct node *left;
    struct node *right;
} node;

// Type constants
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_BOOL 3
#define TYPE_FLOAT 4

// Variable structure
typedef struct var {
    char* name;
    int type;
    struct var* next;
} var;

// Scope structure  
typedef struct scope {
    var* variables;
    struct scope* parent;
    char* scope_name;
} scope;

// Function information structure
typedef struct function_info {
    char* name;
    int param_count;
    int* param_types;        // Array of parameter types
    char** param_names;      // Array of parameter names
    int* has_default;        // Array indicating which params have defaults
    int return_type;         // 0 for no return type, TYPE_INT/STRING/etc for others
    int declaration_position; // Track declaration position or line number
    struct function_info* next;
} function_info;

// Global variables
extern function_info* declared_functions;
extern function_info* current_function;
extern int declaration_counter;
extern int semantic_errors;
extern int current_position;
extern int debug_level;

// Logging functions
void log_debug(const char* message);
void log_info(const char* message);
void log_error(const char* message);
void log_debug_format(const char* format, ...);
void log_info_format(const char* format, ...);
void log_error_format(const char* format, ...);

// Type helper functions
int get_type(char* type_str);
char* get_type_name(int type);

// Scope management functions
scope* mkscope(scope* parent);
void add_variable(scope* curr_scope, char* name, int type);
var* find_variable_in_scope(scope* curr_scope, char* name);
var* find_variable_in_scope_hierarchy(scope* curr_scope, char* name);

// Function management functions
function_info* create_function_info(char* name, int return_type);
function_info* find_function_by_name(char* func_name);
function_info* add_function_declaration(char* func_name, int return_type);
void add_parameter_to_function(function_info* func, char* param_name, int param_type, int has_default_value);

// Expression type analysis
int get_expression_type(node* expr_node, scope* curr_scope);
int check_index_operation(node* node, scope* current_scope);
int check_slice_operation(node* node, scope* current_scope);
int looks_like_string_literal(char* token);

// Statement handlers
void handle_variable_usage(node* var_node, scope* curr_scope);
void handle_declaration(node* declare_node, scope* curr_scope);
void handle_initialization(node* init_node, scope* curr_scope);
void handle_assignment(node* assign_node, scope* curr_scope);
void handle_function_call(node* call_node, scope* curr_scope);
void handle_return_statement(node* return_node, scope* curr_scope);
void handle_if_statement(node* if_node, scope* curr_scope);
void handle_while_statement(node* while_node, scope* curr_scope);

// Parameter processing functions
void process_params(node* node, scope* func_scope);
void process_params_for_function(node* param_node, function_info* func_info, scope* func_scope);
void process_parameter_names_for_type(node* type_node, int param_type, function_info* func_info, scope* func_scope);
void collect_parameter_names(node* node, int param_type, function_info* func_info, scope* func_scope);
int is_valid_parameter_name(char* token);

// Validation functions
void validate_condition_type(node* condition_node, scope* curr_scope, const char* context);
void validate_main_function(node* func_node, scope* func_scope);
int extract_return_type(node* func_node);

// Utility functions
int count_function_arguments(node* args_node);
node** extract_function_arguments(node* args_node, int* arg_count);
int is_variable_usage(node* var_node, node* parent_node);
int is_function_declared(char* func_name);

// Main analysis function
void analyze_node(node* root, node* parent, scope* curr_scope);
void semantic_analysis(struct node* root, struct scope* curr_scope);

#endif // SEMANTIC_ANALYSIS_H