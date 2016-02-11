#include "../include/access_private.hpp"
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
  int m_f(int p) { return 14 * p; }
  static int s_i;
  static const int s_ci = 403;
  static int s_f(int r) { return r + 1; }

public:
  const int &get_m_i() const { return m_i; }
  static const int &get_s_i() { return s_i; }
};
int A::s_i = 404;
// Because we are using a pointer in the implementation, we need to explicitly
// define the const static variable as well, otherwise we'll have linker error.
const int A::s_ci;

ACCESS_PRIVATE_FIELD(A, int, m_i)
void test_access_private_in_lvalue_expr() {
  A a;
  auto &i = access_private::m_i(a);
  ASSERT(i == 3);
  ++i;
  ASSERT(a.get_m_i() == 4);
}
void test_access_private_in_prvalue_expr() {
  auto i = access_private::m_i(A{});
  ASSERT(i == 3);
}
void test_access_private_in_xvalue_expr() {
  A a;
  auto i = access_private::m_i(std::move(a));
  ASSERT(i == 3);
}

namespace NS {
class B {
  int m_i = 3;

public:
  class C {
    int m_i = 3;
  };
};
} // NS

ACCESS_PRIVATE_FIELD(NS::B, int, m_i)
void test_access_private_in_class_in_namespace() {
  NS::B b;
  auto &i = access_private::m_i(b);
  ASSERT(i == 3);
}

ACCESS_PRIVATE_FIELD(NS::B::C, int, m_i)
void test_access_private_in_nested_class() {
  NS::B b;
  auto &i = access_private::m_i(b);
  ASSERT(i == 3);
}

class C {
  const int m_i = 3;
};
ACCESS_PRIVATE_FIELD(C, const int, m_i)
void test_access_private_const_member() {
  C c;
  auto &i = access_private::m_i(c);
  // should not deduce to int&
  static_assert(std::is_same<const int &, decltype(i)>::value, "");
  ASSERT(i == 3);
}

class CA {
  int m_i = 3;

public:
  CA() {}
};
ACCESS_PRIVATE_FIELD(CA, int, m_i)
void test_access_private_const_object() {
  const CA ca;
  auto &i = access_private::m_i(ca);
  // should not deduce to int&
  static_assert(std::is_same<const int &, decltype(i)>::value, "");
  ASSERT(i == 3);
}

template <typename T>
class TemplateA {
  T m_i = 3;
};

ACCESS_PRIVATE_FIELD(TemplateA<int>, int, m_i)
void test_access_private_template_field() {
  TemplateA<int> a;
  auto &i = access_private::m_i(a);
  ASSERT(i == 3);
}

ACCESS_PRIVATE_FUN(A, int(int), m_f)
void test_call_private_in_lvalue_expr() {
  A a;
  int p = 3;
  auto res = call_private::m_f(a, p);
  ASSERT(res == 42);
}
void test_call_private_in_prvalue_expr() {
  auto res = call_private::m_f(A{}, 3);
  ASSERT(res == 42);
}
void test_call_private_in_xvalue_expr() {
  A a;
  auto res = call_private::m_f(std::move(a), 3);
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
  static_assert(std::is_same<const int &, decltype(i)>::value, "");
  ASSERT(i == 403);
}

ACCESS_PRIVATE_STATIC_FUN(A, int(int), s_f)
void test_call_private_static() {
  auto l = call_private_static::A::s_f(4);
  ASSERT(l == 5);
}

int main() {
  test_access_private_in_lvalue_expr();
  test_access_private_in_prvalue_expr();
  test_access_private_in_xvalue_expr();
  test_access_private_in_class_in_namespace();
  test_access_private_in_nested_class();
  test_access_private_const_member();
  test_access_private_const_object();
  test_access_private_template_field();
  test_call_private_in_prvalue_expr();
  test_call_private_in_xvalue_expr();
  test_call_private_in_lvalue_expr();
  test_access_private_static();
  test_access_private_static_const();
  test_call_private_static();
  printf("OK\n");
  return 0;
}

