#ifndef M_TYPES
#define M_TYPES
#pragma once

#include<stdint.h>

namespace m
{
    using U8 = uint8_t;
    using U16 = uint16_t;
    using U32 = uint32_t;
    using U64 = uint64_t;

    using I8 = int8_t;
    using I16 = int16_t;
    using I32 = int32_t;
    using I64 = int64_t;

    using Char = wchar_t;

    using Int = int32_t;
    using UInt = uint32_t;

    using Double = double;
    using Float = float;

    using mBool = bool;
};

#endif //M_TYPES