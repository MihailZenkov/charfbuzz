#ifndef HB_UNICODE_PRIVATE_H
#define HB_UNICODE_PRIVATE_H
extern HB_INTERNAL hb_unicode_funcs_t _hb_unicode_funcs_nil;

struct hb_unicode_funcs_t {
  atomic_int32_t ref_cnt;
  hb_unicode_funcs_t *parent;
  hb_bool_t immutable;

  struct {
    hb_unicode_combining_class_func_t combining_class;
    hb_unicode_eastasian_width_func_t eastasian_width;
    hb_unicode_general_category_func_t general_category;
    hb_unicode_mirroring_func_t mirroring;
    hb_unicode_script_func_t script;
    hb_unicode_compose_func_t compose;
    hb_unicode_decompose_func_t decompose;
    hb_unicode_decompose_compatibility_func_t decompose_compatibility;
  } func;

  struct {
    void *combining_class;
    void *eastasian_width;
    void *general_category;
    void *mirroring;
    void *script;
    void *compose;
    void *decompose;
    void *decompose_compatibility;
  } user_data;

  struct {
    hb_destroy_func_t combining_class;
    hb_destroy_func_t eastasian_width;
    hb_destroy_func_t general_category;
    hb_destroy_func_t mirroring;
    hb_destroy_func_t script;
    hb_destroy_func_t compose;
    hb_destroy_func_t decompose;
    hb_destroy_func_t decompose_compatibility;
  } destroy;
};
hb_bool_t
hb_unicode_is_default_ignorable(hb_codepoint_t ch);
#endif
