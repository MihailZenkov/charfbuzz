// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-ft.h"
#include "hb-shaper-private.h"
#include "hb-face-private.h"

static hb_blob_t *
reference_table(hb_face_t *face HB_UNUSED, hb_tag_t tag, void *user_data)
{
  FT_Face ft_face = (FT_Face)user_data;
  FT_Byte *buffer;
  FT_ULong length = 0;
  FT_Error error;

  //Note: FreeType like HarfBuzz uses the NONE tag for fetching the entire blob 

  error = FT_Load_Sfnt_Table(ft_face, tag, 0, NULL, &length);
  if (error)
    return NULL;

  buffer = (FT_Byte*)malloc(length);
  if (buffer == NULL)
    return NULL;

  error = FT_Load_Sfnt_Table(ft_face, tag, 0, buffer, &length);
  if (error)
    return NULL;

  return hb_blob_create((const char*)buffer,
                        length,
                        HB_MEMORY_MODE_WRITABLE,
                        buffer,
                        free);
}

hb_face_t *
hb_ft_face_create(FT_Face ft_face, hb_destroy_func_t destroy)
{
  hb_face_t *face;

  if (ft_face->stream->read == NULL) {
    hb_blob_t *blob;

    blob = hb_blob_create((const char*)ft_face->stream->base,
			  (unsigned)ft_face->stream->size,
			  //TODO: We assume that it's mmap()'ed, but FreeType
			  //code suggests that there are cases we reach here
			  //but font is not mmapped.  For example, when mmap()
			  //fails.  No idea how to deal with it better here.
			  HB_MEMORY_MODE_READONLY_MAY_MAKE_WRITABLE,
			  ft_face, destroy);
    face = hb_face_create(blob, ft_face->face_index);
    hb_blob_destroy(blob);
  } else {
    face = hb_face_create_for_tables(reference_table, ft_face, destroy);
  }

  hb_face_set_index(face, ft_face->face_index);
  hb_face_set_upem(face, ft_face->units_per_EM);
  return face;
}

static void
hb_ft_face_finalize(FT_Face ft_face)
{
  hb_face_destroy((hb_face_t*)ft_face->generic.data);
}

hb_face_t *
hb_ft_face_create_cached(FT_Face ft_face)
{
  if (!ft_face->generic.data || ft_face->generic.finalizer != (FT_Generic_Finalizer)hb_ft_face_finalize) {
    if (ft_face->generic.finalizer)
      ft_face->generic.finalizer(ft_face);

    ft_face->generic.data = hb_ft_face_create(ft_face, NULL);
    ft_face->generic.finalizer = (FT_Generic_Finalizer)hb_ft_face_finalize;
  }
  return hb_face_reference((hb_face_t*)ft_face->generic.data);
}
