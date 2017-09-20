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

#include "mount.hpp"
#include "dir.hpp"

#if defined(BSD)
#define setpgrp()   setpgid(0,0)
//#define mount(s, t, fs, flags, dp)  mount(s, t, flags, NULL)
#define umount(p) ::unmount(p, 0)
#endif

#if defined(MS_BIND)
#define bind_mount(o, p)    ::mount(o, p, NULL, MS_BIND, NULL)
#define tmp_mount(p)        ::mount("none", p, "tmpfs", 0, "mode=01777")
#define sys_mount(p)        ::mount("none", p, "sysfs", 0, NULL)
#define pts_mount(p)        ::mount("none", p, "devpts", 0, NULL)
#else
#define bind_mount(o, p)    ::mount("nullfs", p, 0, (void *)(o))
#define tmp_mount(p)        (-1)
#define sys_mount(p)        (-1)
#define pts_mount(p)        (-1)
#endif

using namespace std;

bool fsys::mountpoint::verbose = false;

fsys::mountpoint::mountpoint(const std::string& where)
{
    path = where;
}

fsys::mountpoint::~mountpoint()
{
    release();
}       


void fsys::mountpoint::release()
{
    if(path.length() > 0) {
        if(verbose)
            cout << "releasing " << path << endl;
        umount(path.c_str());
        fsys::remove_directory(path);
        path = std::string();
    }
}


fsys::tmpdir::tmpdir(const std::string& where, perms_t mode)
{
    fsys_error err;
    if(!fsys::create_directory(where, mode, err) && err != fsys_error(EEXIST))
        throw bad_path(where);
    if(err)
        fsys::permissions(path, mode);
    path = where;
}

fsys::tmpdir::~tmpdir()
{
    if(path.length() > 0) {
        fsys::remove_directory(path);
        path = std::string();
    }
}

fsys::mount::mount() :
mountpoint()
{
}

fsys::mount::mount(const std::string& where, perms_t mode) :
mountpoint()
{
    if(mode == file_perms::temporary) {
        temp(where, mode);
        return;
    }
}

fsys::mount::mount(const std::string& from, const std::string& where, perms_t mode) :
mountpoint()
{
    bind(from, where, mode);
}

void fsys::mount::proc(const std::string& where)
{
    fsys::create_directory(where, fsys::file_perms::read_all);
#ifdef  CLONE_NEWPID
    int err = ::mount("proc", where.c_str(), "proc", MS_MGC_VAL, NULL);
#else
#if defined(MS_BIND)
    int err = ::mount("none", where.c_str(), "proc", 0, NULL);
#else
    int err = ::mount("procfs", where.c_str(), 0, NULL);
#endif
#endif
    if(err)
        throw bad_mount(where);
    else {
        if(verbose)
            cout << "mounting " << where << endl;
        path = where;
    }
}

void fsys::mount::temp(const std::string& where, perms_t mode)
{
    if(path.length() > 0)
        release();

    fsys::create_directory(where, mode);
    if(tmp_mount(where.c_str()))
        throw bad_mount(where);
    else {
        if(verbose)
            cout << "mounting " << where << endl;
        path = where;
        fsys::permissions(where, mode);
    }
}

void fsys::mount::bind(const std::string& from, const std::string& where, perms_t mode)
{
    if(path.length() > 0)
        release();

    fsys::create_directory(where, mode);
    if(bind_mount(from.c_str(), where.c_str()))
        throw bad_mount(where);
    else {
        if(verbose)
            cout << "mounting " << where << endl;
        path = where;
    }
}

bad_mount::bad_mount(const std::string& path)
{
    msg_path = path;
    msg_text = path + ": failed to mount";  
}

const char *bad_mount::what() const noexcept
{
    return msg_text.c_str();
}

