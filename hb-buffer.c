// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-buffer-private.h"
#include "hb-unicode-private.h"
#include "hb-utf-private.h"

hb_bool_t
hb_segment_properties_equal(const hb_segment_properties_t *a,
                            const hb_segment_properties_t *b)
{
  return a->direction == b->direction &&
         a->script    == b->script    &&
         a->language  == b->language  &&
         a->reserved1 == b->reserved1 &&
         a->reserved2 == b->reserved2;
}

unsigned
hb_buffer_get_length(hb_buffer_t *buffer)
{
  return buffer->len;
}

hb_glyph_info_t *
hb_buffer_get_glyph_infos(hb_buffer_t *buffer, unsigned *length)
{
  if (length)
    *length = buffer->len;
  return buffer->info;
}

hb_glyph_position_t *
hb_buffer_get_glyph_positions(hb_buffer_t *buffer, unsigned *length)
{
  if (!buffer->have_positions)
    hb_buffer_clear_positions(buffer);

  if (length)
    *length = buffer->len;
  return buffer->pos;
}

void
hb_buffer_set_unicode_funcs(hb_buffer_t *buffer,
                            hb_unicode_funcs_t *unicode_funcs)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  if (!unicode_funcs)
    unicode_funcs = hb_unicode_funcs_get_default();

  hb_unicode_funcs_reference(unicode_funcs);
  hb_unicode_funcs_destroy(buffer->unicode);
  buffer->unicode = unicode_funcs;
}

static void
hb_buffer_reverse_range(hb_buffer_t *buffer,
                        unsigned start,
                        unsigned end)
{
  unsigned i, j;

  if (start == end - 1)
    return;

  for (i = start, j = end - 1; i < j; i++, j--) {
    hb_glyph_info_t t;

    t = buffer->info[i];
    buffer->info[i] = buffer->info[j];
    buffer->info[j] = t;
  }

  if (buffer->pos) {
    for (i = start, j = end - 1; i < j; i++, j--) {
      hb_glyph_position_t t;

      t = buffer->pos[i];
      buffer->pos[i] = buffer->pos[j];
      buffer->pos[j] = t;
    }
  }
}

void
hb_buffer_reverse(hb_buffer_t *buffer)
{
  if (!buffer->len)
    return;

  hb_buffer_reverse_range(buffer, 0, buffer->len);
}

void
hb_buffer_set_direction(hb_buffer_t *buffer,
                        hb_direction_t direction)

{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  buffer->props.direction = direction;
}

void
hb_buffer_clear_positions(hb_buffer_t *buffer)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  buffer->have_output = FALSE;
  buffer->have_positions = TRUE;

  buffer->out_len = 0;
  buffer->out_info = buffer->info;

  memset(buffer->pos, 0, sizeof(buffer->pos[0]) * buffer->len);
}

static hb_buffer_t hb_buffer_nil = {
  REF_CNT_INVALID_VAL,//ref_cnt
  &_hb_unicode_funcs_nil,//unicode
  HB_SEGMENT_PROPERTIES_DEFAULT,
  HB_BUFFER_FLAG_DEFAULT,

  HB_BUFFER_CONTENT_TYPE_INVALID,
  FALSE,//in_error
  FALSE,//have_output
  FALSE//have_positions

  //Zero is good enough for everything else.
};

hb_buffer_t *
hb_buffer_get_empty(void)
{
  return &hb_buffer_nil;
}

void
hb_buffer_clear(hb_buffer_t *buffer)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  hb_segment_properties_t default_props = HB_SEGMENT_PROPERTIES_DEFAULT;
  buffer->props = default_props;
  buffer->flags = HB_BUFFER_FLAG_DEFAULT;

  buffer->content_type = HB_BUFFER_CONTENT_TYPE_INVALID;
  buffer->in_error = FALSE;
  buffer->have_output = FALSE;
  buffer->have_positions = FALSE;

  buffer->idx = 0;
  buffer->len = 0;
  buffer->out_len = 0;
  buffer->out_info = buffer->info;

  buffer->serial = 0;
  memset(buffer->allocated_var_bytes, 0, sizeof(buffer->allocated_var_bytes));
  memset(buffer->allocated_var_owner, 0, sizeof(buffer->allocated_var_owner));

  memset(buffer->context, 0, sizeof(buffer->context));
  memset(buffer->context_len, 0, sizeof(buffer->context_len));
}

