#include "../src/expression.h"
#include "../src/skin.h"
#include "test.h"

SUITE(expression_parser);

// example input, this gets defined in a c file? the #defines go in a header
// file?
skin_input_t inputs[] = {{.name = "example",
                          .description = "this is an example input",
                          .nodes =
                              {
                                  {.name = "x", .description = "the x position of the thing"},
                                  {.name = "size", .description = "the size of the thing"},
                              },
                          .num_nodes = 2},
#define example inputs[0]
#define example_x example.nodes[0]
#define example_size example.nodes[1]

                         {.name = "example2",
                          .description = "this is another example input",
                          .nodes =
                              {
                                  {.name = "y", .description = "the y position of the thing"},
                                  {.name = "girth", .description = "the girth of the thing"},
                              },
                          .num_nodes = 2}
#define example2 inputs[1]
#define example2_y example2.nodes[0]
#define example2_girth example2.nodes[1]
};

TEST(expression_parser, basic_add) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + 1");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, nested) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + (1 + 1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_EQ(node->arg->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->arg->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->arg->values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, negate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + -1");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->values[0], -1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, equality) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 == -1");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_EQUALS);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->values[0], -1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, function_name) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "_add(1,1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, function_name_whitespace) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "_add  ( 1.1   ,     1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.1f);
  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  ASSERT(0 < 1);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, mismatched_brackets) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "_add((1,1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, mismatched_brackets2) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "_add(1,1))");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, nocomma) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "_add(1 1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, double_operator) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + + 1");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, double_underscore) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "__add(1, 1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, single_child) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "((1 + (1)))");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);

  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, basic_lookup) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + example_x");
  print_node_tree_verbose(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_EQ(example_x.node, node->arg);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, basic_lookup_failure) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + example");

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, multiple_lookup) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "example2_girth + example_x");
  print_node_tree_verbose(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_EQ(example_x.node, node->arg);
  ASSERT_EQ(example2_girth.node, node->child);

  skin_deinit(sk);
  return 0;
}

SUITE(expression_generator);

TEST(expression_generator, basic_generate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + 1");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);
  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("_add(1,1)", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_generator, single_generate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_init(&sk, inputs, 2);
  skin_node_t* node = expression_parse(sk, "((1))");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("1", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_generator, nested_generate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "1 + (1 + 1)");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("_add(1,_add(1,1))", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_generator, negate_generate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "(1 + -1)");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("_add(1,-1)", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_generator, self_generate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t* node = expression_parse(sk, "(1 + -1)");
  print_node_tree(node, 0);

  char buf1[256];
  int len = expression_generate(node, buf1, 256);
  printf("generated %s\n", buf1);

  ASSERT_STRING_EQ("_add(1,-1)", buf1);

  node = expression_parse(sk, buf1);
  char buf2[256];
  len = expression_generate(node, buf2, 256);
  printf("generated %s\n", buf2);

  ASSERT_STRING_EQ(buf1, buf2);

  skin_deinit(sk);
  return 0;
}

SUITE(node_evaluator);

TEST(node_evaluator, basic_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 2.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, multiple_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {1, 2}, .num_values = 2, .op = SKINOP_NOP};
  skin_node_t right = {.values = {1, 2}, .num_values = 2, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 2);
  ASSERT_FLOAT_EQ(root.values[0], 2.0f);
  ASSERT_FLOAT_EQ(root.values[1], 4.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, empty_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {}, .num_values = 0, .op = SKINOP_NOP};
  skin_node_t right = {.values = {}, .num_values = 0, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 0);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, one_to_empty_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {}, .num_values = 0, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, one_to_many_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {1, 2}, .num_values = 2, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 2.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, many_to_one_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {1, 2}, .num_values = 2, .op = SKINOP_NOP};
  skin_node_t right = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 2);
  ASSERT_FLOAT_EQ(root.values[0], 2.0f);
  ASSERT_FLOAT_EQ(root.values[1], 3.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, nested_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t a = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t b = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t left = {.child = &a, .arg = &b, .op = SKINOP_ADD};
  skin_node_t right = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_ADD};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 3.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, subtract_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {1}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_SUBTRACT};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, product_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_PRODUCT};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 4.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, divisor_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_DIVISOR};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, min_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {-2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_MIN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], -2.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, max_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {-2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_MAX};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 4.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, min_to_none_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {}, .num_values = 0, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_MIN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 4.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, lessthan_true_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {-2}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_LESSTHAN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, lessthan_false_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_LESSTHAN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 0.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, greaterthan_true_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {3.999}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_GREATERTHAN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, greaterthan_false_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_GREATERTHAN};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 0.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, equality_false_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {3.999}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_EQUALS};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 0.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, equality_true_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_EQUALS};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(node_evaluator, equality_none_evaluate) {
  skin_t* sk;
  skin_init(&sk, inputs, 2);

  skin_node_t left = {.values = {4}, .num_values = 1, .op = SKINOP_NOP};
  skin_node_t right = {.values = {}, .num_values = 0, .op = SKINOP_NOP};
  skin_node_t root = {.child = &left, .arg = &right, .op = SKINOP_EQUALS};
  node_evaluate(&root);
  print_node_tree_verbose(&root, 0);
  ASSERT_EQ(root.num_values, 1);
  ASSERT_FLOAT_EQ(root.values[0], 0.0f);

  skin_deinit(sk);
  return 0;
}

int main(int argc, char** argv) {
  run_suite(expression_generator);
  run_suite(node_evaluator);
  run_suite(expression_parser);
}