#include "semantic_analysis.h"

// We need to define the node structure to access AST
typedef struct node {
    char *token;
    struct node *left;
    struct node *right;
} node;

// Define our variable structure
typedef struct var {
    char* name;
    int type;  // We'll use integers for types: 1=int, 2=string, 3=bool, 4=float
    struct var* next;
} var;

// Define the scope structure
typedef struct scope {
    var* variables;
    struct scope* parent;
    char* scope_name;  // For debugging
} scope;

typedef struct function_info {
    char* name;
    int param_count;
    int* param_types;        // Array of parameter types
    char** param_names;      // Array of parameter names
    int* has_default;        // Array indicating which params have defaults
    int return_type;         // 0 for no return type, TYPE_INT/STRING/etc for others
    struct function_info* next;
} function_info;

// Update the global variable declaration
function_info* declared_functions = NULL;
// Global variable to track current function context for return type validation
function_info* current_function = NULL;

// Helper function to create a new function_info
function_info* create_function_info(char* name, int return_type) {
    function_info* new_func = (function_info*)malloc(sizeof(function_info));
    new_func->name = strdup(name);
    new_func->param_count = 0;
    new_func->param_types = NULL;
    new_func->param_names = NULL;
    new_func->has_default = NULL;
    new_func->return_type = return_type;
    new_func->next = declared_functions;
    return new_func;
}

// Updated function to find a function by name
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

// Global variable to track semantic errors
int semantic_errors = 0;

// Type constants
#define TYPE_INT 1
#define TYPE_STRING 2
#define TYPE_BOOL 3
#define TYPE_FLOAT 4

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
    
    printf("  Added variable '%s' of type '%s' to scope %s\n", 
           name, get_type_name(type), 
           curr_scope->scope_name ? curr_scope->scope_name : "global");
}

// Updated function to add a function declaration with return type
function_info* add_function_declaration(char* func_name, int return_type) {
    // Check if function already exists
    function_info* current = declared_functions;
    while (current) {
        if (strcmp(current->name, func_name) == 0) {
            printf("  Semantic Error: Function '%s' already declared\n", func_name);
            semantic_errors++;
            return NULL;
        }
        current = current->next;
    }
    
    // Create new function info
    function_info* new_func = create_function_info(func_name, return_type);
    declared_functions = new_func;
    
    printf("  Function '%s' declared successfully", func_name);
    if (return_type != 0) {
        printf(" (return type: %s)", get_type_name(return_type));
    }
    printf("\n");
    
    return new_func;
}

// Helper function to add a parameter to a function_info
void add_parameter_to_function(function_info* func, char* param_name, int param_type, int has_default_value) {
    func->param_count++;
    
    // Reallocate arrays to accommodate new parameter
    func->param_types = (int*)realloc(func->param_types, func->param_count * sizeof(int));
    func->param_names = (char**)realloc(func->param_names, func->param_count * sizeof(char*));
    func->has_default = (int*)realloc(func->has_default, func->param_count * sizeof(int));
    
    // Add the new parameter (at the end)
    int index = func->param_count - 1;
    func->param_types[index] = param_type;
    func->param_names[index] = strdup(param_name);
    func->has_default[index] = has_default_value;
    
    printf("  Added parameter '%s' (type: %s, has_default: %s) to function '%s'\n",
           param_name, get_type_name(param_type), 
           has_default_value ? "yes" : "no", func->name);
}

