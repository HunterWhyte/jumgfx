/** @file Serializing and deserializing node trees into their expression form in text
 * @author Hunter Whyte
*/
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "skin.h"

#include "expression.h"

#define MAX_EXPRESSION_LENGTH 4096
#define MAX_TOKEN_LENGTH 1024
#define MAX_NUM_TOKENS 512

#define NUM_SPECIAL_CHARS 12
static const char special_chars[NUM_SPECIAL_CHARS] = {'+', '*', '/', '-', '%', '(',
                                                      ')', '_', ',', '>', '<', '='};
static bool is_special(char c) {
  for (int i = 0; i < NUM_SPECIAL_CHARS; i++) {
    if (c == special_chars[i]) {
      return true;
    }
  }
  return false;
}

#define NUM_OPERATOR_CHARS 8
static const char operator_chars[NUM_OPERATOR_CHARS] = {'+', '*', '/', '-', '%', '>', '<', '='};
static bool is_operator(char c) {
  for (int i = 0; i < NUM_OPERATOR_CHARS; i++) {
    if (c == operator_chars[i]) {
      return true;
    }
  }
  return false;
}

static bool is_numeric(char* s) {
  if (s == NULL) {
    return false;
  }
  int len = strlen(s);
  if (len == 0) {
    return false;
  }

  bool point = false;  // only allow one decimal point per number
  for (int i = 0; i < len; i++) {
    if (s[i] < '0' || s[i] > '9') {
      if (!point && (s[i] == '.')) {
        point = true;
        continue;
      }
      return false;
    }
  }

  return true;
}

static skin_operator parse_operator(char* s) {
  if (s == NULL) {
    return SKINOP_NOP;
  }

  switch (s[0]) {
    case '+':
      return SKINOP_ADD;
    case '-':
      return SKINOP_SUBTRACT;
    case '*':
      return SKINOP_PRODUCT;
    case '/':
      return SKINOP_DIVISOR;
    case '<':
      return SKINOP_LESSTHAN;
    case '>':
      return SKINOP_GREATERTHAN;
    case '=':
      if (strlen(s) > 1 && s[1] == '=') {
        return SKINOP_EQUALS;
      } else {
        return SKINOP_NOP;
      }
    default:
      break;
  }

  if (strcmp(s, "add") == 0) {
    return SKINOP_ADD;
  } else if (strcmp(s, "subtract") == 0) {
    return SKINOP_SUBTRACT;
  } else if (strcmp(s, "product") == 0) {
    return SKINOP_PRODUCT;
  } else if (strcmp(s, "divide") == 0) {
    return SKINOP_DIVISOR;
  } else if (strcmp(s, "min") == 0) {
    return SKINOP_MIN;
  } else if (strcmp(s, "max") == 0) {
    return SKINOP_MAX;
  } else if (strcmp(s, "lessthan") == 0) {
    return SKINOP_LESSTHAN;
  } else if (strcmp(s, "greaterthan") == 0) {
    return SKINOP_GREATERTHAN;
  } else if (strcmp(s, "equals") == 0) {
    return SKINOP_EQUALS;
  }

  return SKINOP_NOP;
}

// TODO: temp stub to take error messsages, have to decide how I want to use/handle these
// these are error messages that could potentially be shown to user as parsing/syntax errors
// if they are writing expressions by hand
static void register_error(char* error_string) {
  printf("ERROR: %s\n", error_string);
}

static skin_node_t* get_new_node(skin_t* skin) {
  // TODO: pulling from pool, move this to skin.c?
  assert(skin->num_nodes < NODE_POOL_SIZE);
  skin_node_t* node = &skin->node_pool[skin->num_nodes];
  skin->num_nodes++;
  memset(node, 0, sizeof(skin_node_t));
  node->num_values = 0;
  return node;
}

