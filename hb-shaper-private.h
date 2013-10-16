#ifndef HB_SHAPER_PRIVATE_H
#define HB_SHAPER_PRIVATE_H
//Means: succeeded, but don't need to keep any data.
#define HB_SHAPER_DATA_SUCCEEDED ((void*)+1)
//Means: tried but failed to create.
#define HB_SHAPER_DATA_INVALID ((void*)-1)

typedef hb_bool_t hb_shape_func_t(hb_shape_plan_t *shape_plan,
                                  hb_font_t *font,
                                  hb_buffer_t *buffer,
                                  const hb_feature_t *features,
                                  unsigned num_features);
#ifdef HAVE_GRAPHITE2
hb_shape_func_t hb_graphite2_shape;
#endif
#ifdef HAVE_OT
hb_shape_func_t hb_ot_shape;
#endif
hb_shape_func_t hb_fallback_shape;

struct hb_shaper_pair_t {
  char name[16];
  hb_shape_func_t *func;
};

//For embedding in face / font / ...
struct hb_shaper_data_t {
#ifdef HAVE_GRAPHITE2
  void *graphite2;
#endif
#ifdef HAVE_OT
  void *ot;
#endif
  void *fallback;
};

#define HB_SHAPERS_COUNT (sizeof(struct hb_shaper_data_t)/sizeof(void *))
#define HB_SHAPER_DATA_IS_INVALID(data) ((void*)(data) == HB_SHAPER_DATA_INVALID)

struct hb_shaper_pair_t *
hb_shapers_get(void);
#endif