// Count the number of arguments in a function call
int count_function_arguments(node* args_node) {
    if (!args_node) return 0;
    
    int count = 0;
    node* current = args_node;
    
    // The arguments in the AST are structured as a tree
    // We need to count all non-empty tokens that represent actual arguments
    while (current) {
        // If this node has a token that's not empty, it's an argument
        if (current->token && strlen(current->token) > 0) {
            count++;
            // For single argument, we're done
            if (!current->left && !current->right) {
                break;
            }
        }
        
        // Check left child for arguments
        if (current->left) {
            count += count_function_arguments(current->left);
        }
        
        // Move to right child (next argument in chain)
        current = current->right;
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
    
    // First, count the arguments to allocate the array
    *arg_count = count_function_arguments(args_node);
    
    if (*arg_count == 0) {
        return NULL;
    }
    
    // Allocate array to hold argument nodes
    node** arg_nodes = (node**)malloc(*arg_count * sizeof(node*));
    int index = 0;
    
    // Extract argument nodes
    node* current = args_node;
    
    // Handle single argument case
    if (current->token && strlen(current->token) > 0) {
        arg_nodes[index] = current;
        return arg_nodes;
    }
    
    // Handle multiple arguments connected by empty nodes
    while (current && index < *arg_count) {
        if (current->token && strlen(current->token) > 0) {
            // This is an argument
            arg_nodes[index++] = current;
            break; // Single argument
        } else {
            // This is a connector node
            if (current->left && current->left->token && strlen(current->left->token) > 0) {
                arg_nodes[index++] = current->left;
            }
            current = current->right;
        }
    }
    
    return arg_nodes;
}

// Fully inlined version without separate helper function
void validate_main_function(node* func_node, scope* func_scope) {
    if (!func_node || !func_node->left || !func_node->left->token) {
        return;
    }
    
    // Check if this is the __main__ function
    if (strcmp(func_node->left->token, "__main__") != 0) {
        return;
    }
    
    printf("Validating __main__ function requirements...\n");
    
    node* func_body = func_node->right;
    if (!func_body) return;
    
    int has_params = 0;
    int has_return_type = 0;
    
    // Use a stack-based approach or simple iterative search
    // We'll check all visible nodes in the immediate structure
    
    // Check direct children and their children
    node* nodes_to_check[25] = {func_body, NULL}; // Simple array for BFS
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
        printf("  Semantic Error: __main__ function cannot have parameters\n");
        semantic_errors++;
    } else {
        printf("  __main__ parameters: ✓ (none)\n");
    }
    
    if (has_return_type) {
        printf("  Semantic Error: __main__ function cannot have a return type\n");
        semantic_errors++;
    } else {
        printf("  __main__ return type: ✓ (none)\n");
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

// Check if variable exists in current scope OR any parent scope (for usage check)
var* find_variable_in_scope_hierarchy(scope* curr_scope, char* name) {
    scope* temp_scope = curr_scope;
    
    while (temp_scope != NULL) {
        var* temp_var = temp_scope->variables;
        while (temp_var != NULL) {
            if (strcmp(temp_var->name, name) == 0) {
                return temp_var;
            }
            temp_var = temp_var->next;
        }
        temp_scope = temp_scope->parent;
    }
    return NULL;
}

// Handle variable usage - check if variable is declared
void handle_variable_usage(node* var_node, scope* curr_scope) {
    if (!var_node || !var_node->token) {
        return;
    }
    
    char* var_name = var_node->token;
    
    // Check if variable exists in scope hierarchy
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, var_name);
    
    if (!found_var) {
        printf("  Semantic Error: Variable '%s' used before declaration\n", var_name);
        semantic_errors++;
        return;
    }
    
    // Variable found - could add type information here later
    printf("  Variable '%s' used (type: %s)\n", var_name, get_type_name(found_var->type));
}

// Enhanced function to detect string literals
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
// Updated get_expression_type function with better operator handling
int get_expression_type(node* expr_node, scope* curr_scope) {
    if (!expr_node || !expr_node->token) {
        return 0; // Unknown type
    }
    
    printf("DEBUG get_expression_type: analyzing token='%s'\n", expr_node->token);
    
    // Handle comparison operations - these result in boolean
    if (strcmp(expr_node->token, "==") == 0 || strcmp(expr_node->token, "!=") == 0 ||
        strcmp(expr_node->token, "<") == 0 || strcmp(expr_node->token, ">") == 0 ||
        strcmp(expr_node->token, "<=") == 0 || strcmp(expr_node->token, ">=") == 0) {
        printf("DEBUG: Detected comparison operator\n");
        return TYPE_BOOL;
    }
    
    // Handle logical operations - these result in boolean
    if (strcmp(expr_node->token, "and") == 0 || strcmp(expr_node->token, "or") == 0 ||
        strcmp(expr_node->token, "not") == 0) {
        printf("DEBUG: Detected logical operator\n");
        return TYPE_BOOL;
    }
    
    // Handle arithmetic operations
    if (strcmp(expr_node->token, "+") == 0 || strcmp(expr_node->token, "-") == 0 ||
        strcmp(expr_node->token, "*") == 0 || strcmp(expr_node->token, "/") == 0) {
        printf("DEBUG: Detected arithmetic operator\n");
        // Get types of operands
        int left_type = get_expression_type(expr_node->left, curr_scope);
        int right_type = get_expression_type(expr_node->right, curr_scope);
        
        // If either operand is float, result is float
        if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
            return TYPE_FLOAT;
        }
        // If both are int, result is int
        if (left_type == TYPE_INT && right_type == TYPE_INT) {
            return TYPE_INT;
        }
        return TYPE_INT; // Default for arithmetic
    }
    
    // Check if it's a variable - look up its declared type
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, expr_node->token);
    if (found_var) {
        printf("DEBUG: Found variable '%s' with type %d\n", expr_node->token, found_var->type);
        return found_var->type;
    }
    
    // Check if it's a number literal
    if (expr_node->token[0] >= '0' && expr_node->token[0] <= '9') {
        if (strchr(expr_node->token, '.') != NULL) {
            printf("DEBUG: Detected float literal\n");
            return TYPE_FLOAT;
        } else {
            printf("DEBUG: Detected int literal\n");
            return TYPE_INT;
        }
    }
    
    // Check if it's a boolean literal
    if (strcmp(expr_node->token, "True") == 0 || strcmp(expr_node->token, "False") == 0 ||
        strcmp(expr_node->token, "true") == 0 || strcmp(expr_node->token, "false") == 0) {
        printf("DEBUG: Detected boolean literal\n");
        return TYPE_BOOL;
    }
    
    // Check if it looks like a string literal
    if (looks_like_string_literal(expr_node->token)) {
        printf("DEBUG: Detected string literal by pattern\n");
        return TYPE_STRING;
    }
    
    // Function calls - for now return unknown type
    if (strcmp(expr_node->token, "call") == 0) {
        // We'd need function signature tracking to determine return type
        return 0; // Unknown
    }
    
    // If we reach here, it's likely an undeclared variable or unknown operator
    printf("DEBUG: Undeclared identifier - returning unknown type\n");
    return 0; // Return 0 to indicate we can't determine the type
}