/**
 * @brief helper to create an operator node (not leaf)
*/
static skin_node_t* create_internal_node(skin_t* skin, skin_node_t* val, skin_operator op,
                                skin_node_t* arg) {
  skin_node_t* node = get_new_node(skin);
  node->child = val;
  node->op = op;
  node->arg = arg;
  return node;
}

/**
 * @brief parse a leaf node, either a literal value or a reference to an existing node
*/
static skin_node_t* create_leaf_node(skin_t* skin, char* text, bool negate) {
  if (is_numeric(text)) {
    if (strlen(text) >= MAX_NODE_NAME - 1) {
      register_error("create_leaf_node literal string length exceeds max node name length");
      return NULL;
    }
    float literal;
    literal = negate ? atof(text) * -1.0f : atof(text);
    skin_node_t* node = get_new_node(skin);

    node->values[0] = literal;
    node->num_values = 1;
    if (negate) {
      node->name[0] = '-';
      strcpy(&node->name[1], text);
    } else {
      strcpy(node->name, text);
    }
    return node;
  } else {
    // lookup the property
    // we do this O(N) search because I don't care, somewhat of an optimization is that the
    // input nodes get allocated first so you won't be reading all the general arithmetic nodes etc.
    for (int i = 0; i < skin->num_nodes; i++) {
      skin_node_t* node = &skin->node_pool[i];
      if (node->name[0] != 0) {
        if (strcmp(node->name, text) == 0) {
          return node;
        }
      }
    }
    return NULL;
  }
}

/**
 * @brief recursively builds node tree given tokenized expression
*/
static skin_node_t* parse_token(skin_t* skin, char tokens[][MAX_TOKEN_LENGTH], int num_tokens,
                                int* tokens_used) {
  skin_node_t* val = NULL;
  skin_node_t* arg = NULL;
  bool negate_val = false;
  bool negate_arg = false;
  *tokens_used = 0;

  skin_operator op = SKINOP_NOP;
  // One skin node can be composed of multiple tokens, since there are operators and val/arg
  // we must iterate over all tokens Recursively entering sub expressions
  while (true) {
    if (*tokens_used >= num_tokens) {
      // check that we have parsed correctly
      if (op != SKINOP_NOP && val != NULL && arg != NULL) {
        return create_internal_node(skin, val, op, arg);
      } else if (val != NULL && op == SKINOP_NOP && arg == NULL) {
        return val;
      } else {
        register_error("error invalid expression, mismatched brackets");
        return NULL;
      }
    }
    char* token = tokens[*tokens_used];
    *tokens_used += 1;

    // this is the last token/end of expression
    if (token[0] == ')') {
      // check that we have parsed correctly
      if (op != SKINOP_NOP && val != NULL && arg != NULL) {
        return create_internal_node(skin, val, op, arg);
      } else if (val != NULL && op == SKINOP_NOP && arg == NULL) {
        return val;
      } else {
        register_error("error invalid expression, mismatched brackets");
        return NULL;
      }
    }
    // function
    else if (token[0] == '_') {
      // operator
      op = parse_operator(tokens[*tokens_used]);
      if (op == SKINOP_NOP) {
        register_error("error invalid operator: following _ character");
        return NULL;
      }
      *tokens_used += 1;

      // check that opening bracket follows and consume it
      if (*tokens_used > num_tokens || !(tokens[*tokens_used][0] == '(')) {
        register_error("error all functions must be enclosed in brackets following operator");
        return NULL;
      }
      *tokens_used += 1;
    }
    // arg separator
    else if (token[0] == ',') {
      if (val == NULL || arg != NULL || op == SKINOP_NOP) {
        register_error("error invalid syntax ',' comma not between value and argument");
        return NULL;
      }
    }
    // start of new sub expression
    else if (token[0] == '(') {
      if (val == NULL) {
        int used = 0;
        val = parse_token(skin, &tokens[(*tokens_used)], (num_tokens - *tokens_used), &used);
        if (val == NULL) {
          return NULL;
        }
        *tokens_used += used;
      } else if (arg == NULL) {
        int used = 0;
        arg = parse_token(skin, &tokens[(*tokens_used)], (num_tokens - *tokens_used), &used);
        if (arg == NULL) {
          return NULL;
        }
        *tokens_used += used;
      } else {
        register_error("error invalid syntax opening bracket before closing");
        return NULL;
      }
    } else if (token[0] == '-') {  // handle - as special case since it can be negate or subtract
      if (val == NULL) {           // token is before val and arg
        negate_val = true;
      } else if (op == SKINOP_NOP) {  // token is after val but before operator
        op = SKINOP_SUBTRACT;
      } else if (arg == NULL) {  // token is after val and arg
        negate_arg = true;
      }
    }
    // token is a single char operator
    else if (is_operator(token[0])) {
      if (val == NULL) {
        register_error("error invalid expression operator with no value or argument");
        return NULL;
      } else if (op != SKINOP_NOP) {
        register_error("error invalid expression multiple operators for one value");
        return NULL;
      } else {
        op = parse_operator(token);
        if (op == SKINOP_NOP) {
          register_error("error invalid expression, unrecognized operator");
          return NULL;
        }
      }
    }
    // this token is a string key to another node or a float literal value, parse
    else {
      if (val == NULL) {
        val = create_leaf_node(skin, token, negate_val);
        if (val == NULL) {
          register_error("could not parse node");
          return NULL;
        }
      } else if (arg == NULL) {
        arg = create_leaf_node(skin, token, negate_arg);
        if (arg == NULL) {
          register_error("could not parse node");
          return NULL;
        }
      } else {
        register_error("error invalid syntax, too many arguments defined for function");
        return NULL;
      }
    }
  }
}

