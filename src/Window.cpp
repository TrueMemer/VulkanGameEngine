#include "PCH.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Keyboard.hpp"

#ifdef _WIN32
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

void Window::create(WindowCreateInfo *c)
{
#ifdef _WIN32
	memset(&win32WindowClass, 0, sizeof(WNDCLASSEX));
	win32WindowClass.cbSize = sizeof(WNDCLASSEX);
	win32WindowClass.lpfnWndProc = WndProc;
	win32WindowClass.lpszClassName = LPCSTR(c->title);
	win32WindowClass.hInstance = c->win32Instance;
	win32WindowClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	win32WindowClass.style = CS_OWNDC;
	win32WindowClass.cbClsExtra = 0;
	win32WindowClass.cbWndExtra = 0;

	if (!RegisterClassEx(&win32WindowClass)) 
	{
		LOG_FATAL("Failed to register Window class");
	}

	windowName = c->title;

	DWORD windowStyle;

	if (c->borderless) 
	{
		windowStyle = WS_POPUP;
	}
	else 
	{
		windowStyle = (WS_OVERLAPPED | WS_SYSMENU);
	}

	win32WindowHandler = CreateWindowEx(0, win32WindowClass.lpszClassName, win32WindowClass.lpszClassName, windowStyle,
		c->positionX, c->positionY, c->width, c->height, 0, 0, c->win32Instance, 0);

	if (!win32WindowHandler)
		LOG_FATAL("Could not create win32 window");

	ShowWindow(win32WindowHandler, 1);

	VkWin32SurfaceCreateInfoKHR sci;
	sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	sci.pNext = 0;
	sci.flags = 0;
	sci.hinstance = Engine::win32Instance;
	sci.hwnd = win32WindowHandler;
	
	vkCreateWin32SurfaceKHR(Engine::vkInstance, &sci, 0, &vkSurface);

#endif
}

bool Window::processMessages()
 {
#ifdef _WIN32
	MSG msg;
	if (PeekMessage(&msg, win32WindowHandler, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return true;
	}
	return false;
#endif
}

void Window::destroy()
{
	vkDestroySurfaceKHR(Engine::vkInstance, vkSurface, nullptr);

	#ifdef _WIN32
	UnregisterClass((LPCSTR)windowName.c_str(), Engine::win32Instance);
	#endif
}

#ifdef _WIN32

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_KEYDOWN:
		{
			Keyboard::keyState[wParam] = 1;
			auto check = [](U8 pKey) -> bool { return Keyboard::isKeyPressed(pKey); };
			Event newEvent(Event::KeyDown);
			newEvent.constructKey(wParam,
				check(Key::KC_RIGHT_SHIFT) || check(Key::KC_LEFT_SHIFT),
				check(Key::KC_LEFT_ALT) || check(Key::KC_RIGHT_ALT),
				check(Key::KC_RIGHT_SUPER) || check(Key::KC_LEFT_SUPER),
				check(Key::KC_CAPS),
				check(Key::KC_RIGHT_CTRL) || check(Key::KC_LEFT_CTRL));
			Engine::window->eventQ.pushEvent(newEvent);
			break;
		}
		case WM_KEYUP:
		{
			Keyboard::keyState[wParam] = 0;
			auto check = [](U8 pKey) -> bool { return Keyboard::isKeyPressed(pKey); };
			Event newEvent(Event::KeyUp);
			newEvent.constructKey(wParam,
				check(Key::KC_RIGHT_SHIFT) || check(Key::KC_LEFT_SHIFT),
				check(Key::KC_LEFT_ALT) || check(Key::KC_RIGHT_ALT),
				check(Key::KC_RIGHT_SUPER) || check(Key::KC_LEFT_SUPER),
				check(Key::KC_CAPS),
				check(Key::KC_RIGHT_CTRL) || check(Key::KC_LEFT_CTRL));
			Engine::window->eventQ.pushEvent(newEvent);
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			Engine::isRunning = false;
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);;
}

#endif