void
hb_buffer_reset(hb_buffer_t *buffer)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  hb_unicode_funcs_destroy(buffer->unicode);
  buffer->unicode = hb_unicode_funcs_get_default();

  hb_buffer_clear(buffer);
}

void
hb_buffer_destroy(hb_buffer_t *buffer)
{
  if (!buffer)
    return;
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&buffer->ref_cnt, -1);
  if (hb_atomic_int32_get(&buffer->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&buffer->ref_cnt, REF_CNT_INVALID_VAL);

  hb_unicode_funcs_destroy(buffer->unicode);

  free(buffer->info);
  free(buffer->pos);
  free(buffer);
}

hb_buffer_t *
hb_buffer_create(void)
{
  hb_buffer_t *buffer = calloc(1, sizeof(*buffer));
  if (!buffer)
    return hb_buffer_get_empty();

  hb_atomic_int32_set(&buffer->ref_cnt, 1);
  buffer->unicode = hb_unicode_funcs_get_empty();

  hb_buffer_reset(buffer);
  return buffer;
}

void
hb_buffer_set_script(hb_buffer_t *buffer,
                     hb_script_t  script)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  buffer->props.script = script;
}

//Here is how the buffer works internally:
//
//There are two info pointers: info and out_info.  They always have
//the same allocated size, but different lengths.
//
//As an optimization, both info and out_info may point to the
//same piece of memory, which is owned by info.  This remains the
//case as long as out_len doesn't exceed i at any time.
//In that case, swap_buffers() is no-op and the glyph operations operate
//mostly in-place.
//
//As soon as out_info gets longer than info, out_info is moved over
//to an alternate buffer (which we reuse the pos buffer for!), and its
//current contents (out_len entries) are copied to the new place.
//This should all remain transparent to the user.  swap_buffers() then
//switches info and out_info.

static hb_bool_t
enlarge(hb_buffer_t *buffer, unsigned int size)
{
  if (buffer->in_error)
    return FALSE;

  unsigned int new_allocated = buffer->allocated;
  hb_glyph_position_t *new_pos = NULL;
  hb_glyph_info_t *new_info = NULL;
  hb_bool_t separate_out = buffer->out_info != buffer->info;

  if (hb_unsigned_int_mul_overflows(size, sizeof(buffer->info[0])))
    goto done;

  while (size >= new_allocated)
    new_allocated += (new_allocated >> 1) + 32;

  //ASSERT_STATIC (sizeof (info[0]) == sizeof (pos[0]));
  if (hb_unsigned_int_mul_overflows(new_allocated, sizeof(buffer->info[0])))
    goto done;

  new_pos = (hb_glyph_position_t*)realloc(buffer->pos, new_allocated
                                                      * sizeof(buffer->pos[0]));
  new_info = (hb_glyph_info_t*)realloc(buffer->info, new_allocated
                                                     * sizeof(buffer->info[0]));

done:
  if (!new_pos || !new_info)
    buffer->in_error = TRUE;

  if (new_pos)
    buffer->pos = new_pos;

  if (new_info)
    buffer->info = new_info;

  buffer->out_info = separate_out ? (hb_glyph_info_t*)buffer->pos
                                                                 : buffer->info;
  if (!buffer->in_error)
    buffer->allocated = new_allocated;
  return !buffer->in_error;
}

static hb_bool_t
ensure(hb_buffer_t *buffer, unsigned int size)
{
  return (size < buffer->allocated) ? TRUE : enlarge(buffer, size);
}

static void
clear_context(hb_buffer_t *buffer, unsigned int side)
{
  buffer->context_len[side] = 0;
}

static void
add(hb_buffer_t *buffer, hb_codepoint_t codepoint, unsigned int cluster)
{
  hb_glyph_info_t *glyph;

  if (!ensure(buffer, buffer->len + 1)) return;

  glyph = &buffer->info[buffer->len];

  memset(glyph, 0, sizeof(*glyph));
  glyph->codepoint = codepoint;
  glyph->mask = 1;
  glyph->cluster = cluster;

  buffer->len++;
}

