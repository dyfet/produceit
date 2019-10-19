/*
 * Copyright (C) 2017-2019 David Sugar <tychosoft@gmail.com>.
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

#ifndef	KEYFILE_HPP
#define	KEYFILE_HPP

#include "strings.hpp"
#include <unordered_map>

class keyfile final
{
public:
    keyfile(const keyfile&) = delete;
    keyfile& operator=(const keyfile&) = delete;

    keyfile() = default;
    explicit keyfile(const std::string& path) : keyfile() {
        load(path);
    }

    inline std::unordered_map<std::string, std::string>& operator[](const std::string& section) {
	    return sections[section];
    }

	inline std::unordered_map<std::string, std::string> at(const std::string& section = "_") const {
		return sections.at(section);
	}

	bool load(const std::string& path);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
};

/*!
 * Generic key store parser for [section] key=value config files.
 * \file keyfile.hpp
 */

#endif
