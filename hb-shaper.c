// C99 port from c++ is protected by a GNU Lesser GPLv3
// Copyright © 2013 Sylvain BERTRAND <sylvain.bertrand@gmail.com>
//                                   <sylware@legeek.net>       
#include <string.h>
#include <stdlib.h>

#include "hb.h"
#include "hb-private.h"
#include "hb-atomic-private.h"
#include "hb-shaper-private.h"

static struct hb_shaper_pair_t all_shapers[] = {
#ifdef HAVE_GRAPHITE2
  {"graphite2", hb_graphite2_shape},
#endif
#ifdef HAVE_OT
  {"ot", hb_ot_shape},
#endif
  {"fallback", hb_fallback_shape}
};

//Thread-safe, lock-free, shapers

static struct hb_shaper_pair_t *static_shapers = NULL;

struct hb_shaper_pair_t *
hb_shapers_get(void)
{
  while (1) {
    struct hb_shaper_pair_t *shapers = hb_atomic_ptr_get(&static_shapers);
    if (shapers)
      return shapers;

    char *env = getenv("HB_SHAPER_LIST");
    if (!env || !*env) {
      void *expected = NULL;
      void *desired = &all_shapers[0];
      hb_atomic_ptr_cmpexch(&static_shapers, &expected, &desired);
      return all_shapers;
    }

    //Not found; allocate one
    shapers = malloc(sizeof(all_shapers));
    if (!shapers) {
      void *expected = NULL;
      void *desired = &all_shapers[0];
      hb_atomic_ptr_cmpexch(&static_shapers, &expected, &desired);
      return all_shapers;
    }

    memcpy(shapers, all_shapers, sizeof(all_shapers));

    //Reorder shaper list to prefer requested shapers.
    unsigned i = 0;
    char *end, *p = env;
    for (;;) {
      end = strchr(p, ',');
      if (!end)
        end = p + strlen(p);

      for (unsigned j = i; j < ARRAY_LENGTH(all_shapers); ++j)
        if (end - p == (int)strlen(shapers[j].name) &&
                                    0 == strncmp(shapers[j].name, p, end - p)) {
         //Reorder this shaper to position i
         struct hb_shaper_pair_t t = shapers[j];
         memmove(&shapers[i + 1], &shapers[i], sizeof (shapers[i]) * (j - i));
         shapers[i] = t;
         i++;
        }

      if (!*end)
        break;
      else
        p = end + 1;
    }

    void *expected = NULL;
    if (hb_atomic_ptr_cmpexch(&static_shapers, &expected, &shapers)) {
      return shapers;
    }
    free(shapers);
  }
}
