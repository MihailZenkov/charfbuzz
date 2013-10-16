// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"

//this is actually hb_language_t type
struct hb_language_impl_t {
  const char s[1];
};

static const char canon_map[256] = {
   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0,  '-',  0,   0,
  '0', '1', '2', '3', '4', '5', '6', '7',  '8', '9',  0,   0,   0,   0,   0,   0,
  '-', 'a', 'b', 'c', 'd', 'e', 'f', 'g',  'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w',  'x', 'y', 'z',  0,   0,   0,   0,  '-',
   0,  'a', 'b', 'c', 'd', 'e', 'f', 'g',  'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w',  'x', 'y', 'z',  0,   0,   0,   0,   0
};

static hb_bool_t
lang_equal(hb_language_t v1,
           const void *v2)
{
  const unsigned char *p1 = (const unsigned char*)v1;
  const unsigned char *p2 = (const unsigned char*)v2;

  while (*p1 && *p1 == canon_map[*p2])
    p1++, p2++;
  return *p1 == canon_map[*p2];
}

struct hb_language_item_t {

  struct hb_language_item_t *next;
  hb_language_t lang;
};

static hb_language_t 
lang_assign(const char *s)
{
  hb_language_t lang = (hb_language_t)strdup(s);
  for (unsigned char *p = (unsigned char *)lang; *p; p++)
    *p = canon_map[*p];
  return lang;
}

/* Thread-safe lock-free language list */

static struct hb_language_item_t *lang_items;

static struct hb_language_item_t *
language_item_find_or_insert(const char *key)
{
  struct hb_language_item_t *first_lang_item = hb_atomic_ptr_get(&lang_items);

  while (1) {
    for (struct hb_language_item_t *lang_item = first_lang_item; lang_item;
                                                    lang_item = lang_item->next)
      if (lang_equal(lang_item->lang, key))
        return lang_item;

    //Not found; allocate one.
    struct hb_language_item_t *lang_item = calloc(1, sizeof(*lang_item));
    if (!lang_item)
      return NULL;
    lang_item->next = first_lang_item;
    lang_item->lang = lang_assign(key);

    if (hb_atomic_ptr_cmpexch(&lang_items, &first_lang_item, &lang_item))
      return lang_item;
    free (lang_item);
  }
}

hb_language_t
hb_language_from_string(const char *str, int len)
{
  if (!str || !len || !*str)
    return HB_LANGUAGE_INVALID;

  if (len >= 0) {
    char strbuf[64];
    len = MIN(len, (int)sizeof(strbuf) - 1);
    str = (char*)memcpy(strbuf, str, len);
    strbuf[len] = '\0';
  }

  struct hb_language_item_t *item = language_item_find_or_insert(str);

  return item ? item->lang : HB_LANGUAGE_INVALID;
}

hb_tag_t
hb_tag_from_string(const char *str, int len)
{
  char tag[4];
  unsigned i;

  if (!str || !len || !*str)
    return HB_TAG_NONE;

  if (len < 0 || len > 4)
    len = 4;
  for (i = 0; i < (unsigned)len && str[i]; ++i)
    tag[i] = str[i];
  for (; i < 4; ++i)
    tag[i] = ' ';
  return HB_TAG_CHAR4(tag);
}

const char *
hb_language_to_string(hb_language_t language)
{
  //This is actually NULL-safe!
  return language->s;
}
