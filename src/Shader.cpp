#include "Shader.hpp"

ShaderModule::ShaderModule(Stage s) : stage(s)
{
	switch (stage)
	{
	case Vertex:
		kind = shaderc_shader_kind::shaderc_glsl_vertex_shader; stageMacro = "VERTEX"; return;
	case Fragment:
		kind = shaderc_shader_kind::shaderc_glsl_fragment_shader; stageMacro = "FRAGMENT"; return;
	case Geometry:
		kind = shaderc_shader_kind::shaderc_glsl_geometry_shader; stageMacro = "GEOMETRY"; return;
	}
}

ShaderModule::ShaderModule(Stage s, std::string path) : ShaderModule(s) 
{
	load(path);
}

void ShaderModule::load(std::string path)
{
	int i = 0;
	while (path[i] != '.')
	{
		++i;
		if (i > path.length())
		{
			LOG_WARN("Bad shader file name format: " << path);
		}
	}
	std::string extension;
	extension.assign(&path[i + 1]);
	if (extension == "glsl" || extension == "GLSL")
	{
		language = GLSL;
	}
	else if (extension == "spv" || extension == "SPV")
	{
		language = SPV;
	}
	else
	{
		LOG_WARN("Bad shader file extension: " << path << " - '.glsl' and 'spv' supported");
		language = UNKNOWN;
	}

	File file;
	if (!file.open(path))
	{
		LOG_WARN("Can't open shader file: " << path);
	}

	if (language == GLSL)
	{
		source.resize(file.getSize());
		file.readFile(&source[0]);
	}
	if (language == SPV)
	{
		spvSource.resize(file.getSize());
		file.readFile(&spvSource[0]);
	}
}

void ShaderModule::compile()
{
	if (language == GLSL)
	{
		shaderc::Compiler c;
		shaderc::CompileOptions o;
		o.SetAutoBindUniforms(true);
		o.AddMacroDefinition(stageMacro);
		auto res = c.CompileGlslToSpv(source, kind, path.c_str(), o);
		spvSource.assign(res.begin(), res.end());
	}
}

void ShaderModule::createVulkanModule()
{
	if (spvSource.size() == 0) 
	{
		LOG_FATAL("SPIRV source missing, cannot compile shader: " << path);
		return;
	}
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spvSource.size() * sizeof(int);
	createInfo.pCode = spvSource.data();

	if (vkCreateShaderModule(Engine::renderer->vkLogicalDevice, &createInfo, nullptr, &vkShaderModule) != VK_SUCCESS) 
	{
		LOG_FATAL("Failed to create shader module");
	}
}

void ShaderModule::destroy()
{
	const auto r = Engine::renderer;
	vkDestroyShaderModule(r->vkLogicalDevice, vkShaderModule, nullptr);
}