#include "semantic_analysis.h"
#include <stdarg.h>

// Variable structure
typedef struct var {
    char* name;
    int type;  // We'll use integers for types: 1=int, 2=string, 3=bool, 4=float
    struct var* next;
} var;

// Scope structure
typedef struct scope {
    var* variables;
    struct scope* parent;
    char* scope_name;  // For debugging
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

// Debug level flag: 0 = errors only, 1 = basic info, 2 = verbose debug
int debug_level = 2;  // Default: show basic info, but not detailed debug

// Simple logging function with debug level control
void log_debug(const char* message) {
    if (debug_level >= 2) {
        printf("DEBUG: %s\n", message);
    }
}

void log_info(const char* message) {
    if (debug_level >= 1) {
        printf("INFO: %s\n", message);
    }
}

void log_error(const char* message) {
    // Errors are always shown
    printf("ERROR: %s\n", message);
    semantic_errors++;
}

// Helper functions for formatted messages
void log_debug_format(const char* format, ...) {
    if (debug_level >= 2) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);
        printf("DEBUG: %s\n", buffer);
    }
}

void log_info_format(const char* format, ...) {
    if (debug_level >= 1) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);
        printf("INFO: %s\n", buffer);
    }
}

void log_error_format(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    printf("ERROR: %s\n", buffer);
    semantic_errors++;
}

// Global variable to track declared functions
function_info* declared_functions = NULL;
// Global variable to track current function context for return type validation
function_info* current_function = NULL;
// Global counter for tracking declaration order
int declaration_counter = 0;
// Global variable to track semantic errors
int semantic_errors = 0;
// Global variable to track current position in AST traversal
int current_position = 0;

int get_expression_type(node* expr_node, scope* curr_scope);
int check_index_operation(node* node, scope* current_scope);
int check_slice_operation(node* node, scope* current_scope);
char* get_type_name(int type);

// Type constants
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_BOOL 3
#define TYPE_FLOAT 4

// Helper function to create a new function_info
function_info* create_function_info(char* name, int return_type) {
    function_info* new_func = (function_info*)malloc(sizeof(function_info));
    new_func->name = strdup(name);
    new_func->param_count = 0;
    new_func->param_types = NULL;
    new_func->param_names = NULL;
    new_func->has_default = NULL;
    new_func->return_type = return_type;
    new_func->declaration_position = 0;
    new_func->next = declared_functions;
    return new_func;
}

