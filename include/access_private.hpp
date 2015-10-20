#include <utility>
#include <type_traits>

// Unnamed namespace is used to avoid duplicate symbols if the macros are used
// in several translation units. See test1.
namespace {
namespace detail {

// @tparam TagType, used to declare different "get" funciton overloads for
// different members/statics
template <typename PtrType, PtrType PtrValue, typename TagType>
struct private_cast {
  // Normal lookup cannot find in-class defined (inline) friend functions.
  friend PtrType get(TagType) { return PtrValue; }
};

} // namespace detail
} // namespace

#define _PRIVATE_ACCESS_CONCATENATE_DETAIL(x, y) x##y
#define _PRIVATE_ACCESS_CONCATENATE(x, y)                                      \
  _PRIVATE_ACCESS_CONCATENATE_DETAIL(x, y)

// @param PtrTypeKind E.g if we have "class A", then it can be "A::*" in case of
// members, or it can be "*" in case of statics.
// Class##_##Name is used as a compile-time Tag (type).
#define _ACCESS_PRIVATE(Tag, Class, Type, Name, PtrTypeKind)                   \
  namespace {                                                                  \
  namespace detail {                                                           \
  /* Tag type, used to declare different get funcitons for different members   \
   */                                                                          \
  struct Tag {};                                                               \
  /* Explicit instantiation */                                                 \
  template struct private_cast<decltype(&Class::Name), &Class::Name, Tag>;     \
  /* We can build the PtrType only with two aliases */                         \
  /* E.g. using PtrType = int(int) *; would be illformed */                    \
  using _PRIVATE_ACCESS_CONCATENATE(Alias_, Tag) = Type;                       \
  using _PRIVATE_ACCESS_CONCATENATE(PtrType_, Tag) =                           \
      _PRIVATE_ACCESS_CONCATENATE(Alias_, Tag) PtrTypeKind;                    \
  /* Declare the friend function, now it is visible in namespace scope. Note,  \
   * we could declare it inside the Tag type too, in that case ADL would find  \
   * the declaration. By choosing to declare it here, the Tag type remains a   \
   * simple tag type, it has no other responsibilities. */                     \
  _PRIVATE_ACCESS_CONCATENATE(PtrType_, Tag) get(Tag);                         \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_FIELD(Tag, Class, Type, Name)                          \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)                            \
  namespace {                                                                  \
  namespace access_private {                                                   \
  Type &Name(Class &&t) { return t.*get(detail::Tag{}); }                      \
  Type &Name(Class &t) { return t.*get(detail::Tag{}); }                       \
  /* The following usings are here to avoid duplicate const qualifier warnings \
   */                                                                          \
  using _PRIVATE_ACCESS_CONCATENATE(X, Tag) = Type;                            \
  using _PRIVATE_ACCESS_CONCATENATE(Y, Tag) =                                  \
      const _PRIVATE_ACCESS_CONCATENATE(X, Tag);                               \
  _PRIVATE_ACCESS_CONCATENATE(Y, Tag) & Name(const Class &t) {                 \
    return t.*get(detail::Tag{});                                              \
  }                                                                            \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_FUN(Tag, Class, Type, Name)                            \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)                            \
  namespace {                                                                  \
  namespace call_private {                                                     \
  template <typename Obj,                                                      \
            std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,        \
                                          Class>::value> * = nullptr,          \
            typename... Args>                                                  \
  auto Name(Obj &&o, Args &&... args) {                                        \
    return (std::forward<Obj>(o).*                                             \
            get(detail::Tag{}))(std::forward<Args>(args)...);                  \
  }                                                                            \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_STATIC_FIELD(Tag, Class, Type, Name)                   \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, *)                                   \
  namespace {                                                                  \
  namespace access_private_static {                                            \
  namespace Class {                                                            \
  Type &Name() { return *get(detail::Tag{}); }                                 \
  }                                                                            \
  }                                                                            \
  }

#define _ACCESS_PRIVATE_STATIC_FUN(Tag, Class, Type, Name)                     \
  _ACCESS_PRIVATE(Tag, Class, Type, Name, *)                                   \
  namespace {                                                                  \
  namespace call_private_static {                                              \
  namespace Class {                                                            \
  template <typename... Args> auto Name(Args &&... args) {                     \
    return get(detail::Tag{})(std::forward<Args>(args)...);                    \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  }

#define _PRIVATE_ACCESS_UNIQUE(x) _PRIVATE_ACCESS_CONCATENATE(x, __COUNTER__)

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  _ACCESS_PRIVATE_FIELD(_PRIVATE_ACCESS_UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  _ACCESS_PRIVATE_FUN(_PRIVATE_ACCESS_UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
  _ACCESS_PRIVATE_STATIC_FIELD(_PRIVATE_ACCESS_UNIQUE(Tag), Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  _ACCESS_PRIVATE_STATIC_FUN(_PRIVATE_ACCESS_UNIQUE(Tag), Class, Type, Name)

