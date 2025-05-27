#include "codegen.h"
#include <string.h>

// Global counters for generating unique names
int temp_counter = 1;
int label_counter = 1;

// Helper function: generate new temporary variable
char* new_temp() {
    char* temp = (char*)malloc(10);
    snprintf(temp, 10, "t%d", temp_counter++);
    return temp;
}

// Helper function: generate new label
char* new_label() {
    char* label = (char*)malloc(10);
    snprintf(label, 10, "L%d", label_counter++);
    return label;
}

// Helper function: reset counters for each function
void reset_counters() {
    temp_counter = 1;
    label_counter = 1;
}

// Generate 3AC code from the AST
void generate_3ac(struct node* ast_root, struct scope* global_scope) {
    printf("=== Starting 3AC Code Generation ===\n");
    
    if (!ast_root) {
        printf("// No AST to process\n");
        return;
    }
    
    // Process the function
    if (strcmp(ast_root->token, "function") == 0) {
        generate_function(ast_root);
    }
    
    printf("=== 3AC Generation Completed ===\n\n");
}

// Generate code for a function
void generate_function(struct node* func) {
    if (!func || !func->left) return;
    
    // Reset counters for each function (fresh start)
    reset_counters();
    
    // func->left is the function name 
    char* func_name = func->left->token;
    if (strcmp(func_name, "__main__") == 0)
        func_name = "main";  
    printf("%s:\n", func_name);
    printf("    BeginFunc 0\n");
    
    // func->right contains the function body
    if (func->right) {
        generate_function_body(func->right);
    }
    
    printf("    EndFunc\n");
}

// Generate code for function body
void generate_function_body(struct node* body) {
    if (!body) return;
    
    // Look for the params and statements
    if (body->left && strcmp(body->left->token, "params") == 0) {
        // Skip params for now, go to statements
        if (body->right) {
            generate_statements(body->right);
        }
    } else {
        // Direct statement
        generate_statements(body);
    }
}

// Generate code for multiple statements
void generate_statements(struct node* stmts) {
    if (!stmts) return;
    
    // Check if this is a statement sequence (empty token with left/right children)
    if (strcmp(stmts->token, "") == 0) {
        // Empty token - process children (multiple statements)
        if (stmts->left) generate_statements(stmts->left);   // Process first statement(s)
        if (stmts->right) generate_statements(stmts->right); // Process remaining statement(s)
    } else {
        // Single statement - process it
        generate_statement(stmts);
    }
}

// Generate code for a single statement
void generate_statement(struct node* stmt) {
    if (!stmt) return;
    
    // Check what kind of statement this is
    if (strcmp(stmt->token, "init") == 0) {
        // This is variable initialization
        generate_init_statement(stmt);
    }
    else if (strcmp(stmt->token, "assign") == 0) {
        // This is assignment
        generate_assign_statement(stmt);
    }
    else {
        printf("    // TODO: Statement type '%s'\n", stmt->token);
    }
}

// Generate code for variable initialization
void generate_init_statement(struct node* init) {
    if (!init || !init->left || !init->right) return;
    
    // init->left is declare node, init->right is the value/expression
    struct node* declare = init->left;
    struct node* value_expr = init->right;
    
    if (declare && declare->right && value_expr) {
        char* var_name = declare->right->token;  // Variable name
        
        // Generate expression (could be simple value or complex expression)
        char* expr_result = generate_expression(value_expr);
        
        printf("    %s = %s\n", var_name, expr_result);
        
        // Free the temporary result string if it was allocated
        if (expr_result != value_expr->token) {
            free(expr_result);
        }
    }
}

// Generate code for assignment
void generate_assign_statement(struct node* assign) {
    if (!assign || !assign->left || !assign->right) return;
    
    char* var_name = assign->left->token;     // Variable name
    
    // Generate expression (could be simple value or complex expression)
    char* expr_result = generate_expression(assign->right);
    
    printf("    %s = %s\n", var_name, expr_result);
    
    // Free the temporary result string if it was allocated
    if (expr_result != assign->right->token) {
        free(expr_result);
    }
}

// Generate code for expressions
char* generate_expression(struct node* expr) {
    if (!expr) return NULL;
    
    // Check what type of expression this is
    if (strcmp(expr->token, "+") == 0 ||
        strcmp(expr->token, "-") == 0 ||
        strcmp(expr->token, "*") == 0 ||
        strcmp(expr->token, "/") == 0 ||
        strcmp(expr->token, "%") == 0) {
        // Binary arithmetic operation
        return generate_binary_operation(expr);
    }
    else {
        // Simple literal or identifier - return the token directly
        return expr->token;
    }
}

// Generate code for binary operations
char* generate_binary_operation(struct node* expr) {
    if (!expr || !expr->left || !expr->right) return NULL;
    
    // Generate code for left and right operands (recursively)
    char* left_result = generate_expression(expr->left);
    char* right_result = generate_expression(expr->right);
    
    // Generate a new temporary variable for the result
    char* temp_var = new_temp();
    
    // Emit the 3AC instruction
    printf("    %s = %s %s %s\n", temp_var, left_result, expr->token, right_result);
    
    // Clean up if we generated temporary variables for operands
    if (left_result != expr->left->token) {
        free(left_result);
    }
    if (right_result != expr->right->token) {
        free(right_result);
    }
    
    return temp_var;
}