// Helper function to validate that an expression is of boolean type
void validate_condition_type(node* condition_node, scope* curr_scope, const char* context) {
    if (!condition_node) {
        printf("  Semantic Error: Missing condition in %s\n", context);
        semantic_errors++;
        return;
    }
    
    // Get the type of the condition expression
    int condition_type = get_expression_type(condition_node, curr_scope);
    
    printf("  Validating %s condition type: got %s\n", context, get_type_name(condition_type));
    
    if (condition_type == 0) {
        printf("  Warning: Cannot determine type of condition in %s\n", context);
        return;
    }
    
    if (condition_type != TYPE_BOOL) {
        printf("  Semantic Error: %s condition must be boolean type\n", context);
        printf("    Expected: bool, Got: %s\n", get_type_name(condition_type));
        semantic_errors++;
        return;
    }
    
    printf("  %s condition type validated successfully (bool)\n", context);
}

// Handle return statement validation
void handle_return_statement(node* return_node, scope* curr_scope) {
    if (!current_function) {
        printf("  Semantic Error: Return statement outside of function\n");
        semantic_errors++;
        return;
    }
    
    printf("Found return statement in function '%s'\n", current_function->name);
    
    // Check if function has a declared return type
    int expected_return_type = current_function->return_type;
    
    // If no return value is provided (return;)
    if (!return_node || !return_node->left) {
        printf("  Return with no value\n");
        
        if (expected_return_type != 0) {
            printf("  Semantic Error: Function '%s' declared with return type '%s' but returns no value\n", 
                   current_function->name, get_type_name(expected_return_type));
            semantic_errors++;
        } else {
            printf("  Empty return validated successfully (no return type declared)\n");
        }
        return;
    }
    
    // Get the type of the returned expression
    int actual_return_type = get_expression_type(return_node->left, curr_scope);
    
    printf("  Validating return type: expected %s, got %s\n", 
           get_type_name(expected_return_type),
           get_type_name(actual_return_type));
    
    // Check if function is declared without return type
    if (expected_return_type == 0) {
        printf("  Semantic Error: Function '%s' has no declared return type but returns a value\n", 
               current_function->name);
        semantic_errors++;
        return;
    }
    
    // Check if actual return type matches expected
    if (actual_return_type == 0) {
        printf("  Warning: Cannot determine type of return expression in function '%s'\n", 
               current_function->name);
        return;
    }
    
    if (actual_return_type != expected_return_type) {
        printf("  Semantic Error: Return type mismatch in function '%s'\n", current_function->name);
        printf("    Expected: %s, Got: %s\n", 
               get_type_name(expected_return_type), 
               get_type_name(actual_return_type));
        semantic_errors++;
        return;
    }
    
    printf("  Return statement validated successfully\n");
}

