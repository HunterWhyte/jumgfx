#include "../src/expression.h"
#include "../src/skin.h"
#include "test.h"
#include "../src/input.h"

SUITE(expression_parser);

TEST(expression_parser, basic_add) {
  skin_t* sk;
  skin_init(&sk);

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
  skin_init(&sk);

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
  skin_init(&sk);

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
  skin_init(&sk);

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
  skin_init(&sk);

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
  skin_init(&sk);

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
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "_add((1,1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, mismatched_brackets2) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "_add(1,1))");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, nocomma) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "_add(1 1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, double_operator) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "1 + + 1");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, double_underscore) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "__add(1, 1)");
  print_node_tree(node, 0);

  ASSERT_EQ(node, NULL);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, single_child) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "((1 + (1)))");
  print_node_tree(node, 0);

  ASSERT_EQ(node->op, SKINOP_ADD);
  ASSERT_FLOAT_EQ(node->child->values[0], 1.0f);

  ASSERT_FLOAT_EQ(node->arg->values[0], 1.0f);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, basic_generate) {
  skin_t* sk;
  skin_init(&sk);

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

TEST(expression_parser, single_generate) {
  skin_t* sk;
  skin_init(&sk);

  skin_init(&sk);
  skin_node_t* node = expression_parse(sk, "((1))");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("1", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, nested_generate) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "1 + (1 + 1)");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("_add(1,_add(1,1))", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, negate_generate) {
  skin_t* sk;
  skin_init(&sk);

  skin_node_t* node = expression_parse(sk, "(1 + -1)");
  print_node_tree(node, 0);

  char buf[256];
  int len = expression_generate(node, buf, 256);
  printf("generated %s\n", buf);

  ASSERT_STRING_EQ("_add(1,-1)", buf);

  skin_deinit(sk);
  return 0;
}

TEST(expression_parser, self_generate) {
  skin_t* sk;
  skin_init(&sk);

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

int main(int argc, char** argv) {
  run_suite(expression_parser);
}