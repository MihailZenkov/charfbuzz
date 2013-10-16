#ifndef HB_PRIVATE_H
#define HB_PRIVATE_H
#if __GNUC__ >= 4
#  define HB_UNUSED __attribute__((unused))
#else
#  define HB_UNUSED
#endif

#ifdef __GNUC__
#  define HB_INTERNAL __attribute__((__visibility__("hidden")))
#else
#  define HB_INTERNAL
#endif

#define HB_DEBUG 0
#define FALSE	0
#define TRUE	1
#define REF_CNT_INVALID_VAL -1

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef ARRAY_LENGTH
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

static inline hb_bool_t
hb_unsigned_int_mul_overflows(unsigned int count, unsigned int size)
{
  return (size > 0) && (count >= ((unsigned int) -1) / size);
}

static inline hb_bool_t
hb_codepoint_in_range(hb_codepoint_t u, hb_codepoint_t lo, hb_codepoint_t hi)
{
  if ( ((lo^hi) & lo) == 0 &&
       ((lo^hi) & hi) == (lo^hi) &&
       ((lo^hi) & ((lo^hi) + 1)) == 0 )
    return (u & ~(lo^hi)) == lo;
  else
    return lo <= u && u <= hi;
}

static inline hb_bool_t
hb_codepoint_in_ranges(hb_codepoint_t u, hb_codepoint_t lo1, hb_codepoint_t hi1,
                       hb_codepoint_t lo2, hb_codepoint_t hi2,
                       hb_codepoint_t lo3, hb_codepoint_t hi3)
{
  return hb_codepoint_in_range(u, lo1, hi1)
         || hb_codepoint_in_range(u, lo2, hi2)
         || hb_codepoint_in_range(u, lo3, hi3);
}

#define HB_TAG_CHAR4(s) (HB_TAG(((const char *) s)[0], \
                                ((const char *) s)[1], \
                                ((const char *) s)[2], \
                                ((const char *) s)[3]))
#endif
