// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_IDS_H

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-shaper-private.h"
#include "hb-face-private.h"
#include "hb-blob-private.h"
#include "hb-open-file-private.h"

void
hb_face_make_immutable(hb_face_t *face)
{
  if (hb_atomic_int32_get(&face->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  face->immutable = TRUE;
}

hb_face_t *
hb_face_reference(hb_face_t *face)
{
  if (hb_atomic_int32_get(&face->ref_cnt) != REF_CNT_INVALID_VAL)
    hb_atomic_int32_add(&face->ref_cnt, 1);
  return face;
}

static hb_face_t hb_face_nil = {
  REF_CNT_INVALID_VAL,//ref_cnt
  TRUE,//immutable
  NULL,//ft_face
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
  NULL,//shape_plans
  0,//upem
  0,//num_glyphs
  0,//index
  NULL//reference_table_func
};

hb_face_t *
hb_face_get_empty(void)
{
  return &hb_face_nil;
}

void
hb_face_set_index(hb_face_t *face, unsigned index)
{
  if (hb_atomic_int32_get(&face->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  face->index = index;
}

void
hb_face_set_upem(hb_face_t *face,
                 unsigned upem)
{
  if (hb_atomic_int32_get(&face->ref_cnt) == REF_CNT_INVALID_VAL)
    return;

  face->upem = upem;
}

hb_face_t *
hb_face_create_for_tables(hb_reference_table_func_t reference_table_func,
                          void *user_data,
                          hb_destroy_func_t destroy)
{
  hb_face_t *face = calloc(1, sizeof(*face));
  if (!reference_table_func || !face) {
    if (face)
      free(face);
    if (destroy)
      destroy(user_data);
    return hb_face_get_empty();
  }
  hb_atomic_int32_set(&face->ref_cnt, 1);

  face->reference_table_func = reference_table_func;
  face->user_data = user_data;
  face->destroy = destroy;

  face->upem = 0;
  face->num_glyphs = (unsigned)-1;
  return face;
}

typedef struct hb_face_for_data_closure_t {
  hb_blob_t *blob;
  unsigned index;
} hb_face_for_data_closure_t;

static hb_face_for_data_closure_t *
hb_face_for_data_closure_create(hb_blob_t *blob, unsigned index)
{
  hb_face_for_data_closure_t *closure;

  closure = malloc(sizeof(*closure));
  if (!closure)
    return NULL;

  closure->blob = blob;
  closure->index = index;
  return closure;
}

static void
hb_face_for_data_closure_destroy(hb_face_for_data_closure_t *closure)
{
  hb_blob_destroy(closure->blob);
  free(closure);
}

static hb_blob_t *
hb_face_for_data_reference_table(hb_face_t *face HB_UNUSED,
                                 hb_tag_t tag,
                                 void *user_data)
{
  hb_face_for_data_closure_t *data = (hb_face_for_data_closure_t *)user_data;

  if (tag == HB_TAG_NONE)
    return hb_blob_reference(data->blob);

  //XXX:carefull, we don't use a "null" object like original code
  //be NULL pointers
  struct ot_fnt_file *ot_fnt_file = hb_blob_lock_instance(data->blob);
  struct ot_fnt_face *ot_fnt_face = ot_fnt_file_get_face(ot_fnt_file,
                                                                   data->index);
  struct ot_tbl *ot_tbl = ot_fnt_face_get_tbl_by_tag(ot_fnt_face, tag);

  //XXX:without "null" object return the empty blob
  if (!ot_tbl)
    return hb_blob_get_empty();
  hb_blob_t *blob = hb_blob_create_sub_blob(data->blob, ot_tbl->of,
                                                                   ot_tbl->len);
  return blob;
}

hb_face_t *
hb_face_create(hb_blob_t *blob,
               unsigned index)
{
  hb_face_t *face;

  if (!blob || !hb_blob_get_length(blob))
    return hb_face_get_empty();

  hb_face_for_data_closure_t *closure = hb_face_for_data_closure_create(
                                                hb_blob_reference(blob), index);

  if (!closure)
    return hb_face_get_empty();

  face = hb_face_create_for_tables(
                           hb_face_for_data_reference_table,
                           closure,
                           (hb_destroy_func_t)hb_face_for_data_closure_destroy);

  hb_face_set_index(face, index);
  return face;
}

void
hb_face_destroy(hb_face_t *face)
{
  if (!face)
    return;
  if (hb_atomic_int32_get(&face->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&face->ref_cnt, -1);
  if (hb_atomic_int32_get(&face->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&face->ref_cnt, REF_CNT_INVALID_VAL);

#ifdef HAVE_GRAPHITE2
  if (face->shaper_data.graphite2
      && face->shaper_data.graphite2 != HB_SHAPER_DATA_INVALID
      && face->shaper_data.graphite2 != HB_SHAPER_DATA_SUCCEEDED)
    hb_graphite2_shaper_face_data_destroy(face->shaper_data.graphite2);
#endif
#ifdef HAVE_OT
  if (face->shaper_data.ot
      && face->shaper_data.ot != HB_SHAPER_DATA_INVALID
      && face->shaper_data.ot != HB_SHAPER_DATA_SUCCEEDED)
    hb_ot_shaper_face_data_destroy(face->shaper_data.ot);
#endif
  if (face->shaper_data.fallback
      && face->shaper_data.fallback != HB_SHAPER_DATA_INVALID
      && face->shaper_data.fallback != HB_SHAPER_DATA_SUCCEEDED)
    hb_fallback_shaper_face_data_destroy(face->shaper_data.fallback);

  if (face->destroy)
    face->destroy(face->user_data);
  free(face);
}
