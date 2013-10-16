// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include "hb.h"
#include "hb-private.h"
#include "hb-ot-face-private.h"

hb_bool_t
hb_ot_layout_table_find_script (hb_face_t    *face,
				hb_tag_t      table_tag,
				hb_tag_t      script_tag,
				unsigned int *script_index)
{
  (void)face;
  (void)table_tag;
  (void)script_tag;
  (void)script_index;
  return FALSE;
}

unsigned int
hb_ot_layout_language_get_feature_tags (hb_face_t    *face,
					hb_tag_t      table_tag,
					unsigned int  script_index,
					unsigned int  language_index,
					unsigned int  start_offset,
					unsigned int *feature_count /* IN/OUT */,
					hb_tag_t     *feature_tags /* OUT */)
{
  (void)face;
  (void)table_tag;
  (void)script_index;
  (void)language_index;
  (void)start_offset;
  (void)feature_count;
  (void)feature_tags;
  return 0;
}

hb_bool_t
hb_ot_layout_language_find_feature (hb_face_t    *face,
				    hb_tag_t      table_tag,
				    unsigned int  script_index,
				    unsigned int  language_index,
				    hb_tag_t      feature_tag,
				    unsigned int *feature_index)
{
  (void)face;
  (void)table_tag;
  (void)script_index;
  (void)language_index;
  (void)feature_tag;
  (void)feature_index;
  return FALSE;
}

unsigned int
hb_ot_layout_script_get_language_tags (hb_face_t    *face,
				       hb_tag_t      table_tag,
				       unsigned int  script_index,
				       unsigned int  start_offset,
				       unsigned int *language_count /* IN/OUT */,
				       hb_tag_t     *language_tags /* OUT */)
{
  (void)face;
  (void)table_tag;
  (void)script_index;
  (void)start_offset;
  (void)language_count;
  (void)language_tags;
  return 0;
}

unsigned int
hb_ot_layout_table_get_script_tags (hb_face_t    *face,
				    hb_tag_t      table_tag,
				    unsigned int  start_offset,
				    unsigned int *script_count /* IN/OUT */,
				    hb_tag_t     *script_tags /* OUT */)
{
  (void)face;
  (void)table_tag;
  (void)start_offset;
  (void)script_count;
  (void)script_tags;
  return 0;
}

hb_bool_t
hb_ot_layout_script_find_language (hb_face_t    *face,
				   hb_tag_t      table_tag,
				   unsigned int  script_index,
				   hb_tag_t      language_tag,
				   unsigned int *language_index)
{
  (void)face;
  (void)table_tag;
  (void)script_index;
  (void)language_tag;
  (void)language_index;
  return FALSE;
}

hb_bool_t
hb_ot_layout_language_get_required_feature_index (hb_face_t    *face,
						  hb_tag_t      table_tag,
						  unsigned int  script_index,
						  unsigned int  language_index,
						  unsigned int *feature_index)
{
  (void)face;
  (void)table_tag;
  (void)script_index;
  (void)language_index;
  (void)feature_index;
  return FALSE;
}
