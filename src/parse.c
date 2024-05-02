#include <cyaml/cyaml.h>
#include <skin.h>
#include <stdlib.h>

typedef struct texture {
  char* filepath;
  char* r;
  char* g;
  char* b;
  char* a;
} texture_t;
static const cyaml_schema_field_t texture_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("filepath", CYAML_FLAG_POINTER, texture_t, filepath, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("r", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, texture_t, r, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("g", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, texture_t, g, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("b", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, texture_t, b, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("a", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, texture_t, a, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_END};

typedef struct vertex {
  char* x;
  char* y;
} vertex_t;
static const cyaml_schema_field_t vertex_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("x", CYAML_FLAG_POINTER, vertex_t, x, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("y", CYAML_FLAG_POINTER, vertex_t, y, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_END};

typedef struct item {
  char* name;
  char* x;
  char* y;
  char* w;
  char* h;
  texture_t texture;
} item_t;
static const cyaml_schema_field_t item_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER, item_t, name, 0, MAX_NAME_LENGTH),
    CYAML_FIELD_STRING_PTR("x", CYAML_FLAG_POINTER, item_t, x, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("y", CYAML_FLAG_POINTER, item_t, y, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("w", CYAML_FLAG_POINTER, item_t, w, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("h", CYAML_FLAG_POINTER, item_t, h, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_MAPPING("texture", CYAML_FLAG_DEFAULT, item_t, texture, texture_fields_schema),
    CYAML_FIELD_END};

typedef struct shader {
  char* vert_path;
  char* frag_path;
} shader_t;
static const cyaml_schema_field_t shader_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("vert_path", CYAML_FLAG_POINTER, shader_t, vert_path, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("frag_path", CYAML_FLAG_POINTER, shader_t, frag_path, 0,
                           MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_END};

typedef struct offset {
  char* x;
  char* y;
} offset_t;
static const cyaml_schema_field_t offset_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("x", CYAML_FLAG_POINTER, offset_t, x, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("y", CYAML_FLAG_POINTER, offset_t, y, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_END};

typedef struct mask {
  char* x;
  char* y;
  char* w;
  char* h;
} mask_t;
static const cyaml_schema_field_t mask_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("x", CYAML_FLAG_POINTER, mask_t, x, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("y", CYAML_FLAG_POINTER, mask_t, y, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("w", CYAML_FLAG_POINTER, mask_t, w, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_STRING_PTR("h", CYAML_FLAG_POINTER, mask_t, h, 0, MAX_EXPRESSION_LENGTH),
    CYAML_FIELD_END};

typedef struct layer {
  char* name;
  skin_item_t items[MAX_ITEMS];
  unsigned num_items;
  skin_shader_t shader;
  offset_t offset;
  mask_t mask;
} layer_t;
static const cyaml_schema_field_t layer_fields_schema[] = {
    CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER, layer_t, name, 0, MAX_NAME_LENGTH),

    CYAML_FIELD_SEQUENCE("items", CYAML_FLAG_POINTER | CYAML_FLAG_OPTIONAL, layer_t, items,
                         num_items, &item_fields_schema, 0, CYAML_UNLIMITED),

    CYAML_FIELD_MAPPING("shader", CYAML_FLAG_DEFAULT, layer_t, shader, shader_fields_schema),

    CYAML_FIELD_MAPPING("offset", CYAML_FLAG_DEFAULT, layer_t, offset, offset_fields_schema),

    CYAML_FIELD_MAPPING("mask", CYAML_FLAG_DEFAULT, layer_t, mask, mask_fields_schema),
    CYAML_FIELD_END};