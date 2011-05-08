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
class bsCore;
class bsRenderTarget;
class bsFullScreenQuad;
class bsPixelShader;
class bsVertexShader;
class bsDeferredRenderer;


class Application : public OIS::KeyListener, public OIS::MouseListener
{
public:
	Application(HINSTANCE hInstance, int showCmd, const int windowWidth, const int windowHeight);

	~Application();

	void update(float deltaTime);

	virtual bool keyPressed(const OIS::KeyEvent& arg);

	virtual bool keyReleased(const OIS::KeyEvent& arg);

	virtual bool mouseMoved(const OIS::MouseEvent& arg);

	virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

	virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

	//Returns true if the application should quit.
	inline bool quit() const
	{
		return mQuit;
	}

private:
	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	bool w, a, s, d, space, c, shift, moveDuck, resetDuck, rightMouseDown, leftMouseDown,
		mQuit, pause;

	bsCore*		mCore;

	std::vector<bsRenderTarget*>	mRenderTargets;

	bsFullScreenQuad*	mFullScreenQuad;

	std::shared_ptr<bsVertexShader> mFullscreenVS;
	std::shared_ptr<bsPixelShader> mFullscreenPS;


	bsDeferredRenderer*	mDeferredRenderer;
};

#endif // APPLICATION_H
