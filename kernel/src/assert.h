#ifndef _ASSERT_H
#define _ASSERT_H

#include <kio.h>

#define ASSERT_EQ(a, b)\
   if (a != b) {\
      ksp("[%s:%d] ASSERTION_FAILED! Failed equality for expression (%s == %s) \n\twhere, lhs = %ld and rhs = %ld\n", __FILE__, __LINE__, #a, #b, (u64)a, (u64)b);\
      while(1) {} \
   }


#define ASSERT(a)\
   if (!(a)) {\
      ksp("[%s:%d] ASSERTION_FAILED! Failed equality for expression (%s), expression resolves to False.\n", __FILE__, __LINE__, #a);\
      while(1) {} \
   }

#define ASSERT_LT(a, b)\
   if (a >= b) {\
      ksp("[%s:%d] Failed equality for expression (%s == %s) \n\twhere, lhs = %ld and rhs = %ld\n", __FILE__, __LINE__, #a, #b, (u64)a, (u64)b);\
      while(1) {} \
   }


#define TODO(a)\
    { \
        ksp("[%s:%d] TODO %s\n", __FILE__, __LINE__,  a); \
        while(1) {} \
    }

#endif
