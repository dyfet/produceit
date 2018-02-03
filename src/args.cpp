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

#include <iostream>
#include <algorithm>
#include <cassert>
#include "args.hpp"

static args::Option *first = nullptr;
static args::Option *last = nullptr;
static args *instance = nullptr;

const std::string args::exec_path()
{
	if(!instance)
		return std::string();

	return instance->argv0;
}

const std::string args::exec_name()
{
	if(!instance)
		return std::string();
	
	return instance->name;
}

args::Option::Option(char shortopt, const char *longopt, const char *value, const char *help) noexcept
{
	if(last) {
		last->next = this;
		last = this;
	}
	else
		first = last = this;

	while(longopt && *longopt == '-')
		++longopt;

	short_option = shortopt;
	long_option = longopt;
	uses_option = value;
	help_string = help;
	trigger_option = false;
	used_count = 0;
}

args::Option::~Option()
{
}

void args::Option::fail(const char *reason)
{
	if(long_option)
		throw bad_arg(reason, long_option);
	else
		throw bad_arg(reason, short_option);
}

args::flag::flag(char shortopt, const char *longopt, const char *help, bool single_use) noexcept : 
Option(shortopt, longopt, nullptr, help) 
{
	single = single_use;
}

args::group::group(const char *help) noexcept :
args::Option(0, nullptr, nullptr, help)
{
}

args::string::string(char shortopt, const char *longopt, const char *helpopt, const char *type, const std::string value) noexcept :
args::Option(shortopt, longopt, type, helpopt)
{
	text = value;
}

args::charcode::charcode(char shortopt, const char *longopt, const char *helpopt, const char *type, char value) noexcept :
args::Option(shortopt, longopt, type, helpopt)
{
	code = value;
}

args::number::number(char shortopt, const char *longopt, const char *helpopt, const char *type, long value) noexcept :
args::Option(shortopt, longopt, type, helpopt)
{
	num = value;
}

args::number::number() noexcept
{
	if(instance)
		num = instance->value;
	else
		num = 0l;
}

const std::string args::list::operator[](unsigned item)
{
	if(instance)
		return instance->operator[](item);
	else
		return std::string();
}

size_t args::list::size() 
{
	if(instance)
		return instance->size();
	return 0;
}

void args::group::assign(const char *value) 
{
}

void args::flag::assign(const char *value) 
{
	if(single && used_count)
		fail("option already used");

	++used_count;
}

void args::string::assign(const char *value) 
{
	if(used_count)
		fail("option already assigned");

	text = value;
	++used_count;
}

void args::charcode::assign(const char *value)
{
	long number;
	char *endptr = nullptr;

	if(used_count)
		fail("option already assigned");

	++used_count;
	if(value[1] == 0) {
		code = value[0];
		return;
	}

	number = strtol(value, &endptr, 0);
	if(!endptr || *endptr != 0 || number < 0 || number > 255)
		fail("invalid character value");

	code = (char)number;
}

void args::number::assign(const char *value)
{
	char *endptr = nullptr;

	if(used_count)
		fail("option already assigned");

	++used_count;

	num = strtol(value, &endptr, 0);
	if(!endptr || *endptr != 0)
		fail("invalid value assigned");
}

