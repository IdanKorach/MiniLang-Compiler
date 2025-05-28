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
    printf("=== Starting 3AC Code Generation ===\n\n");
    
    if (!ast_root) {
        printf("// No AST to process\n");
        return;
    }
    
    // Handle multiple functions in the AST
    process_ast_functions(ast_root);
    
    printf("=== 3AC Generation Completed ===\n\n");
}

// Process the AST to find and generate code for functions
void process_ast_functions(struct node* node) {
    if (!node) return;
    
    // If this node is a function, process it
    if (node->token && strcmp(node->token, "function") == 0) {
        generate_function(node);
    }
    
    // Check for more functions in siblings (left/right children)
    process_ast_functions(node->left);
    process_ast_functions(node->right);
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
    printf("    BeginFunc\n");
    
    // func->right contains the function body
    if (func->right) {
        generate_function_body(func->right);
    }
    
    printf("    EndFunc\n\n");
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

    if (strcmp(stmt->token, "") == 0) {
        generate_statements(stmt);
        return;
    }

    // Skip nodes that are handled elsewhere
    if (strcmp(stmt->token, "params") == 0 || 
        strcmp(stmt->token, "return_type") == 0) {
        return;
    }
    
    // Check what kind of statement this is
    if (strcmp(stmt->token, "init") == 0) {
        // This is variable initialization
        generate_init_statement(stmt);
    }
    else if (strcmp(stmt->token, "assign") == 0) {
        // This is assignment
        generate_assign_statement(stmt);
    }
    else if (strcmp(stmt->token, "if") == 0) {
        // This is an if statement
        generate_simple_if(stmt);
    }
    else if (strcmp(stmt->token, "if-else") == 0) {
        // If-else statement 
        generate_if_else(stmt);
    }
    else if (strcmp(stmt->token, "if-elif") == 0) {
        // If-elif chain 
        generate_if_elif(stmt);
    }
    else if (strcmp(stmt->token, "if-elif-else") == 0) {
        // If-elif-else chain 
        generate_if_elif_else(stmt);
    }
    else if (strcmp(stmt->token, "while") == 0) {
        // While loop statement 
        generate_while_statement(stmt);
    }
    else if (strcmp(stmt->token, "call") == 0) {
        // Function call statement
        generate_function_call_statement(stmt);
    }
    else if (strcmp(stmt->token, "return") == 0) {
        // Return statement
        generate_return_statement(stmt);
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
        strcmp(expr->token, "%") == 0 ||
        strcmp(expr->token, "==") == 0 ||
        strcmp(expr->token, "!=") == 0 ||
        strcmp(expr->token, "<") == 0 ||
        strcmp(expr->token, ">") == 0 ||
        strcmp(expr->token, "<=") == 0 ||
        strcmp(expr->token, ">=") == 0) {
        // Binary operation (arithmetic or comparison)
        return generate_binary_operation(expr);
    }
    else if (strcmp(expr->token, "and") == 0) {
        // Logical AND with short-circuit evaluation
        return generate_logical_and(expr);
    }
    else if (strcmp(expr->token, "or") == 0) {
        // Logical OR with short-circuit evaluation
        return generate_logical_or(expr);
    }
    else if (strcmp(expr->token, "not") == 0) {
        // Logical NOT
        return generate_logical_not(expr);
    }
    else if (strcmp(expr->token, "call") == 0) {
        // Function call that returns a value - NEW!
        return generate_function_call_expression(expr);
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

// Generate code for logical AND with short-circuit evaluation
char* generate_logical_and(struct node* expr) {
    if (!expr || !expr->left || !expr->right) return NULL;
    
    // Generate code for left operand
    char* left_result = generate_expression(expr->left);
    
    // Generate labels for short-circuit evaluation
    char* false_label = new_label();
    char* end_label = new_label();
    char* result_temp = new_temp();
    
    // Short-circuit: if left is false, result is false
    printf("    if_false %s goto %s\n", left_result, false_label);
    
    // Left is true, evaluate right operand
    char* right_result = generate_expression(expr->right);
    printf("    %s = %s\n", result_temp, right_result);
    printf("    goto %s\n", end_label);
    
    // Left was false, result is false
    printf("%s:\n", false_label);
    printf("    %s = false\n", result_temp);
    
    printf("%s:\n", end_label);
    
    // Clean up temporaries
    if (left_result != expr->left->token) free(left_result);
    if (right_result != expr->right->token) free(right_result);
    free(false_label);
    free(end_label);
    
    return result_temp;
}

// Generate code for logical OR with short-circuit evaluation
char* generate_logical_or(struct node* expr) {
    if (!expr || !expr->left || !expr->right) return NULL;
    
    // Generate code for left operand
    char* left_result = generate_expression(expr->left);
    
    // Generate labels for short-circuit evaluation
    char* true_label = new_label();
    char* end_label = new_label();
    char* result_temp = new_temp();
    
    // Short-circuit: if left is true, result is true
    printf("    if_true %s goto %s\n", left_result, true_label);
    
    // Left is false, evaluate right operand
    char* right_result = generate_expression(expr->right);
    printf("    %s = %s\n", result_temp, right_result);
    printf("    goto %s\n", end_label);
    
    // Left was true, result is true
    printf("%s:\n", true_label);
    printf("    %s = true\n", result_temp);
    
    printf("%s:\n", end_label);
    
    // Clean up temporaries
    if (left_result != expr->left->token) free(left_result);
    if (right_result != expr->right->token) free(right_result);
    free(true_label);
    free(end_label);
    
    return result_temp;
}

// Generate code for logical NOT
char* generate_logical_not(struct node* expr) {
    if (!expr || !expr->right) return NULL;
    
    // Generate code for operand
    char* operand_result = generate_expression(expr->right);
    char* result_temp = new_temp();
    
    // Generate NOT operation
    printf("    %s = not %s\n", result_temp, operand_result);
    
    // Clean up
    if (operand_result != expr->right->token) {
        free(operand_result);
    }
    
    return result_temp;
}

// Generate simple if statement
void generate_simple_if(struct node* if_node) {
    if (!if_node || !if_node->left || !if_node->right) return;
    
    // if_node->left = condition
    // if_node->right = if body statements
    
    // Generate condition evaluation
    char* condition_result = generate_expression(if_node->left);
    
    // Generate end label for if statement
    char* end_label = new_label();
    
    // Generate conditional jump: if condition is false, skip the if body
    printf("    if_false %s goto %s\n", condition_result, end_label);
    
    // Generate if body
    generate_statements(if_node->right);
    
    // End label
    printf("%s:\n", end_label);
    
    // Clean up
    if (condition_result != if_node->left->token) {
        free(condition_result);
    }
    free(end_label);
}

// Generate if-else statement
void generate_if_else(struct node* if_else_node) {
    if (!if_else_node || !if_else_node->left || !if_else_node->right) return;
    
    // if_else_node->left = if part (contains condition and if body)
    // if_else_node->right = else body
    
    struct node* if_part = if_else_node->left;
    struct node* else_part = if_else_node->right;
    
    if (!if_part || !if_part->left || !if_part->right) return;
    
    // Extract condition and if body from if_part
    struct node* condition = if_part->left;
    struct node* if_body = if_part->right;
    
    // Generate condition evaluation
    char* condition_result = generate_expression(condition);
    
    // Generate labels
    char* else_label = new_label();  // Jump here if condition is false
    char* end_label = new_label();   // Jump here to skip else after if
    
    // Generate conditional jump: if condition is false, go to else
    printf("    if_false %s goto %s\n", condition_result, else_label);
    
    // Generate if body
    generate_statements(if_body);
    
    // Jump to end after if body (skip else)
    printf("    goto %s\n", end_label);
    
    // Else label and body
    printf("%s:\n", else_label);
    generate_statements(else_part);
    
    // End label
    printf("%s:\n", end_label);
    
    // Clean up
    if (condition_result != condition->token) {
        free(condition_result);
    }
    free(else_label);
    free(end_label);
}

// Generate while loop: while condition: { body }
void generate_while_statement(struct node* while_node) {
    if (!while_node || !while_node->left || !while_node->right) return;
    
    // while_node->left = condition
    // while_node->right = loop body
    
    // Generate labels
    char* loop_start_label = new_label();  // L1: loop begins here
    char* loop_end_label = new_label();    // L2: exit loop here
    
    // Loop start label - this is where we come back to
    printf("%s:\n", loop_start_label);
    
    // Evaluate condition
    char* condition_result = generate_expression(while_node->left);
    
    // If condition is false, exit the loop
    printf("    if_false %s goto %s\n", condition_result, loop_end_label);
    
    // Generate loop body
    generate_statements(while_node->right);
    
    // Jump back to start of loop
    printf("    goto %s\n", loop_start_label);
    
    // Loop end label
    printf("%s:\n", loop_end_label);
    
    // Clean up
    if (condition_result != while_node->left->token) {
        free(condition_result);
    }
    free(loop_start_label);
    free(loop_end_label);
}

// Generate if-elif chain
void generate_if_elif(struct node* if_elif_node) {
    if (!if_elif_node || !if_elif_node->left || !if_elif_node->right) return;
    
    // if_elif_node->left = initial if condition
    // if_elif_node->right = sequence containing if body + elif chain
    
    // Generate the initial if condition
    char* condition_result = generate_expression(if_elif_node->left);
    
    // Generate end label for entire chain
    char* end_label = new_label();
    char* first_elif_label = new_label();
    
    // Initial if: if condition is false, go to first elif
    printf("    if_false %s goto %s\n", condition_result, first_elif_label);
    
    // Process the right side: if body + elif chain
    process_if_body_and_elif_chain(if_elif_node->right, first_elif_label, end_label);
    
    // Final end label
    printf("%s:\n", end_label);
    
    // Clean up
    if (condition_result != if_elif_node->left->token) {
        free(condition_result);
    }
    free(first_elif_label);
    free(end_label);
}

// Process the sequence containing if body + elif chain
void process_if_body_and_elif_chain(struct node* sequence, char* elif_start_label, char* end_label) {
    if (!sequence) return;
    
    // sequence->left = if body (assign statement)
    // sequence->right = elif chain
    
    // Generate if body first
    if (sequence->left) {
        generate_statement(sequence->left);  // This is the "assign" statement
        printf("    goto %s\n", end_label);  // Skip all elifs after if body
    }
    
    // Now process the elif chain
    if (sequence->right) {
        process_elif_chain(sequence->right, elif_start_label, end_label);
    }
}

// Process the elif chain recursively
void process_elif_chain(struct node* elif_sequence, char* current_label, char* end_label) {
    if (!elif_sequence) return;
    
    // Print the current label (where we jump if previous condition failed)
    printf("%s:\n", current_label);
    
    if (elif_sequence->token && strcmp(elif_sequence->token, "elif") == 0) {
        // This is a single elif node
        generate_single_elif(elif_sequence, end_label);
    }
    else if (strcmp(elif_sequence->token, "") == 0) {
        // This is a sequence of elifs
        // elif_sequence->left = first elif
        // elif_sequence->right = remaining elifs (or single elif)
        
        if (elif_sequence->left && elif_sequence->left->token && 
            strcmp(elif_sequence->left->token, "elif") == 0) {
            
            // Generate this elif
            char* next_label = new_label();
            generate_single_elif_with_next(elif_sequence->left, next_label, end_label);
            
            // Process remaining elifs
            if (elif_sequence->right) {
                process_elif_chain(elif_sequence->right, next_label, end_label);
            } else {
                // No more elifs, just print the next label
                printf("%s:\n", next_label);
                free(next_label);
            }
        }
    }
}

// Generate a single elif that jumps to end if condition fails (last elif)
void generate_single_elif(struct node* elif_node, char* end_label) {
    if (!elif_node || !elif_node->left || !elif_node->right) return;
    
    // Generate elif condition
    char* condition_result = generate_expression(elif_node->left);
    
    // If condition fails, jump to end (this is the last elif)
    printf("    if_false %s goto %s\n", condition_result, end_label);
    
    // Generate elif body
    generate_statements(elif_node->right);
    
    // Jump to end after body
    printf("    goto %s\n", end_label);
    
    // Clean up
    if (condition_result != elif_node->left->token) {
        free(condition_result);
    }
}

// Generate a single elif that jumps to next_label if condition fails
void generate_single_elif_with_next(struct node* elif_node, char* next_label, char* end_label) {
    if (!elif_node || !elif_node->left || !elif_node->right) return;
    
    // Generate elif condition
    char* condition_result = generate_expression(elif_node->left);
    
    // If condition fails, jump to next elif
    printf("    if_false %s goto %s\n", condition_result, next_label);
    
    // Generate elif body
    generate_statements(elif_node->right);
    
    // Jump to end after body
    printf("    goto %s\n", end_label);
    
    // Clean up
    if (condition_result != elif_node->left->token) {
        free(condition_result);
    }
}

void generate_if_elif_else(struct node* if_elif_else_node) {
    if (!if_elif_else_node || !if_elif_else_node->left || !if_elif_else_node->right) return;
    
    // Based on your actual AST structure:
    // if_elif_else_node->left = if-elif structure
    // if_elif_else_node->right = final else body (direct assignment, no "else" wrapper)
    
    // Generate end label for entire chain
    char* end_label = new_label();
    char* else_label = new_label();
    
    // Process the if-elif part, but make the last elif jump to else_label
    generate_if_elif_with_final_else(if_elif_else_node->left, else_label, end_label);
    
    // Generate the final else part
    printf("%s:\n", else_label);
    generate_statements(if_elif_else_node->right);
    printf("    goto %s\n", end_label);
    
    // Final end label
    printf("%s:\n", end_label);
    
    // Clean up
    free(else_label);
    free(end_label);
}

// Process if-elif but make last elif jump to else instead of end
void generate_if_elif_with_final_else(struct node* if_elif_node, char* else_label, char* end_label) {
    if (!if_elif_node || !if_elif_node->left || !if_elif_node->right) return;
    
    // Generate the initial if condition
    char* condition_result = generate_expression(if_elif_node->left);
    
    // Generate first elif label
    char* first_elif_label = new_label();
    
    // Initial if: if condition is false, go to first elif
    printf("    if_false %s goto %s\n", condition_result, first_elif_label);
    
    // Process the if body and elif chain
    process_if_body_and_elif_with_final_else(if_elif_node->right, first_elif_label, else_label, end_label);
    
    // Clean up
    if (condition_result != if_elif_node->left->token) {
        free(condition_result);
    }
    free(first_elif_label);
}

// Process if body and elif chain, with last elif jumping to else
void process_if_body_and_elif_with_final_else(struct node* sequence, char* elif_start_label, char* else_label, char* end_label) {
    if (!sequence) return;
    
    // Generate if body first
    if (sequence->left) {
        generate_statement(sequence->left);  // This is the "assign" statement
        printf("    goto %s\n", end_label);  // Skip all elifs and else
    }
    
    // Now process the elif chain
    if (sequence->right) {
        process_elif_chain_with_else_destination(sequence->right, elif_start_label, else_label, end_label);
    }
}

// Process elif chain where last elif jumps to else_label if condition fails
void process_elif_chain_with_else_destination(struct node* elif_sequence, char* current_label, char* else_label, char* end_label) {
    if (!elif_sequence) return;
    
    printf("%s:\n", current_label);
    
    if (elif_sequence->token && strcmp(elif_sequence->token, "elif") == 0) {
        // This is a single elif - need to determine if it's the last one
        generate_single_elif_with_else_fallback(elif_sequence, else_label, end_label);
    }
    else if (strcmp(elif_sequence->token, "") == 0) {
        // This is a sequence of elifs
        if (elif_sequence->left && elif_sequence->left->token && 
            strcmp(elif_sequence->left->token, "elif") == 0) {
            
            // Check if there are more elifs after this one
            if (elif_sequence->right && elif_sequence->right->token && 
                strcmp(elif_sequence->right->token, "elif") == 0) {
                // There's another elif after this one
                char* next_label = new_label();
                generate_single_elif_with_next(elif_sequence->left, next_label, end_label);
                process_elif_chain_with_else_destination(elif_sequence->right, next_label, else_label, end_label);
                free(next_label);
            } else if (elif_sequence->right && strcmp(elif_sequence->right->token, "") == 0) {
                // There's a sequence on the right, process it
                char* next_label = new_label();
                generate_single_elif_with_next(elif_sequence->left, next_label, end_label);
                process_elif_chain_with_else_destination(elif_sequence->right, next_label, else_label, end_label);
                free(next_label);
            } else {
                // This is the last elif - it should jump to else if condition fails
                generate_single_elif_with_else_fallback(elif_sequence->left, else_label, end_label);
            }
        }
    }
}

// Generate elif that jumps to else_label if condition fails (for last elif)
void generate_single_elif_with_else_fallback(struct node* elif_node, char* else_label, char* end_label) {
    if (!elif_node || !elif_node->left || !elif_node->right) return;
    
    // Generate elif condition
    char* condition_result = generate_expression(elif_node->left);
    
    // If condition fails, jump to else (not end)
    printf("    if_false %s goto %s\n", condition_result, else_label);
    
    // Generate elif body
    generate_statements(elif_node->right);
    
    // Jump to end after body (skip else)
    printf("    goto %s\n", end_label);
    
    // Clean up
    if (condition_result != elif_node->left->token) {
        free(condition_result);
    }
}

// Generate function call statements
void generate_function_call_statement(struct node* call_stmt) {
    if (!call_stmt || !call_stmt->left) {
        printf("    // ERROR: Invalid function call\n");
        return;
    }
    
    // call_stmt->left should contain the function name
    char* function_name = call_stmt->left->token;
    
    if (!function_name) {
        printf("    // ERROR: Missing function name in call\n");
        return;
    }
    
    // Generate simple function call (no parameters for Phase 1)
    printf("    call %s\n", function_name);
}

// Generate return statement
void generate_return_statement(struct node* return_node) {
    if (!return_node) return;
    
    // Check if there's a return value
    if (return_node->left) {
        // return_node->left = expression to return
        
        // Generate the return expression
        char* return_value = generate_expression(return_node->left);
        
        // Emit return with value
        printf("    return %s\n", return_value);
        
        // Clean up if we generated a temporary
        if (return_value != return_node->left->token) {
            free(return_value);
        }
    } else {
        // Empty return (no value)
        printf("    return\n");
    }
}

// Generate function call expression
char* generate_function_call_expression(struct node* call_expr) {
    if (!call_expr || !call_expr->left) {
        return NULL;
    }
    
    // call_expr->left should contain the function name
    char* function_name = call_expr->left->token;
    
    if (!function_name) {
        return NULL;
    }
    
    // Generate a temporary variable for the return value
    char* result_temp = new_temp();
    
    // Generate LCall instruction (function call with return value)
    printf("    %s = LCall %s\n", result_temp, function_name);
    
    return result_temp;
}