/**
 * @brief Takes a string expression and converts it into a evaluable node tree
*/
skin_node_t* expression_parse(skin_t* skin, const char* expression) {
  // TOKENIZATION
  // ----------------------------------------------------
  char tokens[MAX_NUM_TOKENS][MAX_TOKEN_LENGTH];
  int num_tokens = 0;
  int cur_pos = 0;

  // use statically allocated buffers
  char trimmed[MAX_EXPRESSION_LENGTH];

  // tokenize the expression looking for special chars
  // first trim all whitespace
  int len = strlen(expression);
  if (len > MAX_EXPRESSION_LENGTH) {
    register_error("expression length > max expression length");
    return NULL;
  }
  int trimmed_len = 0;
  for (int i = 0; i < len; i++) {
    if (!(expression[i] == ' ' || expression[i] == '\t' || expression[i] == '\n')) {
      trimmed[trimmed_len++] = expression[i];
    }
  }
  trimmed[trimmed_len] = '\0';

  for (int i = 0; i < trimmed_len; i++) {
    // detect special characters and split accordingly

    // special case for underscores
    // using a _ prefix to mean a function and it can also be used as part of property names
    if (trimmed[i] == '_') {
      // if an _ is in middle of a property then its just part of name and not a special char
      if (i == 0 || is_special(trimmed[i - 1])) {
        // don't add if cur_token is empty (two special chars in a row or at beginning)
        if (cur_pos > 0) {
          // terminate previous token and advance
          tokens[num_tokens][cur_pos] = '\0';
          num_tokens++;
        }
        // add _ as its own token
        tokens[num_tokens][0] = trimmed[i];
        tokens[num_tokens][1] = '\0';
        cur_pos = 0;
        num_tokens++;
      } else {
        tokens[num_tokens][cur_pos++] = trimmed[i];
      }

      // in the case of a regular special token
    } else if (is_special(trimmed[i])) {
      // if there isnt currently an active token that needs to be terminated and advanced
      if (cur_pos > 0) {
        // terminate previous token and advance
        tokens[num_tokens][cur_pos] = '\0';
        num_tokens++;
      }

      // check for == special case as the only special character sequence more than 1 char
      if (trimmed[i] == '=') {
        // TODO ugly because have to check length before we check i+1 to ensure we are not overrunning string
        if (i < (trimmed_len - 1)) {
          if (trimmed[i + 1] == '=') {
            tokens[num_tokens][0] = '=';
            tokens[num_tokens][1] = '=';
            tokens[num_tokens][2] = '\0';
            cur_pos = 0;
            num_tokens++;
            i++;  // skip one ahead since we already checked trimmed[i+1]
          }
        }
      } else {
        tokens[num_tokens][0] = trimmed[i];
        tokens[num_tokens][1] = '\0';
        cur_pos = 0;
        num_tokens++;
      }
    } else {
      tokens[num_tokens][cur_pos] = trimmed[i];
      cur_pos++;
      if (cur_pos >= MAX_TOKEN_LENGTH) {
        register_error("Token exceeded max token length");
      }
      // if this is the end of expression make sure to terminate
      if (i + 1 == trimmed_len) {
        tokens[num_tokens][cur_pos] = '\0';
        num_tokens++;
      }
    }
  }

  // #ifdef DEBUG_EXPRESSION_PARSER
  // print tokens for debug purposes
  printf("Expression: %s \n", expression);
  printf("Trimmed: %s \n", trimmed);
  printf("trimmed_len %d \n", trimmed_len);
  printf("Tokenized: ");
  for (int i = 0; i < num_tokens; i++) {
    printf("[%s] ", tokens[i]);
  }
  printf("\n");
  // #endif

  int tokens_used = 0;
  skin_node_t* node = parse_token(skin, tokens, num_tokens, &tokens_used);
  if (tokens_used != num_tokens) {
    register_error("Tokens left unparsed");
    return NULL;
  }
  return node;
}

