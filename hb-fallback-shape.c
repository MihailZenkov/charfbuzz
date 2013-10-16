#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-buffer-private.h"
// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include "hb-shaper-private.h"
#include "hb-font-private.h"
#include "hb-unicode-private.h"

//------------------------------------------------------------------------------
//shaper face data
struct hb_fallback_shaper_face_data_t {};

struct hb_fallback_shaper_face_data_t *
hb_fallback_shaper_face_data_create(hb_face_t *face HB_UNUSED)
{
  return HB_SHAPER_DATA_SUCCEEDED;
}

void
hb_fallback_shaper_face_data_destroy(
                          struct hb_fallback_shaper_face_data_t *data HB_UNUSED)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//shaper font data
struct hb_fallback_shaper_font_data_t {};

struct hb_fallback_shaper_font_data_t *
hb_fallback_shaper_font_data_create(hb_font_t *font HB_UNUSED)
{
  return HB_SHAPER_DATA_SUCCEEDED;
}

void
hb_fallback_shaper_font_data_destroy(
                          struct hb_fallback_shaper_font_data_t *data HB_UNUSED)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//shaper shape_plan data
struct hb_fallback_shaper_shape_plan_data_t {};

struct hb_fallback_shaper_shape_plan_data_t *
hb_fallback_shaper_shape_plan_data_create(
                                    hb_shape_plan_t *shape_plan HB_UNUSED,
                                    const hb_feature_t *user_features HB_UNUSED,
                                    unsigned num_user_features HB_UNUSED)
{
  return HB_SHAPER_DATA_SUCCEEDED;
}

void
hb_fallback_shaper_shape_plan_data_destroy(
                    struct hb_fallback_shaper_shape_plan_data_t *data HB_UNUSED)
{
}
//------------------------------------------------------------------------------


//shaper
hb_bool_t
hb_fallback_shape(hb_shape_plan_t *shape_plan HB_UNUSED,
                   hb_font_t *font,
                   hb_buffer_t *buffer,
                   const hb_feature_t *features HB_UNUSED,
                   unsigned num_features HB_UNUSED)
{
  hb_codepoint_t space;
  hb_font_get_glyph(font, ' ', 0, &space);

  hb_buffer_clear_positions(buffer);

  unsigned count = buffer->len;

  for (unsigned i = 0; i < count; i++) {
    if (hb_unicode_is_default_ignorable(buffer->info[i].codepoint)) {
      buffer->info[i].codepoint = space;
      buffer->pos[i].x_advance = 0;
      buffer->pos[i].y_advance = 0;
      continue;
    }
    hb_font_get_glyph(font,
                      buffer->info[i].codepoint,
                      0,
                      &buffer->info[i].codepoint);
    hb_font_get_glyph_advance_for_direction(font,
                                            buffer->info[i].codepoint,
                                            buffer->props.direction,
                                            &buffer->pos[i].x_advance,
                                            &buffer->pos[i].y_advance);
    hb_font_subtract_glyph_origin_for_direction(font,
                                                buffer->info[i].codepoint,
                                                buffer->props.direction,
                                                &buffer->pos[i].x_offset,
                                                &buffer->pos[i].y_offset);
  }

  if (HB_DIRECTION_IS_BACKWARD(buffer->props.direction))
    hb_buffer_reverse(buffer);
  return TRUE;
}
