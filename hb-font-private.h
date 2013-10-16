#ifndef HB_FONT_PRIVATE
#define HB_FONT_PRIVATE
struct hb_font_funcs_t {
  atomic_int32_t ref_cnt;
  hb_bool_t immutable;

  //Don't access these directly. Call hb_font_get_*() instead.
  struct {
    hb_font_get_glyph_func_t               glyph;
    hb_font_get_glyph_h_advance_func_t     glyph_h_advance;
    hb_font_get_glyph_v_advance_func_t     glyph_v_advance;
    hb_font_get_glyph_h_origin_func_t      glyph_h_origin;
    hb_font_get_glyph_v_origin_func_t      glyph_v_origin;
    hb_font_get_glyph_h_kerning_func_t     glyph_h_kerning;
    hb_font_get_glyph_v_kerning_func_t     glyph_v_kerning;
    hb_font_get_glyph_extents_func_t       glyph_extents;
    hb_font_get_glyph_contour_point_func_t glyph_contour_point;
    hb_font_get_glyph_name_func_t          glyph_name;
    hb_font_get_glyph_from_name_func_t     glyph_from_name;
  } get;

  struct {
    void *glyph;
    void *glyph_h_advance;
    void *glyph_v_advance;
    void *glyph_h_origin;
    void *glyph_v_origin;
    void *glyph_h_kerning;
    void *glyph_v_kerning;
    void *glyph_extents;
    void *glyph_contour_point;
    void *glyph_name;
    void *glyph_from_name;
  } user_data;

  struct {
    hb_destroy_func_t glyph;
    hb_destroy_func_t glyph_h_advance;
    hb_destroy_func_t glyph_v_advance;
    hb_destroy_func_t glyph_h_origin;
    hb_destroy_func_t glyph_v_origin;
    hb_destroy_func_t glyph_h_kerning;
    hb_destroy_func_t glyph_v_kerning;
    hb_destroy_func_t glyph_extents;
    hb_destroy_func_t glyph_contour_point;
    hb_destroy_func_t glyph_name;
    hb_destroy_func_t glyph_from_name;
  } destroy;
};

struct hb_font_t {
  atomic_int32_t ref_cnt;
  hb_bool_t immutable;
  hb_font_t *parent;
  hb_face_t *face;
  int x_scale;
  int y_scale;
  unsigned int x_ppem;
  unsigned int y_ppem;
  hb_font_funcs_t *klass;
  void *user_data;
  hb_destroy_func_t destroy;
  struct hb_shaper_data_t shaper_data;
};

void
hb_font_get_glyph_advance_for_direction(hb_font_t *font,
                                        hb_codepoint_t glyph,
                                        hb_direction_t direction,
                                        hb_position_t *x, hb_position_t *y);
void
hb_font_subtract_glyph_origin_for_direction(hb_font_t *font,
                                            hb_codepoint_t glyph,
                                            hb_direction_t direction,
                                            hb_position_t *x, hb_position_t *y);

#ifdef HAVE_GRAPHITE2
struct hb_graphite2_shaper_font_data_t;
struct hb_graphite2_shaper_font_data_t *
hb_graphite2_shaper_font_data_create(hb_font_t *font);
void
hb_graphite2_shaper_font_data_destroy(
                                  struct hb_graphite2_shaper_font_data_t *data);
#endif
#ifdef HAVE_OT
struct hb_ot_shaper_font_data_t;
struct hb_ot_shaper_font_data_t *
hb_ot_shaper_font_data_create(hb_font_t *font);
void
hb_ot_shaper_font_data_destroy(struct hb_ot_shaper_font_data_t *data);
#endif
struct hb_fallback_shaper_font_data_t;
struct hb_fallback_shaper_font_data_t *
hb_fallback_shaper_font_data_create(hb_font_t *font);
void
hb_fallback_shaper_font_data_destroy(
                                   struct hb_fallback_shaper_font_data_t *data);
#endif
