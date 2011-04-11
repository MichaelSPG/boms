#ifndef BS_CORECINFO_H
#define BS_CORECINFO_H

#include <Windows.h>//For HWND



/*	Structure which holds all necessary information for starting the engine.
	You can use the isOk function to verify that you've set it up properly.
*/
struct bsCoreCInfo
{
	bsCoreCInfo()
		: assetDirectory("..\\assets\\")
		, worldSize(1000.0f)
		, windowWidth(1280)
		, windowHeight(720)
		//, hWnd(nullptr)
		, hInstance(nullptr)
		, showCmd(-1)
		, windowName("Direct3D 11")
	{}


	//This directory will be scanned by the file system, making it easy to load files it.
	//Default: "..\assets\"
	char*	assetDirectory;

	//The world size is a cube with sides equal to this many meters.
	//Default: 1000.0f
	float	worldSize;
	
	//The dimensions of the window.
	//Default: 1280x720
	int		windowWidth, windowHeight;

	//Handle to the window.
	//Default: null
	//HWND	hWnd;

	//Parameters from WinMain()
	HINSTANCE	hInstance;
	int			showCmd;

	//Name of the window
	//Default: "Direct3D 11"
	char*	windowName;


	//Returns true if the structure is set up properly.
	bool isOk() const
	{
		return assetDirectory && (worldSize > 0.0f) && (windowWidth > 0)
			&& (windowHeight > 0) && windowName && hInstance && (showCmd != -1);
	}
};

#endif // BS_CORECINFO_H