// Handle assignment with type checking
void handle_assignment(node* assign_node, scope* curr_scope) {
    if (!assign_node || !assign_node->left || !assign_node->right) {
        printf("  Semantic Error: Invalid assignment node\n");
        semantic_errors++;
        return;
    }
    
    char* var_name = assign_node->left->token;
    
    // Check if variable is declared
    var* found_var = find_variable_in_scope_hierarchy(curr_scope, var_name);
    if (!found_var) {
        printf("  Semantic Error: Cannot assign to undeclared variable '%s'\n", var_name);
        semantic_errors++;
        return;
    }
    
    // Get the type of the expression being assigned
    int expr_type = get_expression_type(assign_node->right, curr_scope);
    
    if (expr_type == 0) {
        printf("  Warning: Cannot determine type of expression for assignment to '%s'\n", var_name);
        return;
    }
    
    // Check if types match
    if (found_var->type != expr_type) {
        printf("  Semantic Error: Type mismatch in assignment to '%s'\n", var_name);
        printf("    Expected: %s, Got: %s\n", 
               get_type_name(found_var->type), 
               get_type_name(expr_type));
        semantic_errors++;
        return;
    }
    
    printf("  Assignment to '%s' type-checked successfully (%s)\n", 
           var_name, get_type_name(found_var->type));
}

// Handle a parameter - type is the token, param name needs to be found
void handle_parameter(node* param_node, scope* func_scope) {
    if (!param_node || !param_node->token) {
        return;
    }
    
    // Get parameter name - need to handle different structures
    char* param_name = NULL;
    
    if (param_node->left) {
        if (param_node->left->token && strlen(param_node->left->token) > 0) {
            // Direct parameter name
            if (get_type(param_node->left->token) == 0) {
                param_name = param_node->left->token;
            }
        } else if (!param_node->left->token || strlen(param_node->left->token) == 0) {
            // Empty node - parameter name might be deeper
            if (param_node->left->left && param_node->left->left->token) {
                param_name = param_node->left->left->token;
            }
        }
    }
    
    if (!param_name) {
        printf("  Warning: Parameter of type '%s' has no identifiable name\n", param_node->token);
        return;
    }
    
    printf("Found parameter: %s %s\n", param_node->token, param_name);
    
    // Get type integer from type string  
    int type = get_type(param_node->token);
    if (type == 0) {
        printf("  Semantic Error: Unknown parameter type '%s'\n", param_node->token);
        semantic_errors++;
        return;
    }
    
    // Check if parameter already exists in current scope
    if (find_variable_in_scope(func_scope, param_name)) {
        printf("  Semantic Error: Parameter '%s' already declared\n", param_name);
        semantic_errors++;
        return;
    }
    
    // Add parameter to scope
    add_variable(func_scope, param_name, type);
}

// Handle a declaration - add to symbol table with error checking
void handle_declaration(node* declare_node, scope* curr_scope) {
    if (!declare_node || !declare_node->left || !declare_node->right) {
        printf("  Semantic Error: Invalid declaration node\n");
        semantic_errors++;
        return;
    }
    
    char* type_str = declare_node->left->token;
    char* var_name = declare_node->right->token;
    
    // Get type integer from type string
    int type = get_type(type_str);
    if (type == 0) {
        printf("  Semantic Error: Unknown type '%s'\n", type_str);
        semantic_errors++;
        return;
    }
    
    // Check if variable already exists in current scope
    if (find_variable_in_scope(curr_scope, var_name)) {
        printf("  Semantic Error: Variable '%s' already declared in this scope\n", var_name);
        semantic_errors++;
        return;
    }
    
    // Add variable to scope
    add_variable(curr_scope, var_name, type);
}

