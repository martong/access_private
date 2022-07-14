# Introduction

This library is a collection of macros with which we can access private members.
Why would you need this?
Testing.
There are some cases when we want to test a class, but we can't or don't want to modify it.
The reasons behind that might be the following:
  * It is part of a third party software package and
    * Our build system would overwrite the changes we made
    * We don't want to maintain our own version
  * Touching the internals would require tremendous amount of recompilation of client codes, which might not be desired.

Why not use `#define private public`?
Because that's undefined behaviour.
The C++ standard states that the relative order of members in a class with different access specifiers is undefined.

# Usage
```c++
class A {
  int m_i = 3;
};

ACCESS_PRIVATE_FIELD(A, int, m_i)

void foo() {
  A a;
  auto &i = access_private::m_i(a);
  assert(i == 3);
}
```
You can also call private member functions and static private functions.
You can also access static private variables, if they are defined out-of-class.
For detailed usage, please check `test.cpp`.

# How does it work?
The ISO C++ standard specifies that there is no access check in case of explicit
template instantiations (C++14 / 14.7.2 para 12).
We can exploit this by defining a static pointer to member (or a friend function), which holds (returns) the address of the private member.
References:
* https://gist.github.com/dabrahams/1528856
* Advanced C++ FAQs: Volumes 1 & 2, pp 289 - 296

# Limitations

* We cannot access private types. We cannot access private members of private nested types either.
* We cannot call private constructors / destructors.
* We have a link time error in case of only in-class declared `const static` variables. That's because we'd take the address of that, and if that is not defined (i.e the compiler do a compile-time insert of the const value), we'd have an undefined symbol.

# Compilers
I have done tests for the following compilers:
* Apple LLVM version 7.0.0 (clang-700.0.72)
* GCC
  * 5.1.0
  * 4.8.4
  * 4.7.4
* MSVC

Test code is compiled with -std=c++11 .

It requires GCC >=4.8 or clang >=14 to access private overloaded functions.

# Future
I think it would be filling a gap if we could have out-of-class friend declarations in C++, though the committee might not agree with me.
Nevertheless, this can be implemented fairly easy, see https://github.com/martong/clang/tree/out-of-class_friend_attr .

# Notes
There is a [C++ standard *issue*](https://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#2118) that says: 
> Stateful metaprogramming via friend injection techniques should be ill-formed

The `::private_access_detail::private_access<...>` template class implements a friend function `get()`, which is used after class definition.
I am not sure, however, if that issue has been ever fixed.
