#include "StdAfx.h"

#include "bsWindow.h"
#include "bsAssert.h"


bsWindow::bsWindow(int windowWidth, int windowHeight, const std::string& name,
	HINSTANCE hInstance, int showCmd, const WindowResizedCallback& windowResizedCallback)
	: mHwnd(nullptr)
	, mHInstance(hInstance)
	, mName(name)
	, mWindowWidth(windowWidth)
	, mWindowHeight(windowHeight)
	, mWindowResizedCallback(windowResizedCallback)
{
	WNDCLASSEX wndClassex = { 0 };
	wndClassex.cbSize = sizeof(WNDCLASSEX);
	wndClassex.style = CS_HREDRAW | CS_VREDRAW;
	wndClassex.lpfnWndProc = WinProc;
	wndClassex.hInstance = hInstance;
	wndClassex.lpszClassName = name.c_str();
	RegisterClassEx(&wndClassex);

	RECT rect = { 0, 0, windowWidth, windowHeight };

	DWORD winStyleEx = WS_EX_CLIENTEDGE;
	DWORD winStyle = WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;

	//Need to adjust the rect so that the size of the window borders are taken into account
	//when calling CreateWindowEx.
	AdjustWindowRectEx(&rect, winStyle, false, winStyleEx);

	mHwnd = CreateWindowEx(winStyleEx, name.c_str(), name.c_str(), winStyle, CW_USEDEFAULT,
		CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
		hInstance, this);

	SetWindowLongPtr(mHwnd, GWL_USERDATA, (LONG)this);

	ShowWindow(mHwnd, showCmd);
	UpdateWindow(mHwnd);
}

bsWindow::~bsWindow()
{
	DestroyWindow(mHwnd);
	UnregisterClass(mName.c_str(), mHInstance);
}

bool bsWindow::checkForMessages()
{
	MSG msg = { 0 };

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		switch (msg.message)
		{
		case WM_QUIT:
			{
				return false;
			}
			break;

		default:
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			break;
		}
	}

	return true;
}

LRESULT CALLBACK bsWindow::WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Get user data from HWND (pointer to a bsWindow).
	bsWindow* window = (bsWindow*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		{
			//If wParam is minimized, width and height are 0, and it makes no sense to
			//resize window to those values.
			if (wParam != SIZE_MINIMIZED)
			{
				const unsigned int width = LOWORD(lParam);
				const unsigned int height = HIWORD(lParam);

				BS_ASSERT(width != 0);
				BS_ASSERT(height != 0);

				window->mWindowResizedCallback(width, height, *window);
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
