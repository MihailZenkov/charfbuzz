// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stddef.h>
#include <assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-buffer-private.h"
#include "hb-shaper-private.h"
#include "hb-face-private.h"
#include "hb-font-private.h"
#include "hb-shape-plan-private.h"

static hb_shape_plan_t hb_shape_plan_nil = {
  REF_CNT_INVALID_VAL, 
  TRUE,//default_shaper_list
  NULL,//face
  HB_SEGMENT_PROPERTIES_DEFAULT,//props
  NULL,//shaper_func
  NULL,//shaper_name
  {
#ifdef HAVE_GRAPHITE2
    HB_SHAPER_DATA_INVALID,
#endif
#ifdef HAVE_OT
    HB_SHAPER_DATA_INVALID,
#endif
    HB_SHAPER_DATA_INVALID//fallback
  }
};

//TODO no user-feature caching for now.
struct hb_shape_plan_proposal_t
{
  const hb_segment_properties_t props;
  const char * const *shaper_list;
  hb_shape_func_t *shaper_func;
};

hb_shape_plan_t *
hb_shape_plan_get_empty(void)
{
  return &hb_shape_plan_nil;
}

#ifdef HAVE_GRAPHITE2
static inline hb_bool_t
hb_graphite2_shaper_face_data_ensure(hb_face_t *face)
{
  while (1) {
    struct hb_graphite2_shaper_face_data_t *data = hb_atomic_ptr_get(
                                                  &face->shaper_data.graphite2);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_graphite2_shaper_face_data_create(face);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&face->shaper_data.graphite2, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_graphite2_shaper_face_data_destroy(data);
  }
}

static inline hb_bool_t
hb_graphite2_shaper_font_data_ensure(hb_font_t *font)
{
  while (1) {
    struct hb_graphite2_shaper_font_data_t *data = hb_atomic_ptr_get(
                                                  &font->shaper_data.graphite2);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_graphite2_shaper_font_data_create(font);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&font->shaper_data.graphite2, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_graphite2_shaper_font_data_destroy(data);
  }
}
#endif

#ifdef HAVE_OT
static inline hb_bool_t
hb_ot_shaper_face_data_ensure(hb_face_t *face)
{
  while (1) {
    struct hb_ot_shaper_face_data_t *data = hb_atomic_ptr_get(
                                                         &face->shaper_data.ot);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_ot_shaper_face_data_create(face);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&face->shaper_data.ot, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_ot_shaper_face_data_destroy(data);
  }
}

static inline hb_bool_t
hb_ot_shaper_font_data_ensure(hb_font_t *font)
{
  while (1) {
    struct hb_ot_shaper_font_data_t *data = hb_atomic_ptr_get(
                                                         &font->shaper_data.ot);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_ot_shaper_font_data_create(font);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&font->shaper_data.ot, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_ot_shaper_font_data_destroy(data);
  }
}
#endif

static inline hb_bool_t
hb_fallback_shaper_face_data_ensure(hb_face_t *face)
{
  while (1) {
    struct hb_fallback_shaper_face_data_t *data = hb_atomic_ptr_get(
                                                  &face->shaper_data.fallback);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_fallback_shaper_face_data_create(face);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&face->shaper_data.fallback, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_fallback_shaper_face_data_destroy(data);
  }
}

static inline hb_bool_t
hb_fallback_shaper_font_data_ensure(hb_font_t *font)
{
  while (1) {
    struct hb_fallback_shaper_font_data_t *data = hb_atomic_ptr_get(
                                                  &font->shaper_data.fallback);
    if (data)
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (!data)
      data = hb_fallback_shaper_font_data_create(font);

    if (!data)
      data = HB_SHAPER_DATA_INVALID;

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&font->shaper_data.fallback, &expected, &data))
      return !HB_SHAPER_DATA_IS_INVALID(data);

    if (data != HB_SHAPER_DATA_INVALID && data != HB_SHAPER_DATA_SUCCEEDED)
      hb_fallback_shaper_font_data_destroy(data);
  }
}

