#include "../config.hpp"
#include <compiler.hpp>
#include <output.hpp>
#include <keyfile.hpp>

#undef NDEBUG
#include <cassert>

int main(int argc, char **argv)
{
	keyfile keys(SRC_PREFIX "/produceit.conf");
	assert(keys["lxc"]["path"] == "/var/lib/lxc");
}

