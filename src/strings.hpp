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

#ifndef __STRINGS_HPP__
#define __STRINGS_HPP__

#include "compiler.hpp"

#include <string>
#include <cctype>
#include <cstring>
#include <vector>

template<typename S = std::string, typename B = std::string>
inline bool begins_with(const S s, const B b) {
	return s.find(b) == 0;
}

template<typename S = std::string, typename E = std::string>
inline bool ends_with(const S s, const E e) {
	std::string::size_type position = s.rfind(e);
    if (position == std::string::npos)
        return false;
    else
        return (s.size() - position) == e.size();
}

template<typename S = std::string>
inline S upper_case(const S s)
{
    S out;
    for(size_t pos = 0; pos < s.size(); ++pos) {
        out += toupper(s[pos]);
    }
    return out;
}

template<typename S = std::string>
inline S lower_case(const S s)
{
    S out;
    for(size_t pos = 0; pos < s.size(); ++pos) {
        out += tolower(s[pos]);
    }
    return out;
}

std::vector<std::string> split(const std::string& str, const std::string& delim);
/*{
    std::vector<std::string> result;
    std::size_t current, prev = 0;
    current = str.find_first_of(delim);
    while(current != std::string::npos) {
        result.push_back(str.substr(prev, current - prev));
        prev = current + 1;
        current = str.find_first_of(delim, prev);
    }
    result.push_back(str.substr(prev, current - prev));
    return result;
}
*/

inline bool eq(const char *p1, const char *p2)
{
    return !strcmp(p1, p2);
}

inline bool eq(const char *p1, const char *p2, size_t len)
{
    return !strncmp(p1, p2, len);
}

/*!
 * Common string functions and extensions to std::string.
 * \file strings.hpp
 */

#endif
