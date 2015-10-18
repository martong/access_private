#include <utility>
#include <type_traits>

namespace detail {

// @tparam TagType, used to declare different "get" funciton overloads for
// different members/statics
template <typename PtrType, PtrType PtrValue, typename TagType>
struct private_cast {
  // Normal lookup cannot find in-class defined (inline) friend functions.
  friend PtrType get(TagType) { return PtrValue; }
};

} // namespace detail

// @param PtrTypeKind E.g if we have "class A", then it can be "A::*" in case of
// members, or it can be "*" in case of statics.
// Class##_##Name is used as a compile-time Tag (type).
#define _ACCESS_PRIVATE(Tag, Class, Type, Name, PtrTypeKind)                   \
  namespace detail {                                                           \
  /* Tag type, used to declare different get funcitons for different members   \
   */                                                                          \
  struct Tag {};                                                               \
  /* Explicit instantiation */                                                 \
  template struct private_cast<decltype(&Class::Name), &Class::Name, Tag>;     \
  /* We can build the PtrType only with two aliases */                         \
  /* E.g. using PtrType = int(int) *; would be illformed */                    \
  using Alias_##Tag = Type;                                                    \
  using PtrType_##Tag = Alias_##Tag PtrTypeKind;                               \
  /* Declare the friend function, now it is visible in namespace scope. Note,  \
   * we could declare it inside the Tag type too, in that case ADL would find  \
   * the declaration. By choosing to declare it here, the Tag type remains a   \
   * simple tag type, it has no other responsibilities. */                     \
  PtrType_##Tag get(Tag);                                                      \
  }

#define _ACCESS_PRIVATE_FIELD(Tag, Class, Type, Name)                          \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)                            \
  namespace access_private {                                                   \
  Type &Name(Class &&t) { return t.*get(detail::Tag{}); }                      \
  Type &Name(Class &t) { return t.*get(detail::Tag{}); }                       \
  const Type &Name(const Class &t) { return t.*get(detail::Tag{}); }           \
  }

#define _ACCESS_PRIVATE_FUN(Tag, Class, Type, Name)                            \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)                            \
  namespace call_private {                                                     \
  template <typename Obj,                                                      \
            std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,        \
                                          Class>::value> * = nullptr,          \
            typename... Args>                                                  \
  auto Name(Obj &&o, Args &&... args) {                                        \
    return (std::forward<Obj>(o).*                                             \
            get(detail::Tag{}))(std::forward<Args>(args)...);                  \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_STATIC_FIELD(Tag, Class, Type, Name)                   \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, *)                                   \
  namespace access_private_static {                                            \
  namespace Class {                                                            \
  Type &Name() { return *get(detail::Tag{}); }                                 \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_STATIC_FUN(Tag, Class, Type, Name)                     \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, *)                                   \
  namespace call_private_static {                                              \
  namespace Class {                                                            \
  template <typename... Args> auto Name(Args &&... args) {                     \
    return get(detail::Tag{})(std::forward<Args>(args)...);                    \
  }                                                                            \
  }                                                                            \
  }

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define UNIQUE(x) CONCATENATE(x, __COUNTER__)

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  _ACCESS_PRIVATE_FIELD(UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  _ACCESS_PRIVATE_FUN(UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
  _ACCESS_PRIVATE_STATIC_FIELD(UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  _ACCESS_PRIVATE_STATIC_FUN(UNIQUE(Tag), Class, Type, Name)

