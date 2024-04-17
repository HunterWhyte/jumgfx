#include "skin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void skin_init(skin_t** skin_out, skin_input_t* inputs, int num_inputs) {
  skin_t* skin = malloc(sizeof(skin_t));
  // clear pool allocators
  skin->num_nodes = 0;

  // create nodes based on the set of inputs provided
  for (int i = 0; i < num_inputs; i++) {
    for (int j = 0; j < inputs[i].num_nodes; j++) {
      snprintf(skin->node_pool[skin->num_nodes].name, MAX_NODE_NAME, "%s_%s",
               inputs[i].name, inputs[i].nodes[j].name);
      inputs[i].nodes[j].node = &skin->node_pool[skin->num_nodes];
      skin->num_nodes++;
    }
  }

  *skin_out = skin;
  return;
}

void skin_deinit(skin_t* skin) { free(skin); }

void node_evaluate(skin_node_t* root) {
  // =============== LEAF NODE ===============

  // if this is a leaf node then we don't need to evaluate anything and can just
  // return
  if (root->child == NULL && root->arg == NULL) {
    return;
  }

  // check for invalid node states (TODO: just make these impossible with a
  // union?)
  if (root->child == NULL || root->op == SKINOP_NOP) {
    printf("ERROR MALFORMED NODE\n");
    assert(0);
  }

  node_evaluate(root->child);

  // =============== UNARY OPERATORS ===============

  if (root->op == SKINOP_NEGATE) {
    for (int i = 0; i < root->child->num_values; i++) {
      root->values[i] = root->child->values[i] * -1;
    }
    root->num_values = root->child->num_values;
    return;
  }

  if (root->arg == NULL) {
    printf("ERROR MALFORMED NODE\n");
    assert(0);
  }

  // =============== BINARY OPERATORS ===============

  node_evaluate(root->arg);

  for (int i = 0; i < root->child->num_values; i++) {
    root->values[i] = root->child->values[i];
  }
  root->num_values = root->child->num_values;

  // for arithmetic operators if the node length of the arg is less than the
  // node length of the child then we extend the arg values to the length of the
  // array we split this out now sot hat we can avoid doing conditional checking
  // every loop iteration to see if we exceeded the length of the arg node
  int num_ops = MIN(root->num_values, root->arg->num_values);

  // if the argument node is empty then we just return without modifying
  if (num_ops == 0) {
    switch (root->op) {
      case (SKINOP_ADD):
      case (SKINOP_SUBTRACT):
      case (SKINOP_PRODUCT):
      case (SKINOP_DIVISOR):
      case (SKINOP_MIN):
      case (SKINOP_MAX):
        return;
      default:
        break;
    }
  }

  float* arg_vals = root->arg->values;
  int arg_len = root->arg->num_values;

  float* root_vals = root->values;
  int root_len = root->num_values;

  // we are doing loops inside of op switch because we only need to evaluate op
  // once
  // TODO figure out what this actually compiles to vs. the alternative
  switch (root->op) {
    case (SKINOP_ADD):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] += arg_vals[i];
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] += arg_vals[arg_len - 1];
      }
      break;
    case (SKINOP_SUBTRACT):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] -= arg_vals[i];
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] -= arg_vals[arg_len - 1];
      }
      break;
    case (SKINOP_PRODUCT):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] *= arg_vals[i];
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] *= arg_vals[arg_len - 1];
      }
      break;
    case (SKINOP_DIVISOR):
      for (int i = 0; i < num_ops; i++) {
        for (int i = 0; i < num_ops; i++) {
          if (arg_vals[i] == 0) {
            printf("warning divide by zero in skin engine");
          } else {
            root_vals[i] /= arg_vals[i];
          }
        }
        if (arg_vals[arg_len - 1] == 0) {
          printf("warning divide by zero in skin engine");
        } else {
          for (int i = num_ops; i < root_len; i++) {  // extended operation
            root_vals[i] /= arg_vals[arg_len - 1];
          }
        }
      }
      break;
    case (SKINOP_MIN):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] = MIN(root_vals[i], arg_vals[i]);
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] = MIN(root_vals[i], arg_vals[arg_len - 1]);
      }
      break;
    case (SKINOP_MAX):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] = MAX(root_vals[i], arg_vals[i]);
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] = MAX(root_vals[i], arg_vals[arg_len - 1]);
      }
      break;
    case (SKINOP_GREATERTHAN):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] = root_vals[i] > arg_vals[i] ? 1.0 : 0.0f;
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] = root_vals[i] > arg_vals[arg_len - 1] ? 1.0 : 0.0f;
      }
      break;
    case (SKINOP_LESSTHAN):
      for (int i = 0; i < num_ops; i++) {
        root_vals[i] = root_vals[i] < arg_vals[i] ? 1.0 : 0.0f;
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        root_vals[i] = root_vals[i] < arg_vals[arg_len - 1] ? 1.0 : 0.0f;
      }
      break;
    case (SKINOP_EQUALS):
      for (int i = 0; i < num_ops; i++) {
        if (((-EPSILON) < (root_vals[i] - arg_vals[i])) &&
            ((root_vals[i] - arg_vals[i]) < (EPSILON))) {
          root_vals[i] = 1.0f;
        } else {
          root_vals[i] = 0.0f;
        }
      }
      for (int i = num_ops; i < root_len; i++) {  // extended operation
        if (((-EPSILON) < (root_vals[i] - arg_vals[arg_len - 1])) &&
            ((root_vals[i] - arg_vals[arg_len - 1]) < (EPSILON))) {
          root_vals[i] = 1.0f;
        } else {
          root_vals[i] = 0.0f;
        }
      }
      break;
    default:
      printf("ERROR MALFORMED NODE\n");
      assert(0);
      break;
  }

  return;
}