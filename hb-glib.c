// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-glib.h"
#include "hb-unicode-private.h"

GUnicodeScript
hb_glib_script_from_script(hb_script_t script)
{
  return g_unicode_script_from_iso15924(script);
}

hb_script_t
hb_glib_script_to_script(GUnicodeScript script)
{
  return (hb_script_t)g_unicode_script_to_iso15924(script);
}

static hb_unicode_combining_class_t
hb_glib_unicode_combining_class(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                hb_codepoint_t unicode,
                                void *user_data HB_UNUSED)

{
  return (hb_unicode_combining_class_t)g_unichar_combining_class(unicode);
}

static unsigned int
hb_glib_unicode_eastasian_width(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                hb_codepoint_t unicode,
                                void *user_data HB_UNUSED)
{
  return g_unichar_iswide(unicode) ? 2 : 1;
}

static hb_unicode_general_category_t
hb_glib_unicode_general_category(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                 hb_codepoint_t unicode,
                                 void *user_data HB_UNUSED)

{
  //hb_unicode_general_category_t and GUnicodeType are identical
  return (hb_unicode_general_category_t)g_unichar_type(unicode);
}

static hb_codepoint_t
hb_glib_unicode_mirroring(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                          hb_codepoint_t unicode,
                          void *user_data HB_UNUSED)
{
  g_unichar_get_mirror_char(unicode, &unicode);
  return unicode;
}

static hb_script_t
hb_glib_unicode_script(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                       hb_codepoint_t unicode,
                       void *user_data HB_UNUSED)
{
  return hb_glib_script_to_script(g_unichar_get_script(unicode));
}

static hb_bool_t
hb_glib_unicode_compose(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                        hb_codepoint_t a,
                        hb_codepoint_t b,
                        hb_codepoint_t *ab,
                        void *user_data HB_UNUSED)
{
  return g_unichar_compose(a, b, ab);
}

static hb_bool_t
hb_glib_unicode_decompose(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                          hb_codepoint_t ab,
                          hb_codepoint_t *a,
                          hb_codepoint_t *b,
                          void *user_data HB_UNUSED)
{
  return g_unichar_decompose(ab, a, b);
}

static unsigned int
hb_glib_unicode_decompose_compatibility(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                        hb_codepoint_t u,
                                        hb_codepoint_t *decomposed,
                                        void *user_data HB_UNUSED)
{
  return g_unichar_fully_decompose(u,
                                   TRUE,
                                   decomposed,
                                   HB_UNICODE_MAX_DECOMPOSITION_LEN);
}

static hb_unicode_funcs_t hb_glib_unicode_funcs = {
  REF_CNT_INVALID_VAL,//ref_cnt
  NULL,//parent
  TRUE,//immutable
  {//func
    hb_glib_unicode_combining_class,
    hb_glib_unicode_eastasian_width,
    hb_glib_unicode_general_category,
    hb_glib_unicode_mirroring,
    hb_glib_unicode_script,
    hb_glib_unicode_compose,
    hb_glib_unicode_decompose,
    hb_glib_unicode_decompose_compatibility
  },
  {//user_data
    NULL,//combining_class
    NULL,//eastasian_width
    NULL,//general_category
    NULL,//mirroring
    NULL,//script
    NULL,//compose
    NULL,//decompose
    NULL//decompose_compatibility
  },
  {//destroy
    NULL,//combining_class
    NULL,//eastasian_width
    NULL,//general_category
    NULL,//mirroring
    NULL,//script
    NULL,//compose
    NULL,//decompose
    NULL//decompose_compatibility
  }
};

hb_unicode_funcs_t *
hb_glib_get_unicode_funcs(void)
{
  return &hb_glib_unicode_funcs;
}
