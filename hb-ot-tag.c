// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stddef.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-ot.h"

hb_script_t
hb_ot_tag_to_script (hb_tag_t tag)
{
  (void)tag;
  return HB_SCRIPT_INVALID;
}

hb_language_t
hb_ot_tag_to_language (hb_tag_t tag)
{
  (void)tag;
  return HB_LANGUAGE_INVALID;
}

hb_tag_t
hb_ot_tag_from_language (hb_language_t language)
{
  (void)language;
  return HB_OT_TAG_DEFAULT_LANGUAGE;
}

void
hb_ot_tags_from_script (hb_script_t  script,
			hb_tag_t    *script_tag_1,
			hb_tag_t    *script_tag_2)
{
  (void)script;
  (void)script_tag_1;
  (void)script_tag_2;
}
