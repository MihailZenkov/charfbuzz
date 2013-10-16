// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stdlib.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-unicode-private.h"

static hb_unicode_combining_class_t
hb_unicode_combining_class_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                               hb_codepoint_t unicode HB_UNUSED,
                               void *user_data HB_UNUSED)
{
  return HB_UNICODE_COMBINING_CLASS_NOT_REORDERED;
}

static unsigned int
hb_unicode_eastasian_width_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                               hb_codepoint_t unicode HB_UNUSED,
                               void *user_data HB_UNUSED)
{
  return 1;
}

static hb_unicode_general_category_t
hb_unicode_general_category_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                hb_codepoint_t unicode HB_UNUSED,
                                void *user_data HB_UNUSED)
{
  return HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER;
}

static hb_codepoint_t
hb_unicode_mirroring_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                         hb_codepoint_t unicode HB_UNUSED,
                         void *user_data HB_UNUSED)
{
  return unicode;
}

static hb_script_t
hb_unicode_script_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                      hb_codepoint_t unicode HB_UNUSED,
                      void *user_data HB_UNUSED)
{
  return HB_SCRIPT_UNKNOWN;
}

static hb_bool_t
hb_unicode_compose_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                       hb_codepoint_t a HB_UNUSED,
                       hb_codepoint_t b HB_UNUSED,
                       hb_codepoint_t *ab HB_UNUSED,
                       void *user_data HB_UNUSED)
{
  return FALSE;
}

static hb_bool_t
hb_unicode_decompose_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                         hb_codepoint_t ab HB_UNUSED,
                         hb_codepoint_t *a HB_UNUSED,
                         hb_codepoint_t *b HB_UNUSED,
                         void *user_data HB_UNUSED)
{
  return FALSE;
}

static unsigned int
hb_unicode_decompose_compatibility_nil(hb_unicode_funcs_t *ufuncs HB_UNUSED,
                                       hb_codepoint_t u HB_UNUSED,
                                       hb_codepoint_t *decomposed HB_UNUSED,
                                       void *user_data HB_UNUSED)
{
  return 0;
}

