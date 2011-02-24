#include "Application.h"

#include "Log.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "SceneNode.h"


Application::Application()
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mDx11Renderer(nullptr)
	, mResourceManager(nullptr)
	, mSceneGraph(nullptr)
	, mCamera(nullptr)
{
	w = a = s = d = space = c = shift = false;
}

Application::~Application()
{
	Log::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);

	if (mCamera)
	{
		delete mCamera;
	}
	if (mSceneGraph)
	{
		delete mSceneGraph;
	}
	if (mResourceManager)
	{
		delete mResourceManager;
	}
	if (mDx11Renderer)
	{
		delete mDx11Renderer;
	}
}


#include "Cube.h"
//Cube* cube = new Cube();

void Application::init(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
{
#ifdef _DEBUG
	Log::init(pantheios::SEV_DEBUG);
#else
	Log::init(pantheios::SEV_DEBUG);
#endif

	Log::logMessage("Initializing application", pantheios::SEV_NOTICE);


	mDx11Renderer = new Dx11Renderer();
	mDx11Renderer->init(hWnd, renderWindowWidth, renderWindowHeight);

	//OIS
	{
		Log::logMessage("Initializing OIS", pantheios::SEV_NOTICE);

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


//	mCamera = new Camera(45.0f, 100.0f, 0.1f, 1280/720.0f, mSceneGraph);
	mCamera = new Camera(ProjectionInfo(45.0f, 1000.0f, 0.1f, 1280.0f/720.0f), mSceneGraph);

//	cube->create(mDx11Renderer, mResourceManager->getShaderManager());


	//////////////////////////////////////////////////////////////////////////

	SceneNode* node = mSceneGraph->createSceneNode();
	hkVector4 amin(-0.5f, -0.5f, -0.5f);
	hkVector4 amax( 0.5f,  0.5f,  0.5f);

	node->mAABB.m_min = amin;
	node->mAABB.m_max = amax;

	node->setPosition(27.0f, 34.0f, 30.0f);

	mSceneNodes.push_back(node);
	
	mSceneNodes[0]->createDrawableAabb(mDx11Renderer, mResourceManager->getShaderManager());

	//////////////////////////////////////////////////////////////////////////

	Log::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);
}

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	if (w)
	{
		if (shift)
		{
			mCamera->translate(0.0f, 0.0f, 6.0f);
		}
		else
		{
			mCamera->translate(0.0f, 0.0f, 1.2f);
		}
	}
	if (s)
	{
		if (shift)
		{
			mCamera->translate(0.0f, 0.0f, -6.0f);
		}
		else
		{
			mCamera->translate(0.0f, 0.0f, -1.2f);
		}
	}
	if (a)
	{
		if (shift)
		{
			mCamera->translate(-6.0f, 0.0f, 0.0f);
		}
		else
		{
			mCamera->translate(-1.2f, 0.0f, 0.0f);
		}
	}
	if (d)
	{
		if (shift)
		{
			mCamera->translate(6.0f, 0.0f, 0.0f);
		}
		else
		{
			mCamera->translate(1.2f, 0.0f, 0.0f);
		}
	}
	if (space)
	{
		if (shift)
		{
			mCamera->translate(0.0f, 6.0f, 0.0f);
		}
		else
		{
			mCamera->translate(0.0f, 1.0f, 0.0f);
		}
	}
	if (c)
	{
		if (shift)
		{
			mCamera->translate(0.0f, -6.0f, 0.0f);
		}
		else
		{
			mCamera->translate(0.0f, -1.0f, 0.0f);
		}
	}

	static float i = 0.0f;
	i += 0.01f;

	hkVector4 pos = mSceneNodes[0]->getPosition();
	hkQuadReal posQuad = pos.getQuad();

	float sinI = sin(i) * 10.0f;
	float cosI = cos(i) * 10.0f;
	float tanI = -cos(sinI) * 10.0f;

	posQuad.x = sinI;
	posQuad.z = cosI;
	posQuad.y = tanI;

	pos.set(posQuad.x, posQuad.y, posQuad.z, posQuad.w);
	pos.mul4(7.5f);

	mSceneNodes[0]->setPosition(pos);
	
	//////////////////////////////////////////////////////////////////////////
	/*
	CBChangesNever cb;
	XMStoreFloat4x4(&cb.mView, mCamera->getViewProjection());
	mDx11Renderer->getDeviceContext()->UpdateSubresource(mDx11Renderer->getChangesNeverBuffer(), 0, nullptr, &cb, 0, 0);
	*/
	//////////////////////////////////////////////////////////////////////////
	mCamera->updateView();

	mDx11Renderer->preRender();

//	cube->draw(mDx11Renderer);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
//	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
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
	if (arg.key == OIS::KC_G)
	{
		mSceneGraph->displayEmptyAabbs = !mSceneGraph->displayEmptyAabbs;
	}
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = true;
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
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = false;
	}

	return true;
}

bool Application::mouseMoved( const OIS::MouseEvent &arg )
{
	
	mCamera->rotateAboutAxis(Camera::AXIS_Y, -((float)arg.state.X.rel * 0.01f));
	mCamera->rotateAboutAxis(Camera::AXIS_X, -((float)arg.state.Y.rel * 0.01f));
	

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
