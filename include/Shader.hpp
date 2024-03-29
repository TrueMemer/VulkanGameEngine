#pragma once

#include "PCH.hpp"
#include "File.hpp"
#include "Engine.hpp"

#include <shaderc\shaderc.hpp>

class ShaderModule
{
public:

	enum Stage
	{
		Vertex, 
		Fragment, 
		Geometry
	};
	
	ShaderModule(Stage stage);
	ShaderModule(Stage stage, std::string path);

	VkShaderModule getVulkanModule() { return vkShaderModule; }

	void init();
	void load(std::string path);
	void compile();
	void createVulkanModule();

	void destroy();

private:

	VkShaderModule vkShaderModule;

	Stage stage;
	
	shaderc_shader_kind kind;

	std::string stageMacro;

	enum Language 
	{
		GLSL,
		SPV,
		UNKNOWN
	} language;

	std::string path;
	std::string source;
	std::vector<U32> spvSource;

	std::string infoLog;
	std::string debugLog;
};