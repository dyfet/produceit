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

#ifndef	__DIR_HPP__
#define	__DIR_HPP__

#include "fsys.hpp"

class dir
{
public:
    typedef fsys::perms_t perms_t;
    typedef fsys::file_perms file_perms;

    dir(const std::string& path) noexcept;

    dir(dir&& from) noexcept;

    dir() noexcept;

    ~dir();

    inline explicit operator bool() const {
	return state != nullptr;
    }

    inline bool operator!() const {
	return state == nullptr;
    }

    inline bool operator==(const std::string& text) {
	return fsys::equal(get(), text);
    }

    inline bool operator!=(const std::string& text) {
	return !fsys::equal(get(), text);
    }

    inline std::string operator*() {
	return get();
    }

    inline dir& operator++() {
	next();
	return *this;
    }

    void open(const std::string& path);

    std::string get();

    void next();

    void close();

    inline bool is_open() const {
	return state != nullptr;
    }

protected:
    struct stateinfo;
    struct stateinfo *state;

    fsys_error err;

private:
    dir(const dir&) = delete;
    dir& operator=(const dir&) = delete;
};

typedef	dir dir_t;

/*!
 * A directory class to manipulate directories and process directory trees.
 * \file dir.hpp
 */

#endif
