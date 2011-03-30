#ifndef APPLICATION_H
#define APPLICATION_H

#include <OIS.h>
#include <string>
#include <windows.h>

#include "bsDx11Renderer.h"
#include "bsSceneGraph.h"
#include "bsResourceManager.h"


class bsLog;
class bsCamera;
class bsWindow;


class Application : public OIS::KeyListener, public OIS::MouseListener
{
public:
	Application(bsWindow* window);

	~Application();

	void update(float deltaTime);

	virtual bool keyPressed(const OIS::KeyEvent &arg);

	virtual bool keyReleased(const OIS::KeyEvent &arg);

	virtual bool mouseMoved(const OIS::MouseEvent &arg);

	virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

	virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

	//Returns true if the application should quit.
	inline bool quit() const
	{
		return mQuit;
	}

private:
	bsDx11Renderer*	mDx11Renderer;

	bsResourceManager*	mResourceManager;

	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	bsSceneGraph*	mSceneGraph;
	bsCamera*		mCamera;

	bool w, a, s, d, space, c, shift, moveDuck, resetDuck, rightMouseDown, leftMouseDown,
		mQuit;

	std::vector<bsNode*>	mSceneNodes;
	HWND mHwnd;
	bsWindow*	mWindow;
};

#endif // APPLICATION_H
