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

#include "dir.hpp"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

struct dir::stateinfo {
    DIR *handle;
    struct dirent *find;
};

dir::dir() noexcept
{
    state = nullptr;
}

dir::dir(dir&& from) noexcept
{
    state = from.state;
    from.state = nullptr;
}

dir::dir(const std::string& path) noexcept
{
    state = nullptr;
    open(path);
}

dir::~dir()
{
    close();
}

void dir::open(const std::string& path)
{
    close();
    state = new stateinfo;
    state->handle = opendir(path.c_str());
    state->find = nullptr;
    if(!state->handle) {
        err = fsys::error();
        delete state;
        state = nullptr;
        return;
    }
    next();
}

std::string dir::get()
{
    std::string result;
    if(state && state->find) {
        result = state->find->d_name;
        next();
    }
    return result;
} 

void dir::next()
{
    if(!state)
        return;
    
    state->find = ::readdir(state->handle);
    if(!state->find)
        close();
}

void dir::close()
{
    err = 0;

    if(!state)
        return;

    ::closedir(state->handle); 

    delete state;
    state = nullptr;
}

