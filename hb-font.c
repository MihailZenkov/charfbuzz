// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-shaper-private.h"
#include "hb-face-private.h"
#include "hb-font-private.h"

void
hb_font_destroy(hb_font_t *font)
{
  if (!font)
    return;
  if (hb_atomic_int32_get(&font->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&font->ref_cnt, -1);
  if (hb_atomic_int32_get(&font->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&font->ref_cnt, REF_CNT_INVALID_VAL);

#ifdef HAVE_GRAPHITE2
  if (font->shaper_data.graphite2
      && font->shaper_data.graphite2 != HB_SHAPER_DATA_INVALID
      && font->shaper_data.graphite2 != HB_SHAPER_DATA_SUCCEEDED)
    hb_graphite2_shaper_font_data_destroy(font->shaper_data.graphite2);
#endif
#ifdef HAVE_OT
  if (font->shaper_data.ot
      && font->shaper_data.ot != HB_SHAPER_DATA_INVALID
      && font->shaper_data.ot != HB_SHAPER_DATA_SUCCEEDED)
    hb_ot_shaper_font_data_destroy(font->shaper_data.ot);
#endif
  if (font->shaper_data.fallback
      && font->shaper_data.fallback != HB_SHAPER_DATA_INVALID
      && font->shaper_data.fallback != HB_SHAPER_DATA_SUCCEEDED)
    hb_fallback_shaper_font_data_destroy(font->shaper_data.fallback);

  if (font->destroy)
    font->destroy(font->user_data);

  hb_font_destroy(font->parent);
  hb_face_destroy(font->face);
  hb_font_funcs_destroy(font->klass);
  free(font);
}

hb_bool_t
hb_font_get_glyph(hb_font_t *font,
                  hb_codepoint_t unicode,
                  hb_codepoint_t variation_selector,
                  hb_codepoint_t *glyph)
{
  *glyph = 0;
  return font->klass->get.glyph(font,
                                font->user_data,
                                unicode,
                                variation_selector,
                                glyph,
                                font->klass->user_data.glyph);
}

//Convert from parent-font user-space to our user-space
hb_position_t
hb_font_parent_scale_x_distance(hb_font_t *font,
                                hb_position_t v)
{
  if (font->parent && font->parent->x_scale != font->x_scale)
    return v * (int64_t) font->x_scale / font->parent->x_scale;
  return v;
}

hb_position_t
hb_font_parent_scale_y_distance(hb_font_t *font,
                                hb_position_t v)
{
  if (font->parent && font->parent->y_scale != font->y_scale)
    return v * (int64_t) font->y_scale / font->parent->y_scale;
  return v;
}

hb_position_t
hb_font_parent_scale_x_position(hb_font_t *font,
                                hb_position_t v)
{
  return hb_font_parent_scale_x_distance(font, v);
}

hb_position_t
hb_font_parent_scale_y_position(hb_font_t *font,
                                hb_position_t v)
{
  return hb_font_parent_scale_y_distance(font, v);
}

void
hb_font_parent_scale_position(hb_font_t *font,
                              hb_position_t *x,
                              hb_position_t *y)
{
  *x = hb_font_parent_scale_x_position(font, *x);
  *y = hb_font_parent_scale_y_position(font, *y);
}

void
hb_font_parent_scale_distance(hb_font_t *font,
                              hb_position_t *x,
                              hb_position_t *y)
{
  *x = hb_font_parent_scale_x_distance(font, *x);
  *y = hb_font_parent_scale_y_distance(font, *y);
}

hb_position_t
hb_font_get_glyph_h_advance(hb_font_t *font,
                            hb_codepoint_t glyph)
{
  return font->klass->get.glyph_h_advance(
                                        font,
                                        font->user_data,
                                        glyph,
                                        font->klass->user_data.glyph_h_advance);
}


hb_position_t
hb_font_get_glyph_v_advance(hb_font_t *font,
                            hb_codepoint_t glyph)
{
  return font->klass->get.glyph_v_advance(
                                        font,
                                        font->user_data,
                                        glyph,
                                        font->klass->user_data.glyph_v_advance);
}

hb_bool_t
hb_font_get_glyph_h_origin(hb_font_t *font,
                           hb_codepoint_t glyph,
                           hb_position_t *x,
                           hb_position_t *y)
{
  *x = *y = 0;
  return font->klass->get.glyph_h_origin(font,
                                         font->user_data,
                                         glyph,
                                         x, y,
                                         font->klass->user_data.glyph_h_origin);
}

hb_bool_t
hb_font_get_glyph_v_origin(hb_font_t *font,
                           hb_codepoint_t glyph,
                           hb_position_t *x,
                           hb_position_t *y)
{
  *x = *y = 0;
  return font->klass->get.glyph_v_origin(font,
                                         font->user_data,
                                         glyph,
                                         x, y,
                                         font->klass->user_data.glyph_v_origin);
}

hb_position_t
hb_font_get_glyph_h_kerning(hb_font_t *font,
                            hb_codepoint_t left_glyph,
                            hb_codepoint_t right_glyph)
{
  return font->klass->get.glyph_h_kerning(
                                        font,
                                        font->user_data,
                                        left_glyph,
                                        right_glyph,
                                        font->klass->user_data.glyph_h_kerning);
}

hb_position_t
hb_font_get_glyph_v_kerning(hb_font_t *font,
                            hb_codepoint_t top_glyph,
                            hb_codepoint_t bottom_glyph)
{
  return font->klass->get.glyph_v_kerning(
                                        font,
                                        font->user_data,
                                        top_glyph,
                                        bottom_glyph,
                                        font->klass->user_data.glyph_v_kerning);
}

hb_bool_t
hb_font_get_glyph_extents(hb_font_t *font,
                          hb_codepoint_t glyph,
                          hb_glyph_extents_t *extents)
{
  memset(extents, 0, sizeof(*extents));
  return font->klass->get.glyph_extents(font,
                                        font->user_data,
                                        glyph,
                                        extents,
                                        font->klass->user_data.glyph_extents);
}

hb_bool_t
hb_font_get_glyph_contour_point(hb_font_t *font,
                                hb_codepoint_t glyph,
                                unsigned int point_index,
                                hb_position_t *x,
                                hb_position_t *y)
{
  *x = *y = 0;
  return font->klass->get.glyph_contour_point(
                                    font,
                                    font->user_data,
                                    glyph, point_index,
                                    x, y,
                                    font->klass->user_data.glyph_contour_point);
}

hb_bool_t
hb_font_get_glyph_name(hb_font_t *font,
                       hb_codepoint_t glyph,
                       char *name,
                       unsigned int size)
{
  if (size) *name = '\0';
  return font->klass->get.glyph_name(font,
                                     font->user_data,
                                     glyph,
                                     name,
                                     size,
                                     font->klass->user_data.glyph_name);
}

hb_bool_t
hb_font_get_glyph_from_name(hb_font_t *font,
                            const char *name,
                            int len,//-1 means nul-terminated
                            hb_codepoint_t *glyph)
{
  *glyph = 0;
  if (len == -1) len = strlen(name);
  return font->klass->get.glyph_from_name(
                                        font,
                                        font->user_data,
                                        name,
                                        len,
                                        glyph,
                                        font->klass->user_data.glyph_from_name);
}

static hb_bool_t
hb_font_get_glyph_nil(hb_font_t *font,
                      void *font_data HB_UNUSED,
                      hb_codepoint_t unicode,
                      hb_codepoint_t variation_selector,
                      hb_codepoint_t *glyph,
                      void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_get_glyph(font->parent,
                             unicode,
                             variation_selector,
                             glyph);

  *glyph = 0;
  return FALSE;
}

static hb_position_t
hb_font_get_glyph_h_advance_nil(hb_font_t *font,
                                void *font_data HB_UNUSED,
                                hb_codepoint_t glyph,
                                void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_parent_scale_x_distance(font, hb_font_get_glyph_h_advance(
                                                                   font->parent,
                                                                   glyph));
  return font->x_scale;
}

static hb_position_t
hb_font_get_glyph_v_advance_nil(hb_font_t *font,
                                void *font_data HB_UNUSED,
                                hb_codepoint_t glyph,
                                void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_parent_scale_y_distance(font, hb_font_get_glyph_v_advance(
                                                                   font->parent,
                                                                   glyph));
  return font->y_scale;
}

static hb_bool_t
hb_font_get_glyph_h_origin_nil(hb_font_t *font,
                               void *font_data HB_UNUSED,
                               hb_codepoint_t glyph,
                               hb_position_t *x,
                               hb_position_t *y,
                               void *user_data HB_UNUSED)
{
  if (font->parent) {
    hb_bool_t ret = hb_font_get_glyph_h_origin(font->parent, glyph, x, y);
    if (ret)
      hb_font_parent_scale_position(font, x, y);
    return ret;
  }

  *x = *y = 0;
  return FALSE;
}

static hb_bool_t
hb_font_get_glyph_v_origin_nil(hb_font_t *font,
                               void *font_data HB_UNUSED,
                               hb_codepoint_t glyph,
                               hb_position_t *x,
                               hb_position_t *y,
                               void *user_data HB_UNUSED)
{
  if (font->parent) {
    hb_bool_t ret = hb_font_get_glyph_v_origin(font->parent, glyph, x, y);
    if (ret)
      hb_font_parent_scale_position(font, x, y);
    return ret;
  }

  *x = *y = 0;
  return FALSE;
}

static hb_position_t
hb_font_get_glyph_h_kerning_nil(hb_font_t *font,
                                void *font_data HB_UNUSED,
                                hb_codepoint_t left_glyph,
                                hb_codepoint_t right_glyph,
                                void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_parent_scale_x_distance(font, hb_font_get_glyph_h_kerning(
                                                                  font->parent,
                                                                  left_glyph,
                                                                  right_glyph));
  return 0;
}

static hb_position_t
hb_font_get_glyph_v_kerning_nil(hb_font_t *font,
                                void *font_data HB_UNUSED,
                                hb_codepoint_t top_glyph,
                                hb_codepoint_t bottom_glyph,
                                void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_parent_scale_y_distance(font, hb_font_get_glyph_v_kerning(
                                                                 font->parent,
                                                                 top_glyph,
                                                                 bottom_glyph));
  return 0;
}

static hb_bool_t
hb_font_get_glyph_extents_nil(hb_font_t *font,
                              void *font_data HB_UNUSED,
                              hb_codepoint_t glyph,
                              hb_glyph_extents_t *extents,
                              void *user_data HB_UNUSED)
{
  if (font->parent) {
    hb_bool_t ret = hb_font_get_glyph_extents(font->parent, glyph, extents);
    if (ret) {
      hb_font_parent_scale_position(font,
                                    &extents->x_bearing,
                                    &extents->y_bearing);
      hb_font_parent_scale_distance(font, &extents->width, &extents->height);
    }
    return ret;
  }

  memset(extents, 0, sizeof(*extents));
  return FALSE;
}

static hb_bool_t
hb_font_get_glyph_contour_point_nil(hb_font_t *font,
                                    void *font_data HB_UNUSED,
                                    hb_codepoint_t glyph,
                                    unsigned int point_index,
                                    hb_position_t *x,
                                    hb_position_t *y,
                                    void *user_data HB_UNUSED)
{
  if (font->parent) {
    hb_bool_t ret = hb_font_get_glyph_contour_point(font->parent,
                                                    glyph,
                                                    point_index,
                                                    x, y);
    if (ret)
      hb_font_parent_scale_position(font, x, y);
    return ret;
  }

  *x = *y = 0;
  return FALSE;
}

static hb_bool_t
hb_font_get_glyph_name_nil(hb_font_t *font,
                           void *font_data HB_UNUSED,
                           hb_codepoint_t glyph,
                           char *name, unsigned int size,
                           void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_get_glyph_name(font->parent, glyph, name, size);

  if (size) *name = '\0';
  return FALSE;
}

static hb_bool_t
hb_font_get_glyph_from_name_nil(hb_font_t *font,
                                void *font_data HB_UNUSED,
                                const char *name,
                                int len,//-1 means nul-terminated
                                hb_codepoint_t *glyph,
                                void *user_data HB_UNUSED)
{
  if (font->parent)
    return hb_font_get_glyph_from_name(font->parent, name, len, glyph);

  *glyph = 0;
  return FALSE;
}

//A bit higher-level, and with fallback

void
hb_font_get_glyph_advance_for_direction(hb_font_t *font,
                                        hb_codepoint_t glyph,
                                        hb_direction_t direction,
                                        hb_position_t *x, hb_position_t *y)
{
  if (HB_DIRECTION_IS_HORIZONTAL(direction)) {
    *x = hb_font_get_glyph_h_advance(font, glyph);
    *y = 0;
  } else {
    *x = 0;
    *y = hb_font_get_glyph_v_advance(font, glyph);
  }
}

//Internal only
static void
hb_font_guess_v_origin_minus_h_origin(hb_font_t *font,
                                      hb_codepoint_t glyph,
                                      hb_position_t *x, hb_position_t *y)
{
  *x = hb_font_get_glyph_h_advance(font, glyph) / 2;

  //TODO:use font_metics.ascent
  *y = font->y_scale;
}

void
hb_font_get_glyph_origin_for_direction(hb_font_t *font,
                                       hb_codepoint_t glyph,
                                       hb_direction_t direction,
                                       hb_position_t *x, hb_position_t *y)
{
  if (HB_DIRECTION_IS_HORIZONTAL(direction)) {
    if (!hb_font_get_glyph_h_origin(font, glyph, x, y)
                             && hb_font_get_glyph_v_origin(font, glyph, x, y)) {
      hb_position_t dx, dy;
      hb_font_guess_v_origin_minus_h_origin(font, glyph, &dx, &dy);
      *x -= dx; *y -= dy;
    }
  } else {
    if (!hb_font_get_glyph_v_origin(font, glyph, x, y)
                             && hb_font_get_glyph_h_origin(font, glyph, x, y)) {
      hb_position_t dx, dy;
      hb_font_guess_v_origin_minus_h_origin(font, glyph, &dx, &dy);
      *x += dx; *y += dy;
    }
  }
}

void
hb_font_subtract_glyph_origin_for_direction(hb_font_t *font,
                                            hb_codepoint_t glyph,
                                            hb_direction_t direction,
                                            hb_position_t *x, hb_position_t *y)
{
  hb_position_t origin_x, origin_y;

  hb_font_get_glyph_origin_for_direction(font, glyph, direction, &origin_x,
                                                                     &origin_y);

  *x -= origin_x;
  *y -= origin_y;
}

static hb_font_funcs_t hb_font_funcs_nil = {
  REF_CNT_INVALID_VAL,//ref_cnt
  TRUE,//immutable
  {//get
    hb_font_get_glyph_nil,
    hb_font_get_glyph_h_advance_nil,
    hb_font_get_glyph_v_advance_nil,
    hb_font_get_glyph_h_origin_nil,
    hb_font_get_glyph_v_origin_nil,
    hb_font_get_glyph_h_kerning_nil,
    hb_font_get_glyph_v_kerning_nil,
    hb_font_get_glyph_extents_nil,
    hb_font_get_glyph_contour_point_nil,
    hb_font_get_glyph_name_nil,
    hb_font_get_glyph_from_name_nil,
  },
  {//user_data
    NULL,//glyph
    NULL,//glyph_h_advance
    NULL,//glyph_v_advance
    NULL,//glyph_h_origin
    NULL,//glyph_v_origin
    NULL,//glyph_h_kerning
    NULL,//glyph_v_kerning
    NULL,//glyph_extents
    NULL,//glyph_contour_po
    NULL,//glyph_name
    NULL,//glyph_from_name
  },
  {//destroy
    NULL,//glyph
    NULL,//glyph_h_advance
    NULL,//glyph_v_advance
    NULL,//glyph_h_origin
    NULL,//glyph_v_origin
    NULL,//glyph_h_kerning
    NULL,//glyph_v_kerning
    NULL,//glyph_extents
    NULL,//glyph_contour_po
    NULL,//glyph_name
    NULL,//glyph_from_name
  }
};

static hb_font_t hb_font_nil = {
  REF_CNT_INVALID_VAL,//ref_cnt
  TRUE,//immutable
  NULL,//parent
  NULL,//face
  0,//x_scale
  0,//y_scale
  0,//x_ppem
  0,//y_ppem
  NULL,//klass
  NULL,//user_data
  NULL,//destroy
  {
#ifdef HAVE_GRAPHITE2
    HB_SHAPER_DATA_INVALID,
#endif
#ifdef HAVE_OT
    HB_SHAPER_DATA_INVALID,
#endif
    HB_SHAPER_DATA_INVALID//fallback
  },
};

hb_font_funcs_t *
hb_font_funcs_get_empty(void)
{
  return &hb_font_funcs_nil;
}

hb_font_t *
hb_font_get_empty(void)
{
  return &hb_font_nil;
}

void
hb_font_funcs_destroy(hb_font_funcs_t *ffuncs)
{
  if (!ffuncs)
    return;
  if (hb_atomic_int32_get(&ffuncs->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&ffuncs->ref_cnt, -1);
  if (hb_atomic_int32_get(&ffuncs->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&ffuncs->ref_cnt, REF_CNT_INVALID_VAL);

  if (ffuncs->destroy.glyph)
    ffuncs->destroy.glyph(ffuncs->user_data.glyph);
  if (ffuncs->destroy.glyph_h_advance)
    ffuncs->destroy.glyph_h_advance(ffuncs->user_data.glyph_h_advance);
  if (ffuncs->destroy.glyph_v_advance)
    ffuncs->destroy.glyph_v_advance(ffuncs->user_data.glyph_v_advance);
  if (ffuncs->destroy.glyph_h_origin)
    ffuncs->destroy.glyph_h_origin(ffuncs->user_data.glyph_h_origin);
  if (ffuncs->destroy.glyph_v_origin)
    ffuncs->destroy.glyph_v_origin(ffuncs->user_data.glyph_v_origin);
  if (ffuncs->destroy.glyph_h_kerning)
    ffuncs->destroy.glyph_h_kerning(ffuncs->user_data.glyph_h_kerning);
  if (ffuncs->destroy.glyph_v_kerning)
    ffuncs->destroy.glyph_v_kerning(ffuncs->user_data.glyph_v_kerning);
  if (ffuncs->destroy.glyph_extents)
    ffuncs->destroy.glyph_extents(ffuncs->user_data.glyph_extents);
  if (ffuncs->destroy.glyph_contour_point)
    ffuncs->destroy.glyph_contour_point(ffuncs->user_data.glyph_contour_point);
  if (ffuncs->destroy.glyph_name)
    ffuncs->destroy.glyph_name(ffuncs->user_data.glyph_name);
  if (ffuncs->destroy.glyph_from_name)
    ffuncs->destroy.glyph_from_name(ffuncs->user_data.glyph_from_name);

  free(ffuncs);
}

hb_font_t *
hb_font_create(hb_face_t *face)
{
  if (!face)
    face = hb_face_get_empty();

  if (face->ref_cnt == REF_CNT_INVALID_VAL)
    return hb_font_get_empty();

  hb_font_t *font = calloc(1, sizeof(*font));
  if (!font)
    return hb_font_get_empty();

  hb_atomic_int32_set(&font->ref_cnt, 1);
  hb_face_make_immutable(face);
  font->face = hb_face_reference(face);
  font->klass = hb_font_funcs_get_empty();
  return font;
}

void 
hb_font_funcs_set_glyph_func(hb_font_funcs_t *ffuncs,
                             hb_font_get_glyph_func_t func,
                             void *user_data,
                             hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }
  
  if (ffuncs->destroy.glyph)
    ffuncs->destroy.glyph(ffuncs->user_data.glyph);

  if (func) {
    ffuncs->get.glyph = func;
    ffuncs->user_data.glyph = user_data;
    ffuncs->destroy.glyph = destroy;
  } else {
    ffuncs->get.glyph = hb_font_get_glyph_nil;
    ffuncs->user_data.glyph = NULL;
    ffuncs->destroy.glyph = NULL;
  }
}

void 
hb_font_funcs_set_glyph_h_advance_func(hb_font_funcs_t *ffuncs,
                                       hb_font_get_glyph_h_advance_func_t func,
                                       void *user_data,
                                       hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_h_advance)
    ffuncs->destroy.glyph_h_advance(ffuncs->user_data.glyph_h_advance);

  if (func) {
    ffuncs->get.glyph_h_advance = func;
    ffuncs->user_data.glyph_h_advance = user_data;
    ffuncs->destroy.glyph_h_advance = destroy;
  } else {
    ffuncs->get.glyph_h_advance = hb_font_get_glyph_h_advance_nil;
    ffuncs->user_data.glyph_h_advance = NULL;
    ffuncs->destroy.glyph_h_advance = NULL;
  }
}

void 
hb_font_funcs_set_glyph_v_advance_func(hb_font_funcs_t *ffuncs,
                                       hb_font_get_glyph_v_advance_func_t func,
                                       void *user_data,
                                       hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_v_advance)
    ffuncs->destroy.glyph_v_advance(ffuncs->user_data.glyph_v_advance);

  if (func) {
    ffuncs->get.glyph_v_advance = func;
    ffuncs->user_data.glyph_v_advance = user_data;
    ffuncs->destroy.glyph_v_advance = destroy;
  } else {
    ffuncs->get.glyph_v_advance = hb_font_get_glyph_v_advance_nil;
    ffuncs->user_data.glyph_v_advance = NULL;
    ffuncs->destroy.glyph_v_advance = NULL;
  }
}

void 
hb_font_funcs_set_glyph_h_origin_func(hb_font_funcs_t *ffuncs,
                                      hb_font_get_glyph_h_origin_func_t func,
                                      void *user_data,
                                      hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_h_origin)
    ffuncs->destroy.glyph_h_origin(ffuncs->user_data.glyph_h_origin);

  if (func) {
    ffuncs->get.glyph_h_origin = func;
    ffuncs->user_data.glyph_h_origin = user_data;
    ffuncs->destroy.glyph_h_origin = destroy;
  } else {
    ffuncs->get.glyph_h_origin = hb_font_get_glyph_h_origin_nil;
    ffuncs->user_data.glyph_h_origin = NULL;
    ffuncs->destroy.glyph_h_origin = NULL;
  }
}

