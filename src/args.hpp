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

#ifndef	ARGS_HPP
#define	ARGS_HPP

#include "strings.hpp"

#include <vector>
#include <exception>

class args final
{
public:
    enum class Mode { NONE, PLUS, DASH, ALL };

    class Option
    {
    public:
	Option(const Option&) = delete;
	Option& operator=(const Option&) = delete;

	inline operator bool() const {
	    return used_count > 0;
	}

	inline bool operator!() const {
	    return used_count == 0;
	}

    protected:
	friend class args;

	char short_option;
	const char *long_option;
	const char *uses_option;
	const char *help_string;
	bool trigger_option;
	unsigned used_count;

	Option(char shortopt = 0, const char *longopt = nullptr, const char *value = nullptr, const char *help = nullptr) noexcept;

	virtual ~Option();

	virtual void assign(const char *value) = 0;

	void fail(const char *reason);
	
    private:
	Option *next;
    };

    class list final
    {
    public:
	list& operator=(const list&) = delete;
	list(const list&) = delete;

	inline list() noexcept = default;

	const std::string operator[](unsigned index);
	size_t size();
    };	

    class flag final : public Option
    {
    public:
	flag(char short_opt, const char *long_opt = nullptr, const char *help_opt = nullptr, bool single_use = true) noexcept;

	inline unsigned operator*() const {
	    return used_count;
	}

	inline void set(unsigned value = 1) {
	    used_count = value;
	}

	inline flag& operator=(unsigned value) {
	    used_count = value;
	    return *this;
	}

    private:
	bool single;

	void assign(const char *value) final;
    };

    class group final : public Option
    {
    public:
	group(const char *help) noexcept;

    private:
	void assign(const char *value) final;
    };

    class string final : public Option
    {
    public:
	string(char shortopt, const char *longopt = nullptr, const char *help = nullptr, const char *type = "text", const std::string& dvalue = std::string()) noexcept;
		
	inline void set(const std::string& string) {
	    text = string;
	}

	inline string& operator=(const std::string& string) {
	    text = string;
	    return *this;
	}

	inline const std::string operator*() const {
	    return text;
	}

	inline const char *c_str() const {
	    return text.c_str();
	}

    private:
	std::string text;

	void assign(const char *value) final;
    };

    class charcode final : public Option
    {
    public:
	charcode(char shortopt, const char *longopt = nullptr, const char *help = nullptr, const char *type = "text", char value = ' ') noexcept;
		
	inline void set(char value) {
	    code = value;
	}

	inline charcode& operator=(char value) {
	    code = value;
	    return *this;
	}

	inline char operator*() const {
	    return code;
	}

    private:
	char code;
    
	void assign(const char *value) final;
    };

    class number final : public Option
    {
    public:
	number(char shortopt, const char *longopt = nullptr, const char *help = nullptr, const char *type = "text", long value = 0) noexcept;

	number() noexcept;
		
	inline void set(long value) {
	    num = value;
	}

	inline number& operator=(long value) {
	    num = value;
	    return *this;
	}

	inline long operator*() const {
	    return num;
	}

    private:
	long num;

	void assign(const char *value) final;
    };

    args(const args& from) = delete;
    args& operator=(const args& from) = delete;

    args(const char **argv, Mode mode = Mode::NONE);
    
    inline const std::string operator[](unsigned index) const {
	return argv[index];
    }

    inline const std::string operator*() const {
	return argv0;
    }

    inline size_t size() const {
	return argv.size();
    }

    inline const char **paths() const {
	return offset;
    }

    static const std::string exec_path();
    static const std::string exec_name();
    static void help();

private:
    friend class args::number;

    std::vector<std::string> argv;
    std::string argv0;
    std::string name;
    const char **offset;
    long value;
};

class bad_arg final : public std::exception
{
public:
    bad_arg(const char *reason, const char *value) noexcept;
    bad_arg(const char *reason, char code) noexcept;

    const char *what() const noexcept override;

private:
    std::string msg;
};

/*!
 * Argument parsing through initialized args::xxx objects.
 * \file args.hpp
 */

#endif
