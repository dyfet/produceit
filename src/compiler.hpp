/*
 * Copyright 2017 Tycho Softworks.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef	COMPILER_HPP
#define COMPILER_HPP

#ifdef __clang__
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma GCC diagnostic ignored "-Wunused-result"

#if __cplusplus <= 199711L
#error C++11 compliant compiler required
#endif

#ifndef UNUSED
#define UNUSED(x)                     (void)x
#endif

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <string>

#include "config.hpp"

template<typename T>
inline bool is(T& object) {
    return object.operator bool();
}

/*!
 * Provides pragmas, macros, and common includes for use in every compilation unit.  This
 * includes some common inline functions.
 * \file compiler.hpp
 */

#endif