void 
hb_font_funcs_set_glyph_v_origin_func(hb_font_funcs_t *ffuncs,
                                      hb_font_get_glyph_v_origin_func_t func,
                                      void *user_data,
                                      hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_v_origin)
    ffuncs->destroy.glyph_v_origin(ffuncs->user_data.glyph_v_origin);

  if (func) {
    ffuncs->get.glyph_v_origin = func;
    ffuncs->user_data.glyph_v_origin = user_data;
    ffuncs->destroy.glyph_v_origin = destroy;
  } else {
    ffuncs->get.glyph_v_origin = hb_font_get_glyph_v_origin_nil;
    ffuncs->user_data.glyph_v_origin = NULL;
    ffuncs->destroy.glyph_v_origin = NULL;
  }
}

void 
hb_font_funcs_set_glyph_h_kerning_func(hb_font_funcs_t *ffuncs,
                                       hb_font_get_glyph_h_kerning_func_t func,
                                       void *user_data,
                                       hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_h_kerning)
    ffuncs->destroy.glyph_h_kerning(ffuncs->user_data.glyph_h_kerning);

  if (func) {
    ffuncs->get.glyph_h_kerning = func;
    ffuncs->user_data.glyph_h_kerning = user_data;
    ffuncs->destroy.glyph_h_kerning = destroy;
  } else {
    ffuncs->get.glyph_h_kerning = hb_font_get_glyph_h_kerning_nil;
    ffuncs->user_data.glyph_h_kerning = NULL;
    ffuncs->destroy.glyph_h_kerning = NULL;
  }
}

