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

#include <output.hpp>
#include <cassert>

int main(int argc, char **argv)
{
	output() << "Testing output";
	debug() << "Testing debug";
	error() << "Testing error";
}

