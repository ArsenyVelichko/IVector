#pragma once

#include <string>

using GetBrokerFunc = void* (*)();

#ifndef _WIN32

#include <dlfcn.h>

#define LIB void*
#define LOAD_LIBRARY(libName) dlopen((std::string(libName) + ".so").c_str(), RTLD_LAZY)
#define GET_BROKER(handler) dlsym(handler, "getBroker")
#define CLOSE_LIBRARY(handler) dlclose(handler)

#else

#include <windows.h>

#define LIB HINSTANCE
#define LOAD_LIBRARY(libName) ::LoadLibrary(libName)
#define GET_BROKER(lib) ::GetProcAddress(lib, "getBroker")
#define CLOSE_LIBRARY(lib) ::FreeLibrary(lib)

#endif