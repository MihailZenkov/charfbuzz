// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-open-file-private.h"
#include "hb-blob-private.h"

struct hb_blob_t {
  atomic_int32_t ref_cnt;
  hb_bool_t immutable;

  const char *data;
  unsigned length;
  hb_memory_mode_t mode;

  void *user_data;
  hb_destroy_func_t destroy;
};

static hb_blob_t hb_blob_nil = {
    REF_CNT_INVALID_VAL,//ref_cnt
    TRUE,//immutable
    NULL,//data
    0,//length
    HB_MEMORY_MODE_READONLY,//mode
    NULL,//user_data
    NULL//destroy
  };

hb_blob_t *
hb_blob_get_empty(void)
{
  return &hb_blob_nil;
}

hb_blob_t *
hb_blob_reference(hb_blob_t *blob)
{
  if (hb_atomic_int32_get(&blob->ref_cnt) != REF_CNT_INVALID_VAL)
    hb_atomic_int32_add(&blob->ref_cnt, 1);
  return blob;
}

void
hb_blob_make_immutable(hb_blob_t *blob)
{
  if (hb_atomic_int32_get(&blob->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  blob->immutable = TRUE;
}

unsigned
hb_blob_get_length(hb_blob_t *blob)
{
  return blob->length;
}

const char *
hb_blob_get_data(hb_blob_t *blob, unsigned *length)
{
  if (length)
    *length = blob->length;
  return blob->data;
}

static hb_bool_t
try_make_writable_inplace_unix(hb_blob_t *blob)
{
  uintptr_t pagesize = -1, mask, length;
  const char *addr;

  pagesize = (uintptr_t)sysconf(_SC_PAGESIZE);

  if ((uintptr_t) -1L == pagesize)
    return FALSE;

  mask = ~(pagesize-1);
  addr = (const char*)(((uintptr_t)blob->data) & mask);
  length = (const char*)(((uintptr_t)blob->data + blob->length + pagesize-1)
                                                                & mask)  - addr;
  if (-1 == mprotect((void*)addr, length, PROT_READ | PROT_WRITE))
    return FALSE;

  blob->mode = HB_MEMORY_MODE_WRITABLE;
  return TRUE;
}

static hb_bool_t
try_writable_inplace(hb_blob_t *blob)
{
  if (try_make_writable_inplace_unix(blob))
    return TRUE;

  //Failed to make writable inplace, mark that
  blob->mode = HB_MEMORY_MODE_READONLY;
  return FALSE;
}

static void
hb_blob_destroy_user_data(hb_blob_t *blob)
{
  if (blob->destroy) {
    blob->destroy(blob->user_data);
    blob->user_data = NULL;
    blob->destroy = NULL;
  }
}

static hb_bool_t
try_writable(hb_blob_t *blob)
{
  if (blob->immutable)
    return FALSE;

  if (blob->mode == HB_MEMORY_MODE_WRITABLE)
    return TRUE;

  if (blob->mode == HB_MEMORY_MODE_READONLY_MAY_MAKE_WRITABLE
                                                  && try_writable_inplace(blob))
    return TRUE;

  if (blob->mode == HB_MEMORY_MODE_WRITABLE)
    return TRUE;

  char *new_data;

  new_data = malloc(blob->length);
  if (!new_data)
    return FALSE;

  memcpy(new_data, blob->data, blob->length);
  hb_blob_destroy_user_data(blob);
  blob->mode = HB_MEMORY_MODE_WRITABLE;
  blob->data = new_data;
  blob->user_data = new_data;
  blob->destroy = free;
  return TRUE;
}

void
hb_blob_destroy(hb_blob_t *blob)
{
  if (!blob)
    return;
  if (hb_atomic_int32_get(&blob->ref_cnt) == REF_CNT_INVALID_VAL)
    return;
  hb_atomic_int32_add(&blob->ref_cnt, -1);
  if (hb_atomic_int32_get(&blob->ref_cnt) > 0)
    return;
  hb_atomic_int32_set(&blob->ref_cnt, REF_CNT_INVALID_VAL);

  hb_blob_destroy_user_data(blob);
  free (blob);
}

hb_blob_t *
hb_blob_create_sub_blob(hb_blob_t *parent,
                        unsigned offset,
                        unsigned length)
{
  hb_blob_t *blob;

  if (!length || offset >= parent->length)
    return hb_blob_get_empty();

  hb_blob_make_immutable(parent);

  blob = hb_blob_create(parent->data + offset,
                        MIN(length, parent->length - offset),
                        HB_MEMORY_MODE_READONLY,
                        hb_blob_reference(parent),
                        (hb_destroy_func_t)hb_blob_destroy);
  return blob;
}

hb_blob_t *
hb_blob_create(const char *data,
               unsigned length,
               hb_memory_mode_t mode,
               void *user_data,
               hb_destroy_func_t destroy)
{
  hb_blob_t *blob = calloc(1, sizeof(*blob));
  if (!length || !blob) {
    if (blob)
      free(blob);
    if (destroy)
      destroy(user_data);
    return hb_blob_get_empty();
  }
  hb_atomic_int32_set(&blob->ref_cnt, 1);
  blob->immutable = FALSE;

  blob->data = data;
  blob->length = length;
  blob->mode = mode;

  blob->user_data = user_data;
  blob->destroy = destroy;

  if (blob->mode == HB_MEMORY_MODE_DUPLICATE) {
    blob->mode = HB_MEMORY_MODE_READONLY;
    if (!try_writable(blob)) {
      hb_blob_destroy(blob);
      return hb_blob_get_empty();
    }
  }
  return blob;
}

struct ot_fnt_file *
hb_blob_lock_instance(hb_blob_t *blob)
{
    hb_blob_make_immutable (blob);
    const char *base = hb_blob_get_data(blob, NULL);
    return (struct ot_fnt_file*)base;
}
