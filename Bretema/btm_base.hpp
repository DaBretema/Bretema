#pragma once

//===========================
//= INCLUDEs
//===========================

// Std
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <array>

// Glm
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <glm/gtx/vector_angle.hpp>

// Fmt
#include <fmt/format.h>

// Span
#if 1
#    include <span>
#else
#    define TCB_SPAN_NAMESPACE_NAME std
#    include "span.hpp"
#endif

//===========================
//= GLOBAL
//===========================

// Type Aliases

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

template<typename T>
using sPtr = std::shared_ptr<T>;
#define sNew std::make_shared
template<typename T>
using uPtr = std::unique_ptr<T>;
#define uNew std::make_unique

template<typename K, typename V>
using umap = std::unordered_map<K, V>;
template<typename T>
using uset = std::unordered_set<T>;

// General Macros
#define MBU          [[maybe_unused]]
#define BTM_BIT(x)   (1 << x)
#define BTM_BIND(fn) [this](auto &&...args) { return this->fn(args...); }

// Ptr as Str
#define BTM_PTRSTR(p) fmt::format("{}", fmt::ptr(p))

// Logging w/o Format

auto const TrimStr = [](auto const &s, i32 nChars) -> std::string_view
{
    std::string_view const sv = s;

    if (nChars < 1)
        return sv;

    size_t const n = static_cast<size_t>(nChars);
    return sv.substr(sv.length() >= n ? sv.length() - n : 0);
};

#if 0  // verbose
#    define BTM_INFO(msg) fmt::print("[I] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#    define BTM_WARN(msg) fmt::print("[W] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#    define BTM_ERR(msg)  fmt::print("[E] - ({}:{})\n → {}\n", __FILE__, __LINE__, msg)
#else
#    define BTM_INFO(msg) fmt::print("[I] (...{}:{}) - {}\n", TrimStr(__FILE__, 20), __LINE__, msg)
#    define BTM_WARN(msg) fmt::print("[W] (...{}:{}) - {}\n", TrimStr(__FILE__, 20), __LINE__, msg)
#    define BTM_ERR(msg)  fmt::print("[E] (...{}:{}) - {}\n", TrimStr(__FILE__, 20), __LINE__, msg)
#endif

// Logging w/ Format
#define BTM_FMT(msg, ...)   fmt::format(msg, __VA_ARGS__)
#define BTM_INFOF(msg, ...) BTM_INFO(BTM_FMT(msg, __VA_ARGS__))
#define BTM_WARNF(msg, ...) BTM_WARN(BTM_FMT(msg, __VA_ARGS__))
#define BTM_ERRF(msg, ...)  BTM_ERR(BTM_FMT(msg, __VA_ARGS__))

// Custom Assert
#ifndef NDEBUG
#    define BTM_ASSERT(cond) assert(cond)
#    define BTM_ASSERT_X(cond, msg) \
        do                          \
        {                           \
            if (!(cond))            \
            {                       \
                BTM_ERR(msg);       \
                assert(cond);       \
            }                       \
        } while (0)
#else
#    define BTM_ASSERT(cond)
#    define BTM_ASSERT_X(cond, msg)
#endif

// Custom Abort
#define BTM_ABORT(msg) \
    do                 \
    {                  \
        BTM_ERR(msg);  \
        abort();       \
    } while (0)
#define BTM_ABORTF(msg, ...)        \
    do                              \
    {                               \
        BTM_ERRF(msg, __VA_ARGS__); \
        abort();                    \
    } while (0)
