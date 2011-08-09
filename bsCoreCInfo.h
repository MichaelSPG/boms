#pragma once

#include "bsConfig.h"

#include <Windows.h>
#include <string>


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
		, hInstance(nullptr)
		, showCmd(-1)
		, windowName("Direct3D 11")
	{}


	/*	This directory will be scanned by the file system, making it easy to load files it.
		Default: "..\assets\"
	*/
	std::string	assetDirectory;

	/*	The world size is a cube with sides equal to this many meters.
		Default: 1000.0f
	*/
	float	worldSize;
	
	/*	The dimensions of the window.
		Default: 1280x720
	*/
	int		windowWidth, windowHeight;

	/*	Parameters from WinMain()
	*/
	HINSTANCE	hInstance;
	int			showCmd;

	/*	Name of the window
		Default: "Direct3D 11"
	*/
	std::string	windowName;


	/*	Returns true if the structure is set up properly.
	*/
	bool isOk() const
	{
		return assetDirectory.length() && (worldSize > 0.0f) && (windowWidth > 0)
			&& (windowHeight > 0) && windowName.length() && hInstance && (showCmd != -1);
	}
};
