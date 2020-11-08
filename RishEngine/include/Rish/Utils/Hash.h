#pragma once
#ifndef RISH_ENGINE_UTILS_HASH_H
#define RISH_ENGINE_UTILS_HASH_H

#include <Rish/rlpch.h>

// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

#define RL_MAKE_HASHABLE(type, ...)                        \
    namespace std {                                        \
        template<> struct hash<type> {                     \
            std::size_t operator()(const type &t) const {  \
                std::size_t ret = 0;                       \
                hash_combine(ret, __VA_ARGS__);            \
                return ret;                                \
            }                                              \
        };                                                 \
    }

#endif //GAME_PROJECT_HASH_H
