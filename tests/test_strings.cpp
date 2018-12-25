#include "../config.hpp"
#include <compiler.hpp>
#include <output.hpp>
#include <strings.hpp>

#undef NDEBUG
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
}