#define BTM_ABORT_IF(cond, msg)                  \
    do                                           \
    {                                            \
        if (cond)                                \
            BTM_ABORTF("{} --> {}", #cond, msg); \
    } while (0)
#define BTM_ABORTF_IF(cond, msg, ...)                                      \
    do                                                                     \
    {                                                                      \
        if (cond)                                                          \
            BTM_ABORTF("{} --> {}", #cond, fmt::format(msg, __VA_ARGS__)); \
    } while (0)

// C++ Contiguous container to C raw data
#define BTM_SIZEOF(type, v) static_cast<type>((v.empty() ? 0u : v.size() * sizeof(v[0])))
#define BTM_SIZEOFu32(v)    BTM_SIZEOF(u32, v)
#define BTM_SIZE(type, v)   static_cast<type>(v.size())
#define BTM_SIZEu32(v)      BTM_SIZE(u32, v)
#define BTM_DATA(type, v)   reinterpret_cast<type>(v.data())

// clang-format off
// . Defer
// This allow you to call things like file.close(), right after file.open(),
//   to be sure that we don't forget to call it at the end of the function
//   and ensure that it's called even if an exception is thrown
#define BTM_CONCAT_(a, b) a##b
#define BTM_CONCAT(a, b) BTM_CONCAT_(a,b)
using   BTM_DEFER_PTR = std::unique_ptr<void,std::function<void(void*)>>;
#define BTM_DEFER_(name, fn) auto name = BTM_DEFER_PTR([](){static int a=0; return &a;}(), [&](void*){fn;})
#define BTM_DEFER(fn) BTM_DEFER_(BTM_CONCAT(__defer__, __LINE__), fn)
// clang-format on

//===========================
//= BRETEMA
//===========================

namespace btm
{

// GLOBAL CONSTS
inline glm::vec3 constexpr RIGHT = { 1, 0, 0 };
inline glm::vec3 constexpr UP    = { 0, 1, 0 };
inline glm::vec3 constexpr FRONT = { 0, 0, 1 };

inline float constexpr INF      = std::numeric_limits<float>::infinity();
inline glm::vec2 constexpr INF2 = { INF, INF };
inline glm::vec3 constexpr INF3 = { INF, INF, INF };
inline glm::vec4 constexpr INF4 = { INF, INF, INF, INF };

inline float constexpr ZERO      = 0.f;
inline glm::vec2 constexpr ZERO2 = { 0.f, 0.f };
inline glm::vec3 constexpr ZERO3 = { 0.f, 0.f, 0.f };
inline glm::vec4 constexpr ZERO4 = { 0.f, 0.f, 0.f, 0.f };

inline float PI      = 3.14159265359f;
inline float TAU     = 2.f * PI;
inline float HALF_PI = PI * 0.5f;

// VOID PTR WITH DATA
struct CoolPtr
{
    u32         bytes = 0;
    u32         count = 0;
    void const *data  = nullptr;  // @dani : const ??
};
template<typename T>
inline CoolPtr asCoolPtr(std::vector<T> const &v)
{
    return { BTM_SIZEOFu32(v), BTM_SIZEu32(v), BTM_DATA(void const *, v) };
}

// MATHS
inline float clampRot(float angle)
{
    auto turns = floorf(angle / 360.f);
    return angle - 360.f * turns;
}

// USER INPUT
namespace UI
{

enum struct State
{
    Release,
    Press,
    Hold,
};

enum struct Mouse
{
    _1 = 0,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,

    Left   = _1,
    Right  = _2,
    Middle = _3,
};

enum struct Key
{
    Unknown = -1,

    // Keyboard
    Space        = 32,
    Apostrophe   = 39,     /* ' */
    Comma        = 44,     /* , */
    Minus        = 45,     /* - */
    Period       = 46,     /* . */
    Dot          = Period, /* . */
    Slash        = 47,     /* / */
    Num0         = 48,
    Num1         = 49,
    Num2         = 50,
    Num3         = 51,
    Num4         = 52,
    Num5         = 53,
    Num6         = 54,
    Num7         = 55,
    Num8         = 56,
    Num9         = 57,
    Semicolon    = 59, /* ; */
    Equal        = 61, /* = */
    A            = 65,
    B            = 66,
    C            = 67,
    D            = 68,
    E            = 69,
    F            = 70,
    G            = 71,
    H            = 72,
    I            = 73,
    J            = 74,
    K            = 75,
    L            = 76,
    M            = 77,
    N            = 78,
    O            = 79,
    P            = 80,
    Q            = 81,
    R            = 82,
    S            = 83,
    T            = 84,
    U            = 85,
    V            = 86,
    W            = 87,
    X            = 88,
    Y            = 89,
    Z            = 90,
    LeftBracket  = 91, /* [ */
    BackSlash    = 92, /* \ */
    RightBracket = 93, /* ] */
    GraveAccent  = 96, /* ` */

    // Navigation
    Escape    = 256,
    Enter     = 257,
    Tab       = 258,
    Backspace = 259,
    Insert    = 260,
    Delete    = 261,
    Right     = 262,
    Left      = 263,
    Down      = 264,
    Up        = 265,
    PageUp    = 266,
    PageDown  = 267,
    Home      = 268,
    End       = 269,

    // Locks and more
    CapsLock    = 280,
    ScrollLock  = 281,
    NumLock     = 282,
    PrintScreen = 283,
    Pause       = 284,
    Menu        = 348,

    // Function keys
    F1  = 290,
    F2  = 291,
    F3  = 292,
    F4  = 293,
    F5  = 294,
    F6  = 295,
    F7  = 296,
    F8  = 297,
    F9  = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,

    // KeyPad (numeric keyboard)
    Kp0         = 320,
    Kp1         = 321,
    Kp2         = 322,
    Kp3         = 323,
    Kp4         = 324,
    Kp5         = 325,
    Kp6         = 326,
    Kp7         = 327,
    Kp8         = 328,
    Kp9         = 329,
    KpDecimal   = 330,
    KpDivide    = 331,
    KpMultiply  = 332,
    KpSubstract = 333,
    KpAdd       = 334,
    KpEnter     = 335,
    KpEqual     = 336,

    // Mods
    LeftShift  = 340,
    LeftCtrl   = 341,
    LeftAlt    = 342,
    LeftSuper  = 343,
    LeftMeta   = LeftSuper,
    RightShift = 344,
    RightCtrl  = 345,
    RightAlt   = 346,
    RightSuper = 347,
    RightMeta  = RightSuper,
};

using MouseState = umap<Mouse, State>;
using KeyState   = umap<Key, State>;

class Info
{
public:
    Info(glm::vec2 displ, glm::vec2 wheel, glm::vec2 cursor, MouseState *ms, KeyState *ks) :
      mDispl(std::move(displ)),
      mWheel(std::move(wheel)),
      mCursor(std::move(cursor)),
      mMouse(ms),
      mKeys(ks)
    {
    }

    inline glm::vec2 displ() const { return mDispl; }

    inline glm::vec2 wheel() const { return mWheel; }

    inline glm::vec2 cursor() const { return mCursor; }

    inline bool pressed(Key k, bool ignoreHold = false) const
    {
        if (!mKeys)
            return false;
        auto const K = *mKeys;
        return K.count(k) > 0 ? (K.at(k) == State::Press or (!ignoreHold and K.at(k) == State::Hold)) : false;
    }

    inline bool pressed(Mouse m, bool ignoreHold = false) const
    {
        if (!mMouse)
            return false;
        auto const M = *mMouse;
        return M.count(m) > 0 ? (M.at(m) == State::Press or (!ignoreHold and M.at(m) == State::Hold)) : false;
    }

private:
    glm::vec2   mDispl  = ZERO2;
    glm::vec2   mWheel  = ZERO2;
    glm::vec2   mCursor = ZERO2;
    MouseState *mMouse  = nullptr;
    KeyState   *mKeys   = nullptr;
};

}  // namespace UI

// COLORS
namespace Color
{
glm::vec3 constexpr Red          = { 1.f, 0.f, 0.f };
glm::vec3 constexpr Green        = { 0.f, 1.f, 0.f };
glm::vec3 constexpr Blue         = { 0.f, 0.f, 1.f };
glm::vec3 constexpr Magenta      = { 1.f, 0.f, 1.f };
glm::vec3 constexpr Yellow       = { 1.f, 1.f, 0.f };
glm::vec3 constexpr Cyan         = { 0.f, 1.f, 1.f };
glm::vec3 constexpr Lime         = { .5f, 1.f, 0.f };
glm::vec3 constexpr Orange       = { 1.f, .3f, 0.f };
glm::vec3 constexpr StrongYellow = { 1.f, .5f, 0.f };
}  // namespace Color

inline i32 constexpr sMaxFloatPrint = 3;  // fmt:glm float precission

}  // namespace btm

//
//
//
//
//
//
//
//
//
//

//=====================================
// PRINT HELPERS
//=====================================

// GLM
template<>
struct fmt::formatter<glm::vec2>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec2 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::sMaxFloatPrint;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f})", v.x, fp, v.y, fp);
    }
};
template<>
struct fmt::formatter<glm::vec3>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec3 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::sMaxFloatPrint;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f},{:.{}f})", v.x, fp, v.y, fp, v.z, fp);
    }
};
template<>
struct fmt::formatter<glm::vec4>
{
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const glm::vec4 &v, FormatContext &ctx) const -> decltype(ctx.out())
    {
        auto const &fp = btm::sMaxFloatPrint;
        return fmt::format_to(ctx.out(), "({:.{}f},{:.{}f},{:.{}f},{:.{}f})", v.x, fp, v.y, fp, v.z, fp, v.w, fp);
    }
};