// Handle if-statement validation
void handle_if_statement(node* if_node, scope* curr_scope) {
    if (!if_node) return;
    
    printf("Found if statement\n");
    
    // The structure of if-node varies, but typically:
    // if_node->left is the condition
    // if_node->right is the body (or next part of if-elif-else chain)
    
    // Find the condition node
    node* condition = if_node->left;
    
    // Validate the condition type
    validate_condition_type(condition, curr_scope, "if-statement");
}

// Handle while-statement validation
void handle_while_statement(node* while_node, scope* curr_scope) {
    if (!while_node) return;
    
    printf("Found while statement\n");
    
    // The structure of while-node:
    // while_node->left is the condition
    // while_node->right is the body
    
    // Find the condition node
    node* condition = while_node->left;
    
    // Validate the condition type
    validate_condition_type(condition, curr_scope, "while-loop");
}

// Helper function to process parameters under a params node
void process_params(node* node, scope* func_scope) {
    if (!node) return;
    
    // Check if this is a parameter (type followed by identifier)
    if (node->token && get_type(node->token) != 0) {
        handle_parameter(node, func_scope);
    }
    
    // Recursively process children
    process_params(node->left, func_scope);
    process_params(node->right, func_scope);
}

// Debug function
void debug_print_node(node* n, char* context) {
    if (!n) {
        printf("DEBUG %s: node is NULL\n", context);
        return;
    }
    printf("DEBUG %s: token='%s', left=%p, right=%p\n", 
           context, n->token ? n->token : "NULL", n->left, n->right);
    if (n->left) {
        printf("  left token: '%s'\n", n->left->token ? n->left->token : "NULL");
    }
    if (n->right) {
        printf("  right token: '%s'\n", n->right->token ? n->right->token : "NULL");
    }
}

// Updated is_variable_usage function to handle operators correctly
int is_variable_usage(node* var_node, node* parent_node) {
    printf("DEBUG: Checking is_variable_usage for token='%s', parent='%s'\n",
           var_node ? (var_node->token ? var_node->token : "NULL") : "NULL",
           parent_node ? (parent_node->token ? parent_node->token : "NULL") : "NULL");
    
    if (!var_node || !var_node->token) {
        printf("DEBUG: Rejected - NULL node or token\n");
        return 0;
    }
    
    // Skip empty tokens
    if (strlen(var_node->token) == 0) {
        printf("DEBUG: Rejected - empty token\n");
        return 0;
    }
    
    // Skip if this looks like a string literal
    if (looks_like_string_literal(var_node->token)) {
        printf("DEBUG: Rejected - looks like string literal\n");
        return 0;
    }
    
    // Skip if this is a type name
    if (get_type(var_node->token) != 0) {
        printf("DEBUG: Rejected - is a type name\n");
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
        strcmp(var_node->token, "while") == 0 ||
        strcmp(var_node->token, "return") == 0) {
        printf("DEBUG: Rejected - is a keyword/operator\n");
        return 0;
    }
    
    // NEW: Skip arithmetic and comparison operators
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
        printf("DEBUG: Rejected - is an operator\n");
        return 0;
    }
    
    // Skip numbers
    if (var_node->token[0] >= '0' && var_node->token[0] <= '9') {
        printf("DEBUG: Rejected - is a number\n");
        return 0;
    }
    
    // Skip boolean literals
    if (strcmp(var_node->token, "True") == 0 ||
        strcmp(var_node->token, "False") == 0 ||
        strcmp(var_node->token, "true") == 0 ||
        strcmp(var_node->token, "false") == 0) {
        printf("DEBUG: Rejected - boolean literal\n");
        return 0;
    }
    
    // Check all the skip conditions
    if (parent_node && parent_node->token) {
        if (strcmp(parent_node->token, "function") == 0) {
            printf("DEBUG: Rejected - function name\n");
            return 0;
        }
        if (strcmp(parent_node->token, "declare") == 0) {
            printf("DEBUG: Rejected - part of declaration\n");
            return 0;
        }
        if (strcmp(parent_node->token, "assign") == 0 && parent_node->left == var_node) {
            printf("DEBUG: Rejected - left side of assignment\n");
            return 0;
        }
        if (strcmp(parent_node->token, "call") == 0 && parent_node->left == var_node) {
            printf("DEBUG: Rejected - function name in call\n");
            return 0;
        }
    }
    
    printf("DEBUG: ACCEPTED as variable usage!\n");
    return 1;
}

// Updated is_function_declared to use new structure
int is_function_declared(char* func_name) {
    return find_function_by_name(func_name) != NULL;
}

