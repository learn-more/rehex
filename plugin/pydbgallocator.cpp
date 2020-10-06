
#include "platform.hpp"

#if defined(_DEBUG) && defined(_WIN32)
#include <crtdbg.h>
#endif

#include "pydbgallocator.hpp"
#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern "C"
{
	// Python internal functions that clean up more data
	void _PyImport_Fini2(void);
	void _PyPathConfig_ClearGlobal(void);
	void _Py_ClearStandardStreamEncoding(void);
	void _Py_ClearArgcArgv(void);
	void _PyRuntime_Finalize(void);
}

static void*
_REHex_RawMalloc(void* ctx, size_t size)
{
	if (size == 0)
		size = 1;

	// Ignore python allocations for leak tracking
#if defined(_DEBUG) && defined(_WIN32)
	return _malloc_dbg(size, _CRT_BLOCK, nullptr, 0);
#else
	return malloc(size);
#endif
}

static void*
_REHex_RawCalloc(void* ctx, size_t nelem, size_t elsize)
{
	if (nelem == 0 || elsize == 0) {
		nelem = 1;
		elsize = 1;
	}

#if defined(_DEBUG) && defined(_WIN32)
	return _calloc_dbg(nelem, elsize, _CRT_BLOCK, nullptr, 0);
#else
	return calloc(nelem, elsize);
#endif
}

static void*
_REHex_RawRealloc(void* ctx, void* ptr, size_t size)
{
	if (size == 0)
		size = 1;

#if defined(_DEBUG) && defined(_WIN32)
	return _realloc_dbg(ptr, size, _CRT_BLOCK, nullptr, 0);
#else
	return realloc(ptr, size);
#endif
}

static void
_REHex_RawFree(void* ctx, void* ptr)
{
#if defined(_DEBUG) && defined(_WIN32)
	return _free_dbg(ptr, _CRT_BLOCK);
#else
	free(ptr);
#endif
}

static PyMemAllocatorEx _dbgallocator = {
	NULL,
	_REHex_RawMalloc,
	_REHex_RawCalloc,
	_REHex_RawRealloc,
	_REHex_RawFree
};


void pydbgallocator::init()
{
	PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &_dbgallocator);
}


void pydbgallocator::exit()
{
	_PyImport_Fini2();
	_PyPathConfig_ClearGlobal();
	_Py_ClearStandardStreamEncoding();
	_Py_ClearArgcArgv();
	_PyRuntime_Finalize();
}