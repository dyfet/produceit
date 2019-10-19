/*
 * Copyright (C) 2018-2019 David Sugar <tychosoft@gmail.com>.
 *
 * This file is free software; as a special exception the author gives
 * unlimited permission to copy and/or distribute it, with or without
 * modifications, as long as this notice is preserved.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "config.hpp"
#include <compiler.hpp>
#include <output.hpp>
#include <strings.hpp>
#include <cassert>

int main(int argc, char **argv)
{
	std::string text = "hi,bye,gone";
	auto list = split(text, ",");
	output() << list[0];
	output() << list[1];
	output() << list[2];
	assert(list.size() == 3);
	assert(list[0] == "hi");
	assert(list[1] == "bye");
	assert(list[2] == "gone");

	assert(upper_case("Hi There") == "HI THERE");
	assert(lower_case<std::string>("Hi There") == "hi there");
	assert(strip("   testing ") == "testing");
	assert(begins_with<std::string>("belong", "be"));
	assert(ends_with("belong", "ong"));

	std::string input = "[test]";
	assert(begins_with(input, "["));
	assert(!ends_with(input, "Z"));
	assert(ends_with(input, "]"));
	assert(begins_with(input, std::string("[")));
	assert(ends_with(input, std::string("]")));

}