// Fixed handle_initialization - no recursive analyze_node call
void handle_initialization(node* init_node, scope* curr_scope) {
    printf("=== DEBUG: Entering handle_initialization ===\n");
    debug_print_node(init_node, "init_node");
    
    if (!init_node || !init_node->left || !init_node->right) {
        printf("  Semantic Error: Invalid initialization node\n");
        semantic_errors++;
        return;
    }
    
    printf("DEBUG: About to handle declaration...\n");
    // First handle the declaration part
    handle_declaration(init_node->left, curr_scope);
    
    printf("DEBUG: Declaration handled, now checking initialization expression...\n");
    debug_print_node(init_node->right, "init_expression");
    
    // IMPORTANT: Check if the right side is a variable usage
    // Don't call analyze_node recursively - just check this specific node
    if (is_variable_usage(init_node->right, init_node)) {
        printf("DEBUG: Right side is a variable usage, checking...\n");
        handle_variable_usage(init_node->right, curr_scope);
    } else {
        printf("DEBUG: Right side is not a variable usage\n");
    }
    
    // Then check type compatibility
    node* declare_node = init_node->left;
    if (!declare_node || !declare_node->left || !declare_node->right) {
        return;
    }
    
    char* var_name = declare_node->right->token;
    char* type_str = declare_node->left->token;
    int expected_type = get_type(type_str);
    
    printf("DEBUG: Getting type of expression '%s'...\n", 
           init_node->right->token ? init_node->right->token : "NULL");
    
    // Get the type of the initialization expression
    int expr_type = get_expression_type(init_node->right, curr_scope);
    
    printf("DEBUG: Expression type = %d (%s), Expected type = %d (%s)\n",
           expr_type, get_type_name(expr_type), 
           expected_type, get_type_name(expected_type));
    
    // If we can't determine the type (likely due to undeclared variable),
    // we've already reported the usage error, so skip type checking
    if (expr_type == 0) {
        printf("  Type checking skipped due to undeclared identifier\n");
        return;
    }
    
    // Check if types match
    if (expected_type != expr_type) {
        printf("  Semantic Error: Type mismatch in initialization of '%s'\n", var_name);
        printf("    Expected: %s, Got: %s\n", 
               get_type_name(expected_type), 
               get_type_name(expr_type));
        semantic_errors++;
        return;
    }
    
    printf("  Initialization of '%s' type-checked successfully (%s)\n", 
           var_name, get_type_name(expected_type));
    printf("=== DEBUG: Exiting handle_initialization ===\n");
}

// Enhanced handle_function_call with argument count and type validation
void handle_function_call(node* call_node, scope* curr_scope) {
    if (!call_node || !call_node->left || !call_node->left->token) {
        printf("  Semantic Error: Invalid function call node\n");
        semantic_errors++;
        return;
    }
    
    char* func_name = call_node->left->token;
    
    printf("Found function call: %s\n", func_name);
    
    // Check if function has been declared
    function_info* func_info = find_function_by_name(func_name);
    if (!func_info) {
        printf("  Semantic Error: Function '%s' called before declaration\n", func_name);
        semantic_errors++;
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
    
    // Validate argument count
    printf("  Validating argument count: passed=%d, required=%d-%d\n", 
           args_passed, min_required_args, total_params);
    
    if (args_passed < min_required_args) {
        printf("  Semantic Error: Too few arguments for function '%s'\n", func_name);
        printf("    Expected at least %d arguments, got %d\n", min_required_args, args_passed);
        semantic_errors++;
        return;
    }
    
    if (args_passed > total_params) {
        printf("  Semantic Error: Too many arguments for function '%s'\n", func_name);
        printf("    Expected at most %d arguments, got %d\n", total_params, args_passed);
        semantic_errors++;
        return;
    }
    
    printf("  Function call '%s' argument count validated successfully\n", func_name);
    
    // NEW: Validate argument types
    if (args_passed > 0 && call_node->right) {
        printf("  Validating argument types...\n");
        
        // Extract argument nodes
        int arg_count;
        node** arg_nodes = extract_function_arguments(call_node->right, &arg_count);
        
        if (arg_nodes && arg_count == args_passed) {
            for (int i = 0; i < args_passed; i++) {
                // Get the type of the argument
                int arg_type = get_expression_type(arg_nodes[i], curr_scope);
                int expected_type = func_info->param_types[i];
                
                printf("    Arg %d: expected %s, got %s\n", 
                       i + 1, 
                       get_type_name(expected_type),
                       get_type_name(arg_type));
                
                if (arg_type == 0) {
                    printf("  Warning: Cannot determine type of argument %d for function '%s'\n", 
                           i + 1, func_name);
                } else if (arg_type != expected_type) {
                    printf("  Semantic Error: Type mismatch for argument %d in function '%s'\n", 
                           i + 1, func_name);
                    printf("    Expected: %s, Got: %s\n", 
                           get_type_name(expected_type), 
                           get_type_name(arg_type));
                    semantic_errors++;
                }
            }
        }
        
        // Free the allocated array
        if (arg_nodes) {
            free(arg_nodes);
        }
        
        printf("  Argument type validation completed\n");
    }
}

// Helper function to extract return type from function AST
int extract_return_type(node* func_node) {
    if (!func_node || !func_node->right) return 0;
    
    // Navigate the AST structure to find return_type node
    node* current = func_node->right;
    
    // The structure varies, so we need to search for "return_type" token
    // Use a simple BFS approach
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
        
        // Add children to queue
        if (check_node->left && rear < 50) {
            nodes_to_check[rear++] = check_node->left;
        }
        if (check_node->right && rear < 50) {
            nodes_to_check[rear++] = check_node->right;
        }
    }
    
    return 0; // No return type found
}

