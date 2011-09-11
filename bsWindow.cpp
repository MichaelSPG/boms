#include "StdAfx.h"

#include "bsWindow.h"



LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


bsWindow::bsWindow(int windowWidth, int windowHeight, const std::string& name,
	HINSTANCE hInstance, int showCmd)
	: mHwnd(nullptr)
	, mHInstance(hInstance)
	, mName(name)
	, mWindowWidth(windowWidth)
	, mWindowHeight(windowHeight)
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
	DWORD winStyle = WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	//Need to adjust the rect so that the size of the window borders are taken into account
	//when calling CreateWindowEx.
	AdjustWindowRectEx(&rect, winStyle, false, winStyleEx);

	mHwnd = CreateWindowEx(winStyleEx, name.c_str(), name.c_str(), winStyle, CW_USEDEFAULT,
		CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
		hInstance, nullptr);

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
		if (msg.message == WM_QUIT)
		{
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}
