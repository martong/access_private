#include <utility>
#include <type_traits>

namespace detail {

template <typename Tag, typename Tag::type x> struct private_cast {
  friend typename Tag::type get(Tag) { return x; }
};

template <typename Tag, typename Member> struct private_cast_helper {
  using type = Member;
  friend type get(Tag);
};

} // namespace detail

#define __ACCESS_PRIVATE(Class, Type, Name)                                    \
  namespace detail {                                                           \
  using Class##_##Name##_sign = Type;                                          \
  struct Class##_##Name                                                        \
      : private_cast_helper<Class##_##Name, Class##_##Name##_sign Class::*> {  \
  };                                                                           \
  template struct private_cast<Class##_##Name, &Class::Name>;                  \
  }

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  __ACCESS_PRIVATE(Class, Type, Name)                                          \
  namespace access_member {                                                    \
  Type &Name(Class &&t) { return t.*get(detail::Class##_##Name{}); }           \
  Type &Name(Class &t) { return t.*get(detail::Class##_##Name{}); }            \
  const Type &Name(const Class &t) {                                           \
    return t.*get(detail::Class##_##Name{});                                   \
  }                                                                            \
  }

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  __ACCESS_PRIVATE(Class, Type, Name)                                          \
  namespace call_member {                                                      \
  template <typename Obj,                                                      \
            std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,        \
                                          Class>::value> * = nullptr,          \
            typename... Args>                                                  \
  auto Name(Obj &&o, Args &&... args) {                                        \
    return (std::forward<Obj>(o).*                                             \
            get(detail::Class##_##Name{}))(std::forward<Args>(args)...);       \
  }                                                                            \
  }

#define __ACCESS_PRIVATE_STATIC(Class, Type, Name)                             \
  namespace detail {                                                           \
  using Class##_##Name##_sign = Type;                                          \
  struct Class##_##Name                                                        \
      : private_cast_helper<Class##_##Name, Class##_##Name##_sign *> {};       \
  template struct private_cast<Class##_##Name, &Class::Name>;                  \
  }

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
  __ACCESS_PRIVATE_STATIC(Class, Type, Name)                                   \
  namespace access_static {                                                    \
  namespace Class {                                                            \
  Type &Name() { return *get(detail::Class##_##Name{}); }                      \
  }                                                                            \
  }

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  __ACCESS_PRIVATE_STATIC(Class, Type, Name)                                   \
  namespace call_static {                                                      \
  namespace Class {                                                            \
  template <typename... Args> auto Name(Args &&... args) {                     \
    return get(detail::Class##_##Name{})(std::forward<Args>(args)...);         \
  }                                                                            \
  }                                                                            \
  }

