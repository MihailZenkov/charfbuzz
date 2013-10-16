// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stddef.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-open-file-private.h"

static struct ot_fnt_face *
ttc_hdr_ver1_get_face(struct ttc_hdr_ver1 *ttc_hdr_ver1, unsigned i)
{
  if (!ttc_hdr_ver1)
    return NULL;

  uint32_t of = be32toh(ttc_hdr_ver1->of_tbls_ofs[i]);
  uint8_t *base = (uint8_t*)ttc_hdr_ver1;//the beginning of the file
  return (struct ot_fnt_face*)(base + of);
}

static struct ot_fnt_face *
ttc_hdr_get_face(struct ttc_hdr *ttc_hdr, unsigned i)
{
  if (!ttc_hdr)
    return NULL;

  switch (ttc_hdr->u.hdr.ver.major) {
  case 2://version 2 is compatible with version 1
  case 1: return ttc_hdr_ver1_get_face(&ttc_hdr->u.ver1, i);
  default:return NULL;
  }
}

struct ot_fnt_face *
ot_fnt_file_get_face(struct ot_fnt_file *ot_fnt_file, unsigned i)
{
  if (!ot_fnt_file)
    return NULL;

  uint32_t tag = be32toh(ot_fnt_file->u.tag);

  switch (tag) {
  //Note: for non-collection SFNT data we ignore index.  This is because
  //Apple dfont container is a container of SFNT's.  So each SFNT is a
  //non-TTC, but the index is more than zero.
  case CFF_TAG://All the non-collection tags
  case TRUE_TAG:
  case TYP1_TAG:
  case TRUETYPE_TAG:
    return &ot_fnt_file->u.fnt_face;
  case TTC_TAG:
    return ttc_hdr_get_face(&ot_fnt_file->u.ttc_hdr, i);
  default:
    return NULL;
  }
}

static struct tbl_rec *
ot_fnt_face_get_tbl(struct ot_fnt_face *ot_fnt_face, unsigned i)
{
  //XXX:check is useless in known code paths
  if (!ot_fnt_face)
    return NULL;

  if (i >= ot_fnt_face->tbls_n)
     return NULL;//XXX:original code use a "null" object
  return &ot_fnt_face->tbls[i];
}

static hb_bool_t
ot_fnt_face_find_tbl_idx(struct ot_fnt_face *ot_fnt_face,
                        hb_tag_t tag,
                        unsigned *tbl_idx)
{
  //XXX:check is useless in known code paths
  if (!ot_fnt_face)
    return FALSE;

  unsigned cnt = ot_fnt_face->tbls_n;
  for (unsigned i = 0; i < cnt; ++i) {
    uint32_t tbl_tag = be32toh(ot_fnt_face->tbls[i].tag);
    if (tag == tbl_tag) {
      if (tbl_idx) *tbl_idx = i;
      return TRUE;
    }
  }
  if (tbl_idx) *tbl_idx = IDX_NOT_FOUND;
  return FALSE;
}

struct tbl_rec *
ot_fnt_face_get_tbl_by_tag(struct ot_fnt_face *ot_fnt_face, hb_tag_t tag)
{
  if (!ot_fnt_face)
    return NULL;

  unsigned tbl_idx;
  ot_fnt_face_find_tbl_idx(ot_fnt_face, tag, &tbl_idx);
  return ot_fnt_face_get_tbl(ot_fnt_face, tbl_idx);
}
