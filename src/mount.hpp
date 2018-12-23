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

#ifndef MOUNT_HPP
#define MOUNT_HPP

#include "compiler.hpp"
#include "dir.hpp"

#ifdef  HAVE_UNSHARE
#include <sched.h>
#endif

#ifdef  HAVE_PERSONALITY
#include <sys/personality.h>
#endif

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

namespace fsys {

	class mountpoint
	{
	public:
		mountpoint(const mountpoint&) = delete;
		const mountpoint& operator=(const mountpoint&) = delete;

        explicit mountpoint(const std::string& where);
		~mountpoint();

		bool operator!() const {
			return path.length() == 0;
		}

        explicit operator bool() const {
			return path.length() > 0;
		}

		static void trace(bool flag) {
			verbose = flag;
		}

		void release();

	protected:
		inline mountpoint() = default;
        std::string path;

		static bool verbose;
	};

	class tmpdir final
	{
	public:
		tmpdir(const std::string& where, perms_t mode = file_perms::owner_all);
		~tmpdir();

		inline const std::string operator*() const {
			return path;
		}
	
	private:
		std::string path;
	};

	class mount : public mountpoint
	{
	public:
        mount() = default;
		mount(const std::string& where, perms_t mode = file_perms::owner_all);
		mount(const std::string& from, const std::string& where, perms_t mode = file_perms::owner_all | file_perms::group_all);

		void bind(const std::string& from, const std::string& where, perms_t mode = file_perms::owner_all | file_perms::group_all);

		void temp(const std::string& where, perms_t mode = file_perms::temporary);
		void proc(const std::string& where);
	};

} // end namespace

class bad_mount final : public std::exception
{
public:
	const std::string path() const {
		return msg_path;
	}

	const char *what() const noexcept override;

protected:
    explicit bad_mount(const std::string& path);

private:
	friend class fsys::mount;

	std::string msg_text;
	std::string msg_path;
};

/*!
 * Raii patterned mount creation and management classes.  This is meant to create and
 * support mounts automatically unmount when falling out of scope.
 * \file mount.hpp
 */

#endif
