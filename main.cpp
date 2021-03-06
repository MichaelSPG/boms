#include "StdAfx.h"

#include <vld.h>

#include <windows.h>

#include "Application.h"
#include "bsTimer.h"
#include "bsAssert.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int showCmd)
{
	VLDDisable();

	const int windowWidth = 1280;
	const int windowHeight = 720;
	
	try
	{
		Application application(hInstance, showCmd, windowWidth, windowHeight);

		bsTimer timer;
		float startTime = 0.0f;
		float deltaTime = 16.67f;

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
		BS_ASSERT(!"Caught a known exception");
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		MessageBox(nullptr, "Caught ... exception", "Error", MB_ICONSTOP | MB_SETFOREGROUND);
		BS_ASSERT(!"Caught an unknown exception");
		exit(EXIT_FAILURE);
	}

	return 0;
}