// Helper function to find a function by name in the global list
function_info* find_function_by_name(char* func_name) {
    function_info* current = declared_functions;
    while (current) {
        if (strcmp(current->name, func_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Helper function to get type from string
int get_type(char* type_str) {
    if (strcmp(type_str, "int") == 0) return TYPE_INT;
    if (strcmp(type_str, "string") == 0) return TYPE_STRING;
    if (strcmp(type_str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(type_str, "float") == 0) return TYPE_FLOAT;
    return 0; // none
}

// Helper function to get type name from type integer
char* get_type_name(int type) {
    switch(type) {
        case TYPE_INT: return "int";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        case TYPE_FLOAT: return "float";
        default: return "none";
    }
}

// Create a new scope
scope* mkscope(scope* parent) {
    scope* new_scope = (scope*)malloc(sizeof(scope));
    new_scope->variables = NULL;
    new_scope->parent = parent;
    new_scope->scope_name = NULL;
    return new_scope;
}

// Add a variable to the current scope
void add_variable(scope* curr_scope, char* name, int type) {
    var* new_var = (var*)malloc(sizeof(var));
    new_var->name = strdup(name);
    new_var->type = type;
    new_var->next = curr_scope->variables;
    curr_scope->variables = new_var;
    
    log_debug_format("Added variable '%s' of type '%s' to scope %s", 
                   name, get_type_name(type), 
                   curr_scope->scope_name ? curr_scope->scope_name : "global");
}

// Helper function to add a function declaration
function_info* add_function_declaration(char* func_name, int return_type) {
    // Check if function already exists
    function_info* current = declared_functions;
    while (current) {
        if (strcmp(current->name, func_name) == 0) {
            log_error_format("Function '%s' already declared", func_name);
            return NULL;
        }
        current = current->next;
    }
    
    // Create new function info
    function_info* new_func = create_function_info(func_name, return_type);
    new_func->declaration_position = declaration_counter++;
    declared_functions = new_func;
    
    if (return_type != 0) {
        log_info_format("Function '%s' declared successfully (return type: %s)", 
                      func_name, get_type_name(return_type));
    } else {
        log_info_format("Function '%s' declared successfully", func_name);
    }
    
    return new_func;
}

// Helper function to add a parameter to a function_info
void add_parameter_to_function(function_info* func, char* param_name, int param_type, int has_default_value) {
    func->param_count++;
    
    func->param_types = (int*)realloc(func->param_types, func->param_count * sizeof(int));
    func->param_names = (char**)realloc(func->param_names, func->param_count * sizeof(char*));
    func->has_default = (int*)realloc(func->has_default, func->param_count * sizeof(int));
    
    // Add the new parameter (at the end)
    int index = func->param_count - 1;
    func->param_types[index] = param_type;
    func->param_names[index] = strdup(param_name);
    func->has_default[index] = has_default_value;
}

// Count the number of arguments in a function call
int count_function_arguments(node* args_node) {
    if (!args_node) return 0;
    
    // If this is a direct argument (not a comma-separated list)
    if (args_node->token && strcmp(args_node->token, "") != 0) {
        return 1;  // This is a single argument
    }
    
    // For a comma-separated list, count them 
    int count = 0;
    node* curr = args_node;
    
    // The first level has the first argument and a continuation on the right
    if (curr->left) count++;
    
    curr = curr->right;
    while (curr) {
        if (curr->token && strcmp(curr->token, "") != 0) {
            count++;
            break;
        } else if (curr->left) {
            count++;
        }
        curr = curr->right;
    }
    
    return count;
}

// Helper function to extract argument nodes from function call
// Returns an array of argument nodes and sets count
node** extract_function_arguments(node* args_node, int* arg_count) {
    if (!args_node) {
        *arg_count = 0;
        return NULL;
    }
    
    // Count the arguments
    *arg_count = count_function_arguments(args_node);
    
    if (*arg_count == 0) {
        return NULL;
    }
    
    // Allocate array to hold argument nodes
    node** arg_nodes = (node**)malloc(*arg_count * sizeof(node*));
    int index = 0;
    
    // If this is a direct argument (not a comma-separated list)
    if (args_node->token && strcmp(args_node->token, "") != 0) {
        arg_nodes[0] = args_node;
        return arg_nodes;
    }
    
    // For a comma-separated list, extract the arguments
    node* curr = args_node;
    
    // First argument is in left child of first node
    if (curr->left) {
        arg_nodes[index++] = curr->left;
    }
    
    curr = curr->right;
    while (curr && index < *arg_count) {
        if (curr->token && strcmp(curr->token, "") != 0) {
            arg_nodes[index++] = curr;
            break;
        } else if (curr->left) {
            arg_nodes[index++] = curr->left;
        }
        curr = curr->right;
    }
    
    return arg_nodes;
}

// Helper function to validate the __main__ function
void validate_main_function(node* func_node, scope* func_scope) {
    if (!func_node || !func_node->left || !func_node->left->token) {
        return;
    }
    
    // Check if this is the __main__ function
    if (strcmp(func_node->left->token, "__main__") != 0) {
        return;
    }
    
    log_info("Validating __main__ function requirements...");
        
    node* func_body = func_node->right;
    if (!func_body) return;
    
    int has_params = 0;
    int has_return_type = 0;
    
    // Check direct children and their children - BFS
    node* nodes_to_check[25] = {func_body, NULL}; 
    int front = 0, rear = 1;
    
    while (front < rear && rear < 25) {
        node* current = nodes_to_check[front++];
        if (!current) continue;
        
        if (current->token) {
            if (strcmp(current->token, "params") == 0) {
                if (current->left || current->right) {
                    has_params = 1;
                }
            }
            else if (strcmp(current->token, "return_type") == 0) {
                has_return_type = 1;
            }
        }
        
        // Add children to queue
        if (current->left && rear < 25) {
            nodes_to_check[rear++] = current->left;
        }
        if (current->right && rear < 25) {
            nodes_to_check[rear++] = current->right;
        }
    }
    
    if (has_params) {
        log_error("__main__ function cannot have parameters");
    } else {
        log_info("__main__ parameters: ✓ (none)");
    }
    
    if (has_return_type) {
        log_error("__main__ function cannot have a return type");
    } else {
        log_info("__main__ return type: ✓ (none)");
    }
}

// Check if variable exists in current scope (for redeclaration check)
var* find_variable_in_scope(scope* curr_scope, char* name) {
    var* current = curr_scope->variables;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Uncomment log_debug_format lines to enable a more deep debug logging
// Check if variable exists in current scope OR any parent scope (for usage check)
var* find_variable_in_scope_hierarchy(scope* curr_scope, char* name) {
    // log_debug_format("Looking for variable '%s' in scope hierarchy", name);
    scope* temp_scope = curr_scope;
    
    while (temp_scope != NULL) {
        // log_debug_format("Checking scope: %s", temp_scope->scope_name ? temp_scope->scope_name : "unnamed");
        var* temp_var = temp_scope->variables;
        while (temp_var != NULL) {
            if (strcmp(temp_var->name, name) == 0) {
                // log_debug_format("Found variable '%s' in scope %s", name, 
                                // temp_scope->scope_name ? temp_scope->scope_name : "unnamed");
                return temp_var;
            }
            temp_var = temp_var->next;
        }
        // log_debug_format("Variable '%s' not found in scope %s, checking parent", name, 
                        // temp_scope->scope_name ? temp_scope->scope_name : "unnamed");
        temp_scope = temp_scope->parent;
    }
    // log_debug_format("Variable '%s' not found in any scope", name);
    return NULL;
}

// Handle variable usage - check if variable is declared
void handle_variable_usage(node* var_node, scope* curr_scope) {
    if (!var_node || !var_node->token) {
        return;
    }
    
    char* var_name = var_node->token;
    
    log_debug_format("Found variable usage: %s", var_name);
    
    // Check if variable exists in scope hierarchy
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, var_name);
    
    if (!found_var) {
        log_error_format("Variable '%s' used before declaration", var_name);
        return;
    }
    
    log_debug_format("Variable '%s' used (type: %s)", var_name, get_type_name(found_var->type));
}

// Function to detect string literals
int looks_like_string_literal(char* token) {
    if (!token) return 0;
    
    // Check if token starts and ends with quotes (double or single)
    int len = strlen(token);
    if (len >= 2) {
        if ((token[0] == '"' && token[len-1] == '"') ||
            (token[0] == '\'' && token[len-1] == '\'')) {
            return 1;
        }
    }
    
    // Check for common string literal patterns
    // If it contains spaces, it's likely a string literal
    if (strchr(token, ' ') != NULL) return 1;
    
    // If it contains quotes (escaped), it's a string literal
    if (strstr(token, "\\\"") != NULL || strstr(token, "\\'") != NULL) return 1;
    
    // If it contains common escape sequences
    if (strstr(token, "\\n") != NULL || strstr(token, "\\t") != NULL || 
        strstr(token, "\\\\") != NULL) return 1;
    
    // If it contains special characters that are common in strings
    if (strchr(token, ':') != NULL || strchr(token, '@') != NULL || 
        strchr(token, '#') != NULL || strchr(token, '$') != NULL ||
        strchr(token, '%') != NULL || strchr(token, '^') != NULL ||
        strchr(token, '&') != NULL || strchr(token, '*') != NULL ||
        strchr(token, '(') != NULL || strchr(token, ')') != NULL) return 1;
    
    return 0;
}

// Get the type of an expression node
int get_expression_type(node* expr_node, scope* curr_scope) {
    if (!expr_node || !expr_node->token) {
        return 0; // Unknown type
    }
    
    log_debug_format("get_expression_type: analyzing token='%s'", expr_node->token);
    
    // Handle arithmetic operators
    if (strcmp(expr_node->token, "+") == 0 || 
        strcmp(expr_node->token, "-") == 0 ||
        strcmp(expr_node->token, "*") == 0 || 
        strcmp(expr_node->token, "/") == 0 ||
        strcmp(expr_node->token, "%") == 0 ||
        strcmp(expr_node->token, "**") == 0) {
        
        log_debug("Detected arithmetic operator");
        
        // Get types of operands
        int left_type = get_expression_type(expr_node->left, curr_scope);
        int right_type = get_expression_type(expr_node->right, curr_scope);
        
        // Special case for string concatenation (only for + operator)
        if (strcmp(expr_node->token, "+") == 0 && 
            (left_type == TYPE_STRING || right_type == TYPE_STRING)) {
            return TYPE_STRING;  // Result of string concatenation is a string
        }
        
        // Skip type checking if either operand has unknown type (likely undeclared variable)
        if (left_type == 0 || right_type == 0) {
            return 0; // Don't report additional type errors
        }
        
        // Type checking for arithmetic operators (only if both types are known)
        if (left_type != TYPE_INT && left_type != TYPE_FLOAT) {
            log_error_format("Left operand of '%s' must be numeric (int or float), got '%s'", 
                        expr_node->token, get_type_name(left_type));
            return 0; 
        }
        
        // Right operand must also be numeric
        if (right_type != TYPE_INT && right_type != TYPE_FLOAT) {
            log_error_format("Right operand of '%s' must be numeric (int or float), got '%s'", 
                        expr_node->token, get_type_name(right_type));
            return 0; 
        }
        
        // Result is int if both operands are int, otherwise float
        if (left_type == TYPE_INT && right_type == TYPE_INT) {
            return TYPE_INT;
        } else {
            return TYPE_FLOAT;
        }
    }
    
    // Handle logical operators: and, or
    if (strcmp(expr_node->token, "and") == 0 || strcmp(expr_node->token, "or") == 0) {
        log_debug("Detected logical operator");
        
        int left_type = get_expression_type(expr_node->left, curr_scope);
        int right_type = get_expression_type(expr_node->right, curr_scope);
        
        // Both operands must be boolean
        if (left_type != TYPE_BOOL) {
            log_error_format("Left operand of '%s' must be boolean, got '%s'", 
                           expr_node->token, get_type_name(left_type));
            return 0; 
        }
        
        if (right_type != TYPE_BOOL) {
            log_error_format("Right operand of '%s' must be boolean, got '%s'", 
                           expr_node->token, get_type_name(right_type));
            return 0; 
        }
        
        return TYPE_BOOL; // Result is boolean
    }
    
    // Handle not operator
    if (strcmp(expr_node->token, "not") == 0) {
        log_debug("Detected 'not' operator");
        
        int operand_type = get_expression_type(expr_node->right, curr_scope);
        
        // Operand must be boolean
        if (operand_type != TYPE_BOOL) {
            log_error_format("Operand of 'not' must be boolean, got '%s'", 
                           get_type_name(operand_type));
            return 0; 
        }
        
        return TYPE_BOOL; // Result is boolean
    }
    
    // Handle comparison operators: <, >, <=, >=
    if (strcmp(expr_node->token, "<") == 0 || 
        strcmp(expr_node->token, ">") == 0 ||
        strcmp(expr_node->token, "<=") == 0 || 
        strcmp(expr_node->token, ">=") == 0) {
        
        log_debug("Detected comparison operator");
        
        int left_type = get_expression_type(expr_node->left, curr_scope);
        int right_type = get_expression_type(expr_node->right, curr_scope);
        
        // Skip type checking if either operand has unknown type
        if (left_type == 0 || right_type == 0) {
            return TYPE_BOOL; // Still return bool for the comparison, but don't report type errors
        }
        
        // Both operands must be numeric (int or float)
        if (left_type != TYPE_INT && left_type != TYPE_FLOAT) {
            log_error_format("Left operand of '%s' must be numeric (int or float), got '%s'", 
                        expr_node->token, get_type_name(left_type));
            return 0; 
        }
        
        if (right_type != TYPE_INT && right_type != TYPE_FLOAT) {
            log_error_format("Right operand of '%s' must be numeric (int or float), got '%s'", 
                        expr_node->token, get_type_name(right_type));
            return 0; 
        }
        
        return TYPE_BOOL; // Result is always boolean
    }
    
    // Handle equality operators: ==, !=
    if (strcmp(expr_node->token, "==") == 0 || strcmp(expr_node->token, "!=") == 0) {
        log_debug("Detected equality operator");
        
        int left_type = get_expression_type(expr_node->left, curr_scope);
        int right_type = get_expression_type(expr_node->right, curr_scope);
        
        // Both operands must be of the same type
        if (left_type != right_type) {
            log_error_format("Operands of '%s' must be of the same type, got '%s' and '%s'", 
                           expr_node->token, get_type_name(left_type), get_type_name(right_type));
            return 0; 
        }
        
        return TYPE_BOOL; // Result is always boolean
    }
    
    // Handle string indexing
    if (strcmp(expr_node->token, "index") == 0) {
        log_debug("Detected string indexing operation");
        return check_index_operation(expr_node, curr_scope);
    }

    // Handle string slicing
    if (strcmp(expr_node->token, "slice") == 0) {
        log_debug("Detected string slicing operation");
        return check_slice_operation(expr_node, curr_scope);
    }

    // Handle string slicing with step
    if (strcmp(expr_node->token, "slice_step") == 0) {
        log_debug("Detected string slicing with step operation");
        return check_slice_operation(expr_node, curr_scope);
    }
    
    // Check if it's a variable - look up its declared type
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, expr_node->token);
    if (found_var) {
        log_debug_format("Found variable '%s' with type %d", expr_node->token, found_var->type);
        return found_var->type;
    }
    
    // Check if it's a number literal
    if (expr_node->token[0] >= '0' && expr_node->token[0] <= '9') {
        if (strchr(expr_node->token, '.') != NULL) {
            log_debug("Detected float literal");
            return TYPE_FLOAT;
        } else {
            log_debug("Detected int literal");
            return TYPE_INT;
        }
    }
    
    // Check if it's a boolean literal
    if (strcmp(expr_node->token, "True") == 0 || strcmp(expr_node->token, "False") == 0 ||
        strcmp(expr_node->token, "true") == 0 || strcmp(expr_node->token, "false") == 0) {
        log_debug("Detected boolean literal");
        return TYPE_BOOL;
    }
    
    // Check if it looks like a string literal
    if (looks_like_string_literal(expr_node->token)) {
        log_debug("Detected string literal by pattern");
        return TYPE_STRING;
    }
    
    if (strcmp(expr_node->token, "call") == 0) {
        if (!expr_node->left || !expr_node->left->token) {
            log_debug("Invalid function call in expression");
            return 0;
        }
        
        char* func_name = expr_node->left->token;
        log_debug_format("Getting return type for function call: %s", func_name);
        
        // Find the function in our declared functions list
        function_info* func_info = find_function_by_name(func_name);
        if (!func_info) {
            log_error_format("Function '%s' called before declaration", func_name); 
            log_debug_format("Function '%s' not found, can't determine return type", func_name);
            return 0;
        }
        
        // Check if function was declared before current position
        if (func_info->declaration_position >= current_position) {
            log_error_format("Function '%s' called before declaration", func_name);
            return 0;
        }
        
        // Return the function's return type
        log_debug_format("Function '%s' has return type: %s", 
                    func_name, get_type_name(func_info->return_type));
        return func_info->return_type;
    }         
    
    // If we reach here, it's likely an undeclared variable or unknown operator
    if (expr_node->token[0] >= 'a' && expr_node->token[0] <= 'z' || 
        expr_node->token[0] >= 'A' && expr_node->token[0] <= 'Z' || 
        expr_node->token[0] == '_') {
        // This looks like an identifier that wasn't found
        log_error_format("Variable '%s' used before declaration", expr_node->token);
        return 0; // Still return 0, but we've reported the real error
    }

    log_debug("Undeclared identifier - returning unknown type");
    return 0; // Return 0 to indicate we can't determine the type
}

// Helper function to validate that an expression is of boolean type
void validate_condition_type(node* condition_node, scope* curr_scope, const char* context) {
    if (!condition_node) {
        log_error_format("Missing condition in %s", context);
        return;
    }
    
    log_debug_format("Validating %s condition type", context);
    
    // Get the type of the condition expression
    int condition_type = get_expression_type(condition_node, curr_scope);
    
    if (condition_type == 0) {
        log_info_format("Cannot determine type of condition in %s", context);
        return;
    }
    
    if (condition_type != TYPE_BOOL) {
        log_error_format("%s condition must be boolean type. Expected: bool, Got: %s", 
                        context, get_type_name(condition_type));
        return;
    }
    
    log_info_format("%s condition type validated successfully (bool)", context);
}

// Handle string indexing operations
int check_index_operation(node* node, scope* current_scope) {
    if (!node || strcmp(node->token, "index") != 0) {
        log_error("Internal error: check_index_operation called on non-index node");
        return 0;
    }
    
    // Get the string expression (LHS)
    if (!node->left) {
        log_error("Invalid index operation: missing variable");
        return 0;
    }
    
    // Get the type of the string expression
    int string_type = 0;
    if (node->left->token) {
        // Check if it's a variable name
        var* found_var = find_variable_in_scope_hierarchy(current_scope, node->left->token);
        if (found_var) {
            string_type = found_var->type;
            log_debug_format("Found variable '%s' with type %d for indexing", node->left->token, string_type);
        } else {
            // Otherwise, evaluate as an expression
            string_type = get_expression_type(node->left, current_scope);
        }
    } else {
        string_type = get_expression_type(node->left, current_scope);
    }
    
    if (string_type != TYPE_STRING) {
        log_error_format("Index operator '[]' can only be used with string type, got '%s'", 
                        get_type_name(string_type));
        return 0;
    }
    
    // Now check if the index (RHS) is an integer
    if (!node->right) {
        log_error("Invalid index operation: missing index expression");
        return 0;
    }
    
    // Get the type of the index expression
    int index_type = 0;
    if (node->right->token) {
        // Check if it's a variable name
        var* found_var = find_variable_in_scope_hierarchy(current_scope, node->right->token);
        if (found_var) {
            index_type = found_var->type;
            log_debug_format("Found variable '%s' with type %d for index", node->right->token, index_type);
        } else {
            // Try to evaluate as a literal
            if (node->right->token[0] >= '0' && node->right->token[0] <= '9') {
                index_type = TYPE_INT;
                log_debug("Detected int literal for index");
            } else {
                // Evaluate as an expression
                index_type = get_expression_type(node->right, current_scope);
            }
        }
    } else {
        index_type = get_expression_type(node->right, current_scope);
    }
    
    if (index_type != TYPE_INT) {
        log_error_format("String index must be of integer type, got '%s'", get_type_name(index_type));
        return 0;
    }
    
    // If both checks pass, the result is a string (a single character is still a string in our language)
    log_info("String indexing operation validated successfully");
    return TYPE_STRING;
}

// Handle string slicing operations
int check_slice_operation(node* node, scope* current_scope) {
    if (!node || (strcmp(node->token, "slice") != 0 && strcmp(node->token, "slice_step") != 0)) {
        log_error("Internal error: check_slice_operation called on non-slice node");
        return 0;
    }
    
    // Get the string expression (left child)
    if (!node->left) {
        log_error("Invalid slice operation: missing variable");
        return 0;
    }
    
    // Check if the string expression is a string type
    int string_type = 0;
    if (node->left->token) {
        // Check if it's a variable name
        var* found_var = find_variable_in_scope_hierarchy(current_scope, node->left->token);
        if (found_var) {
            string_type = found_var->type;
            log_debug_format("Found variable '%s' with type %d for slicing", node->left->token, string_type);
        } else {
            // Otherwise, evaluate as an expression
            string_type = get_expression_type(node->left, current_scope);
        }
    } else {
        string_type = get_expression_type(node->left, current_scope);
    }
    
    if (string_type != TYPE_STRING) {
        log_error_format("Slice operator '[::]' can only be used with string type, got '%s'", 
                        get_type_name(string_type));
        return 0;
    }
    
    // Now check if the slice indices are integers
    if (!node->right) {
        log_error("Invalid slice operation: missing slice indices");
        return 0;
    }
    
    // For regular slicing (slice), we have a node structure like:
    // node -> right -> (left, right) for start and end indices
    if (strcmp(node->token, "slice") == 0) {
        // Check start index type (could be NULL for default)
        if (node->right->left && strcmp(node->right->left->token, "0") != 0) {
            int start_type = get_expression_type(node->right->left, current_scope);
            if (start_type != TYPE_INT) {
                log_error_format("String slice start index must be of integer type, got '%s'", 
                            get_type_name(start_type));
                return 0;
            }
        }
        
        // Check end index type (could be NULL for default)
        if (node->right->right && strcmp(node->right->right->token, "-1") != 0) {
            int end_type = get_expression_type(node->right->right, current_scope);
            if (end_type != TYPE_INT) {
                log_error_format("String slice end index must be of integer type, got '%s'", 
                            get_type_name(end_type));
                return 0;
            }
        }
    }
    // For slice with step (slice_step), we have a more complex structure:
    // node -> right -> (left, right) where left is another node with (left, right) for start and end
    else if (strcmp(node->token, "slice_step") == 0) {
        // Check if we have a valid start/end node
        if (node->right->left) {
            // Check start index
            if (node->right->left->left && strcmp(node->right->left->left->token, "0") != 0) {
                int start_type = get_expression_type(node->right->left->left, current_scope);
                if (start_type != TYPE_INT) {
                    log_error_format("String slice start index must be of integer type, got '%s'", 
                                get_type_name(start_type));
                    return 0;
                }
            }
            
            // Check end index
            if (node->right->left->right && strcmp(node->right->left->right->token, "-1") != 0) {
                int end_type = get_expression_type(node->right->left->right, current_scope);
                if (end_type != TYPE_INT) {
                    log_error_format("String slice end index must be of integer type, got '%s'", 
                                get_type_name(end_type));
                    return 0;
                }
            }
        }
        
        // Check step type
        if (node->right->right) {
            int step_type = get_expression_type(node->right->right, current_scope);
            if (step_type != TYPE_INT) {
                log_error_format("String slice step must be of integer type, got '%s'", 
                            get_type_name(step_type));
                return 0;
            }
        }
    }
    
    // If all checks pass, the result is a string
    log_info("String slice operation validated successfully");
    return TYPE_STRING;
}

// Handle return statement validation
void handle_return_statement(node* return_node, scope* curr_scope) {
    if (!current_function) {
        log_error("Return statement outside of function");
        return;
    }

    log_info_format("Found return statement in function '%s'", current_function->name);

    // Check if function has a declared return type
    int expected_return_type = current_function->return_type;
    
    // If no return value is provided (return;)
    if (!return_node || !return_node->left) {
        log_debug("Return with no value");
        
        if (expected_return_type != 0) {
            log_error_format("Function '%s' declared with return type '%s' but returns no value", 
                           current_function->name, get_type_name(expected_return_type));
        } else {
            log_debug("Empty return validated successfully (no return type declared)");
        }
        return;
    }
    
    // Get the type of the returned expression
    int actual_return_type = get_expression_type(return_node->left, curr_scope);
    
    log_debug_format("Validating return type: expected %s, got %s", 
                   get_type_name(expected_return_type), get_type_name(actual_return_type));
    
    // Check if function is declared without return type
    if (expected_return_type == 0) {
        log_error_format("Function '%s' has no declared return type but returns a value", 
                       current_function->name);
        return;
    }
    
    // Check if actual return type matches expected
    if (actual_return_type == 0) {
        log_info_format("Cannot determine type of return expression in function '%s'", 
                      current_function->name);
        return;
    }
    
    if (actual_return_type != expected_return_type) {
        log_error_format("Return type mismatch in function '%s'. Expected: %s, Got: %s", 
                       current_function->name, get_type_name(expected_return_type), 
                       get_type_name(actual_return_type));
        return;
    }
    
    log_info("Return statement validated successfully");
}

// Handle assignment with type checking
void handle_assignment(node* assign_node, scope* curr_scope) {
    if (!assign_node || !assign_node->left || !assign_node->right) {
        log_error("Invalid assignment node");
        return;
    }
    
    char* var_name = assign_node->left->token;
    
    // Check if variable is declared
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, var_name);
    if (!found_var) {
        log_error_format("Cannot assign to undeclared variable '%s'", var_name);
        return;
    }
    
    int expr_type = get_expression_type(assign_node->right, curr_scope);
    
    if (expr_type == 0) {
        log_info_format("Cannot determine type of expression for assignment to '%s'", var_name);
        return;
    }
    
    // Check if types match
    if (found_var->type != expr_type) {
        log_error_format("Type mismatch in assignment to '%s'. Expected: %s, Got: %s", 
                       var_name, get_type_name(found_var->type), get_type_name(expr_type));
        return;
    }
    
    log_debug_format("Assignment to '%s' type-checked successfully (%s)", 
                   var_name, get_type_name(found_var->type));
}

// Handle a parameter - type is the token, param name needs to be found
void handle_parameter(node* param_node, scope* func_scope) {
    if (!param_node || !param_node->token) {
        return;
    }

    char* param_name = NULL;
    
    if (param_node->left) {
        if (param_node->left->token && strlen(param_node->left->token) > 0) {
            if (get_type(param_node->left->token) == 0) {
                param_name = param_node->left->token;
            }
        } else if (!param_node->left->token || strlen(param_node->left->token) == 0) {
            if (param_node->left->left && param_node->left->left->token) {
                param_name = param_node->left->left->token;
            }
        }
    }
    
    if (!param_name) {
        log_error_format("Parameter of type '%s' has no identifiable name", param_node->token);
        return;
    }
    
    // Debug information about parameter processing
    log_debug_format("Processing parameter '%s' of type '%s'", param_name, param_node->token);
    
    int type = get_type(param_node->token);
    if (type == 0) {
        log_error_format("Unknown parameter type '%s'", param_node->token);
        return;
    }
    
    // Check if parameter already exists in current scope
    if (find_variable_in_scope(func_scope, param_name)) {
        log_error_format("Parameter '%s' already declared", param_name);
        return;
    }
    
    // Add parameter to scope
    add_variable(func_scope, param_name, type);
    log_info_format("Added parameter '%s' of type '%s'", param_name, get_type_name(type));
}

// Handle a declaration - add to symbol table with error checking
void handle_declaration(node* declare_node, scope* curr_scope) {
    if (!declare_node || !declare_node->left || !declare_node->right) {
        log_error("Invalid declaration node");
        return;
    }
    
    char* type_str = declare_node->left->token;
    char* var_name = declare_node->right->token;
    
    log_debug_format("Processing declaration: %s %s", type_str, var_name);
    
    int type = get_type(type_str);
    if (type == 0) {
        log_error_format("Unknown type '%s'", type_str);
        return;
    }
    
    if (find_variable_in_scope(curr_scope, var_name)) {
        log_error_format("Variable '%s' already declared in this scope", var_name);
        return;
    }
    
    add_variable(curr_scope, var_name, type);
    log_debug_format("Variable '%s' of type '%s' added to scope", var_name, get_type_name(type));
}

// Handle if-statement validation
void handle_if_statement(node* if_node, scope* curr_scope) {
    if (!if_node) return;

    node* condition = if_node->left;
    
    validate_condition_type(condition, curr_scope, "if-statement");
}

// Handle while-statement validation
void handle_while_statement(node* while_node, scope* curr_scope) {
    if (!while_node) return;
    
    node* condition = while_node->left;
    
    validate_condition_type(condition, curr_scope, "while-loop");
}

// Helper function to process parameters under a params node
void process_params(node* node, scope* func_scope) {
    if (!node) return;
    
    if (node->token && get_type(node->token) != 0) {
        handle_parameter(node, func_scope);
    }
    
    process_params(node->left, func_scope);
    process_params(node->right, func_scope);
}

// Function to handle operators correctly
int is_variable_usage(node* var_node, node* parent_node) {
    if (!var_node || !var_node->token) {
        return 0;
    }
    
    if (strlen(var_node->token) == 0) {
        return 0;
    }
    
    if (looks_like_string_literal(var_node->token)) {
        return 0;
    }
    
    if (get_type(var_node->token) != 0) {
        return 0;
    }
    
    // Skip keywords/operators
    if (strcmp(var_node->token, "assign") == 0 ||
        strcmp(var_node->token, "declare") == 0 ||
        strcmp(var_node->token, "init") == 0 ||
        strcmp(var_node->token, "function") == 0 ||
        strcmp(var_node->token, "params") == 0 ||
        strcmp(var_node->token, "return_type") == 0 ||
        strcmp(var_node->token, "call") == 0 ||
        strcmp(var_node->token, "if") == 0 ||
        strcmp(var_node->token, "if-else") == 0 ||  
        strcmp(var_node->token, "if-elif") == 0 ||  
        strcmp(var_node->token, "if-elif-else") == 0 || 
        strcmp(var_node->token, "elif") == 0 ||
        strcmp(var_node->token, "while") == 0 ||
        strcmp(var_node->token, "pass") == 0 ||
        strcmp(var_node->token, "index") == 0 ||
        strcmp(var_node->token, "slice") == 0 ||    
        strcmp(var_node->token, "slice_step") == 0 ||
        strcmp(var_node->token, "return") == 0) {
        return 0;
    }
    
    if (strcmp(var_node->token, "+") == 0 ||
        strcmp(var_node->token, "-") == 0 ||
        strcmp(var_node->token, "*") == 0 ||
        strcmp(var_node->token, "/") == 0 ||
        strcmp(var_node->token, "%") == 0 ||
        strcmp(var_node->token, "**") == 0 ||
        strcmp(var_node->token, "==") == 0 ||
        strcmp(var_node->token, "!=") == 0 ||
        strcmp(var_node->token, "<") == 0 ||
        strcmp(var_node->token, ">") == 0 ||
        strcmp(var_node->token, "<=") == 0 ||
        strcmp(var_node->token, ">=") == 0 ||
        strcmp(var_node->token, "and") == 0 ||
        strcmp(var_node->token, "or") == 0 ||
        strcmp(var_node->token, "not") == 0) {
        return 0;
    }
    
    if (var_node->token[0] >= '0' && var_node->token[0] <= '9') {
        return 0;
    }
    
    if (strcmp(var_node->token, "True") == 0 ||
        strcmp(var_node->token, "False") == 0 ||
        strcmp(var_node->token, "true") == 0 ||
        strcmp(var_node->token, "false") == 0) {
        return 0;
    }
    
    // Check all the skip conditions
    if (parent_node && parent_node->token) {
        if (strcmp(parent_node->token, "function") == 0) {
            return 0;
        }
        if (strcmp(parent_node->token, "declare") == 0) {
            return 0;
        }
        if (strcmp(parent_node->token, "assign") == 0 && parent_node->left == var_node) {
            return 0;
        }
        if (strcmp(parent_node->token, "call") == 0 && parent_node->left == var_node) {
            return 0;
        }
    }
    return 1;
}

// Check if a function has been declared
int is_function_declared(char* func_name) {
    return find_function_by_name(func_name) != NULL;
}

// Handle initialization of variables
void handle_initialization(node* init_node, scope* curr_scope) {
    if (!init_node || !init_node->left || !init_node->right) {
        log_error("Invalid initialization node");
        return;
    }
    
    log_debug_format("=== DEBUG: Entering handle_initialization ===");
    
    // First handle the declaration part
    handle_declaration(init_node->left, curr_scope);
    
    log_debug("Declaration handled, now checking initialization expression...");
    
    // Check if the right side is a variable usage
    if (is_variable_usage(init_node->right, init_node)) {
        log_debug("Right side is a variable usage, checking...");
        handle_variable_usage(init_node->right, curr_scope);
    } else {
        log_debug("Right side is not a variable usage");
    }
    
    // Then check type compatibility
    node* declare_node = init_node->left;
    if (!declare_node || !declare_node->left || !declare_node->right) {
        return;
    }
    
    char* var_name = declare_node->right->token;
    char* type_str = declare_node->left->token;
    int expected_type = get_type(type_str);
    
    log_debug_format("Getting type of initialization expression...");
    
    // Get the type of the initialization expression
    int expr_type = get_expression_type(init_node->right, curr_scope);
    
    log_debug_format("Expression type = %d (%s), Expected type = %d (%s)",
                   expr_type, get_type_name(expr_type), 
                   expected_type, get_type_name(expected_type));
    
    // If we can't determine the type (likely due to undeclared variable),
    // we've already reported the usage error, so skip type checking
    if (expr_type == 0) {
        log_debug("Type checking skipped due to undetermined expression type");
        return;
    }
    
    // Check if types match
    if (expected_type != expr_type) {
        log_error_format("Type mismatch in initialization of '%s'. Expected: %s, Got: %s", 
                       var_name, get_type_name(expected_type), get_type_name(expr_type));
        return;
    }
    
    log_info_format("Initialization of '%s' type-checked successfully (%s)", 
                  var_name, get_type_name(expected_type));
    
    log_debug_format("=== DEBUG: Exiting handle_initialization ===");
}

// Handle function call validation
void handle_function_call(node* call_node, scope* curr_scope) {
    if (!call_node || !call_node->left || !call_node->left->token) {
        log_error("Invalid function call node");
        return;
    }
    
    char* func_name = call_node->left->token;
    log_info_format("Found function call: %s", func_name);
    
    // Check if function has been declared
    function_info* func_info = find_function_by_name(func_name);
    if (!func_info) {
        log_error_format("Function '%s' called before declaration", func_name);
        return;
    }

    // Check if function was declared before current position
    if (func_info->declaration_position >= current_position) {
        log_error_format("Function '%s' called before declaration", func_name);
        return;
    }
    
    // Count the arguments passed to the function
    int args_passed = 0;
    if (call_node->right) {
        args_passed = count_function_arguments(call_node->right);
    }
    
    // Calculate minimum required arguments (parameters without defaults)
    int min_required_args = 0;
    int total_params = func_info->param_count;
    
    for (int i = 0; i < total_params; i++) {
        if (!func_info->has_default[i]) {
            min_required_args++;
        }
    }
    
    log_debug_format("Validating argument count: passed=%d, required=%d-%d", 
                   args_passed, min_required_args, total_params);
    
    if (args_passed < min_required_args) {
        log_error_format("Too few arguments for function '%s'. Expected at least %d, got %d", 
                        func_name, min_required_args, args_passed);
        return;
    }
    
    if (args_passed > total_params) {
        log_error_format("Too many arguments for function '%s'. Expected at most %d, got %d", 
                        func_name, total_params, args_passed);
        return;
    }
    
    log_info_format("Function call '%s' argument count validated successfully", func_name);
    
    // Validate argument types
    if (args_passed > 0 && call_node->right) {
        log_debug("Validating argument types...");
        
        // Extract argument nodes
        int arg_count;
        node** arg_nodes = extract_function_arguments(call_node->right, &arg_count);
        
        if (arg_nodes && arg_count == args_passed) {
            for (int i = 0; i < args_passed; i++) {
                // Get the type of the argument
                int arg_type = get_expression_type(arg_nodes[i], curr_scope);
                int expected_type = func_info->param_types[i];
                
                log_debug_format("Arg %d: expected %s, got %s", 
                               i + 1, get_type_name(expected_type), get_type_name(arg_type));
                
                if (arg_type == 0) {
                    log_info_format("Cannot determine type of argument %d for function '%s'", 
                                  i + 1, func_name);
                } else if (arg_type != expected_type) {
                    log_error_format("Type mismatch for argument %d in function '%s'. Expected: %s, Got: %s", 
                                   i + 1, func_name, get_type_name(expected_type), get_type_name(arg_type));
                }
            }
        }
        
        // Free the allocated array
        if (arg_nodes) {
            free(arg_nodes);
        }
        
        log_debug("Argument type validation completed");
    }
}

// Helper function to extract return type from function AST
int extract_return_type(node* func_node) {
    if (!func_node || !func_node->right) return 0;
    
    // Navigate the AST structure to find return_type node
    node* current = func_node->right;
    
    // Using a simple BFS approach to traverse the AST and find the return_type
    node* nodes_to_check[50] = {current};
    int front = 0, rear = 1;
    
    while (front < rear && rear < 50) {
        node* check_node = nodes_to_check[front++];
        if (!check_node) continue;
        
        if (check_node->token && strcmp(check_node->token, "return_type") == 0) {
            // Found return_type node, extract the actual type
            if (check_node->left && check_node->left->token) {
                return get_type(check_node->left->token);
            }
        }
        
        if (check_node->left && rear < 50) {
            nodes_to_check[rear++] = check_node->left;
        }
        if (check_node->right && rear < 50) {
            nodes_to_check[rear++] = check_node->right;
        }
    }
    
    return 0; // No return type found
}

// Helper function to process parameters for a function
void process_params_for_function(node* param_node, function_info* func_info, scope* func_scope) {
    if (!param_node || !func_info) return;
    
    // Check if this is a parameter (type followed by identifier)
    if (param_node->token && get_type(param_node->token) != 0) {
        // This is a parameter type node
        char* param_name = NULL;
        int has_default_value = 0;
        node* default_value_node = NULL;
        
        // Extract parameter name and default value
        if (param_node->left) {
            if (param_node->left->token && strlen(param_node->left->token) > 0) {
                if (get_type(param_node->left->token) == 0) {
                    param_name = param_node->left->token;
                    // Check if there's a default value (right side of the param name node)
                    if (param_node->left->left) {
                        has_default_value = 1;
                        default_value_node = param_node->left->left;
                    }
                }
            } else if (!param_node->left->token || strlen(param_node->left->token) == 0) {
                if (param_node->left->left && param_node->left->left->token) {
                    param_name = param_node->left->left->token;
                    if (param_node->left->right) {
                        has_default_value = 1;
                        default_value_node = param_node->left->right;
                    }
                }
            }
        }
        
        if (param_name) {
            int param_type = get_type(param_node->token);
            
            log_info_format("Found parameter: %s %s", param_node->token, param_name);
            add_parameter_to_function(func_info, param_name, param_type, has_default_value);
            
            // Add to the function scope (not a different scope)
            add_variable(func_scope, param_name, param_type);

            // Check default value type if it exists
            if (has_default_value && default_value_node) {
                log_debug_format("Checking default value for parameter '%s'...", param_name);

                // Get the type of the default value
                int default_type = get_expression_type(default_value_node, func_scope);
                
                if (default_type == 0) {
                    log_info_format("Cannot determine type of default value for '%s'", param_name);
                } else if (default_type != param_type) {
                    log_error_format("Default value type mismatch for parameter '%s'. Parameter type: %s, Default value type: %s", 
                                   param_name, get_type_name(param_type), get_type_name(default_type));
                } else {
                    log_debug_format("Default value type OK: %s", get_type_name(default_type));
                }
            }
        }
    }
    
    process_params_for_function(param_node->left, func_info, func_scope);
    process_params_for_function(param_node->right, func_info, func_scope);
}

// Function to analyze the AST and perform semantic checks
void analyze_node(node* root, node* parent, scope* curr_scope) {
    if (!root) return;

    log_debug_format("analyze_node called with token='%s'", 
                    root->token ? root->token : "NULL");

    // Increment position counter
    current_position++;
   
    // Check if this is a function declaration
    if (root->token && strcmp(root->token, "function") == 0) {
        // Create a scope for this function
        scope* func_scope = mkscope(curr_scope);
        
        if (root->left && root->left->token) {
            func_scope->scope_name = strdup(root->left->token);
            log_info_format("Entering function scope: %s", func_scope->scope_name);
            
            // Extract return type from AST
            int return_type = extract_return_type(root);
            
            // Create and track function declaration with return type
            function_info* func_info = add_function_declaration(root->left->token, return_type);
            
            if (func_info) {
                // SET CURRENT FUNCTION CONTEXT for return validation
                function_info* previous_function = current_function;
                current_function = func_info;
                
                validate_main_function(root, func_scope);
                
                // Find and process params node
                node* current = root->right;
                node* params_node = NULL;
                
                // Find the params node
                node* nodes_to_check[50] = {current};
                int front = 0, rear = 1;
                
                while (front < rear && rear < 50) {
                    node* check_node = nodes_to_check[front++];
                    if (!check_node) continue;
                    
                    if (check_node->token && strcmp(check_node->token, "params") == 0) {
                        params_node = check_node;
                        break;
                    }
                    
                    if (check_node->left && rear < 50) {
                        nodes_to_check[rear++] = check_node->left;
                    }
                    if (check_node->right && rear < 50) {
                        nodes_to_check[rear++] = check_node->right;
                    }
                }
                
                // Process params if found
                if (params_node) {
                    log_info("Processing parameters...");
                    process_params_for_function(params_node, func_info, func_scope);
                }
                
                // Now process the function body with the same scope
                analyze_node(root->right, root, func_scope);
                
                // RESTORE PREVIOUS FUNCTION CONTEXT
                current_function = previous_function;
            }
        }
        return;
    }

    // Handle if-elif-else separately from if-elif
    if (root->token && strcmp(root->token, "if-elif-else") == 0) {
        log_info("Processing if-elif-else statement");
        
        // For if-elif-else:
        // root->left = if-elif structure (no direct condition to validate)
        // root->right = final else body
        
        // Process the if-elif part (left child)
        if (root->left) {
            analyze_node(root->left, root, curr_scope);
        }
        
        // Process the else part (right child) with new scope
        if (root->right) {
            scope* else_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-else-block", curr_scope->scope_name);
                else_scope->scope_name = strdup(name_buffer);
            } else {
                else_scope->scope_name = strdup("else-block");
            }
            
            analyze_node(root->right, root, else_scope);
        }
        
        return; // Skip normal traversal
    }

    // Handle regular if-elif (keep existing logic)
    if (root->token && strcmp(root->token, "if-elif") == 0) {
        // Process the condition part (left child)
        if (root->left) {
            // Validate the condition
            validate_condition_type(root->left, curr_scope, "if-elif");
            
            // Process left child (should be an if condition)
            analyze_node(root->left, root, curr_scope);
        }
        
        // Process the body/branches (right child)
        if (root->right) {
            // Create a new scope for blocks
            scope* block_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-if-elif-block", curr_scope->scope_name);
                block_scope->scope_name = strdup(name_buffer);
            } else {
                block_scope->scope_name = strdup("if-elif-block");
            }
            
            analyze_node(root->right, root, block_scope);
        }
        
        return; // Skip normal child traversal since we've handled it specifically
    }

    // Special handling for elif node
    if (root->token && strcmp(root->token, "elif") == 0) {
        // Validate the condition
        validate_condition_type(root->left, curr_scope, "elif");
        
        // Process the condition (left child)
        if (root->left) {
            analyze_node(root->left, root, curr_scope);
        }
        
        // Process the body (right child) with a new scope
        if (root->right) {
            scope* elif_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-elif-block", curr_scope->scope_name);
                elif_scope->scope_name = strdup(name_buffer);
            } else {
                elif_scope->scope_name = strdup("elif-block");
            }
            
            analyze_node(root->right, root, elif_scope);
        }
        
        return; // Skip normal traversal
    }
    
    // Special handling for if-else
    if (root->token && strcmp(root->token, "if-else") == 0) {
        // Process the if part (left child)
        if (root->left) {
            analyze_node(root->left, root, curr_scope);
        }
        
        // Process the else part (right child) with a new scope
        if (root->right) {
            scope* else_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-else-block", curr_scope->scope_name);
                else_scope->scope_name = strdup(name_buffer);
            } else {
                else_scope->scope_name = strdup("else-block");
            }
            
            analyze_node(root->right, root, else_scope);
        }
        
        return; // Skip normal traversal
    }

    if (root->token && strcmp(root->token, "index") == 0) {
        log_info("Found string indexing operation");
        
        // Handle the string expression (left child)
        if (root->left) {
            if (is_variable_usage(root->left, root)) {
                handle_variable_usage(root->left, curr_scope);
            } else {
                analyze_node(root->left, root, curr_scope);
            }
        }
        
        // Handle the index expression (right child)
        if (root->right) {
            if (is_variable_usage(root->right, root)) {
                handle_variable_usage(root->right, curr_scope);
            } else {
                analyze_node(root->right, root, curr_scope);
            }
        }
        
        // We've handled the children directly, so return to avoid processing them again
        return;
    }
    
    // Check if this is an if statement
    if (root->token && strcmp(root->token, "if") == 0) {
        // Validate the condition first
        handle_if_statement(root, curr_scope);
        
        // Now handle the body with a new scope
        if (root->right) {
            // Create a new scope for the if-block
            scope* block_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-if-block", curr_scope->scope_name);
                block_scope->scope_name = strdup(name_buffer);
            } else {
                block_scope->scope_name = strdup("if-block");
            }
            
            // Process the if body with the new scope
            analyze_node(root->right, root, block_scope);
            
            // Skip the normal child traversal since we've already handled the body
            return;
        }
    }

    // Check if this is a while statement
    if (root->token && strcmp(root->token, "while") == 0) {
        // Validate the condition first
        handle_while_statement(root, curr_scope);
        
        if (root->right) {
            // Create a new scope for the while-block
            scope* block_scope = mkscope(curr_scope);
            if (curr_scope->scope_name) {
                char name_buffer[256];
                sprintf(name_buffer, "%s-while-block", curr_scope->scope_name);
                block_scope->scope_name = strdup(name_buffer);
            } else {
                block_scope->scope_name = strdup("while-block");
            }
            
            // Process the while body with the new scope
            analyze_node(root->right, root, block_scope);
            
            // Skip the normal child traversal
            return;
        }
    }

    if (root->token && (strcmp(root->token, "if-else") == 0 || 
                        strcmp(root->token, "if-elif") == 0 || 
                        strcmp(root->token, "if-elif-else") == 0)) {
        node* if_part = root->left;  
        if (if_part && strcmp(if_part->token, "if") == 0) {
            handle_if_statement(if_part, curr_scope);
        }
    }       

    // Check if this is a code block 
    if (root->token && strcmp(root->token, "") == 0 && root->left && root->right) {
        // This might be a code block
        scope* block_scope = mkscope(curr_scope);
        block_scope->scope_name = strdup("block");
        
        analyze_node(root->left, root, block_scope);
        analyze_node(root->right, root, block_scope);
        return;
    }

    // Check if this is a return statement
    if (root->token && strcmp(root->token, "return") == 0) {
        handle_return_statement(root, curr_scope);
    }
   
    // Check if this is a params node
    if (root->token && strcmp(root->token, "params") == 0) {
        process_params(root, curr_scope);
        return;
    }

    // Check if this is an initialization node
    if (root->token && strcmp(root->token, "init") == 0) {
        if (root->left && root->left->right && root->left->right->token) {
            log_info_format("Found initialization: %s", root->left->right->token);
        }
        handle_initialization(root, curr_scope);
        return;
    }

    // Check if this is a declaration node (standalone, not part of init)
    if (root->token && strcmp(root->token, "declare") == 0) {
        if (root->left && root->right && root->left->token && root->right->token) {
            log_info_format("Found declaration: %s %s", root->left->token, root->right->token);
            handle_declaration(root, curr_scope);
        }
    }
    
    // Check if this is an assignment node
    if (root->token && strcmp(root->token, "assign") == 0) {
        if (root->left && root->left->token) {
            log_info_format("Found assignment: %s", root->left->token);
            handle_assignment(root, curr_scope);
        }
    }
    
    // Check if this is a function call
    if (root->token && strcmp(root->token, "call") == 0) {
        handle_function_call(root, curr_scope);
    }
    
    // Check if this is a variable usage
    if (is_variable_usage(root, parent)) {
        handle_variable_usage(root, curr_scope);
    }
    
    analyze_node(root->left, root, curr_scope);
    analyze_node(root->right, root, curr_scope);
}

// Main function for semantic analysis
void semantic_analysis(struct node* root, scope* curr_scope) {
    // Cast the struct node* to our node* type
    node* ast_root = (node*)root;

    log_info("=== Starting semantic analysis ===");

    // Initialize function tracking list
    declared_functions = NULL;
    
    // Initialize counters for declaration order tracking
    declaration_counter = 0;
    current_position = 0;
    
    // Analyze the entire AST
    analyze_node(ast_root, NULL, curr_scope);

    if (semantic_errors == 0) {
        printf("=== Semantic analysis completed successfully ===\n\n");
    } else {
        log_error_format("=== Semantic analysis failed with %d error(s) ===", semantic_errors);
    }
}