/*
 * Copyright 2017-2018 Tycho Softworks.
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
#include "args.hpp"
#include "fsys.hpp"
#include "output.hpp"
#include "keyfile.hpp"
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>

#define COUNTER "lxc-count-%s"

using namespace std;

namespace { // anon namespace for application
args::flag helpflag('h', "--help", "display this list");
args::flag althelp('?', nullptr, nullptr);
args::string lxcname('n', "--name", "lxc instance name");
args::flag preserve('p', "--preserve", "preserve environment");
args::string userid('u', "--user", "alternate user login");
args::flag xserver('X', nullptr, "X server support");
sem_t *sem = nullptr;
keyfile etc_config;

void shutdown(int code)
{
    char buf[64];
    int count = -1;

    if(sem) {
        sem_wait(sem);
        sem_getvalue(sem, &count);
        sem_close(sem);
    }

    if(count == 0) {
        output() << "stopping " << *lxcname << "...";
        snprintf(buf, sizeof(buf), "lxc-stop -n %s", lxcname.c_str());
        system(buf);
        snprintf(buf, sizeof(buf), COUNTER, lxcname.c_str());
        sem_unlink(buf);
    }
    if(code)
        exit(code);
}

void startup()
{
    char buf[64];
    int count;

    snprintf(buf, sizeof(buf), COUNTER, lxcname.c_str());
    sem = sem_open(buf, O_RDWR | O_CREAT, 0600, 0);
    if(!sem)
        crit(10) << "*** lxcuser: cannot open semaphore";

    sem_getvalue(sem, &count);
    sem_post(sem);
    if(count != 0)
        return;

    output() << "starting " << *lxcname << "...";
    snprintf(buf, sizeof(buf), "lxc-start -n %s", lxcname.c_str());
    if(system(buf))
        crit(-1) << "*** lxcuser: start failed";

    signal(SIGINT, &shutdown);
    signal(SIGQUIT, &shutdown);
    signal(SIGTERM, &shutdown);

    snprintf(buf, sizeof(buf), "lxc-wait -n %s -s RUNNING", lxcname.c_str());
    system(buf);
    sleep(2);
}

bool is_member(struct group *grp, const char *user)
{
    unsigned pos = 0;
    const char *member;
    while((member = grp->gr_mem[pos++]) != nullptr) {
        if(eq(member, user))
            return true;
    }   
    return false;
}

} // end anon namespace

int main(int argc, const char **argv)
{
    int exit_code = 0;
    std::string reason;

    ::signal(SIGINT, SIG_IGN); // NOLINT
    ::signal(SIGTERM, SIG_IGN); // NOLINT

    try {
        if(geteuid())
            throw runtime_error("requires sudo or setuid binary");

        args cli(argv);
        if(is(helpflag) || is(althelp) || argc < 1) {
            output() << "Usage: lxcuser [options]";
            output() << "Startup and login to an lxc session.";
            output() << "\nOptions:";
            args::help();
            output() << "Report bugs to tychosoft@gmail.com";
            ::exit(0);
        }

        if(!is(lxcname))
            throw runtime_error("no lxc name specified");

        auto pwd = getpwuid(getuid());
        auto grp = getgrnam("produceit");

        if(!pwd)
            throw runtime_error("cannot identify current user");

        if(!grp)
            throw runtime_error("group entry missing");

        if(getuid() && !is_member(grp, pwd->pw_name))
            throw runtime_error(std::string(pwd->pw_name) + ": not member of group");

        etc_config["lxc"] = {
            {"path", "/var/lib/lxc"},
            {"user", pwd->pw_name},
        };

        if(!etc_config.load(ETC_PREFIX "/produceit.conf"))
            etc_config.load("/etc/produceit.conf");

        auto lxc_path = etc_config["lxc"]["path"] + "/" + *lxcname;
        if(!fsys::is_directory(lxc_path))
            throw bad_path(lxc_path);

        if(!is(userid))
            userid.set(etc_config["lxc"]["user"]);
        else if(getuid()) {
            grp = getgrnam("sudo");
            if(!grp)
                grp = getgrnam("wheel");
            
            if(!is_member(grp, pwd->pw_name))
                throw runtime_error(std::string(pwd->pw_name) + ": has to be in sudo or wheel to change user id");    
        }

        char path[64];
        endpwent();
        endgrent();
        setuid(0); // NOLINT
        setgid(0); // NOLINT
        startup();
        if(is(preserve))
            snprintf(path, sizeof(path), "lxc-attach -n %s -- /bin/su -p %s", lxcname.c_str(), userid.c_str());
        else
            snprintf(path, sizeof(path), "lxc-attach -n %s -- /bin/su - %s", lxcname.c_str(), userid.c_str());
        system(path);
        shutdown(exit_code);
    }
    catch(const bad_path& e) {
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
        error() << "*** lxcuser: " << reason;

    return exit_code;
}   
