//
// Created by chenjs on 19-6-17.
//

#ifndef IMAGEPROCESSOR6_UTILS_H
#define IMAGEPROCESSOR6_UTILS_H

#include <cstddef>

typedef void* any_ptr;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef i64 isize;
typedef u64 usize;

template <typename T>
using ptr = T*;

template <typename T>
using lVal = T&;

template <typename T>
using rVal = T&&;

template <typename T>
using fn = std::function<T>;

template <typename T>
using consumer = std::function<void(T)>;

template <typename T>
using supplier = std::function<T()>;

template <typename T>
using predicate = std::function<bool(T)>;

using runnable = std::function<void()>;

#define trivial default

#endif //IMAGEPROCESSOR6_UTILS_H
