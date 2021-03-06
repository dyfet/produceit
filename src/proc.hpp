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

#ifndef	PROC_HPP
#define PROC_HPP

#include <csignal>
#include <unordered_map>
#include "compiler.hpp"

void fork_command(const char **argv, bool output = true);
bool fork_handler(bool output = true);
const char **create_env(const std::unordered_map<std::string,std::string>& env);

/*!
 * Common process handling functions.
 * \file proc.hpp
 */

#endif
