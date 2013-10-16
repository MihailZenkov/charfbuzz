#ifndef HB_SHAPE_PLAN_PRIVATE_H
#define HB_SHAPE_PLAN_PRIVATE_H
struct hb_shape_plan_t
{
  atomic_int32_t ref_cnt;

  hb_bool_t default_shaper_list;
  hb_face_t *face;
  hb_segment_properties_t props;

  hb_shape_func_t *shaper_func;
  char *shaper_name;

  struct hb_shaper_data_t shaper_data;
};

#ifdef HAVE_GRAPHITE2
struct hb_graphite2_shaper_shape_plan_data_t;
struct hb_graphite2_shaper_shape_plan_data_t *
hb_graphite2_shaper_shape_plan_data_create(hb_shape_plan_t *shape_plan,
                                           const hb_feature_t *user_features,
                                           unsigned num_user_features);
void
hb_graphite2_shaper_shape_plan_data_destroy(
                            struct hb_graphite2_shaper_shape_plan_data_t *data);
#endif
#ifdef HAVE_OT
struct hb_ot_shaper_shape_plan_data_t;
struct hb_ot_shaper_shape_plan_data_t *
hb_ot_shaper_shape_plan_data_create(hb_shape_plan_t *shape_plan,
                                    const hb_feature_t *user_features,
                                    unsigned num_user_features);
void
hb_ot_shaper_shape_plan_data_destroy(
                                   struct hb_ot_shaper_shape_plan_data_t *data);
#endif
struct hb_fallback_shaper_shape_plan_data_t;
struct hb_fallback_shaper_shape_plan_data_t *
hb_fallback_shaper_shape_plan_data_create(hb_shape_plan_t *shape_plan,
                                          const hb_feature_t *user_features,
                                          unsigned num_user_features);
void
hb_fallback_shaper_shape_plan_data_destroy(
                             struct hb_fallback_shaper_shape_plan_data_t *data);
#endif
