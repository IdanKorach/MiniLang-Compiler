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
int get_expression_type(node* expr_node, scope* curr_scope) {
    if (!expr_node || !expr_node->token) {
        return 0; // Unknown type
    }
    
    printf("DEBUG get_expression_type: analyzing token='%s'\n", expr_node->token);
    
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
    
    // Check if it looks like a string literal using our helper function
    if (looks_like_string_literal(expr_node->token)) {
        printf("DEBUG: Detected string literal by pattern\n");
        return TYPE_STRING;
    }
    
    // If it's not a declared variable and not any of the above literals,
    // it's an ERROR - don't assume it's a string!
    if (!found_var) {
        // If we reach here, it's likely an undeclared variable
        // Don't assume type - let the variable usage check handle the error
        printf("DEBUG: Undeclared identifier - returning unknown type\n");
        return 0; // Return 0 to indicate we can't determine the type
    }
    
    // Handle arithmetic operations (rest of the function stays the same)
    if (strcmp(expr_node->token, "+") == 0 || strcmp(expr_node->token, "-") == 0 ||
        strcmp(expr_node->token, "*") == 0 || strcmp(expr_node->token, "/") == 0) {
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
    }
    
    // Comparison operations result in boolean
    if (strcmp(expr_node->token, "==") == 0 || strcmp(expr_node->token, "!=") == 0 ||
        strcmp(expr_node->token, "<") == 0 || strcmp(expr_node->token, ">") == 0 ||
        strcmp(expr_node->token, "<=") == 0 || strcmp(expr_node->token, ">=") == 0) {
        return TYPE_BOOL;
    }
    
    // Logical operations result in boolean
    if (strcmp(expr_node->token, "and") == 0 || strcmp(expr_node->token, "or") == 0 ||
        strcmp(expr_node->token, "not") == 0) {
        return TYPE_BOOL;
    }
    
    // Function calls - for now return unknown type
    if (strcmp(expr_node->token, "call") == 0) {
        // We'd need function signature tracking to determine return type
        return 0; // Unknown
    }
    
    printf("DEBUG: Unknown expression type\n");
    return 0; // Unknown type
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

// Debug version of is_variable_usage
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

// Handle function call validation
void handle_function_call(node* call_node, scope* curr_scope) {
    if (!call_node || !call_node->left || !call_node->left->token) {
        printf("  Semantic Error: Invalid function call node\n");
        semantic_errors++;
        return;
    }
    
    char* func_name = call_node->left->token;
    
    printf("Found function call: %s\n", func_name);
    
    // Check if function has been declared
    if (!is_function_declared(func_name)) {
        printf("  Semantic Error: Function '%s' called before declaration\n", func_name);
        semantic_errors++;
        return;
    }
    
    printf("  Function call '%s' validated successfully\n", func_name);
    
    // TODO: In the future, we could add parameter validation here
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

// Enhanced process_params to collect parameter information for function_info
void process_params_for_function(node* node, function_info* func_info, scope* func_scope) {
    if (!node || !func_info) return;
    
    // Check if this is a parameter (type followed by identifier)
    if (node->token && get_type(node->token) != 0) {
        // This is a parameter type node
        char* param_name = NULL;
        int has_default_value = 0;
        
        // Extract parameter name - same logic as before
        if (node->left) {
            if (node->left->token && strlen(node->left->token) > 0) {
                if (get_type(node->left->token) == 0) {
                    param_name = node->left->token;
                    // Check if there's a default value (right side of the param name node)
                    if (node->left->left) {
                        has_default_value = 1;
                    }
                }
            } else if (!node->left->token || strlen(node->left->token) == 0) {
                if (node->left->left && node->left->left->token) {
                    param_name = node->left->left->token;
                    if (node->left->right) {
                        has_default_value = 1;
                    }
                }
            }
        }
        
        if (param_name) {
            int param_type = get_type(node->token);
            
            // Add to function info
            add_parameter_to_function(func_info, param_name, param_type, has_default_value);
        }
    }
    
    // Recursively process children
    process_params_for_function(node->left, func_info, func_scope);
    process_params_for_function(node->right, func_info, func_scope);
}

// Check if a node represents a variable usage (not declaration/assignment)
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
            }
        }
        
        analyze_node(root->left, root, func_scope);
        analyze_node(root->right, root, func_scope);
        return;
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