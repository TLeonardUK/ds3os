/*
 * Dark Souls 3 - Open Server
 * Copyright (C) 2021 Tim Leonard
 *
 * This program is free software; licensed under the MIT license.
 * You should have received a copy of the license along with this program.
 * If not, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

// Better ways to handle this if we go for a partial specialization approach.

template <typename T>
T SwapEndian(const T& Input)
{
    T Output = T();

    int ByteCount = sizeof(T);
    for (int i = 0; i < ByteCount; i++)
    {
        int OutputOffset = i;
        int InputOffset = (ByteCount - 1) - i;

        *(reinterpret_cast<char*>(&Output) + OutputOffset) = *(reinterpret_cast<const char*>(&Input) + InputOffset);
    }

    return Output;
}

template <typename T>
T BigEndianToHostOrder(const T& Input)
{
#if __BIG_ENDIAN__
    return Input;
#else
    return SwapEndian(Input);
#endif
}

template <typename T>
T LittleEndianToHostOrder(const T& Input)
{
#if __BIG_ENDIAN__
    return SwapEndian(Input);
#else
    return Input;
#endif
}

template <typename T>
T HostOrderToBigEndian(const T& Input)
{
#if __BIG_ENDIAN__
    return Input;
#else
    return SwapEndian(Input);
#endif
}

template <typename T>
T HostOrderToLittleEndian(const T& Input)
{
#if __BIG_ENDIAN__
    return SwapEndian(Input);
#else
    return Input;
#endif
}