void 
hb_font_funcs_set_glyph_v_kerning_func(hb_font_funcs_t *ffuncs,
                                       hb_font_get_glyph_v_kerning_func_t func,
                                       void *user_data,
                                       hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_v_kerning)
    ffuncs->destroy.glyph_v_kerning(ffuncs->user_data.glyph_v_kerning);

  if (func) {
    ffuncs->get.glyph_v_kerning = func;
    ffuncs->user_data.glyph_v_kerning = user_data;
    ffuncs->destroy.glyph_v_kerning = destroy;
  } else {
    ffuncs->get.glyph_v_kerning = hb_font_get_glyph_v_kerning_nil;
    ffuncs->user_data.glyph_v_kerning = NULL;
    ffuncs->destroy.glyph_v_kerning = NULL;
  }
}

void 
hb_font_funcs_set_glyph_extents_func(hb_font_funcs_t *ffuncs,
                                     hb_font_get_glyph_extents_func_t func,
                                     void *user_data,
                                     hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_extents)
    ffuncs->destroy.glyph_extents(ffuncs->user_data.glyph_extents);

  if (func) {
    ffuncs->get.glyph_extents = func;
    ffuncs->user_data.glyph_extents = user_data;
    ffuncs->destroy.glyph_extents = destroy;
  } else {
    ffuncs->get.glyph_extents = hb_font_get_glyph_extents_nil;
    ffuncs->user_data.glyph_extents = NULL;
    ffuncs->destroy.glyph_extents = NULL;
  }
}

