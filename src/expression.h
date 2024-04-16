#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "skin.h"
#include "stdio.h"
#include "stdlib.h"

skin_node_t* expression_parse(skin_t* skin, const char* expression);
int expression_generate(skin_node_t* root, char* buf, int buf_size);

static char* operator_strings[] = {
    [SKINOP_NOP] = "NOP",           [SKINOP_ADD] = "add",
    [SKINOP_SUBTRACT] = "subtract", [SKINOP_PRODUCT] = "product",
    [SKINOP_DIVISOR] = "divide",    [SKINOP_NEGATE] = "negate",
    [SKINOP_MIN] = "min",           [SKINOP_MAX] = "max",
    [SKINOP_LESSTHAN] = "lessthan", [SKINOP_GREATERTHAN] = "greaterthan",
    [SKINOP_EQUALS] = "equals",
};

static inline char* op_to_string(skin_operator op) {
  return operator_strings[op];
}

static inline void print_node_tree_intermediates(skin_node_t* p, int indent) {
  if (p == NULL) {
    return;
  }

  if (p->arg) {
    print_node_tree_intermediates(p->arg, indent + 1);
  }
  for (int i = 0; i < indent; i++) {
    printf("\t");
  }
  // print the value TODO: only works for the floats for now
  if (p->name[0] != 0) {
    printf("(%s [%d] {", p->name, p->num_values);
  } else {  // print the operator
    printf("(%s [%d] {", op_to_string(p->op), p->num_values);
  }
  for(int i = 0; i < p->num_values; i++){
    printf("%.2f, ", p->values[i]);
  }
  printf("}\n");

  if (p->child) {
    print_node_tree_intermediates(p->child, indent + 1);
  }
}

static inline void print_node_tree(skin_node_t* p, int indent) {
  if (p == NULL) {
    return;
  }

  if (p->arg) {
    print_node_tree(p->arg, indent + 1);
  }
  for (int i = 0; i < indent; i++) {
    printf("\t");
  }
  // print the value TODO: only works for the floats for now
  if (p->name[0] != 0) {
    printf("%s\n", p->name);
  } else {  // print the operator
    printf("%s\n", op_to_string(p->op));
  }
  if (p->child) {
    print_node_tree(p->child, indent + 1);
  }
}

#ifdef __cplusplus
}
#endif