#include "codegen.h"
#include <string.h>

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
    
    printf("=== 3AC Generation Completed ===\n");
}

// Generate code for a function
void generate_function(struct node* func) {
    if (!func || !func->left) return;
    
    // func->left is the function name and func->right is the body
    char* func_name = func->left->token;
    printf("%s\n", func_name);
    printf("BeginFunc 0\n");
    
    // func->right contains the function body
    if (func->right) {
        generate_function_body(func->right);
    }
    
    printf("EndFunc\n");
}

// Generate code for function body
void generate_function_body(struct node* body) {
    if (!body) return;
    
    // Look for the params and statements
    if (body->left && strcmp(body->left->token, "params") == 0) {
        // Skip params for now, go to statements
        if (body->right) {
            generate_statement(body->right);
        }
    } else {
        // Direct statement
        generate_statement(body);
    }
}

// Generate code for statements
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
    else if (strcmp(stmt->token, "") == 0) {
        // Empty token - process children (multiple statements)
        if (stmt->left) generate_statement(stmt->left);
        if (stmt->right) generate_statement(stmt->right);
    }
    else {
        printf("  // TODO: Statement type '%s'\n", stmt->token);
    }
}

// Generate code for variable initialization
void generate_init_statement(struct node* init) {
    if (!init || !init->left || !init->right) return;
    
    // init->left is declare node, init->right is the value
    struct node* declare = init->left;
    struct node* value = init->right;
    
    if (declare && declare->right && value) {
        char* var_name = declare->right->token;  // Variable name
        char* var_value = value->token;          // Value 
        
        printf("  %s = %s\n", var_name, var_value);
    }
}

// Generate code for assignment
void generate_assign_statement(struct node* assign) {
    if (!assign || !assign->left || !assign->right) return;
    
    char* var_name = assign->left->token;     // Variable name
    char* var_value = assign->right->token;   // Value
    
    printf("  %s = %s\n", var_name, var_value);
}