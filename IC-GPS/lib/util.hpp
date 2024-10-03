#pragma once
#ifndef __UTIL_HPP__
#define __UTIL_HPP__

template <class T>
const T clamp(const T &v, const T &lo, const T &hi)
{
    if (v < lo)
        return lo;
    if (hi < v)
        return hi;

    return v;
}

#endif // __UTIL_HPP__