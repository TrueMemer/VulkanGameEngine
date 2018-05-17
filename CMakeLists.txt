cmake_minimum_required(VERSION 3.7.1)

set(DEFAULT_BUILD_TYPE "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

project(VulkanEngine)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB INCLUDES "${INCLUDE_DIR}/*.hpp")
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp")

if(NOT VK_LIBRARY)
	message(WARNING "Vulkan library not found please specify it with -DVK_LIBRARY='Path to your vulkan lib' or use the cmake-gui to set it")
	set(VK_LIBRARY "" CACHE FILEPATH "Path to your Vulkan library file")
	set(ERROR "YES")
endif()
if(NOT VK_INCLUDE_DIR)
	message(WARNING "Vulkan include directory not found please specify it with -DVK_INCLUDE_DIR='Path to your vulkan include dirrectory' or use the cmake-gui to set it")
	set(VK_INCLUDE_DIR "" CACHE PATH "Path to your Vulkan include directory")
	set(ERROR "YES")
endif()
if(NOT GLM_INCLUDE_DIR)
	message(WARNING "Can't find GLM include directory. Specify it with -DVK_INCLUDE_DIR='/path/to/glm' or use the cmake-gui")
	set(GLM_INCLUDE_DIR "" CACHE PATH "Path to GLM include folder")
	set(ERROR "YES")
endif()

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(ERROR)
	message(FATAL_ERROR "Error occured check output")
endif()

include_directories(${INCLUDE_DIR} ${VK_INCLUDE_DIR} ${GLM_INCLUDE_DIR})

add_executable(VulkanEngine ${SOURCES} ${INCLUDES})

target_link_libraries(VulkanEngine ${VK_LIBRARY})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanEngine)
set_target_properties(VulkanEngine PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/") 