// Enhanced process_params_for_function with default value type checking
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
            
            // Add to function info
            add_parameter_to_function(func_info, param_name, param_type, has_default_value);
            
            // NEW: Check default value type if it exists
            if (has_default_value && default_value_node) {
                printf("  Checking default value for parameter '%s'...\n", param_name);
                
                // Get the type of the default value
                int default_type = get_expression_type(default_value_node, func_scope);
                
                if (default_type == 0) {
                    printf("    Warning: Cannot determine type of default value for '%s'\n", param_name);
                } else if (default_type != param_type) {
                    printf("  Semantic Error: Default value type mismatch for parameter '%s'\n", param_name);
                    printf("    Parameter type: %s, Default value type: %s\n", 
                           get_type_name(param_type), get_type_name(default_type));
                    semantic_errors++;
                } else {
                    printf("    Default value type OK: %s\n", get_type_name(default_type));
                }
            }
        }
    }
    
    // Recursively process children
    process_params_for_function(param_node->left, func_info, func_scope);
    process_params_for_function(param_node->right, func_info, func_scope);
}

// Updated analyze_node function with return handling and current function tracking

void analyze_node(node* root, node* parent, scope* curr_scope) {
    if (!root) return;
    
    printf("DEBUG: analyze_node called with token='%s'\n", 
           root->token ? root->token : "NULL");
    
    // Updated function handling in analyze_node
    if (root->token && strcmp(root->token, "function") == 0) {
        scope* func_scope = mkscope(curr_scope);
        
        if (root->left && root->left->token) {
            func_scope->scope_name = strdup(root->left->token);
            printf("Entering function scope: %s\n", func_scope->scope_name);
            
            // Extract return type from AST
            int return_type = extract_return_type(root);
            
            // Create and track function declaration with return type
            function_info* func_info = add_function_declaration(root->left->token, return_type);
            
            if (func_info) {
                // SET CURRENT FUNCTION CONTEXT for return validation
                function_info* previous_function = current_function;
                current_function = func_info;
                
                // Validate __main__ function requirements
                validate_main_function(root, func_scope);
                
                // Find and process parameters with enhanced function info tracking
                node* current = root->right;
                node* nodes_to_check[50] = {current};
                int front = 0, rear = 1;
                
                while (front < rear && rear < 50) {
                    node* check_node = nodes_to_check[front++];
                    if (!check_node) continue;
                    
                    if (check_node->token && strcmp(check_node->token, "params") == 0) {
                        printf("Processing parameters...\n");
                        process_params_for_function(check_node, func_info, func_scope);
                        break; // We found and processed params, no need to continue
                    }
                    
                    // Add children to queue
                    if (check_node->left && rear < 50) {
                        nodes_to_check[rear++] = check_node->left;
                    }
                    if (check_node->right && rear < 50) {
                        nodes_to_check[rear++] = check_node->right;
                    }
                }
                
                // Analyze function body with current function context
                analyze_node(root->left, root, func_scope);
                analyze_node(root->right, root, func_scope);
                
                // RESTORE PREVIOUS FUNCTION CONTEXT
                current_function = previous_function;
            }
        }
        return;
    }
    
    // Check if this is an if statement
    if (root->token && strcmp(root->token, "if") == 0) {
        handle_if_statement(root, curr_scope);
        // Continue with normal traversal to check the condition and body
    }

    // Check if this is a while statement
    if (root->token && strcmp(root->token, "while") == 0) {
        handle_while_statement(root, curr_scope);
        // Continue with normal traversal to check the condition and body
    }

    // Also handle if-else, if-elif, if-elif-else variants
    if (root->token && (strcmp(root->token, "if-else") == 0 || 
                        strcmp(root->token, "if-elif") == 0 || 
                        strcmp(root->token, "if-elif-else") == 0)) {
        // For these complex if statements, we need to find the condition
        // The structure might be nested, but the condition is usually in the first if node
        node* if_part = root->left;  // This should be the if node
        if (if_part && strcmp(if_part->token, "if") == 0) {
            handle_if_statement(if_part, curr_scope);
        }
        // Continue with normal traversal
    }       

    // Check if this is a return statement
    if (root->token && strcmp(root->token, "return") == 0) {
        handle_return_statement(root, curr_scope);
        // Continue with normal traversal to validate the return expression
    }
   
    // Check if this is a params node
    if (root->token && strcmp(root->token, "params") == 0) {
        printf("Processing parameters...\n");
        process_params(root, curr_scope);
        return;
    }
    
    // Check if this is an initialization node
    if (root->token && strcmp(root->token, "init") == 0) {
        printf("Found initialization: ");
        if (root->left && root->left->right && root->left->right->token) {
            printf("%s\n", root->left->right->token);
        }
        handle_initialization(root, curr_scope);
        // Don't continue with normal traversal - init is fully handled
        return;
    }
    
    // Check if this is a declaration node (standalone, not part of init)
    if (root->token && strcmp(root->token, "declare") == 0) {
        printf("Found declaration: ");
        if (root->left && root->right && root->left->token && root->right->token) {
            printf("%s %s\n", root->left->token, root->right->token);
            handle_declaration(root, curr_scope);
        }
    }
    
    // Check if this is an assignment node
    if (root->token && strcmp(root->token, "assign") == 0) {
        printf("Found assignment to: ");
        if (root->left && root->left->token) {
            printf("%s\n", root->left->token);
            handle_assignment(root, curr_scope);
        }
    }
    
    // Check if this is a function call
    if (root->token && strcmp(root->token, "call") == 0) {
        handle_function_call(root, curr_scope);
        // Continue with normal traversal to check arguments
    }
    
    // Check if this is a variable usage
    if (is_variable_usage(root, parent)) {
        printf("DEBUG: Found variable usage: %s\n", root->token);
        handle_variable_usage(root, curr_scope);
    }
    
    // Recursively check children with parent context
    analyze_node(root->left, root, curr_scope);
    analyze_node(root->right, root, curr_scope);
}

// Temporary debug function to print all collected function info
void debug_print_functions() {
    printf("\n=== DEBUG: All collected functions ===\n");
    function_info* current = declared_functions;
    int count = 0;
    
    while (current) {
        count++;
        printf("Function %d: %s\n", count, current->name);
        printf("  Return type: %s\n", get_type_name(current->return_type));
        printf("  Parameter count: %d\n", current->param_count);
        
        for (int i = 0; i < current->param_count; i++) {
            printf("    Param %d: %s %s", i+1, 
                   get_type_name(current->param_types[i]),
                   current->param_names[i]);
            if (current->has_default[i]) {
                printf(" (has default)");
            }
            printf("\n");
        }
        printf("\n");
        current = current->next;
    }
    printf("=== End debug ===\n\n");
}

// Main semantic analysis function - no more duplicate messages!
void semantic_analysis(struct node* root, scope* curr_scope) {
    // Cast the struct node* to our node* type
    node* ast_root = (node*)root;

    // Initialize function tracking list
    declared_functions = NULL;
    
    // Analyze the entire AST
    analyze_node(ast_root, NULL, curr_scope);

    debug_print_functions();
}