//must be public
hb_unicode_funcs_t _hb_unicode_funcs_nil = {
  REF_CNT_INVALID_VAL,//ref_cnt
  NULL,//parent
  TRUE,//immutable
  {//func
    hb_unicode_combining_class_nil,
    hb_unicode_eastasian_width_nil,
    hb_unicode_general_category_nil,
    hb_unicode_mirroring_nil,
    hb_unicode_script_nil,
    hb_unicode_compose_nil,
    hb_unicode_decompose_nil,
    hb_unicode_decompose_compatibility_nil
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

//Default_Ignorable codepoints:
//
//Note that as of Oct 2012 (Unicode 6.2), U+180E MONGOLIAN VOWEL SEPARATOR
//is NOT Default_Ignorable, but it really behaves in a way that it should
//be.  That has been reported to the Unicode Technical Committee for
//consideration.  As such, we include it here, since Uniscribe removes it.
//It *is* in Unicode 6.3 however.  U+061C ARABIC LETTER MARK from Unicode
//6.3 is also added manually.  The new Unicode 6.3 bidi formatting
//characters are encoded in a block that was Default_Ignorable already.
//
//Note: While U+115F and U+1160 are Default_Ignorable, we do NOT want to
//hide them, as the way Uniscribe has implemented them is with regular
//spacing glyphs, and that's the way fonts are made to work.  As such,
//we make exceptions for those two.
//
//Gathered from:
//http://unicode.org/cldr/utility/list-unicodeset.jsp?a=[:DI:]&abb=on&ucd=on&esc=on
//
//Last updated to the page with the following versions:
//Version 3.6; ICU version: 50.0.1.0; Unicode version: 6.1.0.0
//
//4,167 Code Points
//
//[\u00AD\u034F\u115F\u1160\u17B4\u17B5\u180B-\u180D\u200B-\u200F\u202A-\u202E\u2060-\u206F\u3164\uFE00-\uFE0F\uFEFF\uFFA0\uFFF0-\uFFF8\U0001D173-\U0001D17A\U000E0000-\U000E0FFF]
//
//00AD ;SOFT HYPHEN
//034F ;COMBINING GRAPHEME JOINER
//#115F ;HANGUL CHOSEONG FILLER
//#1160 ;HANGUL JUNGSEONG FILLER
//17B4 ;KHMER VOWEL INHERENT AQ
//17B5 ;KHMER VOWEL INHERENT AA
//180B..180D ;MONGOLIAN FREE VARIATION SELECTOR THREE
//200B..200F ;RIGHT-TO-LEFT MARK
//202A..202E ;RIGHT-TO-LEFT OVERRIDE
//2060..206F ;NOMINAL DIGIT SHAPES
//3164 ;HANGUL FILLER
//FE00..FE0F ;VARIATION SELECTOR-16
//FEFF ;ZERO WIDTH NO-BREAK SPACE
//FFA0 ;HALFWIDTH HANGUL FILLER
//FFF0..FFF8 ;<unassigned-FFF8>
//1D173..1D17A ;MUSICAL SYMBOL END PHRASE
//E0000..E0FFF ;<unassigned-E0FFF>

hb_bool_t
hb_unicode_is_default_ignorable(hb_codepoint_t ch)
{
  hb_codepoint_t plane = ch >> 16;
  if (plane == 0) {
    //BMP 
    hb_codepoint_t page = ch >> 8;
    switch (page) {
    case 0x00: return ch == 0x00AD;
    case 0x03: return ch == 0x034F;
    case 0x06: return ch == 0x061C;
    case 0x17: return hb_codepoint_in_range(ch, 0x17B4, 0x17B5);
    case 0x18: return hb_codepoint_in_range(ch, 0x180B, 0x180E);
    case 0x20: return hb_codepoint_in_ranges(ch, 0x200B, 0x200F,
                                                 0x202A, 0x202E,
                                                 0x2060, 0x206F);
    case 0x31: return ch == 0x3164;
    case 0xFE: return hb_codepoint_in_range(ch, 0xFE00, 0xFE0F) || ch == 0xFEFF;
    case 0xFF: return hb_codepoint_in_range(ch, 0xFFF0, 0xFFF8) || ch == 0xFFA0;
    default: return FALSE;
    }
  } else {
    //Other planes
    switch (plane) {
    case 0x01: return hb_codepoint_in_range(ch, 0x0001D173, 0x0001D17A);
    case 0x0E: return hb_codepoint_in_range(ch, 0x000E0000, 0x000E0FFF);
    default: return FALSE;
    }
  }
}

hb_unicode_funcs_t *
hb_unicode_funcs_get_empty(void)
{
  return &_hb_unicode_funcs_nil;
}

hb_unicode_funcs_t *
hb_unicode_funcs_reference(hb_unicode_funcs_t *ufuncs)
{
  if (hb_atomic_int32_get(&ufuncs->ref_cnt) != REF_CNT_INVALID_VAL)
    hb_atomic_int32_add(&ufuncs->ref_cnt, 1);
  return ufuncs;
}

extern hb_unicode_funcs_t *hb_glib_get_unicode_funcs(void);
extern hb_unicode_funcs_t *hb_nil_get_unicode_funcs(void);

hb_unicode_funcs_t *
hb_unicode_funcs_get_default(void)
{
#ifdef HAVE_GLIB
  return hb_glib_get_unicode_funcs();
#else
  return hb_unicode_funcs_get_empty();
#endif
}

void
hb_unicode_funcs_destroy(hb_unicode_funcs_t *ufuncs)
{
  if (!ufuncs)
    return;
  if (hb_atomic_int32_get(&ufuncs->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&ufuncs->ref_cnt, -1);
  if (hb_atomic_int32_get(&ufuncs->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&ufuncs->ref_cnt, REF_CNT_INVALID_VAL);

  if (ufuncs->destroy.combining_class)
    ufuncs->destroy.combining_class(ufuncs->user_data.combining_class);
  if (ufuncs->destroy.eastasian_width)
    ufuncs->destroy.eastasian_width(ufuncs->user_data.eastasian_width);
  if (ufuncs->destroy.general_category)
    ufuncs->destroy.general_category(ufuncs->user_data.general_category);
  if (ufuncs->destroy.mirroring)
    ufuncs->destroy.mirroring(ufuncs->user_data.mirroring);
  if (ufuncs->destroy.script)
    ufuncs->destroy.script(ufuncs->user_data.script);
  if (ufuncs->destroy.compose)
    ufuncs->destroy.compose(ufuncs->user_data.compose);
  if (ufuncs->destroy.decompose)
    ufuncs->destroy.decompose(ufuncs->user_data.decompose);
  if (ufuncs->destroy.decompose_compatibility)
    ufuncs->destroy.decompose_compatibility(
                                     ufuncs->user_data.decompose_compatibility);

  hb_unicode_funcs_destroy(ufuncs->parent);
  free (ufuncs);
}