struct utf {
  unsigned bytes_n;
  unsigned (*len)(void *text);
  void *(*ptr_offset)(void *text, unsigned offset);
  void *(*prev)(void *text, void *start, hb_codepoint_t *unicode);
  unsigned (*diff)(void *a, void *b);
  void *(*next)(void *text, void *end, hb_codepoint_t *unicode);
};

static struct utf utf8 = {
  sizeof(uint8_t),
  hb_utf_strlen_utf8,
  hb_utf_ptr_offset_utf8,
  hb_utf_prev_utf8,
  hb_utf_diff_utf8,
  hb_utf_next_utf8
};

static HB_UNUSED struct utf utf16 = {
  sizeof(uint16_t),
  hb_utf_strlen_utf16,
  hb_utf_ptr_offset_utf16,
  hb_utf_prev_utf16,
  hb_utf_diff_utf16,
  hb_utf_next_utf16
};

static HB_UNUSED struct utf utf32 = {
  sizeof(uint32_t),
  hb_utf_strlen_utf32,
  hb_utf_ptr_offset_utf32,
  hb_utf_prev_utf32,
  hb_utf_diff_utf32,
  hb_utf_next_utf32
};

//to unroll the original c++, could have used a macro
//ASSEMBLY:maybe worth to be unrolled to fine tuned assembly
static void
hb_buffer_add_utf(hb_buffer_t *buffer,
                  struct utf *utf,
                  void *text,
                  int text_length,
                  unsigned item_offset,
                  int item_length)
{
  assert(buffer->content_type == HB_BUFFER_CONTENT_TYPE_UNICODE ||
      (!buffer->len && buffer->content_type == HB_BUFFER_CONTENT_TYPE_INVALID));

  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  if (text_length == -1)
    text_length = utf->len(text);

  if (item_length == -1)
    item_length = text_length - item_offset;

  ensure(buffer, buffer->len + item_length * utf->bytes_n / 4);

  //If buffer is empty and pre-context provided, install it.
  //This check is written this way, to make sure people can
  //provide pre-context in one add_utf() call, then provide
  //text in a follow-up call.  See:
  //
  //https://bugzilla.mozilla.org/show_bug.cgi?id=801410#c13
  if (!buffer->len && item_offset > 0) {
    //Add pre-context
    clear_context(buffer, 0);
    void *prev = utf->ptr_offset(text, item_offset);
    void *start = text;
    while (start < prev && buffer->context_len[0] < HB_BUFFER_CONTEXT_LENGTH) {
      hb_codepoint_t u;
      prev = utf->prev(prev, start, &u);
      buffer->context[0][buffer->context_len[0]++] = u;
    }
  }

  void *next = utf->ptr_offset(text, item_offset);
  void *end = utf->ptr_offset(next, item_length);
  while (next < end) {
    hb_codepoint_t u;
    void *old_next = next;
    next = utf->next(next, end, &u);
    add(buffer, u, utf->diff(old_next, text));
  }

  //Add post-context
  clear_context(buffer, 1);
  end = utf->ptr_offset(text, text_length);
  end = text + text_length;
  while (next < end && buffer->context_len[1] < HB_BUFFER_CONTEXT_LENGTH) {
    hb_codepoint_t u;
    next = utf->next(next, end, &u);
    buffer->context[1][buffer->context_len[1]++] = u;
  }

  buffer->content_type = HB_BUFFER_CONTENT_TYPE_UNICODE;
}

void
hb_buffer_add_utf8(hb_buffer_t *buffer,
                   const char *text,
                   int text_length,
                   unsigned int item_offset,
                   int item_length)
{
  hb_buffer_add_utf(buffer,
                    &utf8,
                    (void*)text,
                    text_length,
                    item_offset,
                    item_length);
}

void
hb_buffer_set_flags(hb_buffer_t *buffer,
                    hb_buffer_flags_t flags)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  buffer->flags = flags;
}

void
hb_buffer_set_language(hb_buffer_t *buffer,
                       hb_language_t language)
{
  if (hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  buffer->props.language = language;
}

hb_direction_t
hb_buffer_get_direction(hb_buffer_t *buffer)
{
  return buffer->props.direction;
}

void
hb_buffer_add(hb_buffer_t *buffer,
	      hb_codepoint_t codepoint,
	      unsigned int cluster)
{
  add(buffer, codepoint, cluster);
  clear_context(buffer, 1);
}
