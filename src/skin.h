#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define EPSILON 0.000001

typedef enum op {
  SKINOP_NOP = 0,
  // arithmetic operators
  SKINOP_ADD,
  SKINOP_SUBTRACT,
  SKINOP_PRODUCT,
  SKINOP_DIVISOR,
  SKINOP_NEGATE,  // unary negate
  // comparison operators
  SKINOP_MIN,
  SKINOP_MAX,
  SKINOP_LESSTHAN,
  SKINOP_GREATERTHAN,
  SKINOP_EQUALS
} skin_operator;

typedef enum skin_error {
  SKINERR_SUCCESS = 0,
  SKINERR_EXPRESSION_ERROR,
} skin_error;

#define MAX_NODE_NAME 256
#define MAX_VALUES 4096
/**
 * @brief The basic unit of the skin engine are nodes. A node performs an 
 * operation or holds a value.
 * 
 * Nodes are allocated from a pool that is generated on skin parsing.
 * We get a pointer to a node on initialization and use that. At the end of skin
 * lifetime we free entire pool of nodes.
*/
typedef struct skin_node_t skin_node_t;
struct skin_node_t {
  char name[MAX_NODE_NAME];
  // defines type of node
  skin_operator op;

  // primary argument for operators
  skin_node_t* child;
  // second argument
  skin_node_t* arg;

  float values[MAX_VALUES];
  int num_values;
};

#define NODE_POOL_SIZE 4096
#define INPUT_VALUE_POOL_SIZE 4096
#define LITERAL_POOL_SIZE 4096
typedef struct skin_t {
  // memory pool of nodes, gets allocated at parse time
  int num_nodes;
  skin_node_t node_pool[NODE_POOL_SIZE];
} skin_t;

void skin_init(skin_t** skin);
void skin_deinit(skin_t* skin);
void node_evaluate(skin_node_t* root);

#ifdef __cplusplus
}
#endif