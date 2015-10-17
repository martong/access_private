#include "include/access_private.hpp"
#include <cstdio>
#include <cstdlib>

#define ASSERT(CONDITION)                                                      \
  do                                                                           \
    if (!(CONDITION)) {                                                        \
      printf("Assertion failure %s:%d ASSERT(%s)\n", __FILE__, __LINE__,       \
             #CONDITION);                                                      \
      abort();                                                                 \
    }                                                                          \
  while (0)

class A {
  int m_i = 3;
  int m_f(int) { return 42; }
  static int s_i;
  static const int s_ci = 403;
  static int s_f(int r) { return r + 1; }

public:
  const auto &get_m_i() const { return m_i; }
  static const auto &get_s_i() { return s_i; }
};
int A::s_i = 404;
// Because we are using a pointer in the implementation, we need to explicitly
// define the const static variable as well :(
const int A::s_ci;

class C {
  class C2 {
    static int g(int) { return 45; }
  };
};

ACCESS_PRIVATE_FIELD(A, int, m_i)
void test_access_private_in_lvalue_expr() {
  A a;
  auto &i = access_private::m_i(a);
  ASSERT(i == 3);
  ++i;
  ASSERT(a.get_m_i() == 4);
}
void test_access_private_in_rvalue_expr() {
  auto i = access_private::m_i(A{});
  ASSERT(i == 3);
}

ACCESS_PRIVATE_FUN(A, int(int), m_f)
void test_call_private_in_lvalue_expr() {
  A a;
  int p = 3;
  auto res = call_private::m_f(a, p);
  ASSERT(res == 42);
}
void test_call_private_in_rvalue_expr() {
  auto res = call_private::m_f(A{}, 3);
  ASSERT(res == 42);
}

ACCESS_PRIVATE_STATIC_FIELD(A, int, s_i)
void test_access_private_static() {
  auto &i = access_private_static::A::s_i();
  ASSERT(i == 404);
  ++i;
  ASSERT(A::get_s_i() == 405);
}

ACCESS_PRIVATE_STATIC_FIELD(A, const int, s_ci)
void test_access_private_static_const() {
  auto &i = access_private_static::A::s_ci();
  ASSERT(i == 403);
}

ACCESS_PRIVATE_STATIC_FUN(A, int(int), s_f)
void test_call_private_static() {
  auto l = call_private_static::A::s_f(4);
  ASSERT(l == 5);
}

int main() {
  test_access_private_in_lvalue_expr();
  test_access_private_in_rvalue_expr();
  test_call_private_in_rvalue_expr();
  test_call_private_in_lvalue_expr();
  test_access_private_static();
  test_access_private_static_const();
  test_call_private_static();
  printf("OK\n");
  return 0;
}

