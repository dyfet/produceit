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

#include "output.hpp"
#include <iostream>
#include <mutex>

using namespace std;

static std::mutex ioguard;

output::~output()
{
	lock_guard<mutex> lock(ioguard);
	cout << rdbuf() << endl;
}

debug::~debug()
{
#ifndef NDEBUG
	lock_guard<mutex> lock(ioguard);
	cout << rdbuf() << endl;
#endif
}

error::~error()
{
	lock_guard<mutex> lock(ioguard);
	cerr << rdbuf() << endl;
}

crit::~crit()
{
	lock_guard<mutex> lock(ioguard);
	cerr << rdbuf() << endl;

	if(excode)
		::exit(excode);
}

