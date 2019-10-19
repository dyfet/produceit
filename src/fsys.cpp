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

#include <iostream>
#include <fstream>
#include "fsys.hpp"
#include "strings.hpp"
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

fsys_error fsys::lasterr = 0;

bool fsys::is_hidden_file(const std::string& path)
{
    std::string::size_type pos = path.find_last_of("/\\:");
    if(pos == std::string::npos && path[0] == '.')
        return true;
    else if(pos != std::string::npos && path[pos + 1] == '.')
        return true;
    return false;
}

time_t fsys::last_write_time(const std::string& path, fsys_error& err)
{
	auto stat = status(path, err);
	return stat.modify;
}

fsys::status_t fsys::status(const std::string& path, fsys_error& err)
{
    struct stat ino{};
    file_status info{};

    memset(&info, 0, sizeof(info));
    memset(&ino, 0, sizeof(ino));

    info.type = file_type::none;
    info.perms = file_perms::no_perms;
	info.access = info.modify = info.change = 0;

    if(!stat(path.c_str(), &ino)) {
        info.perms = static_cast<file_perms>((ino.st_mode) & 07777); // NOLIMIT
        info.access = ino.st_atime;
        info.modify = ino.st_mtime;
        info.change = ino.st_ctime;
        info.size = static_cast<uintmax_t>(ino.st_size);
        switch(S_IFMT & ino.st_mode) {
        case S_IFCHR:
            info.type = file_type::character;
            break;
        case S_IFREG:
            info.type = file_type::regular;
            break;
        case S_IFDIR:
            info.type = file_type::directory;
            break;
        case S_IFIFO:
            info.type = file_type::fifo;
            break;
        case S_IFBLK:
            info.type = file_type::block;
            break;
        case S_IFLNK:
            info.type = file_type::symlink;
            break;
        case S_IFSOCK:
            info.type = file_type::socket;
            break;
        default:
            info.type = file_type::other;
        }
    }
    else
        err = fsys::error();
    return info;
}

void fsys::current_path(const std::string& path, fsys_error& err)
{
    if (chdir(path.c_str()))
		err = fsys::error();
}

const std::string fsys::current_path(fsys_error& err)
{
    char path[256];
    if (nullptr == getcwd(path, sizeof(path))) {
		err = fsys::error();
		return std::string();
	}
    return std::string(path);
}

bool fsys::permissions(const std::string& path, perms_t value, fsys_error& err)
{
    if(chmod(path.c_str(), static_cast<mode_t>(value))) {
		err = fsys::error();
		return false;
	}
    return true;
}

bool fsys::owner(const std::string& path, uid_t uid, gid_t gid, fsys_error& err)
{
    if(chown(path.c_str(), uid, gid)) {
		err = fsys::error();
		return false;
	}
	return true;
}

bool fsys::exists(const std::string& path)
{
    return access(path.c_str(), F_OK) == 0;
}

bool fsys::equal(const std::string& l, const std::string& r)
{
#if defined(__apple__)
    return lower_case(l) == lower_case(r);
#else
    return l == r;
#endif
}

std::string fsys::extname(const std::string& path)
{
    std::string::size_type lpos = path.find_last_of("/\\:");
    if(lpos == std::string::npos)
        lpos = path.find_last_of('.');
    else
        lpos = path.substr(++lpos).find_last_of('.');
    if(lpos == std::string::npos)
        return std::string();

#if defined(__apple__)
    return lower_case(path.substr(++lpos));
#else
    return path.substr(++lpos);
#endif
}

std::string fsys::dirname(const std::string& path)
{
    std::string::size_type lpos = path.find_last_of("/\\:");
    std::string::size_type fpos = path.find_first_of("/\\:");
    if(lpos == std::string::npos)
        return ".";
    if(lpos == fpos && lpos == path.size() - 1)
        return ".";

    return path.substr(0, lpos);
}

std::string fsys::basename(const std::string& path, const std::string& ext)
{
    std::string::size_type pos = path.find_last_of("/\\:");
    std::string out = path;

    if(pos)
        out = path.substr(++pos);

#if defined(__apple__)
    if(ext.size() && ends_with(lower_case(out), lower_case(ext)))
        out = out.substr(0, out.size() - ext.size());
#else
    if(!ext.empty() && ends_with(out, ext))
        out = out.substr(0, out.size() - ext.size());
#endif
    return out;
}

bool fsys::create_directory(const std::string& path, perms_t mode, fsys_error& err)
{
    if(::mkdir(path.c_str(), static_cast<mode_t>(mode))) {
		err =  fsys::error();
		return false;
	}
    return true;
}

bool fsys::remove_directory(const std::string& path, fsys_error& err)
{
    if(::rmdir(path.c_str())) {
        err = fsys::error();
		return false;
	}
    return true;
}

bool fsys::rename(const std::string& from, const std::string& to, fsys_error& err)
{
	if(::rename(from.c_str(), to.c_str())) {
		err = fsys::error();
		return false;
	}
	return true;
}

bool fsys::remove(const std::string& path, fsys_error& err)
{
	if(::remove(path.c_str())) {
		err = fsys::error();
		return false;
	}
	return true;
}

bool fsys::remove_dsc(const std::string& source, fsys_error& err)
{
	if(!exists(source))
		return false;

	auto prefix = fsys::dirname(source);
	auto basename = fsys::basename(source);
	std::ifstream changes(source);
	std::string buffer;
	bool files = false;

	while(getline(changes, buffer)) {
        if(buffer.substr(0, 6) == "Files:") {
            files = true;
            continue;
        }
        if(buffer[0] != ' ' || !files) {
            files = false;
            continue;
        }
		auto file = split(buffer).back();
		if(!remove(prefix + "/" + file, err)) // NOLINT
			return false;
    }
	return remove(source, err);
}

bool fsys::copy_dsc(const std::string& source, const std::string& target, fsys_error& err)
{
	if(!exists(source))
		return false;

	auto prefix = fsys::dirname(source);
	auto basename = fsys::basename(source);
	std::ifstream changes(source);
	std::string buffer;
	bool files = false;

	while(getline(changes, buffer)) {
        if(buffer.substr(0, 6) == "Files:") {
            files = true;
            continue;
        }
        if(buffer[0] != ' ' || !files) {
            files = false;
            continue;
        }
		auto file = split(buffer).back();
		if(!copy_file(prefix + "/" + file, target + "/" + file, err)) // NOLINT
			return false;
    }
	return copy_file(source, target + "/" + basename, err);
}

bool fsys::copy_file(const std::string& from, const std::string& to, fsys_error& err)
{
	std::ifstream source(from, ios::binary);
	if(!source.is_open()) {
		err = fsys::error();
		return false;
	}

	std::ofstream target(to + ".tmp", ios::binary);
	if(!target.is_open()) {
		err = fsys::error();
		return false;
	}

	auto ino = fsys::status(from);
	target << source.rdbuf();
	source.close();
	target.close();
	if(!fsys::permissions(to + ".tmp", ino.perms)) {
		remove(to + ".tmp");
		return false;
	}
	if(!rename(to + ".tmp", to, err)) {
		remove(to + ".tmp");
		return false;
	}
	return true;
}

fsys_error fsys::error()
{
	return {errno};
}

bad_pkg::bad_pkg(const std::string& path)
{
	msg_path = path;
	msg_text = path + ": invalid source package";
}

const char *bad_pkg::what() const noexcept
{
	return msg_text.c_str();
}

bad_path::bad_path(const std::string& path)
{
	msg_path = path;
	msg_text = path + ": not available";
}

const char *bad_path::what() const noexcept
{
	return msg_text.c_str();
}
