#ifndef HB_FACE_PRIVATE_H
#define HB_FACE_PRIVATE_H
struct plan_node_t {
  hb_shape_plan_t *shape_plan;
  struct plan_node_t *next;
};

struct hb_face_t {
  atomic_int32_t ref_cnt;
  hb_bool_t immutable;
  FT_Face ft_face;
  void *user_data;
  hb_destroy_func_t destroy;
  struct hb_shaper_data_t shaper_data;
  struct plan_node_t *shape_plans;
  unsigned upem;
  unsigned num_glyphs;
  unsigned index;
  hb_reference_table_func_t reference_table_func;
};

#ifdef HAVE_GRAPHITE2
struct hb_graphite2_shaper_face_data_t;
struct hb_graphite2_shaper_face_data_t *
hb_graphite2_shaper_face_data_create(hb_face_t *face);
void
hb_graphite2_shaper_face_data_destroy(
                                    struct hb_graphite2_shaper_face_data *data);
#endif
#ifdef HAVE_OT
struct hb_ot_shaper_face_data_t;
struct hb_ot_shaper_face_data_t *
hb_ot_shaper_face_data_create(hb_face_t *face);
void
hb_ot_shaper_face_data_destroy(struct hb_ot_shaper_face_data_t *data);
#endif
struct hb_fallback_shaper_face_data_t;
struct hb_fallback_shaper_face_data_t *
hb_fallback_shaper_face_data_create(hb_face_t *face);
void
hb_fallback_shaper_face_data_destroy(
                                     struct hb_fallback_shaper_face_data_t *data);
#endif
