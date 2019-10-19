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

#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <climits>
#include "proc.hpp"

using namespace std;

const char **create_env(const std::unordered_map<string,string>& env)
{
	auto envp = new const char*[env.size() + 1];
	int pos = 0;
    for(const auto& pair : env) {
		auto entry = pair.first + "=" + pair.second;
		envp[pos++] = ::strdup(entry.c_str());
	}
	envp[pos] = nullptr;
	return envp;
}

void fork_command(const char **argv, bool output)
{
	if(!fork_handler(output)) {
		execvp(*argv, (char *const *)argv); // NOLINT
		cerr << "*** failed to execute " << *argv << endl;
		exit(-1);
	}
}

bool fork_handler(bool output)
{
	pid_t pid = fork();
	int status;
	if(pid) {
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status))
			throw WEXITSTATUS(status);
		else
			return true;
	}
	::signal(SIGINT, SIG_DFL);
	::signal(SIGTERM, SIG_DFL);

	if(!output) {
		auto fd = ::open("/dev/null", O_WRONLY); // NOLINT
		::dup2(fd, 1);
		::close(fd);
	}
	for(int fd = 3; fd < _POSIX_OPEN_MAX; ++fd)
		::close(fd);

	return false;
}
