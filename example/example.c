// including the source file in order to test static functions
#include "../src/skin.h"
#include "../src/expression.h"

int main(int argc, char** argv) {
  skin_t sk;
  skin_node_t* node = expression_parse(&sk, "1 - (1 + 1)");
  print_node_tree(node, 0);

  node = expression_parse(&sk, "_add(-1, (1 + -1))");
  print_node_tree(node, 0);

  node = expression_parse(&sk, "_add(-1, (1 + -1))");
  print_node_tree(node, 0);
}