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

#ifndef __TEMPORARY_HPP__
#define __TEMPORARY_HPP__

#include "compiler.hpp"

template <typename T>
class temporary
{
public:
    temporary(size_t size = 1) {
        array = std::get_temporary_buffer<T>(size);
        new(array.first) T[array.second];
    }    

    ~temporary() {
        if(array.first) {
            std::return_temporary_buffer(array.first);
            array.first = nullptr;
            array.second = 0;
        }
    }

    operator T&() const {
        return array.first[0];
    }        

    T& operator*() const {
        return array.first[0];
    }

    T* operator->() const {
        return &array.first[0];
    }

    operator bool() const {
        return array.first != NULL;
    }

    bool operator!() const {
        return array.first == NULL;
    }

    temporary& operator=(const T& initial) {
        array.first[0] = initial;
        return *this;
    }

    void release() {
        if(array.first) {
            std::return_temporary_buffer(array.first);
            array.first = nullptr;
            array.second = 0;
        }
    }

    size_t count() const {
        return array.second;
    }

    T& operator[](size_t index) const {
        if(index >= (size_t)array.second)
            throw std::out_of_range("temporary array out of bound");
        return array.first[index];
    }

    T* operator()(size_t index) const {
        if(index >= array.second)
            throw std::out_of_range("temporary array out of bound");
        return &array.first[index];
    }

    void operator()(size_t index, const T value) {
        if(index >= array.second)
            throw std::out_of_range("temporary array out of bound");
        array.first[index] = value;
    }

protected:
    std::pair<T*, std::ptrdiff_t> array;

private:
    temporary(const temporary& tmp) = delete;
    temporary& operator=(const temporary& tmp) = delete;
};

/*!
 * Construct temporary heap objects with scope in fast storage.
 * \file temporary.hpp
 */

#endif
