#pragma once

#include <kio.h>

#define ASSERT_EQ(a, b)\
   if (a != b) {\
      ksp("[%s:%d] Failed equality for expression %s == %s \n\twhere, lhs = %d and rhs = %d\n", __FILE__, __LINE__, #a, #b, a, b);\
      while(1) {} \
   }
