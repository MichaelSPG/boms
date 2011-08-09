#pragma once

#include "bsConfig.h"

#include <windows.h>
#include <string>


/*	Wrapper class for Windows windows.
*/
class bsWindow
{
public:
	/*	Creates a HWND with specified width, height and name from the provided HINSTANCE.
	*/
	bsWindow(int windowWidth, int windowHeight, const std::string& name,
		HINSTANCE hInstance, int showCmd);

	~bsWindow();

	inline HWND getHwnd() const
	{
		return mHwnd;
	}

	inline HINSTANCE getHinstance() const
	{
		return mHInstance;
	}

	inline int getWindowWidth() const
	{
		return mWindowWidth;
	}

	inline int getWindowHeight() const
	{
		return mWindowHeight;
	}

	//Returns false if the program should quit
	bool checkForMessages();

private:
	HWND		mHwnd;
	HINSTANCE	mHInstance;
	std::string	mName;

	int		mWindowWidth;
	int		mWindowHeight;
};