static void
hb_shape_plan_plan(hb_shape_plan_t *shape_plan,
                   const hb_feature_t *user_features,
                   unsigned num_user_features,
                   const char * const *shaper_list)
{
  struct hb_shaper_pair_t *shapers = hb_shapers_get();

  if (!shaper_list) {
    for (unsigned i = 0; i < HB_SHAPERS_COUNT; ++i)
      if (0)
        ;
#ifdef HAVE_GRAPHITE2
      else if (shapers[i].func == hb_graphite2_shape) {
        if (hb_graphite2_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.graphite2 =
                hb_graphite2_shaper_shape_plan_data_create(shape_plan,
                                                           user_features,
                                                           num_user_features);
          shape_plan->shaper_func = hb_graphite2_shape;
          shape_plan->shaper_name ="graphite2";
          return;
        }
      }
#endif
#ifdef HAVE_OT
      else if (shapers[i].func == hb_ot_shape) {
        if (hb_ot_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.ot = hb_ot_shaper_shape_plan_data_create(
                                                             shape_plan,
                                                             user_features,
                                                             num_user_features);
          shape_plan->shaper_func = hb_ot_shape;
          shape_plan->shaper_name ="ot";
          return;
        }
      }
#endif
      else if (shapers[i].func == hb_fallback_shape) {
        if (hb_fallback_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.fallback =
                 hb_fallback_shaper_shape_plan_data_create(shape_plan,
                                                           user_features,
                                                           num_user_features);
          shape_plan->shaper_func = hb_fallback_shape;
          shape_plan->shaper_name = "fallback";
          return;
        }
      }
  } else {
    for (; *shaper_list; ++shaper_list)
      if (0)
        ;
#ifdef HAVE_GRAPHITE2
      else if (0 == strcmp(*shaper_list, "graphite2")) {
        if (hb_graphite2_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.graphite2 =
                hb_graphite2_shaper_shape_plan_data_create(shape_plan,
                                                           user_features,
                                                           num_user_features);
          shape_plan->shaper_func = hb_graphite2_shape;
          shape_plan->shaper_name = "graphite2";
          return;
        }
      }
#endif
#ifdef HAVE_OT
      else if (0 == strcmp(*shaper_list, "ot")) {
        if (hb_ot_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.ot = hb_ot_shaper_shape_plan_data_create(
                                                             shape_plan,
                                                             user_features,
                                                             num_user_features);
          shape_plan->shaper_func = hb_ot_shape;
          shape_plan->shaper_name = "ot";
          return;
        }
      }
#endif
      else if (0 == strcmp(*shaper_list, "fallback")) {
        if (hb_fallback_shaper_face_data_ensure(shape_plan->face)) {
          shape_plan->shaper_data.fallback =
                 hb_fallback_shaper_shape_plan_data_create(shape_plan,
                                                             user_features,
                                                             num_user_features);
          shape_plan->shaper_func = hb_fallback_shape;
          shape_plan->shaper_name = "fallback";
          return;
        }
      }
  }
}

hb_shape_plan_t *
hb_shape_plan_create(hb_face_t *face,
                     const hb_segment_properties_t *props,
                     const hb_feature_t *user_features,
                     unsigned int num_user_features,
                     const char * const *shaper_list)
{
  assert(props->direction != HB_DIRECTION_INVALID);

  if (!face)
    face = hb_face_get_empty();

  if (!props || hb_atomic_int32_get(&face->ref_cnt) == REF_CNT_INVALID_VAL)
    return hb_shape_plan_get_empty();

  hb_shape_plan_t *shape_plan = calloc(1, sizeof(*shape_plan));
  if (!shape_plan)
    return hb_shape_plan_get_empty();
  hb_atomic_int32_set(&shape_plan->ref_cnt, 1);

  shape_plan->default_shaper_list = shaper_list == NULL;
  hb_face_make_immutable(face);
  shape_plan->face = hb_face_reference(face);
  shape_plan->props = *props;

  hb_shape_plan_plan(shape_plan, user_features, num_user_features, shaper_list);
  return shape_plan;
}

static hb_bool_t
hb_shape_plan_matches(hb_shape_plan_t *shape_plan,
                      struct hb_shape_plan_proposal_t *proposal)
{
  return hb_segment_properties_equal(&shape_plan->props, &proposal->props) &&
          ((shape_plan->default_shaper_list && proposal->shaper_list == NULL) ||
                            (shape_plan->shaper_func == proposal->shaper_func));
}

hb_shape_plan_t *
hb_shape_plan_reference(hb_shape_plan_t *shape_plan)
{
  if (hb_atomic_int32_get(&shape_plan->ref_cnt) != REF_CNT_INVALID_VAL)
    hb_atomic_int32_add(&shape_plan->ref_cnt, 1);
  return shape_plan;
}

