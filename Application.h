#ifndef APPLICATION_H
#define APPLICATION_H

#include <OIS.h>
#include <string>
#include <windows.h>

#include "Dx11Renderer.h"
#include "ToString.h"
#include "SceneGraph.h"
#include "ResourceManager.h"


class Log;
class Camera;


class Application : public OIS::KeyListener, public OIS::MouseListener
{
public:
	Application(HWND hWnd, int renderWindowWidth, int renderWindowHeight);
	~Application();

	void update(float deltaTime);


	virtual bool keyPressed(const OIS::KeyEvent &arg);

	virtual bool keyReleased(const OIS::KeyEvent &arg);

	virtual bool mouseMoved(const OIS::MouseEvent &arg);

	virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

	virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

private:
	Dx11Renderer*	mDx11Renderer;

	ResourceManager*	mResourceManager;

	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	SceneGraph*	mSceneGraph;
	Camera* mCamera;

	bool w, a, s, d, space, c, shift;

	std::vector<Node*>	mSceneNodes;
	HWND mHwnd;
};

#endif