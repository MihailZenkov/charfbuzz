#ifndef HB_BUFFER_PRIVATE_H
#define HB_BUFFER_PRIVATE_H
struct hb_buffer_t {
  atomic_int32_t ref_cnt;
  hb_unicode_funcs_t *unicode;//Unicode functions
  hb_segment_properties_t props;//Script, language, direction
  hb_buffer_flags_t flags;//BOT / EOT / etc.

  //Buffer contents

  hb_buffer_content_type_t content_type;

  hb_bool_t in_error;//Allocation failed
  hb_bool_t have_output;//Whether we have an output buffer going on
  hb_bool_t have_positions;//Whether we have positions

  unsigned idx;//Cursor into ->info and ->pos arrays
  unsigned len;//Length of ->info and ->pos arrays
  unsigned out_len;//Length of ->out array if have_output

  unsigned allocated;//Length of allocated arrays
  hb_glyph_info_t *info;
  hb_glyph_info_t *out_info;
  hb_glyph_position_t *pos;

  unsigned serial;

  //These reflect current allocations of the bytes in glyph_info_t's var1 and
  //var2.
  uint8_t allocated_var_bytes[8];
  const char *allocated_var_owner[8];

  //Text before / after the main buffer contents.
  //Always in Unicode, and ordered outward.
  //Index 0 is for "pre-context", 1 for "post-context".
  #define HB_BUFFER_CONTEXT_LENGTH 5
  hb_codepoint_t context[2][HB_BUFFER_CONTEXT_LENGTH];
  unsigned context_len[2];
};

void
hb_buffer_clear_positions(hb_buffer_t *buffer);
#endif
