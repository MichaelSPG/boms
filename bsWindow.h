#pragma once

#include <windows.h>

#include <functional>
#include <string>


/*	Wrapper class for Windows windows.
*/
class bsWindow
{
public:
	typedef std::function<void(unsigned int width, unsigned int height, const bsWindow&)>
		WindowResizedCallback;


	/*	Creates a HWND with specified width, height and name from the provided HINSTANCE.
	*/
	bsWindow(int windowWidth, int windowHeight, const std::string& name,
		HINSTANCE hInstance, int showCmd, const WindowResizedCallback& windowResizedCallback);

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


	static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND		mHwnd;
	HINSTANCE	mHInstance;
	std::string	mName;

	int		mWindowWidth;
	int		mWindowHeight;

	WindowResizedCallback	mWindowResizedCallback;
};
