#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>

#include "skin.h"

#define MAX_NODES 64

typedef struct skin_input {
  char* name;
  char* description;
  skin_node_t* node;
} skin_input_t;

typedef struct input_group {
  char* name;
  char* description;
  skin_input_t inputs[MAX_NODES];
  int num_inputs;
} skin_input_group_t;

skin_input_group_t group_example = {
    .name = "example",
    .description = "this is an example input",
    .inputs =
        {
            {.name = "x", .description = "the x position of the thing"},
            {.name = "size", .description = "the size of the thing"},
        },
    .num_inputs = 2};

#define example &group_example
#define example_x &example.inputs[0]
#define example_size &example.inputs[1]

skin_input_group_t group_example2 = {
    .name = "example2",
    .description = "this is another example input",
    .inputs =
        {
            {.name = "y", .description = "the y position of the thing"},
            {.name = "girth", .description = "the girth of the thing"},
        },
    .num_inputs = 2};

#define example2 &group_example2
#define example2_y &example2.inputs[0]
#define example2_girth &example2.inputs[1]

#ifdef __cplusplus
}
#endif