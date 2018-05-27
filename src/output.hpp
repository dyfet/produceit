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

#ifndef __OUTPUT_HPP__
#define __OUTPUT_HPP__

#include <sstream>

class output final : public std::stringstream
{
public:
	output& operator=(const output&) = delete;
	output(const output&) = delete;

	output() : std::stringstream() {};

	~output() override;
};

class debug final : public std::stringstream
{
public:
	debug& operator=(const debug&) = delete;
	debug(const debug&) = delete;

	debug() : std::stringstream() {};

	~debug() override;
};

class error final : public std::stringstream
{
public:
	error& operator=(const error&) = delete;
	error(const error&) = delete;

	error() : std::stringstream() {};

	~error() override;
};

class crit final : public std::stringstream
{
public:
	crit& operator=(const crit&) = delete;
	crit(const crit&) = delete;

	crit(int code) : std::stringstream(), excode(code) {};

	~crit() override;

private:
	int excode;
};

#endif		
