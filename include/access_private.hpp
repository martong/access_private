#include <utility>
#include <type_traits>

namespace detail {

template <typename PtrType, PtrType PtrValue, typename TagType>
struct private_cast {
  // Normal lookup cannot find in-class defined (inline) friend functions.
  friend PtrType get(TagType) { return PtrValue; }
};

} // namespace detail

// @param PtrTypeKind E.g if we have "class A", then it can be "A::*" in case of
// members, or it can be "*" in case of statics.
// Class##_##Name is used as a compile-time Tag (type).
#define _ACCESS_PRIVATE(Class, Type, Name, PtrTypeKind)                        \
  namespace detail {                                                           \
  /* Tag type, used to declare different get funcitons for different members   \
   */                                                                          \
  struct Class##_##Name {};                                                    \
  /* Explicit instantiation */                                                 \
  template struct private_cast<decltype(&Class::Name), &Class::Name,           \
                               Class##_##Name>;                                \
  /* We can build the PtrType only with two aliases */                         \
  /* E.g. using PtrType = int(int) *; would be illformed */                    \
  using Alias_##Class##_##Name = Type;                                         \
  using PtrType_##Class##_##Name = Alias_##Class##_##Name PtrTypeKind;         \
  /* Declare the friend function, now it is visible in namespace scope. Note,  \
   * we could declare it inside the Tag type too, in that case ADL would find  \
   * the declaration. By choosing to declare it here, the Tag type remains a   \
   * simple tag type, it has no other responsibilities. */                     \
  PtrType_##Class##_##Name get(Class##_##Name);                                \
  }

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  _ACCESS_PRIVATE(Class, Type, Name, Class::*)                                 \
  namespace access_private {                                                   \
  Type &Name(Class &&t) { return t.*get(detail::Class##_##Name{}); }           \
  Type &Name(Class &t) { return t.*get(detail::Class##_##Name{}); }            \
  const Type &Name(const Class &t) {                                           \
    return t.*get(detail::Class##_##Name{});                                   \
  }                                                                            \
  }

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  _ACCESS_PRIVATE(Class, Type, Name, Class::*)                                 \
  namespace call_private {                                                     \
  template <typename Obj,                                                      \
            std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,        \
                                          Class>::value> * = nullptr,          \
            typename... Args>                                                  \
  auto Name(Obj &&o, Args &&... args) {                                        \
    return (std::forward<Obj>(o).*                                             \
            get(detail::Class##_##Name{}))(std::forward<Args>(args)...);       \
  }                                                                            \
  }

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
  _ACCESS_PRIVATE(Class, Type, Name, *)                                        \
  namespace access_private_static {                                            \
  namespace Class {                                                            \
  Type &Name() { return *get(detail::Class##_##Name{}); }                      \
  }                                                                            \
  }

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  _ACCESS_PRIVATE(Class, Type, Name, *)                                        \
  namespace call_private_static {                                              \
  namespace Class {                                                            \
  template <typename... Args> auto Name(Args &&... args) {                     \
    return get(detail::Class##_##Name{})(std::forward<Args>(args)...);         \
  }                                                                            \
  }                                                                            \
  }

