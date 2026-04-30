#include "stdafx.h"
#include <tge/windows/WindowsWindow.h>
#include "resource.h"
#include <WinUser.h>
#include <tge/ImGui/ImGuiInterface.h>

using namespace Tga;

WindowsWindow::WindowsWindow(void)
	:myWndProcCallback(nullptr)
{
}


WindowsWindow::~WindowsWindow(void)
{
}

bool WindowsWindow::Init(const EngineConfiguration& aWindowConfig, HINSTANCE& aHInstanceToFill, HWND*& aHwnd)
{
	myWndProcCallback = aWindowConfig.myWinProcCallback;
	HINSTANCE instance = GetModuleHandle(NULL);
	aHInstanceToFill = instance;

	ZeroMemory(&myWindowClass, sizeof(WNDCLASSEX));
	myWindowClass.cbSize = sizeof(WNDCLASSEX);
	myWindowClass.style = CS_HREDRAW | CS_VREDRAW;
	myWindowClass.lpfnWndProc = WindowProc;
	myWindowClass.hInstance = instance;
	myWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	myWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	myWindowClass.lpszClassName = L"WindowClass1";
	myWindowClass.hIcon = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
	myWindowClass.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
	RegisterClassEx(&myWindowClass);

	const auto& windowSize = aWindowConfig.myWindowSize;

	RECT wr = { 0, 0, static_cast<long>(windowSize.x), static_cast<long>(windowSize.y) };

	DWORD windowStyle = 0;
	if (aWindowConfig.myBorderless || aWindowConfig.myStartInFullScreen)
	{
		windowStyle = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		windowStyle = WS_OVERLAPPEDWINDOW;
	}

	if (!aHwnd)
	{
		myWindowHandle = CreateWindowEx(
			WS_EX_APPWINDOW,
			L"WindowClass1",    
			aWindowConfig.myApplicationName.c_str(),    
			windowStyle,    
			0, 0,
			wr.right - wr.left,    
			wr.bottom - wr.top,    
			NULL,    
			NULL,    
			instance,    
			NULL);    

		ShowWindow(myWindowHandle, (aWindowConfig.myStartInFullScreen || aWindowConfig.myStartMaximized) ? SW_MAXIMIZE : SW_SHOWDEFAULT);
		aHwnd = &myWindowHandle;
	}
	else
	{
		myWindowHandle = *aHwnd;
	}

	SetWindowLongPtr(myWindowHandle, GWLP_USERDATA, (LONG_PTR)this);

	myResolution = windowSize;
	myResolutionWithBorderDifference = myResolution;
	if (aWindowConfig.myBorderless == false)
	{
		RECT r;
		GetClientRect(myWindowHandle, &r); 
		int horizontal = r.right - r.left;
		int vertical = r.bottom - r.top;

		int diffX = windowSize.x - horizontal;
		int diffY = windowSize.y - vertical;

		SetResolution(windowSize + Vector2ui(diffX, diffY));
		myResolutionWithBorderDifference = windowSize + Vector2ui(diffX, diffY);
	}

	INFO_PRINT("%s %i %i", "Windows created with size ", windowSize.x, windowSize.y);

	return true;
}


#ifndef _RETAIL
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
LRESULT WindowsWindow::LocWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef _RETAIL
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return 1; 
	}
#endif
	if (myWndProcCallback)
	{
		return myWndProcCallback(hWnd, message, wParam, lParam); 
	}
	return 0; 
}

LRESULT CALLBACK WindowsWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowsWindow* windowsClass = (WindowsWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (windowsClass)
	{
		const LRESULT result = windowsClass->LocWindowProc(hWnd, message, wParam, lParam); 
		if (result != 0) 
		{
			return result; 
		}
	}

	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;

	case WM_SIZE:
	{
		if (Engine::GetInstance())
			Engine::GetInstance()->SetWantToUpdateSize();
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Tga::WindowsWindow::SetResolution(Vector2ui aResolution)
{
	::SetWindowPos(myWindowHandle, 0, 0, 0, aResolution.x, aResolution.y, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void Tga::WindowsWindow::Close()
{
	DestroyWindow(myWindowHandle);
}