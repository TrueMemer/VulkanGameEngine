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

#include <unordered_map>

#include <chrono>

#include <queue>

#include <functional>

#include <array>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi