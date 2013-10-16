// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stddef.h>
#include <assert.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-buffer-private.h"
#include "hb-shaper-private.h"
#include "hb-shape-plan-private.h"
#include "hb-font-private.h"

hb_bool_t
hb_shape_full(hb_font_t *font,
              hb_buffer_t *buffer,
              const hb_feature_t *features,
              unsigned num_features,
              const char * const *shaper_list)
{
  if (!buffer->len)
    return TRUE;

  assert(buffer->content_type == HB_BUFFER_CONTENT_TYPE_UNICODE);

  hb_shape_plan_t *shape_plan = hb_shape_plan_create_cached(font->face,
                           &buffer->props, features, num_features, shaper_list);

  hb_bool_t res = hb_shape_plan_execute(shape_plan, font, buffer, features,
                                                                  num_features);
  hb_shape_plan_destroy(shape_plan);

  if (res)
    buffer->content_type = HB_BUFFER_CONTENT_TYPE_GLYPHS;
  return res;
}

void
hb_shape(hb_font_t *font,
         hb_buffer_t *buffer,
         const hb_feature_t *features,
         unsigned num_features)
{
  hb_shape_full(font, buffer, features, num_features, NULL);
}
