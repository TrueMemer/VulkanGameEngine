// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 

#include "targetver.hpp"
#include <windows.h>

#undef min
#undef max

#define VK_USE_PLATFORM_WIN32_KHR

#endif

#define ENABLE_VULKAN_VALIDATION
#include <vulkan/vulkan.h>

#define LOG_INFO(m) { std::cout << "INFO: " << m << std::endl; }

#define LOG_WARN(m) { std::cout << __FILE__ << " Line: " << __LINE__ << std::endl; std::cout << "WARN: " << m << std::endl; }

#define LOG_FATAL(m) { std::cout << __FILE__ << " Line: " << __LINE__ << std::endl; std::cout << "FATAL: " << m << std::endl; DebugBreak(); }

#include "Types.hpp"

#include <iostream>

#include <vector>

#include <map>

#include <limits>

#include <cstddef>

#include <algorithm>

#include <fstream>

#include <chrono>

#include <queue>