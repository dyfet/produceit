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

#include <list>
#include <fstream>
#include <cstring>
#include <csignal>
#include "proc.hpp"
#include "mount.hpp"
#include "args.hpp"
#include "keyfile.hpp"
#include "output.hpp"
#include <cstdlib>
#include <climits>
#include <sys/utsname.h>
#include <sys/wait.h>

#ifndef UID_MAX
#define UID_MAX 1000
#endif

using namespace std;

namespace { // anon namespace exclusive to buildit
args::flag helpflag('h', "--help", "display this list");
args::flag althelp('?', nullptr, nullptr);
args::string arch('a', "--arch", "cpu architecture");
args::flag binonly('b', "--binary", "binary architecture only");
args::flag depends('d', "--depends", "install build dependencies");
args::flag update('u', "--update", "update before packaging");
args::flag verbose('v', "--verbose", "display operations");
keyfile etc_config;

const char *apt_update[] = {
    "apt-get update",
    "/usr/bin/apt-get",
    "update",
    nullptr,
};

const char *apt_upgrade[] = {
    "apt-get dist-upgrade",
    "/usr/bin/apt-get",
    "-y",
    "dist-upgrade",
    nullptr,
};

const char *apt_autoremove[] = {
    "apt-get autoremove",
    "/usr/bin/apt-get",
    "-y",
    "autoremove",
    nullptr,
};

const char *apt_autoclean[] = {
    "apt-get autoclean",
    "/usr/bin/apt-get",
    "autoclean",
    nullptr,
};

const char *dnf_update[] = {
    "dnf update",
    "/usr/bin/dnf",
    "update",
    nullptr,
};

const char *dnf_clean[] = {
    "dnf clean",
    "/usr/bin/dnf",
    "clean",
    "all",
    nullptr,
};

void cpu(std::string id) {
    if (id[0] == 'i' && id[2] == '8' && id[3] == '6')
        id = "i386";
    else if (id == std::string("x86_64"))
        id = "amd64";
    auto qemu = etc_config["cpu"][id];
    if (qemu.length() > 0)
        ::setenv("QEMU_CPU", qemu.c_str(), 1);
    arch.set(id);
    if (id == "amd64")
        ::setenv("ARCH", "x86_64", 1);
    else
        ::setenv("ARCH", (*arch).c_str(), 1);
}

void debian(const std::string &pkg) {
    fsys::mount tmp_mount("/tmp", fsys::file_perms::temporary);
    fsys::create_directory("/tmp/buildd");
    std::string from = "/var/origin/" + pkg;
    std::ifstream dsc(from);
    std::string buffer;
    std::vector<std::string> deps;
    while (std::getline(dsc, buffer)) {
        if (buffer.substr(0, 14) == "Build-Depends:") {
            auto list = split(buffer.substr(14), ",");
            for (const auto &item : list) {
                deps.push_back(strip(item));
            }
        }
    }
    dsc.close();

    if (fork_handler(is(verbose)))
        return;

    // not using depends yet...

    fsys::current_path("/tmp/buildd");
    std::string pkgclean = pkg.substr(0, pkg.find_first_of('_')) + "-build-deps";

    const char *bflag = "-b";
    if (is(binonly))
        bflag = "-B";

    try {
        const char *dpkg_source[] = {"dpkg-source", "-x", from.c_str(), "source", nullptr};
        const char *dpkg_build[] = {"dpkg-buildpackage", bflag, "-us", nullptr};
        const char *dpkg_depends[] = {"mk-build-deps", "-i", nullptr};
        const char *dpkg_clean[] = {"apt-get", "-y", "purge", "--auto-remove", pkgclean.c_str(), nullptr};

        fork_command(dpkg_source, is(verbose));
        fsys::current_path("source");
        if (is(depends))
            fork_command(dpkg_depends, is(verbose));
        fork_command(dpkg_build, is(verbose));
        if (is(depends))
            fork_command(dpkg_clean, is(verbose));
        dir results("..");
        std::string entry;
        bool changes = false;
        while (!(entry = results.get()).empty()) {
            if (entry.substr(entry.find_last_of('.') + 1) == "changes") {
                changes = true;
                if (!fsys::copy_dsc("../" + entry, "/var/results")) {
                    cerr << "*** buildit: " + entry + ": failed to copy out" << endl;
                    throw -1;
                }
            }
        }
        if (!changes)
            error() << "## " + pkg + ": failed";
        else
            output() << "## " + pkg + ": success";
    }
    catch (int exiting) {
        ::exit(exiting);
    }
    ::exit(0);
}

void task(const char *argv[], const char *envp[]) {
    fsys::mount tmp_mount("/tmp", fsys::file_perms::temporary);
    fsys::create_directory("/tmp/buildd");

    if (fork_handler(is(verbose)))
        return;

    fsys::current_path("/tmp/buildd");
    execve(argv[0],(char *const *)argv, (char *const *) envp); // NOLINT
    ::exit(-1);
}
} // anonymous namespace

int main(int argc, const char **argv)
{
    int exit_code = 0;
    std::string reason;

    ::signal(SIGINT, SIG_IGN); // NOLINT
    ::signal(SIGTERM, SIG_IGN); // NOLINT

    try {
        std::string distro, qemu_static;
        std::list<const char **> update_tasks;
        const char **pkg_paths = nullptr;
#ifdef  HAVE_PERSONALITY
        unsigned long exec_pmode = 0;
#endif
        umask(002);

        if(geteuid())
            throw runtime_error("requires sudo or setuid binary");

        args cli(argv);
        if(is(helpflag) || is(althelp) || argc < 1) {
            output() << "Usage: buildit [options] distro [pkgs..]";
            output() << "Build package in chroot environment.";
            output() << "\nOptions:";
            args::help();
            output() << "Report bugs to tychosoft@gmail.com";
            ::exit(0);
        }

        pkg_paths = cli.paths();
        if(!pkg_paths[0])
            throw runtime_error("no distro specified");

        distro = *(pkg_paths++);

        etc_config["system"] = {
            {"rootfs",  VAR_PREFIX "/lib/produceit/rootfs"},
            {"homes",   VAR_PREFIX "/lib/produceit/homes"},
            {"source",  VAR_PREFIX "/lib/produceit/source"},
            {"archive", VAR_PREFIX "/lib/produceit/archive"},
            {"prefix", "/tmp/.produceit"},
        };

        etc_config["env"] = {
            {"PATH", "/bin:/usr/bin:/bin/usr/local/bin"},
            {"LANG", "C"},
            {"LC_ALL", "C"},
            {"LANGUAGE", "C"},
            {"DEBIAN_FRONTEND", "noninteractive"},
        };

        etc_config["shell"][distro] = "/bin/bash";
        etc_config["env"]["PATH"] = "/sbin:/usr/sbin:/usr/local/sbin:" + etc_config["env"]["PATH"];

        if(!etc_config.load(ETC_PREFIX "/produceit.conf"))
            etc_config.load("/etc/produceit.conf");

        auto rootfs = etc_config["system"]["rootfs"];
        auto homefs = etc_config["system"]["homefs"] + "/root";
        auto source = etc_config["system"]["source"];

        struct utsname uts{};
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

        gid_t nobody = UID_MAX;
        auto grp = getgrnam("produceit");
        if(!grp)
            throw runtime_error("produceit group entry missing");

        auto pwd = getpwnam("nobody");
        if(pwd)
            nobody = pwd->pw_uid;

        auto distrofs = rootfs + "/" + distro + "_" + *arch;
        if(!fsys::exists(distrofs))
            throw bad_path("rootfs/" + distro + "_" + *arch);
        if(!fsys::exists(homefs))
            fsys::create_directory(homefs, fsys::file_perms::owner_all);

        auto archive = etc_config["system"]["archive"];
        auto binary = archive + "/" + distro;

        if(!fsys::exists(binary)) {
            fsys::create_directory(archive, fsys::file_perms::owner_all | fsys::file_perms::group_read | fsys::file_perms::group_exe | fsys::file_perms::others_read | fsys::file_perms::others_exe);
            if(!fsys::create_directory(binary, fsys::file_perms::owner_all | fsys::file_perms::group_all | fsys::file_perms::others_read | fsys::file_perms::others_exe))
                throw bad_path("archive/" + distro);
            fsys::owner(binary, 0, grp->gr_gid);
        }

        if(!fsys::exists(source)) {
            if(!fsys::create_directory(source, fsys::file_perms::owner_all | fsys::file_perms::group_all))
                throw bad_path("source/");
            fsys::owner(source, 0, grp->gr_gid);
        }

        // create environment, including for logins

        etc_config["env"] = {
            {"HOME", "/root"},
            {"USER", "root"},
            {"LOGNAME", "root"},
            {"USERNAME", "root"},
            {"UID", "0"},
            {"CWD", "/tmp/buildd"},
            {"SHELL", "/bin/sh"},
            {"PRODUCEIT", std::to_string(getpid())},
        };

        // create session...
        fsys::tmpdir prefix(etc_config["system"]["prefix"]);
        fsys::tmpdir session(*prefix + "/" + std::to_string(getpid()));
        fsys::tmpdir sources(*prefix + "/sources");
        fsys::mount::trace(is(verbose));

        if(is(verbose)) {
            output() << "creating " << *session;
            output() << "distribution: " << distro;
            output() << "architecture: " << *arch;
        }

        // detach what we can...
#ifdef HAVE_UNSHARE
        if(unshare(CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWIPC))
            throw runtime_error("cannot detach process space");
#endif

        // copy distro support
        auto copy = etc_config["copy"][distro];

        if(copy.length() == 0)
            copy = "/etc/shadow /etc/gshadow /etc/group /etc/passwd /etc/sudoers /etc/hosts /etc/resolv.conf /etc/hostname";
        auto files = split(copy, " ,;");
        for(const auto& path : files) {
            if(is(verbose))
                output() << "copy " << path;
            if(!fsys::copy_file(path, distrofs + "/" + path)) { // NOLINT
                if(path == "/etc/gshadow")
                    continue;
                throw bad_path(path);
            }
        }

        // count and copy source packages
        int pkg_count = 0;
        while(pkg_paths && pkg_paths[pkg_count]) {
            const char *cp = ::strrchr(pkg_paths[pkg_count], '/');
            if(cp)
                ++cp;
            else
                cp = pkg_paths[pkg_count];
            const char *ext = strrchr(cp, '.');
            auto len = strlen(cp);
            if(len > 7 && !strcmp(cp + len - 7, ".src.rpm")) { // NOLINT
                if(!fsys::copy_file(pkg_paths[pkg_count], source + "/" + cp))
                    throw bad_pkg(cp);
            }
            else if(ext && !strcmp(ext, ".dsc")) {
                if(!fsys::copy_dsc(pkg_paths[pkg_count], source))
                    throw bad_pkg(cp);
            }
            else
                throw bad_pkg(cp);
            pkg_paths[pkg_count++] = cp;
        }

        // begin mounting
        fsys::mount top_mount(distrofs, *session, fsys::file_perms::owner_all);
        fsys::mount top_owner(homefs, *session + "/root", fsys::file_perms::owner_all);
        fsys::mount dev_mount("/dev", *session + "/dev");
        fsys::mount sys_mount("/sys", *session + "/sys");
        fsys::mount pts_mount("/dev/pts", *session + "/dev/pts");
        fsys::mount shm_mount("/dev/shm", *session + "/dev/shm");
        fsys::mount tmp_mount(*session + "/var/tmp", fsys::file_perms::temporary);

        // more setup
        setuid(0); // NOLINT
        setgid(nobody); // NOLINT
        setegid(nobody); // NOLINT
        seteuid(0); // NOLINT
        endpwent();
        endgrent();

        fsys::mount source_mount(source, *session + "/var/origin");
        fsys::mount result_mount(fsys::current_path(), *session + "/var/results");
        fsys::mount archive_mount(binary, *session + "/var/archive");
        if(fsys::exists(*session + "/etc/apt/sources.list.d")) {
            if(is(update)) {
                update_tasks.push_back(apt_update);
                update_tasks.push_back(apt_upgrade);
                update_tasks.push_back(apt_autoremove);
                update_tasks.push_back(apt_autoclean);
            }

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
            if(is(update)) {
                update_tasks.push_back(dnf_update);
                update_tasks.push_back(dnf_clean);
            }

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

        fsys::mountpoint proc_release(*session + "/proc");

        // create pid1
        if(fork_handler())
            return 0;

        chroot((*session).c_str());
        fsys::mount proc_mount;
        proc_mount.proc("/proc");

#ifdef  HAVE_PERSONALITY
        if(exec_pmode)
            personality(exec_pmode);
#endif

        auto envp = create_env(etc_config["env"]);
        try {
            output() << "update tasks " << update_tasks.size();
            for(auto args : update_tasks) {
                output() << "-- task " << args[0];
                task(++args, envp);
            }

            output() << "build tasks " << pkg_count;
            while(pkg_paths && *pkg_paths) {
                output() << "-- build " << *pkg_paths;
                const char *ext = strrchr(*pkg_paths, '.');
                if(!strcmp(ext, ".dsc"))
                    debian(*pkg_paths);
                ++pkg_paths;
            }
        }
        catch(const bad_mount& e) {
            error() << "*** buildit: " << e.what();
            exit_code = -1;
        }
        catch(int status) {
            exit_code = status;
        }
        ::exit(exit_code);

    }
    catch(const bad_pkg& e) {
        reason = e.what();
        exit_code = 4;
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
        error() << "*** buildit: " << reason;

    return exit_code;
}
