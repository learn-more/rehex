
#include "platform.hpp"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pybind11/embed.h>

#include "pyenvironment.hpp"
#include "hooks.hpp"

namespace py = pybind11;


struct CaptureOutput
{
	void write(const char* str)
	{
		plugin_hooks::log(str);
	}

	void flush()
	{
	}
};

//namespace
//{
//	PYBIND11_EMBEDDED_MODULE(rehex, m)
//	{
//		// `m` is a `py::module` which is used to bind functions and classes
//		m.def("test", test);
//
//		//py::class_<PrintCatch>(m, "PrintCatch")
//		//	.def("write", &PrintCatch::write)
//		//	.def("flush", &PrintCatch::flush);
//	}
//}

	//wxDir dir("E:\\Projects\\rehex\\plugins");
	//wxString filename;
	//if (dir.GetFirst(&filename, "*.py", wxDIR_FILES))
	//{
	//	do
	//	{
	//		wxFileName fullpath(dir.GetName(), filename);
	//		extensions.push_back(new Extension(fullpath.GetName(), fullpath.GetFullPath()));
	//	} while (dir.GetNext(&filename));
	//}

	//for (Extension* ext : extensions)
	//{
	//	try
	//	{
	//		ext->init();
	//	}
	//	catch (std::runtime_error& exc)
	//	{
	//		printf("%s\n", exc.what());
	//		OutputDebugStringA(exc.what());
	//		OutputDebugStringA("\n");
	//	}
	//}


void pyenvironment::init(const wxString& program)
{
	Py_DontWriteBytecodeFlag = 1;
	Py_VerboseFlag = 2;

	Py_SetProgramName(program.c_str().AsWChar());

	// HACK: since we use a statically compiled python on windows (no python38.dll!) python is unable to find 'home'
	wxString python_share = "E:\\Projects\\rehex\\src\\vcpkg\\vcpkg\\installed\\x64-windows-static\\share\\python3";
	Py_SetPythonHome(python_share.c_str().AsWChar());

	try
	{
		// Now initialize python itself
		py::initialize_interpreter(true);


		// Redirect stdout and stderr
		py::module sys = py::module::import("sys");
		// Register the CaptureOutput object (no init, so python cannot create it!)
		py::class_<CaptureOutput>(sys, "CaptureOutput")
			.def("write", &CaptureOutput::write)
			.def("flush", &CaptureOutput::flush);

		// Replace stdout and stderr
		sys.attr("stdout") = CaptureOutput();
		sys.attr("stderr") = CaptureOutput();

		// Print some stuff
		py::exec("print('hello from python!')");
	}
	catch (const std::runtime_error& exc)
	{
		wxString output = exc.what();
		if (!output.EndsWith("\n"))
		{
			output += "\n";
		}
		plugin_hooks::log(output);
	}
}


void pyenvironment::exit()
{
	py::module sys = py::module::import("sys");
	// Clear out our hooks, so that we do not leak objects
	sys.attr("stdout") = py::object();
	sys.attr("stderr") = py::object();

	py::finalize_interpreter();
}