void
hb_shape_plan_destroy(hb_shape_plan_t *shape_plan)
{
  if (!shape_plan)
    return;
  if (hb_atomic_int32_get(&shape_plan->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&shape_plan->ref_cnt, -1);
  if (hb_atomic_int32_get(&shape_plan->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&shape_plan->ref_cnt, REF_CNT_INVALID_VAL);

#ifdef HAVE_GRAPHITE2
  if (shape_plan->shaper_data.graphite2 &&
      shape_plan->shaper_data.graphite2 != HB_SHAPER_DATA_INVALID &&
      shape_plan->shaper_data.graphite2 != HB_SHAPER_DATA_SUCCEEDED)
    hb_graphite2_shaper_shape_plan_data_destroy(
                                             shape_plan->shaper_data.graphite2);
#endif
#ifdef HAVE_OT
  if (shape_plan->shaper_data.ot &&
      shape_plan->shaper_data.ot != HB_SHAPER_DATA_INVALID &&
      shape_plan->shaper_data.ot != HB_SHAPER_DATA_SUCCEEDED)
    hb_ot_shaper_shape_plan_data_destroy(shape_plan->shaper_data.ot);
#endif
  if (shape_plan->shaper_data.fallback &&
      shape_plan->shaper_data.fallback != HB_SHAPER_DATA_INVALID &&
      shape_plan->shaper_data.fallback != HB_SHAPER_DATA_SUCCEEDED)
    hb_fallback_shaper_shape_plan_data_destroy(
                                              shape_plan->shaper_data.fallback);

  hb_face_destroy(shape_plan->face);
  free(shape_plan);
}

hb_shape_plan_t *
hb_shape_plan_create_cached(hb_face_t *face,
                            const hb_segment_properties_t *props,
                            const hb_feature_t *user_features,
                            unsigned int num_user_features,
                            const char * const *shaper_list)
{
  if (num_user_features)
    return hb_shape_plan_create(face, props, user_features, num_user_features,
                                                                   shaper_list);

  struct hb_shape_plan_proposal_t proposal = {
    *props,
    shaper_list,
    NULL
  };

  if (shaper_list) {
    //Choose shaper.  Adapted from hb_shape_plan_plan().
    for (const char * const *shaper_item = shaper_list; *shaper_item;
                                                                  shaper_item++)
      if (0)
	;
#ifdef HAVE_GRAPHITE2
      else if (0 == strcmp(*shaper_item, "graphite2")) {
         if (hb_graphite2_shaper_face_data_ensure(face))
           proposal.shaper_func = hb_graphite2_shape;
      }
#endif
#ifdef HAVE_OT
      else if (0 == strcmp(*shaper_item, "ot")) {
         if (hb_ot_shaper_face_data_ensure(face))
           proposal.shaper_func = hb_ot_shape;
      }
#endif
      else if (0 == strcmp(*shaper_item, "fallback")) {
         if (hb_fallback_shaper_face_data_ensure(face))
           proposal.shaper_func = hb_fallback_shape;
      }

    if (!proposal.shaper_list)
      return hb_shape_plan_get_empty();
  }

  struct plan_node_t *cached_plan_nodes = hb_atomic_ptr_get(&face->shape_plans);
  hb_shape_plan_t *shape_plan;
  while (1) {
    for (struct plan_node_t *node = cached_plan_nodes; node; node = node->next)
      if (hb_shape_plan_matches(node->shape_plan, &proposal))
        return hb_shape_plan_reference(node->shape_plan);

    //Not found.

    shape_plan = hb_shape_plan_create(face, props, user_features,
                                                num_user_features, shaper_list);

    struct plan_node_t *node = calloc(1, sizeof(*node));
    if (!node)
      return shape_plan;

    node->shape_plan = shape_plan;
    node->next = cached_plan_nodes;

    if (hb_atomic_ptr_cmpexch(&face->shape_plans, &cached_plan_nodes, &node))
      break;

    hb_shape_plan_destroy(shape_plan);
    free(node);
  }

  //Release our reference on face.
  hb_face_destroy(face);
  return hb_shape_plan_reference(shape_plan);
}

hb_bool_t
hb_shape_plan_execute(hb_shape_plan_t *shape_plan,
                      hb_font_t *font,
                      hb_buffer_t *buffer,
                      const hb_feature_t *features,
                      unsigned num_features)
{
  if (hb_atomic_int32_get(&shape_plan->ref_cnt) == REF_CNT_INVALID_VAL
      || hb_atomic_int32_get(&font->ref_cnt) == REF_CNT_INVALID_VAL
      || hb_atomic_int32_get(&buffer->ref_cnt) == REF_CNT_INVALID_VAL)
    return FALSE;

  assert(shape_plan->face == font->face);
  assert(hb_segment_properties_equal(&shape_plan->props, &buffer->props));

  if (0)
    ;
#ifdef HAVE_GRAPHITE2
  else if (shape_plan->shaper_func == hb_graphite2_shape)
    return shape_plan->shaper_data.graphite2
           && hb_graphite2_shaper_font_data_ensure(font)
           && hb_graphite2_shape(shape_plan, font, buffer, features,
                                                                  num_features);
#endif
#ifdef HAVE_OT
  else if (shape_plan->shaper_func == hb_ot_shape)
    return shape_plan->shaper_data.ot
           && hb_ot_shaper_font_data_ensure(font)
           && hb_ot_shape(shape_plan, font, buffer, features, num_features);
#endif
  else if (shape_plan->shaper_func == hb_fallback_shape)
    return shape_plan->shaper_data.fallback
           && hb_fallback_shaper_font_data_ensure(font)
           && hb_fallback_shape(shape_plan, font, buffer, features,
                                                                  num_features);
  return FALSE;
}

