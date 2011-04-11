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
		//quit = true;
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


bsWindow::bsWindow(const int windowWidth, const int windowHeight, const char* name,
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
	wndClassex.lpszClassName = name;
	//wndClassex.hCursor = (HCURSOR)LoadImage(nullptr, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_SHARED);

	RegisterClassEx(&wndClassex);

	int additionalWidth = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	int additionalHeight = (GetSystemMetrics(SM_CYFIXEDFRAME) * 2)
		+ GetSystemMetrics(SM_CYCAPTION);

	RECT rect = { 0, 0, windowWidth + additionalWidth, windowHeight + additionalHeight };

	DWORD winStyleEx = WS_EX_CLIENTEDGE;
	DWORD winStyle = WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	
	//WS_EX_CLIENTEDGE

	AdjustWindowRectEx(&rect, winStyle, false, winStyleEx);

	mHwnd = CreateWindowEx(winStyleEx, name, name, winStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(mHwnd, showCmd);
	UpdateWindow(mHwnd);
}

bsWindow::~bsWindow()
{
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