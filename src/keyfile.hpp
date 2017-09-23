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

#ifndef	__KEYFILE_HPP__
#define	__KEYFILE_HPP__

#include "strings.hpp"
#include <map>

class keyfile final
{
public:
	inline keyfile() {};

	inline std::map<std::string, std::string>& operator[](const std::string& section) {
		return sections[section];
	}

	inline std::map<std::string, std::string> at(const std::string& section = "_") const {
		return sections.at(section);
	}

	bool load(const std::string& path);

private:
	keyfile(const keyfile&) = delete;
	keyfile& operator=(const keyfile&) = delete;

	std::map<std::string, std::map<std::string, std::string>> sections;
};

/*!
 * Generic key store parser for [section] key=value config files.
 * \file keyfile.hpp
 */

#endif
