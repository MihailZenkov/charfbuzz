#ifndef HB_UTF_PRIVATE_H
#define HB_UTF_PRIVATE_H
//------------------------------------------------------------------------------
//utf8
void *
hb_utf_next_utf8(void *text,
                 void *end,
                 hb_codepoint_t *unicode);
void *
hb_utf_prev_utf8(void *text,
                 void *start,
                 hb_codepoint_t *unicode);
void *
hb_utf_ptr_offset_utf8(void *text, unsigned offset);
unsigned
hb_utf_strlen_utf8(void *text);
unsigned
hb_utf_diff_utf8(void *a, void *b);

//------------------------------------------------------------------------------
//utf16
void *
hb_utf_next_utf16(void *text,
                  void *end,
                  hb_codepoint_t *unicode);
void *
hb_utf_prev_utf16(void *text,
	          void *start,
	          hb_codepoint_t *unicode);
void *
hb_utf_ptr_offset_utf16(void *text, unsigned offset);
unsigned
hb_utf_strlen_utf16(void *text);
unsigned
hb_utf_diff_utf16(void *a, void *b);

//------------------------------------------------------------------------------
//utf32
void *
hb_utf_next_utf32(void *text,
            void *end HB_UNUSED,
            hb_codepoint_t *unicode);
void *
hb_utf_prev_utf32(void *text,
                  void *start HB_UNUSED,
                  hb_codepoint_t *unicode);
void *
hb_utf_ptr_offset_utf32(void *text, unsigned offset);
unsigned
hb_utf_strlen_utf32(void *text);
unsigned
hb_utf_diff_utf32(void *a, void *b);
#endif
