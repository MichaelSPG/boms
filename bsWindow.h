#ifndef BSWINDOW_H
#define BSWINDOW_H

#include "bsConfig.h"

#include <windows.h>
#include <string>


class bsWindow
{
public:
	bsWindow(const int windowWidth, const int windowHeight, const char* name,
		HINSTANCE hInstance, int showCmd);

	~bsWindow();

	inline HWND getHwnd() const
	{
		return mHwnd;
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

#endif // BSWINDOW_H