void 
hb_font_funcs_set_glyph_contour_point_func(
                                    hb_font_funcs_t *ffuncs,
                                    hb_font_get_glyph_contour_point_func_t func,
                                    void *user_data,
                                    hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_contour_point)
    ffuncs->destroy.glyph_contour_point(ffuncs->user_data.glyph_contour_point);

  if (func) {
    ffuncs->get.glyph_contour_point = func;
    ffuncs->user_data.glyph_contour_point = user_data;
    ffuncs->destroy.glyph_contour_point = destroy;
  } else {
    ffuncs->get.glyph_contour_point = hb_font_get_glyph_contour_point_nil;
    ffuncs->user_data.glyph_contour_point = NULL;
    ffuncs->destroy.glyph_contour_point = NULL;
  }
}

void 
hb_font_funcs_set_glyph_name_func(hb_font_funcs_t *ffuncs,
                                  hb_font_get_glyph_name_func_t func,
                                  void *user_data,
                                  hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_name)
    ffuncs->destroy.glyph_name(ffuncs->user_data.glyph_name);

  if (func) {
    ffuncs->get.glyph_name = func;
    ffuncs->user_data.glyph_name = user_data;
    ffuncs->destroy.glyph_name = destroy;
  } else {
    ffuncs->get.glyph_name = hb_font_get_glyph_name_nil;
    ffuncs->user_data.glyph_name = NULL;
    ffuncs->destroy.glyph_name = NULL;
  }
}

