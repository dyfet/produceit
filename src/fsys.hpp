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

#ifndef	FSYS_HPP
#define	FSYS_HPP

#include "compiler.hpp"
#include <cerrno>
#include <ctime>
#include <unistd.h>

class fsys_error
{
public:
    inline fsys_error() noexcept {
        errcode = 0;
    }

    inline fsys_error(int value) noexcept {
        errcode = value;
    }

    inline fsys_error(const fsys_error& copy) noexcept {
        errcode = copy.errcode;
    }

    inline fsys_error(fsys_error&& from) noexcept {
        errcode = from.errcode;
        from.errcode = 0;
    }

    inline ~fsys_error() = default;

    inline bool operator==(const fsys_error& other) const noexcept {
        return errcode == other.errcode;
    }

    inline bool operator!=(const fsys_error& other) const noexcept {
        return errcode != other.errcode;
    }

    inline int operator*() const noexcept {
        return errcode;
    }

    explicit inline operator int() const noexcept {
        return errcode;
    }

    explicit inline operator bool() const noexcept {
        return errcode > 0;
    }

    inline bool operator!() const noexcept {
        return errcode == 0;
    }

    inline fsys_error& operator=(int value) noexcept {
        errcode = value;
        return *this;
    }

    inline fsys_error& operator=(const fsys_error& copy) noexcept = default;

    inline fsys_error& operator=(fsys_error&& from) noexcept {
        errcode = from.errcode;
        from.errcode = 0;
        return *this;
    }

private:
    int errcode;
};

//namespace saves on static public member defs...
namespace fsys {
    enum class file_perms : unsigned
    {
        no_perms        = 0,

        owner_read      =  0400,
        owner_write     =  0200,
        owner_exe       =  0100,
        owner_all       =  0700,

        group_read      =   040,
        group_write     =   020,
        group_exe       =   010,
        group_all       =   070,

        others_read     =    04,
        others_write    =    02,
        others_exe      =    01,
        others_all      =    07,

        shared_access   = static_cast<unsigned>(owner_all | group_all | others_read | others_exe),

        all_all     = static_cast<unsigned>(owner_all | group_all | others_all), // 0777
        read_all    = static_cast<unsigned>(owner_read | owner_exe | group_read | group_exe | others_read | others_exe),

        set_uid_on_exe  = 04000,
        set_gid_on_exe  = 02000,
        sticky_bit      = 01000,
        temporary       = 01777,

        perms_mask      = static_cast<unsigned>(all_all | set_uid_on_exe | set_gid_on_exe | sticky_bit), // 07777

        perms_not_known = 0xffff,

        add_perms       = 0x1000,
        remove_perms    = 0x2000,
        symlink_perms   = 0x4000
    };

    inline file_perms operator&(file_perms __x, file_perms __y) {
        return static_cast<file_perms> (static_cast<int>(__x) & static_cast<int>(__y));
    }

    inline file_perms operator|(file_perms __x, file_perms __y) {
        return static_cast<file_perms> (static_cast<int>(__x) | static_cast<int>(__y));
    }

    inline file_perms operator^(file_perms __x, file_perms __y) {
        return static_cast<file_perms> (static_cast<int>(__x) ^ static_cast<int>(__y));
    }

    inline file_perms operator~(file_perms __x) {
        return static_cast<file_perms>(~static_cast<int>(__x));
    }

    inline file_perms &operator&=(file_perms &__x, file_perms __y) {
        __x = __x & __y;
        return __x;
    }

    inline file_perms &operator|=(file_perms &__x, file_perms __y) {
        __x = __x | __y;
        return __x;
    }

    inline file_perms &operator^=(file_perms &__x, file_perms __y) {
        __x = __x ^ __y;
        return __x;
    }

    using perms_t = file_perms;

    enum class file_type {
        none = 0,
        regular,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        other
    };

    class file_status
    {
    public:
        perms_t perms;
        file_type type;
        time_t access, modify, change;
        uintmax_t size;
    };

    using status_t = file_status;

    extern fsys_error lasterr;
    fsys_error error();
    std::string extname(const std::string& path);
    std::string basename(const std::string& path, const std::string& ext = "");
    std::string dirname(const std::string& path);
    bool equal(const std::string& l, const std::string& r);
    bool permissions(const std::string& path, perms_t mode, fsys_error& err = lasterr);
    bool owner(const std::string& path, uid_t uid = getuid(), gid_t gid = getgid(), fsys_error& err = lasterr);
    bool exists(const std::string& path);
    time_t last_write_time(const std::string& path, fsys_error& err = lasterr);
    status_t status(const std::string& path, fsys_error& err = lasterr);
    void current_path(const std::string& path, fsys_error& err = lasterr);
    const std::string current_path(fsys_error& err = lasterr);
    bool create_directory(const std::string& path, perms_t mode = file_perms::owner_all | file_perms::group_all, fsys_error& err = lasterr);
    bool remove_directory(const std::string& path, fsys_error& err = lasterr);
    bool remove_dsc(const std::string& source, fsys_error& err = lasterr);
    bool copy_dsc(const std::string& source, const std::string& target, fsys_error& err = lasterr);
    bool copy_file(const std::string& from, const std::string& to, fsys_error& err = lasterr);
    bool rename(const std::string& from, const std::string& to, fsys_error& err = lasterr);
    bool remove(const std::string& path, fsys_error& err = lasterr);

    inline bool is_directory(file_status s) {
        return s.type == file_type::directory;
    }

    inline bool is_directory(const std::string& path) {
        return is_directory(status(path));
    }

    inline bool is_regular_file(file_status s) {
        return s.type == file_type::regular;
    }

    inline bool is_regular_file(const std::string& path) {
        return is_regular_file(status(path));
    }

    inline bool is_block_file(file_status s) {
        return s.type == file_type::block;
    }

    inline bool is_block_file(const std::string& path) {
        return is_block_file(status(path));
    }

    inline bool is_character_file(file_status s) {
        return s.type == file_type::character;
    }

    inline bool is_character_file(const std::string& path) {
        return is_character_file(status(path));
    }

    inline bool is_fifo(file_status s) {
        return s.type == file_type::fifo;
    }

    inline bool is_fifo(const std::string& path) {
        return is_fifo(status(path));
    }

    inline bool is_other(file_status s) {
        return s.type == file_type::other;
    }

    inline bool is_other(const std::string& path) {
        return is_other(status(path));
    }

    inline bool is_symlink(file_status s) {
        return s.type == file_type::symlink;
    }

    inline bool is_symlink(const std::string& path) {
        return is_symlink(status(path));
    }

    inline bool is_socket(file_status s) {
        return s.type == file_type::socket;
    }

    inline bool is_socket(const std::string& path) {
        return is_socket(status(path));
    }

    bool is_hidden_file(const std::string& path);
};

class bad_pkg final : public std::exception
{
public:
    explicit bad_pkg(const std::string& path);

	const std::string path() const {
		return msg_path;
	}

	const char *what() const noexcept override;

private:
	std::string msg_text;
	std::string msg_path;
};

class bad_path final : public std::exception
{
public:
    explicit bad_path(const std::string& path);

	const std::string path() const {
		return msg_path;
	}

	const char *what() const noexcept override;

private:
	std::string msg_text;
	std::string msg_path;
};

/*!
 * File system manipulations.  This is a simplified version of the filesystem offered in
 * C++17, and is meant to be code pattern related for easy migration in the future.
 * \file fsys.hpp
 */

/*!
 * \namespace fsys
 * A namespace for filesystem-like functions.  This is more convenient as a namespace since
 * the underlying code is just type definitions and what would be static members only.
 */

#endif