static int node_to_string(skin_node_t* root, char* buf, int buf_size) {
  int ret;

  assert(buf_size > 0);

  // end condition, this is a leaf node
  if ((root->op == SKINOP_NOP) && (root->child == NULL)) {
    // output just the node name which is either literal value or input key
    assert(root->name != NULL);
    int len = strlen(root->name);
    assert(len < buf_size);
    memcpy(buf, root->name, len);  // copy out without null terminator
    return len;
  }  // special case unary operator negate
  else if (root->op == SKINOP_NEGATE) {
    assert(root->child != NULL);
    buf[0] = '-';
    ret = node_to_string(root->child, &buf[1], buf_size - 1);
    if (ret <= 0) {
      return ret;
    }
    return ret + 1;  // add char for - symbol
  } else if ((root->child != NULL) && (root->arg != NULL) && (root->op != SKINOP_NOP)) {
    // binary operator

    int used = 0;
    // first we do the operator function itself (or symbols?)
    buf[used] = '_';
    used += 1;

    int len = strlen(operator_strings[root->op]);
    assert(used + len < buf_size);
    memcpy(&buf[used], operator_strings[root->op], len);
    used += len;

    assert(used + 1 < buf_size);
    buf[used] = '(';
    used++;

    ret = node_to_string(root->child, &buf[used], buf_size - used);
    if (ret <= 0) {  // error case, return
      return ret;
    }
    used += ret;

    assert(used + 1 < buf_size);
    buf[used] = ',';
    used++;

    ret = node_to_string(root->arg, &buf[used], buf_size - used);
    if (ret <= 0) {  // error case, return
      return ret;
    }
    used += ret;

    assert(used + 1 < buf_size);
    buf[used] = ')';
    used++;

    return used;
  }

  register_error("malformed node");
  return -1;
}

/**
 * @brief Takes a node tree and generates a string expression
*/
int expression_generate(skin_node_t* root, char* buf, int buf_size) {
  int len = node_to_string(root, buf, buf_size - 1);
  if (len > 0) {
    buf[len] = '\0';  // null terminate the string
  }
  return len;
}