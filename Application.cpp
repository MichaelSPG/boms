#include "Application.h"

#include "Log.h"
#include "SceneGraph.h"
#include "Camera.h"


Application::Application() : 
	mInputManager(nullptr),
	mKeyboard(nullptr),
	mMouse(nullptr),
	mDx11Renderer(nullptr),
	mResourceManager(nullptr),
	mSceneGraph(nullptr)
{
	w = a = s = d = space = c = false;
}

Application::~Application()
{
	Log::log("Shutting down", LOG_SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);
}


#include "Cube.h"
Cube* cube = new Cube();

void Application::init(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
{
#ifdef _DEBUG
	Log::init(LOG_SEV_DEBUG);
#else
	Log::init(LOG_SEV_DEBUG);
#endif

	Log::log("Initializing application", LOG_SEV_NOTICE);


	mDx11Renderer = new Dx11Renderer();
	mDx11Renderer->init(hWnd, renderWindowWidth, renderWindowHeight);

	//OIS
	{
		Log::log("Initializing OIS", LOG_SEV_NOTICE);

		OIS::ParamList paramList;;
		paramList.insert(OIS::ParamList::value_type("WINDOW", STR(unsigned long(hWnd))));
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND"))); 
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE"))); 
		paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NOWINKEY")));

		mInputManager = OIS::InputManager::createInputSystem(paramList);

		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
		

		mMouse->getMouseState().height = renderWindowHeight;
		mMouse->getMouseState().width = renderWindowWidth;

		mKeyboard->setEventCallback(this);
		mMouse->setEventCallback(this);
	}

	mResourceManager = new ResourceManager();
	mResourceManager->initShaderManager(mDx11Renderer);

	mSceneGraph = new SceneGraph();
	mSceneGraph->init(4, mDx11Renderer, mResourceManager->getShaderManager());

	
	auto bert = mResourceManager->getShaderManager()->getVertexShader("Wireframe.fx");


//	mCamera = new Camera(45.0f, 100.0f, 0.1f, 1280/720.0f, mSceneGraph);
	mCamera = new Camera(ProjectionInfo(45.0f, 1000.0f, 0.1f, 1280.0f/720.0f), mSceneGraph);

	cube->create(mDx11Renderer, mResourceManager->getShaderManager());

	Log::log("Initialization completed successfully", LOG_SEV_NOTICE);
}

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	if (w)
	{
		mCamera->translate(0.0f, 0.0f, 1.2f);
	}
	if (s)
	{
		mCamera->translate(0.0f, 0.0f, -1.2f);
	}
	if (a)
	{
		mCamera->translate(-1.2f, 0.0f, 0.0f);
	}
	if (d)
	{
		mCamera->translate(1.2f, 0.0f, 0.0f);
	}
	if (space)
	{
		mCamera->translate(0.1f, 1.0f, 0.0f);
	}
	if (c)
	{
		mCamera->translate(0.1f, -1.0f, 0.0f);
	}
	
	//////////////////////////////////////////////////////////////////////////
	/*
	CBChangesNever cb;
	XMStoreFloat4x4(&cb.mView, mCamera->getViewProjection());
	mDx11Renderer->getDeviceContext()->UpdateSubresource(mDx11Renderer->getChangesNeverBuffer(), 0, nullptr, &cb, 0, 0);
	*/
	//////////////////////////////////////////////////////////////////////////
	mCamera->updateView();

	mDx11Renderer->preRender();

	cube->draw(mDx11Renderer);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	mSceneGraph->drawAABBs(mDx11Renderer);


	//mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDx11Renderer->render();

	mDx11Renderer->present();
}

bool Application::keyPressed(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_W)
	{
		w = true;
	}
	if (arg.key == OIS::KC_S)
	{
		s = true;
	}
	if (arg.key == OIS::KC_D)
	{
		d = true;
	}
	if (arg.key == OIS::KC_A)
	{
		a = true;
	}
	if (arg.key == OIS::KC_SPACE)
	{
		space = true;
	}
	if (arg.key == OIS::KC_C)
	{
		c = true;
	}

	switch (arg.key)
	{
	case OIS::KC_ADD:
		{
			auto pi = mCamera->getProjectionInfo();
			pi.mAspectRatio += 0.025f;
			mCamera->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mCamera->getProjectionInfo();
			pi.mAspectRatio -= 0.025f;
			mCamera->setProjectionInfo(pi);
		}
		break;
	}


	return true;
}

bool Application::keyReleased(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_W)
	{
		w = false;
	}
	if (arg.key == OIS::KC_S)
	{
		s = false;
	}
	if (arg.key == OIS::KC_D)
	{
		d = false;
	}
	if (arg.key == OIS::KC_A)
	{
		a = false;
	}
	if (arg.key == OIS::KC_SPACE)
	{
		space = false;
	}
	if (arg.key == OIS::KC_C)
	{
		c = false;
	}

	return true;
}

bool Application::mouseMoved( const OIS::MouseEvent &arg )
{
	
	mCamera->rotateAboutAxis(Camera::AXIS_Y, (float)arg.state.X.rel * 0.01f);
	mCamera->rotateAboutAxis(Camera::AXIS_X, (float)arg.state.Y.rel * 0.01f);
	

	return true;
}

bool Application::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}

bool Application::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}