void 
hb_font_funcs_set_glyph_from_name_func(hb_font_funcs_t *ffuncs,
                                       hb_font_get_glyph_from_name_func_t func,
                                       void *user_data,
                                       hb_destroy_func_t destroy)
{
  if (ffuncs->immutable) {
    if (destroy)
      destroy(user_data);
    return;
  }

  if (ffuncs->destroy.glyph_from_name)
    ffuncs->destroy.glyph_from_name(ffuncs->user_data.glyph_from_name);

  if (func) {
    ffuncs->get.glyph_from_name = func;
    ffuncs->user_data.glyph_from_name = user_data;
    ffuncs->destroy.glyph_from_name = destroy;
  } else {
    ffuncs->get.glyph_from_name = hb_font_get_glyph_from_name_nil;
    ffuncs->user_data.glyph_from_name = NULL;
    ffuncs->destroy.glyph_from_name = NULL;
  }
}

hb_font_funcs_t *
hb_font_funcs_create(void)
{
  hb_font_funcs_t *ffuncs = calloc(1, sizeof(*ffuncs));
  if (!ffuncs)
    return hb_font_funcs_get_empty();
  hb_atomic_int32_set(&ffuncs->ref_cnt, 1);

  ffuncs->get = hb_font_funcs_nil.get;
  return ffuncs;
}

