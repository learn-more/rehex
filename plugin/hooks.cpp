
#include "platform.hpp"

#include "hooks.hpp"
#include "pydbgallocator.hpp"
#include "pyenvironment.hpp"
#include "luaenvironment.hpp"


void plugin_hooks::init(const wxString& program)
{
	// Initialize the debug allocator
	pydbgallocator::init();

	// Initialize the basic runtime
	pyenvironment::init(program);


	luaenvironment::init(program);

	log("Initializing plugins:");
}


void plugin_hooks::exit()
{
	luaenvironment::exit();


	// Shutdown the interpreter
	pyenvironment::exit();

	// Shutdown the debug allocator (must be last!)
	pydbgallocator::exit();
}
