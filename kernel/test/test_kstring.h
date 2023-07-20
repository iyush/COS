#ifndef TEST_KSTRING_H
#define TEST_KSTRING_H
#include <stdbool.h>
#include "../kstring.h"
#include "../kio.h"
#include <stdarg.h>

#define ASSERT_EQ(a, b)\
   if (a != b) {\
      kprint("[%s:%d] Failed equality for expression %s == %s \n\twhere, lhs = %d and rhs = %d\n", __FILE__, __LINE__, #a, #b, a, b);\
      test_failed++;\
   }

#define ASSERT_EQ_STR(a, b)\
   if (strcmp(a, b) != 0) {\
      kprint("[%s:%d] Failed string equality %s != %s\n", __FILE__, __LINE__, a, b);\
      test_failed++;\
   }

int test_strlen() {
   int test_failed = 0;
   ASSERT_EQ(strlen("A"), 1);
   ASSERT_EQ(strlen("AB"), 2);
   ASSERT_EQ(strlen("ABC"), 3);
   ASSERT_EQ(strlen("ABAB"), 4);
   ASSERT_EQ(strlen(""), 0);

   return test_failed;
}

int test_strcmp() {
   int test_failed = 0;
   ASSERT_EQ(strcmp("ABA", "ABA"), 0);
   ASSERT_EQ(strcmp("ABZ", "ABA"), 1);
   ASSERT_EQ(strcmp("ABA", "ABZ"), -1);
   ASSERT_EQ(strcmp("AB", "ABZ"), -1);
   ASSERT_EQ(strcmp("ABZ", "AB"), 1);
   ASSERT_EQ(strcmp("", ""), 0);
   ASSERT_EQ(strcmp("", ""), 0);

   return test_failed;
}

void test_s(char * buffer, char * s, ...) {
   va_list va;
   va_start(va, s);

   kvsprintf(buffer, s, va);

   va_end(va);
}

void set_buffer(char * buffer, char val) {
   while (*buffer != '\0') {
      *buffer = val;
      buffer++;
   }

}

int test_kvstringf() {
   int test_failed = 0;

   char buffer[256] = {0};
   test_s(buffer, "%c\n", 'c');
   ASSERT_EQ_STR(buffer, "c\n");

   set_buffer(buffer, 0);
   test_s(buffer, "%d", 123123123);
   ASSERT_EQ_STR(buffer, "123123123");

   set_buffer(buffer, 0);
   test_s(buffer, "%x", 0xdeadbeef);
   ASSERT_EQ_STR(buffer, "deadbeef");

   test_s(buffer, "%x", 0b11011110101011011011111011101111);
   ASSERT_EQ_STR(buffer, "deadbeef");

   test_s(buffer, "%b", 0xdeadbeef);
   ASSERT_EQ_STR(buffer, "11011110101011011011111011101111");

   set_buffer(buffer, 0);
   test_s(buffer, "%lx", 0xdeadbeefdeadbeef);
   ASSERT_EQ_STR(buffer, "deadbeefdeadbeef");

   set_buffer(buffer, 0);
   test_s(buffer, "%s", "Hello world");
   ASSERT_EQ_STR(buffer, "Hello world");

   set_buffer(buffer, 0);
   test_s(buffer, "%%");
   ASSERT_EQ_STR(buffer, "%");

   set_buffer(buffer, 0);
   test_s(buffer, "%s", "");
   ASSERT_EQ_STR(buffer, "");

   test_s(buffer, "(%s) and char: (%c)\n", "Hello", 'c');
   ASSERT_EQ_STR(buffer, "(Hello) and char: (c)\n");
    
    return test_failed;

}


void test_kstring_all() {
   int failed_test = 0;
   failed_test += test_strlen();
   failed_test += test_strcmp();
   failed_test += test_kvstringf();

   if (failed_test == 0) {
      kprint("[KString]: All tests passed\n");
   } else {
      kprint("[KString]: %d tests failed\n", failed_test);
   }
}


#endif
