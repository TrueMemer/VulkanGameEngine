#pragma once

#include "PCH.hpp"
#include "Event.hpp"

struct WindowCreateInfo 
{
	WindowCreateInfo() : positionX(0), positionY(0), borderless(0) {}

#ifdef _WIN32
	HINSTANCE win32Instance;
#endif

	int width;
	int height;

	const char *title;
	
	int positionX;
	int positionY;

	bool borderless;

};

class Window
{
public:
	Window() {};
	~Window() {};

	void create(WindowCreateInfo *c);
	void destroy();

	VkSurfaceKHR vkSurface;

	U32 resX;
	U32 resY;

	std::string windowName;

	EventQ eventQ;

#ifdef _WIN32
	
	HINSTANCE win32Instance;
	WNDCLASSEX win32WindowClass;
	HWND win32WindowHandler;

#endif

	bool processMessages();
};

