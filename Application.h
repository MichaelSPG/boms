#ifndef APPLICATION_H
#define APPLICATION_H

#include <OIS.h>
#include <string>
#include <windows.h>

class bsLog;
class bsCore;
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

	void createSomeLights();

private:
	OIS::InputManager	*mInputManager;
	OIS::Keyboard		*mKeyboard;
	OIS::Mouse			*mMouse;

	bool w, a, s, d, space, c, shift, rightMouseDown, leftMouseDown, mQuit, pause;
	float mCameraSpeed;

	bsCore*		mCore;

	bsDeferredRenderer*	mDeferredRenderer;
};

#endif // APPLICATION_H
