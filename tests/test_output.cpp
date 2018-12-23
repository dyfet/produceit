#include <output.hpp>
#include <cassert>

int main(int argc, char **argv)
{
	output() << "Testing output";
	debug() << "Testing debug";
	error() << "Testing error";
}

