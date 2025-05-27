#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node;
struct scope;

typedef struct node {
    char *token;
    struct node *left;
    struct node *right;
} node;

// Function declarations
struct scope* mkscope(struct scope* parent);
void semantic_analysis(struct node* root, struct scope* curr_scope);

// Global variables
extern int semantic_errors;

#endif // SEMANTIC_ANALYSIS_H