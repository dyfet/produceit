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

#include <list>
#include <fstream>
#include "proc.hpp"
#include "mount.hpp"
#include "args.hpp"
#include "keyfile.hpp"
#include "output.hpp"
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>

using namespace std;

static args::flag helpflag('h', "--help", "display this list");
static args::flag althelp('?', nullptr, nullptr);
static args::string arch('a', "--arch", "cpu architecture");
static args::flag shell('s', "--shell", "apply current shell");
static args::flag verbose('v', "--verbose", "display operations");
static args::flag xserver('X', nullptr, "X server support");
static keyfile etc_config;

static void cpu(std::string id)
{
    if(id[0] == 'i' && id[2] == '8' && id[3] == '6')
        id = "i386";
    else if(id == std::string("x86_64"))
        id = "amd64";
    auto qemu = etc_config["cpu"][id];
    if(qemu.length() > 0)
        ::setenv("QEMU_CPU", qemu.c_str(), 1);
    arch.set(id);
    if(id == "amd64")
        ::setenv("ARCH", "x86_64", 1);
    else
        ::setenv("ARCH", (*arch).c_str(), 1);
}

int main(int argc, const char **argv)
{
	int exit_code = 0;
	std::string reason;

    ::signal(SIGINT, SIG_IGN);
    ::signal(SIGTERM, SIG_IGN);

	try {
        std::string cwd, distro, qemu_static;
        const char **exec_paths = nullptr;
        int exec_gid, exec_uid;
        auto exec_mask = umask(002);
#ifdef  HAVE_PERSONALITY
        unsigned long exec_pmode = 0;
#endif

		if(geteuid())
			throw runtime_error("requires sudo or setuid binary");

        fsys_error err;
        cwd = fsys::current_path(err);
		if(is(err))
			throw runtime_error("cannot access current directory");

		args cli(argv);
		if(is(helpflag) || is(althelp) || argc < 1) {
			output() << "Usage: shellit [options] [command [..args]]";
			output() << "Execute in chroot environment.";
			output() << "\nOptions:";
			args::help();
			output() << "Report bugs to tychosoft@gmail.com";
			::exit(0);
		}

		exec_paths = cli.paths();
        exec_uid = getuid();
		exec_gid = getgid();

		if(!exec_paths[0])
			throw runtime_error("no distro specified");
		distro = *(exec_paths++);

		etc_config["system"] = {
			{"rootfs",	VAR_PREFIX "/lib/produceit/rootfs"},
			{"homefs",	VAR_PREFIX "/lib/produceit/homes"},
			{"archive",	VAR_PREFIX "/lib/produceit/archive"},
			{"prefix", "/tmp/.produceit"},
		};

        etc_config["env"] = {
            {"PATH", "/bin:/usr/bin:/bin/usr/local/bin"},
            {"LANG", "C"},
            {"LC_ALL", "C"},
            {"LANGUAGE", "C"},
        };

        etc_config["shell"][distro] = "/bin/bash";

		if(!etc_config.load(ETC_PREFIX "/produceit.conf"))
			etc_config.load("/etc/produceit.conf");

        if(!exec_uid)
            etc_config["env"]["PATH"] = "/sbin:/usr/sbin:/usr/local/sbin:" + etc_config["env"]["PATH"];

		auto rootfs = etc_config["system"]["rootfs"];
		auto homefs = etc_config["system"]["homefs"];

		if(!exec_uid && *exec_paths)
			throw runtime_error("root access only for login");

        struct utsname uts;
        uname(&uts);
		if(is(arch)) {
            if(etc_config["qemu"][*arch].length() > 0)
                qemu_static = "/usr/bin/qemu-" + etc_config["qemu"][*arch] + "-static";
			cpu(*arch);
#ifdef HAVE_PERSONALITY
            auto arch_type = etc_config["personality"][*arch];
            if(arch_type == "32")
                exec_pmode = PER_LINUX32;
            else if(*arch == "i386")
                exec_pmode = PER_LINUX32;
#endif
        }
        else
            cpu(uts.machine);

        int nobody = 65535;
        struct group *grp = getgrnam("produceit");
		if(!grp)
			throw runtime_error("produceit group entry missing");

        struct passwd *pwd = getpwnam("nobody");
        if(pwd)
            nobody = pwd->pw_uid;

		pwd = getpwuid((uid_t)exec_uid);
		
		if(exec_uid) {
			if(!pwd)
				throw runtime_error("running as unknown user");

            if(exec_paths[0])
    			exec_gid = pwd->pw_gid;
            else
                exec_gid = grp->gr_gid;

			unsigned pos = 0;
			bool found = false;
            const char *member;
			while((member = grp->gr_mem[pos++]) != nullptr) {
				if(eq(member, pwd->pw_name)) {
					found = true;
					break;
				}
			}	
			
			if(!found)
				throw runtime_error(std::string(pwd->pw_name) + ": not member of produceit");
		}	

        auto distrofs = rootfs + "/" + distro + "_" + *arch;
        if(!fsys::exists(distrofs))
            throw bad_path("rootfs/" + distro + "_" + *arch);

        auto homeuser = homefs + "/" + distro;
        if(!fsys::exists(homeuser)) {
            fsys::file_perms mask = fsys::file_perms::owner_all | fsys::file_perms::group_all;
            fsys:create_directory(homefs, fsys::file_perms::owner_all | fsys::file_perms::group_read | fsys::file_perms::group_exe | fsys::file_perms::others_read | fsys::file_perms::others_exe);
            if(!fsys::create_directory(homeuser, mask))
                throw bad_path("homefs/" + distro);
            fsys::permissions(homeuser, mask | fsys::file_perms::set_gid_on_exe);
            fsys::owner(homeuser, nobody, grp->gr_gid);
        }
        
        auto rootuser = homefs + "/root";
        if(!fsys::exists(rootuser))
            fsys::create_directory(rootuser, fsys::file_perms::owner_all);

		auto archive = etc_config["system"]["archive"];
        auto binary = archive + "/" + distro;

        if(!fsys::exists(binary)) {
            fsys::create_directory(archive, fsys::file_perms::owner_all | fsys::file_perms::group_read | fsys::file_perms::group_exe | fsys::file_perms::others_read | fsys::file_perms::others_exe);
            if(!fsys::create_directory(binary, fsys::file_perms::owner_all | fsys::file_perms::group_all | fsys::file_perms::others_read | fsys::file_perms::others_exe))
                throw bad_path("archive/" + distro);
            fsys::owner(binary, 0, grp->gr_gid);
        }

        // create environment, including for logins

        ::setenv("HOME", pwd->pw_dir, 1);
        ::setenv("USER", pwd->pw_name, 1);
        ::setenv("LOGNAME", pwd->pw_name, 1);
        ::setenv("USERNAME", pwd->pw_name, 1);
        ::setenv("UID", std::to_string(pwd->pw_uid).c_str(), 1);

        if(is(shell) && ::getenv("SHELL"))
            etc_config["env"]["SHELL"] = ::getenv("SHELL");
        else
            etc_config["env"]["SHELL"] = etc_config["shell"][distro];
        ::setenv("SHELL", etc_config["env"]["SHELL"].c_str(), 1);

        etc_config["env"]["HOME"] = pwd->pw_dir;
        etc_config["env"]["USER"] = etc_config["env"]["LOGNAME"] = etc_config["env"]["USERNAME"] = pwd->pw_name;
        etc_config["env"]["UID"] = std::to_string(pwd->pw_uid);
        etc_config["env"]["PRODUCEIT"] = std::to_string(getpid());

        std::list<string> copy_env = {"TERM", "SSH_CONNECTION", "SSH_CLIENT", "SSH_TTY", "ARCH", "HOSTNAME", "HOSTTYPE", "LINES", "SYSID"};
        for(const auto &env_id : copy_env) {
            const char *cp = ::getenv(env_id.c_str());
            if(cp != nullptr)
                etc_config["env"][env_id] = cp;
        }
            
        // create session...
        fsys::tmpdir prefix(etc_config["system"]["prefix"]);
        fsys::tmpdir session(*prefix + "/" + std::to_string(getpid()));
		fsys::mount::trace(is(verbose));

        if(is(verbose)) {
            output() << "creating " << *session;
            output() <<  "distribution: " << distro;
            output() << "architecture: " << *arch;
        }

        // detach what we can...
#ifdef HAVE_UNSHARE
        int namespc = CLONE_NEWNS | CLONE_NEWPID;
        if(!is(xserver))
            namespc |= CLONE_NEWIPC;

        if(unshare(namespc))
            throw runtime_error("cannot detach process space");
#endif

        // copy
        auto copy = etc_config["copy"][distro];

        if(copy.length() == 0)
            copy = "/etc/shadow /etc/gshadow /etc/group /etc/passwd /etc/sudoers /etc/hosts /etc/resolv.conf /etc/hostname";
        auto files = split(copy, " ,;");
        for(const auto path : files) {
            if(is(verbose))
                output() << "copy " << path;
            if(!fsys::copy_file(path, distrofs + "/" + path))
                throw bad_path(path);
        }

        // begin mounting
        fsys::mount top_mount(distrofs, *session, fsys::file_perms::owner_all);
        fsys::mount top_owner(rootuser, *session + "/root", fsys::file_perms::owner_all);
        fsys::mount dev_mount("/dev", *session + "/dev");
        fsys::mount sys_mount("/sys", *session + "/sys");
        fsys::mount pts_mount("/dev/pts", *session + "/dev/pts");
        fsys::mount shm_mount("/dev/shm", *session + "/dev/shm");
        fsys::mount tmp_mount(*session + "/tmp", fsys::file_perms::temporary);
        fsys::mount vtmp_mount(*session + "/var/tmp", fsys::file_perms::temporary);

        // for X support
        fsys::mount lib_dbus, run_dbus;
        if(is(xserver)) {
            lib_dbus.bind("/var/lib/dbus", *session + "/var/lib/dbus");
            run_dbus.bind("/var/run/dbus", *session + "/var/run/dbus");
        }

        // add archive repository
        fsys::mount archive_mount(binary, *session + "/var/archive");
        if(fsys::exists(*session + "/etc/apt/sources.list.d")) {
            auto pkg_archive = *session + "/var/archive/";
            std::ofstream repo(*session + "/etc/apt/sources.list.d/chroot.tmp");
            if(repo.is_open()) {
                if(is(verbose))
                    output() << "creating chroot.list";
                if(fsys::exists(pkg_archive + "/Packages"))
                    repo << "deb file:///var/archive/ ./" << endl;
                if(fsys::exists(pkg_archive + "/Sources"))
                    repo << "deb-src file:///var/archive/ ./" << endl;
                repo.close();
                fsys::rename(*session + "/etc/apt/sources.list.d/chroot.tmp", *session + "/etc/apt/sources.list.d/chroot.list");
            }
        }
        else if(fsys::exists(*session + "/etc/yum.repos.d")) {
            auto pkg_archive = *session + "/var/archive/repodata";
            std::ofstream repo(*session + "/etc/yum.repos.d/chroot.tmp");
            if(repo.is_open()) {
                if(is(verbose))
                    output() << "creating chroot.repo";
                if(fsys::exists(pkg_archive)) {
                    repo << "[chroot]" << endl;
                    repo << "name = chroot packages" << endl;
                    repo << "baseurl = file:///var/archive/" << endl << endl;
                }
                repo.close();
                fsys::rename(*session + "/etc/yum.repos.d/chroot.tmp", *session + "/etc/yum.repos.d/chroot.repo");
            }
        }

        // home is based on mode...
        auto userpath = *session + pwd->pw_dir;
        fsys::mount home_mount;
        if(exec_uid >= 1000) {
            if(exec_paths[0])
                home_mount.bind(pwd->pw_dir, userpath);
            else
                home_mount.bind(homeuser, userpath);
        }

        fsys::mountpoint proc_release(*session + "/proc");

        if(fork_handler())
            return 0;
    
        chroot((*session).c_str());
        fsys::mount proc_mount;
        proc_mount.proc("/proc");

        setgid((gid_t)exec_gid);
        setuid((uid_t)exec_uid);

#ifdef  HAVE_PERSONALITY
        if(exec_pmode)
            personality(exec_pmode);
#endif

        if(!exec_paths[0]) {
            auto env = etc_config["env"];

            env["CWD"] = pwd->pw_dir;
            ::setenv("PATH", env["PATH"].c_str(), 1);
            fsys::current_path(pwd->pw_dir);
            endpwent();
            endgrent();

            const char *shell[3] = {
                env["SHELL"].c_str(),
                "-l",
                nullptr,
            };
            umask(022);
            ::execve(shell[0], (char *const*)shell, (char *const *)create_env(env));
        }
        else {
            ::setenv("CWD", cwd.c_str(), 1);
            fsys::current_path(cwd);
            endpwent();
            endgrent();
            umask(exec_mask);
            ::execvp(exec_paths[0], (char *const*)exec_paths);
        }
        ::exit(-1);
	}
    catch(const bad_path& e) {
        reason = e.what();
        exit_code = 4;
    }
	catch(const bad_mount& e) {
		reason = e.what();
		exit_code = 4;
	}
	catch(const bad_arg& e) {
		reason = e.what();
		exit_code = 3;
	}
	catch(const runtime_error& e) {
		reason = e.what();
		exit_code = 2;
	}
	catch(const exception& e) {
		reason = e.what();
		exit_code = 1;
	}
    catch(int exiting) {
        ::exit(exiting);
    }

	if(exit_code > 0)
		error() << "*** shellit: " << reason;

	return exit_code;
}	
