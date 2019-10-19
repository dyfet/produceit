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

#include "keyfile.hpp"
#include "strings.hpp"
#include <fstream>

bool keyfile::load(const std::string& path)
{
	std::string whitespace(" \t\n\r");
	std::string filename = path;
	if(path[0] == '~')
		filename = std::string(::getenv("HOME")) + path.substr(1);
	else if(path[0] != '/')
		return false;

	std::ifstream file(filename);
	if(!file.is_open())
		return false;

	std::string input;
	std::string section = "_";

	while(std::getline(file, input)) {
		auto begin = input.find_first_not_of(whitespace);
		if(begin == std::string::npos)
			continue;

		auto end = input.find_last_not_of(whitespace);
		auto len = end - begin + 1;
		if(end == std::string::npos)
			len = end;
		input = input.substr(begin, len);
		if(begins_with(input, "[") && ends_with(input, "]")) {
			section = input.substr(1, input.size() - 2);
			continue;
		}
		if(!isalnum(input[0]))
			continue;

		auto pos = input.find_first_of('=');
		if(pos < 1 || pos == std::string::npos)
			continue;

		auto key = input.substr(0, pos);
		auto value = input.substr(pos + 1);
		end = key.find_last_not_of(whitespace);
		if(end != std::string::npos)
			key = key.substr(0, ++end);
		begin = value.find_first_not_of(whitespace);
		if(begin != 0)
			value = value.substr(begin);
		sections[section][key] = value;
	}
	return true;
}
