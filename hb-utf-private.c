// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <string.h>
#include <stdint.h>

#include "hb.h"
#include "hb-private.h"

//------------------------------------------------------------------------------
//utf8
#define HB_UTF8_COMPUTE(Char, Mask, Len) \
  if (Char < 128) { Len = 1; Mask = 0x7f; } \
  else if ((Char & 0xe0) == 0xc0) { Len = 2; Mask = 0x1f; } \
  else if ((Char & 0xf0) == 0xe0) { Len = 3; Mask = 0x0f; } \
  else if ((Char & 0xf8) == 0xf0) { Len = 4; Mask = 0x07; } \
  else Len = 0;

void *
hb_utf_next_utf8(void *text,
                 void *end,
                 hb_codepoint_t *unicode)
{
  uint8_t *text_utf8 = text;
  uint8_t *end_utf8 = end;
  hb_codepoint_t c = *text_utf8, mask;
  unsigned len;

  //TODO check for overlong sequences?

  HB_UTF8_COMPUTE (c, mask, len);
  if (!len || (unsigned) (end_utf8 - text_utf8) < len) {
    *unicode = -1;
    return text_utf8 + 1;
  } else {
    hb_codepoint_t result;
    unsigned i;
    result = c & mask;
    for (i = 1; i < len; i++) {
	if ((text_utf8[i] & 0xc0) != 0x80) {
	    *unicode = -1;
	    return text_utf8 + 1;
        }
	result <<= 6;
	result |= (text_utf8[i] & 0x3f);
    }
    *unicode = result;
    return text_utf8 + len;
  }
}

void *
hb_utf_prev_utf8(void *text,
                 void *start,
                 hb_codepoint_t *unicode)
{
  uint8_t *text_utf8 = text;
  uint8_t *start_utf8 = start;

  uint8_t *end = text_utf8--;
  while (start_utf8 < text_utf8 && (*text_utf8 & 0xc0) == 0x80
                                                         && end - text_utf8 < 4)
    text_utf8--;

  hb_codepoint_t c = *text_utf8, mask;
  unsigned len;

  //TODO check for overlong sequences?

  HB_UTF8_COMPUTE(c, mask, len);
  if (!len || (unsigned)(end - text_utf8) != len) {
    *unicode = -1;
    return end - 1;
  } else {
    hb_codepoint_t result;
    unsigned i;
    result = c & mask;
    for (i = 1; i < len; i++) {
	result <<= 6;
	result |= (text_utf8[i] & 0x3f);
    }
    *unicode = result;
    return text_utf8;
  }
}

void *
hb_utf_ptr_offset_utf8(void *text, unsigned offset)
{
  uint8_t *text_utf8 = text;
  return text_utf8 + offset;
}
 
unsigned
hb_utf_strlen_utf8(void *text)
{
  return strlen(text);
}

unsigned
hb_utf_diff_utf8(void *a, void *b)
{
  uint8_t *a_utf8 = a;
  uint8_t *b_utf8 = b;
  return a_utf8 - b_utf8;
}

//------------------------------------------------------------------------------
//utf16
void *
hb_utf_next_utf16(void *text,
                  void *end,
                  hb_codepoint_t *unicode)
{
  uint16_t *text_utf16 = text;
  uint16_t *end_utf16 = end;

  hb_codepoint_t c = *text_utf16++;

  if (hb_codepoint_in_range(c, 0xd800, 0xdbff)) {
    //high surrogate
    hb_codepoint_t l;
    if (text_utf16 < end_utf16 && ((l = *text_utf16),
                                    hb_codepoint_in_range(l, 0xdc00, 0xdfff))) {
      //low surrogate
      *unicode = (c << 10) + l - ((0xd800 << 10) - 0x10000 + 0xdc00);
       text_utf16++;
    } else
      *unicode = -1;
  } else
    *unicode = c;
  return text_utf16;
}

void *
hb_utf_prev_utf16(void *text,
	          void *start,
	          hb_codepoint_t *unicode)
{
  uint16_t *text_utf16 = text;
  uint16_t *start_utf16 = start;
  hb_codepoint_t c = *--text_utf16;

  if (hb_codepoint_in_range(c, 0xdc00, 0xdfff)) {
    //low surrogate
    hb_codepoint_t h;
    if (start_utf16 < text_utf16
         && ((h = *(text_utf16 - 1)), hb_codepoint_in_range(h, 0xd800, 0xdbff))) {
      //high surrogate
      *unicode = (h << 10) + c - ((0xd800 << 10) - 0x10000 + 0xdc00);
       text_utf16--;
    } else
      *unicode = -1;
  } else
    *unicode = c;
  return text_utf16;
}

void *
hb_utf_ptr_offset_utf16(void *text, unsigned offset)
{
  uint16_t *text_utf16 = text;
  return text_utf16 + offset;
}

unsigned
hb_utf_strlen_utf16(void *text)
{
  uint16_t *text_utf16 = text;
  
  unsigned l = 0;
  while (*text_utf16++) l++;
  return l;
}

unsigned
hb_utf_diff_utf16(void *a, void *b)
{
  uint16_t *a_utf16 = a;
  uint16_t *b_utf16 = b;
  return a_utf16 - b_utf16;
}
//------------------------------------------------------------------------------
//utf32
void *
hb_utf_next_utf32(void *text,
            void *end HB_UNUSED,
            hb_codepoint_t *unicode)
{
  uint32_t *text_utf32 = text;
  *unicode = *text_utf32++;
  return text_utf32;
}

void *
hb_utf_prev_utf32(void *text,
                  void *start HB_UNUSED,
                  hb_codepoint_t *unicode)
{
  uint32_t *text_utf32 = text;
  *unicode = *--text_utf32;
  return text_utf32;
}

void *
hb_utf_ptr_offset_utf32(void *text, unsigned offset)
{
  uint32_t *text_utf32 = text;
  return text_utf32 + offset;
}

unsigned
hb_utf_strlen_utf32(void *text)
{
  uint32_t *text_utf32 = text;

  unsigned l = 0;
  while (*text_utf32++) l++;
  return l;
}

unsigned
hb_utf_diff_utf32(void *a, void *b)
{
  uint32_t *a_utf32 = a;
  uint32_t *b_utf32 = b;
  return a_utf32 - b_utf32;
}