args::args(const char **list, mode_t mode)
{
	assert(list != nullptr);

	if(instance)
		abort();

	instance = this;

	bool skip;

	const char *arg = list[0];
	const char *base = arg;
	const char *s1 = strrchr(arg, '/');
	const char *s2 = strrchr(arg, '\\');

	if(s1 || s2)
		base = std::max(++s1, ++s2);
	else {
		s1 = strrchr(arg, ':');
		if(s1)
			base = ++s1;
	}

	name = lower_case(std::string(base));

	argv0 = list[0];
	++list;

	while(*list) {
		arg = *(list++);
		skip = false;
		if(eq(arg, "--"))
			break;

		switch(mode) {
		case DASH:
		case ALL:
			if(arg[0] == '-' && arg[1] >= '0' && arg[1] <= '9') {
				if(value)
					throw bad_arg("numeric value already set", arg);

				value = atol(arg);
				skip = true;
			}
		default:
			break;
		}

		switch(mode) {
		case PLUS:
		case ALL:
			if(arg[0] == '+' && arg[1] >= '0' && arg[1] <= '9') {
				if(value)
					throw bad_arg("numeric value already set", arg);

				value = atol(++arg);
				skip = true;
			}
		default:
			break;
		}

		if(skip)
			continue;

		if(*arg != '-') {
			--list;
			break;
		}

		bool has_short = true;
		const char *op_value = nullptr;

		++arg;
		if(*arg == '-') {
			++arg;
			has_short = false;
		}

		Option *op = first;

		// long argument parsing first...
		while(op) {
			if(!op->long_option) {
				op = op->next;
				continue;
			}
			size_t len = strlen(op->long_option);
			op_value = nullptr;
			if(eq(op->long_option, arg, len)) {
				if(arg[len] == '=')
					op_value = arg + len + 1;
				else if(op->uses_option)
					op_value = *(list++);
				if(op_value && !op->uses_option)
					throw bad_arg("invalid argument", arg);

				if(!op_value && op->uses_option)
					throw bad_arg("missing argument", arg);

				break;	
			}
			op = op->next;
		}

		if(op) {
			op->assign(op_value);
			continue;
		}

		if(!has_short)
			throw bad_arg("unknown command option", arg);

		while(*arg) {
			op = first;
			while(op) {
				if(op->short_option == *arg)
					break;
				op = op->next;
			}
			
			if(!op)
				throw bad_arg("unknown option", *arg);

			op_value = nullptr;

			if(!op->trigger_option && op->uses_option) {
				op_value = *(list++);
				if(op_value == nullptr)
					throw bad_arg("argument missing", arg);
			}			
			op->assign(op_value);
			++arg;
		}
	}

	offset = list;
	while(*list)
		argv.emplace_back(*(list++));
}

void args::help()
{
	Option *op = first;
	size_t count = 0;

	while(op != nullptr) {
		if(!op->help_string) {
			op = op->next;
			continue;
		}
		
		size_t hp = 0;

		if(op->short_option && op->long_option && op->uses_option && !op->trigger_option) {
			std::cout << "  -" << op->short_option << " .., ";
            hp = 9;
        }
        else if(op->short_option && op->long_option) {
			std::cout << "  -" << op->short_option << ", ";
            hp = 6;
        }
        else if(op->long_option) {
			std::cout << "  ";
            hp = 2;
        }
        else if(op->uses_option) {
			std::cout << "  -" << op->short_option << " " << op->uses_option;
            hp = 5 + strlen(op->uses_option);
        }
        else if(op->short_option) {
			std::cout << "  -" << op->short_option;
            hp = 5;
        }
        else {      // grouping separator
            if(count)
				std::cout << std::endl << op->help_string << ":" << std::endl;
            else
				std::cout << op->help_string << ":" << std::endl;
            op = op->next;
            continue;
        }

		++count;

        if(op->long_option && op->uses_option) {
			std::cout << "--" << op->long_option << "=" << op->uses_option;
            hp += strlen(op->long_option) + strlen(op->uses_option) + 3;
        }   
        else if(op->long_option) {
			std::cout << "--" << op->long_option;
            hp += strlen(op->long_option) + 2;
        }

		if(hp > 29) {
			std::cout << std::endl;
            hp = 0;
        }
        while(hp < 30) {
			std::cout << ' ';
            ++hp;
        }

		const char *hs = op->help_string;
        while(*hs) {
            if(*hs == '\n' || (((*hs == ' ' || *hs == '\t')) && (hp > 75))) {
                std::cout << std::endl << "                              ";
                hp = 30;
            }
            else if(*hs == '\t') {
                if(!(hp % 8)) {
					std::cout << ' ';
                    ++hp;
                }
                while(hp % 8) {
					std::cout << ' ';
                    ++hp;
                }
            }
            else
				std::cout << *hs;
            ++hs;
        }

		std::cout << std::endl;
		op = op->next;
	}
}

bad_arg::bad_arg(const char *reason, const char *value) noexcept
{
	if(*value != '+' && *value != '-')
		msg += "--";

	msg += value;
	msg += ": ";
	msg += reason;
}

bad_arg::bad_arg(const char *reason, const char code) noexcept
{
	msg += "-";
	msg += code;
	msg += ": ";
	msg += reason;
}

const char *bad_arg::what() const noexcept
{
	return msg.c_str();
}