void
hb_font_set_funcs(hb_font_t *font,
                  hb_font_funcs_t *klass,
                  void *font_data,
                  hb_destroy_func_t destroy)
{
  if (font->immutable) {
    if (destroy)
      destroy(font_data);
    return;
  }

  if (font->destroy)
    font->destroy(font->user_data);

  if (!klass)
    klass = hb_font_funcs_get_empty();

  hb_font_funcs_reference(klass);
  hb_font_funcs_destroy(font->klass);
  font->klass = klass;
  font->user_data = font_data;
  font->destroy = destroy;
}

hb_font_funcs_t *
hb_font_funcs_reference(hb_font_funcs_t *ffuncs)
{
  if (hb_atomic_int32_get(&ffuncs->ref_cnt) != REF_CNT_INVALID_VAL)
    hb_atomic_int32_add(&ffuncs->ref_cnt, 1);
  return ffuncs;
}

void
hb_font_set_scale(hb_font_t *font,
                  int x_scale,
                  int y_scale)
{
  if (font->immutable)
    return;

  font->x_scale = x_scale;
  font->y_scale = y_scale;
}

void
hb_font_set_ppem(hb_font_t *font,
                 unsigned int x_ppem,
                 unsigned int y_ppem)
{
  if (font->immutable)
    return;

  font->x_ppem = x_ppem;
  font->y_ppem = y_ppem;
}
