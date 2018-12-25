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

#ifndef STRINGS_HPP
#define STRINGS_HPP

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
inline bool ends_with(const S& s, const E& e) {
    std::string::size_type position = s.rfind(e);
    if (position == std::string::npos)
        return false;
    if(s.size() < e.size())
        return false;
    else
        return s.substr(s.size() - position) == e;
}

template<typename S = std::string>
inline S upper_case(const S& s)
{
    S out;
    for(size_t pos = 0; pos < s.size(); ++pos) {
        out += toupper(s[pos]);
    }
    return out;
}

template<typename S = std::string>
inline S lower_case(const S& s)
{
    S out;
    for(size_t pos = 0; pos < s.size(); ++pos) {
        out += tolower(s[pos]);
    }
    return out;
}

inline std::string strip(const std::string& str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}

inline bool eq(const char *p1, const char *p2)
{
    if(!p1 && !p2)
        return true;
    if(!p1 || !p2)
        return false;
    return !strcmp(p1, p2);
}

inline bool eq(const char *p1, const char *p2, size_t len)
{
    if(!p1 && !p2)
        return true;
    if(!p1 || !p2)
        return false;
    return !strncmp(p1, p2, len);
}

// convenience function for string conversions if not explicit for template
inline std::string upper_case(const char *s)
{
    return upper_case(std::string(s));
}

inline std::string lower_case(const char *s)
{
    return lower_case(std::string(s));
}

inline std::string strip(const char *s)
{
    return strip(std::string(s));
}

inline bool begins_with(const char *s, const char *b)
{
    return begins_with<std::string>(s, b);
}

inline bool ends_with(const char *s, const char *e)
{
    return ends_with<std::string,std::string>(s, e);
}



std::vector<std::string> split(const std::string& str, const std::string& delim = " ");

/*!
 * Common string functions and extensions to std::string.
 * \file strings.hpp
 */

#endif
