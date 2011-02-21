#include <windows.h>
#include <exception>

#include "Application.h"
#include "Timer.h"

bool quit = false;

const char name[] = "Direct3D";

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND hWnd = nullptr;
	MSG msg = {0};
	WNDCLASSEX wndClassex = {0};

	const int windowWidth = 1280;
	const int windowHeight = 720;

	wndClassex.cbSize = sizeof(WNDCLASSEX);
	wndClassex.style = CS_HREDRAW | CS_VREDRAW;
	wndClassex.lpfnWndProc = WinProc;
	wndClassex.hInstance = hInstance;
	wndClassex.lpszClassName = name;
	//wndClassex.hCursor = (HCURSOR)LoadImage(nullptr, MAKEINTRESOURCE(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_SHARED);

	RegisterClassEx(&wndClassex);

	RECT rect = {0, 0, windowWidth, windowHeight};

	DWORD winStyleEx = WS_EX_CLIENTEDGE;
	DWORD winStyle = WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;

	AdjustWindowRectEx(&rect, winStyle, false, winStyleEx);

	hWnd = CreateWindowEx(winStyleEx, name, name, winStyle, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
		rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

	Application application;
	try
	{
		application.init(hWnd, windowWidth, windowHeight);
	}
	catch (std::exception &e)
	{
		MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		MessageBox(nullptr, "An unexpected error has occurred", "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		exit(EXIT_FAILURE);
	}

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Timer timer;
	timer.start();

	const unsigned int maxFps = 60;
	const float maxFrameTime = (1.0f / maxFps) * 1000.0f;

	float startTime(0);
	float deltaTime(1);

	const float weightRatio(2.0f / maxFps);
	float timeMs(1.0f);
	float updateFps(0);


	if (maxFps)
	{
		try
		{
			while (!quit)
			{
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
						break;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				startTime = timer.getTimeMilliSeconds();
				application.update(deltaTime);

				deltaTime = timer.getTimeMilliSeconds() - startTime;

				if (deltaTime < maxFrameTime)
				{
					Sleep(static_cast<DWORD>(maxFrameTime - deltaTime));
				}

				deltaTime = timer.getTimeMilliSeconds() - startTime;

				timeMs = timeMs * (1.0f - weightRatio) + deltaTime * weightRatio;

				updateFps += deltaTime;

				//Update fps no more often than every 500 ms.
				if (updateFps > 500)
				{
					SetWindowText(hWnd, std::string("FPS: " + ToString::toString((unsigned int)((1 / timeMs) * 1000))).c_str());
					updateFps = 0;
				}
			}
		}

		catch (std::exception &e)
		{
			MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		try
		{
			while (!quit)
			{
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
						break;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}


				//startTime = timer.getTime();
				startTime = timer.getTimeMilliSeconds();
				application.update(deltaTime);

				//deltaTime = timer.getTime() - startTime;
				deltaTime = timer.getTimeMilliSeconds() - startTime;
			}
		}

		catch (std::exception &e)
		{
			MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
			exit(EXIT_FAILURE);
		}
	}


	UnregisterClass(name, hInstance);

	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		quit = true;
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}