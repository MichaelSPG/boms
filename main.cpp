#include "bsConfig.h"

#include <windows.h>
#include <cassert>

#include "Application.h"
#include "bsTimer.h"
#include "bsWindow.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int showCmd)
{
	const int windowWidth = 1280;
	const int windowHeight = 720;

	Application application(new bsWindow(windowWidth, windowHeight, "Direct3D",
		hInstance, showCmd));

	bsTimer timer;
	float startTime = 0.0f;
	float deltaTime = 16.67f;

	try
	{
		while (!application.quit())
		{
			startTime = timer.getTimeMilliSeconds();
			application.update(deltaTime);

			deltaTime = timer.getTimeMilliSeconds() - startTime;
		}
	}
	catch (const std::exception& e)
	{
		MessageBox(nullptr, e.what(), "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		assert(false);
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		MessageBox(nullptr, "Caught ... exception", "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		assert(false);
		exit(EXIT_FAILURE);
	}

	return 0;
}
