#ifndef HB_ATOMIC_PRIVATE_H
#define HB_ATOMIC_PRIVATE_H
#ifdef __GNUC__
#define atomic_int32_t int32_t
static HB_UNUSED atomic_int32_t hb_atomic_int32_add(atomic_int32_t *p,
                                                    atomic_int32_t val)
{
  return __atomic_add_fetch(p, val, __ATOMIC_SEQ_CST);
}

static HB_UNUSED atomic_int32_t hb_atomic_int32_get(atomic_int32_t *p)
{
  atomic_int32_t r;
   __atomic_load(p, &r,  __ATOMIC_SEQ_CST);
  return r;
}

static HB_UNUSED void hb_atomic_int32_set(atomic_int32_t *p,
                                          atomic_int32_t v)
{
   __atomic_store(p, &v, __ATOMIC_SEQ_CST);
}

static HB_UNUSED void *hb_atomic_ptr_get(void *P)
{
  void **P_real = (void**)P;
  void *ret;
  __atomic_load(P_real, &ret, __ATOMIC_SEQ_CST);
  return ret;
}

static HB_UNUSED hb_bool_t hb_atomic_ptr_cmpexch(void *ptr,
                                                 void *expected,
                                                 void *desired)
{
  void **ptr_real = (void**)ptr;
  void **expected_real = (void**)expected;
  void **desired_real =(void**)desired;
  return  __atomic_compare_exchange(ptr_real,
                                    expected_real,
                                    desired_real,
                                    FALSE,
                                    __ATOMIC_SEQ_CST,
                                    __ATOMIC_SEQ_CST);
}
#else//__GNUC__
#  error "missing definitions of atomic operations"
#endif
#endif
