#include "Application.h"

#include <algorithm>
#include <cassert>

#include "bsTextManager.h"

#include "Log.h"
#include "Timer.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "bsRenderStats.h"
#include "bsColorUtil.h"


Application::Application(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mDx11Renderer(nullptr)
	, mResourceManager(nullptr)
	, mSceneGraph(nullptr)
	, mCamera(nullptr)

	, mHwnd(hWnd)
{
	//////////////////////////////////////////////////////////////////////////
	w = a = s = d = space = c = shift = false;

	Log::init(pantheios::SEV_DEBUG);
	Log::logMessage("Initializing application", pantheios::SEV_NOTICE);

	mDx11Renderer = new Dx11Renderer(hWnd, renderWindowWidth, renderWindowHeight);
	//mDx11Renderer->init(hWnd, renderWindowWidth, renderWindowHeight);

	//OIS
	{
		Log::logMessage("Initializing OIS");

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
	mResourceManager->initAll("../assets", mDx11Renderer);
	/*
	mResourceManager->initFileSystem("../assets");
	mResourceManager->initShaderManager(mDx11Renderer);
	mResourceManager->initMeshManager(mDx11Renderer);
	*/
	mSceneGraph = new SceneGraph();
	mSceneGraph->init(4, mDx11Renderer, mResourceManager);

	mCamera = new Camera(ProjectionInfo(45.0f, 1000.0f, 0.1f, 1280.0f/720.0f), mSceneGraph);

	Log::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);
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

void Application::update(float deltaTime)
{
	mDx11Renderer->preRender();

	mKeyboard->capture();
	mMouse->capture();
	
	if (w)
	{
		mCamera->translate(0.0f, 0.0f, shift ? 6.0f : 1.2f);
	}
	if (s)
	{
		mCamera->translate(0.0f, 0.0f, shift ? -6.0f : -1.2f);
	}
	if (a)
	{
		mCamera->translate(shift ? -6.0f : -1.2f, 0.0f, 0.0f);
	}
	if (d)
	{
		mCamera->translate(shift ? 6.0f : 1.2f, 0.0f, 0.0f);
	}
	if (space)
	{
		mCamera->translate(0.0f, shift ? 6.0f : 1.2f, 0.0f);
	}
	if (c)
	{
		mCamera->translate(0.0f, shift ? -6.0f : -1.2f, 0.0f);
	}

	mCamera->update();

	
	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	
//	mSceneGraph->getRootNode()->drawAABB(mDx11Renderer);


	static std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0u;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0u;
	d.AlignedByteOffset = 0u;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0u;
	inputLayout.push_back(d);

	d.SemanticName = "NORMAL";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 12u;
	inputLayout.push_back(d);

	d.SemanticName = "TEXCOORD";
	d.AlignedByteOffset = 24u;
	d.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(d);

	static auto vs = mResourceManager->getShaderManager()->getVertexShader("HLSL_Basic.fx", inputLayout);
	static auto ps = mResourceManager->getShaderManager()->getPixelShader("HLSL_Basic.fx");

	//////////////////////////////////////////////////////////////////////////
	static auto context = mDx11Renderer->getDeviceContext();

	static auto roi = mResourceManager->getMeshManager()->getMesh("roi.bsm");
	static auto duck = mResourceManager->getMeshManager()->getMesh("duck.bsm");
	static auto teapot = mResourceManager->getMeshManager()->getMesh("teapot.bsm");
	static auto gourd = mResourceManager->getMeshManager()->getMesh("gourd.bsm");


	static SceneNode* testNode1 = mSceneGraph->createSceneNode(hkVector4(0.0f, 50.0f, 0.0f));
	static SceneNode* testNode2 = mSceneGraph->createSceneNode(hkVector4(50.0f, 0.0f, 50.0f));
	static SceneNode* testNode3 = mSceneGraph->createSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static SceneNode* testNode4 = mSceneGraph->createSceneNode(hkVector4(0.0f, 0.0f, -50.0f));

	static bool once = false;
	if (!once)
	{
		testNode1->attachRenderable(roi);
		testNode1->attachRenderable(duck);
		testNode2->attachRenderable(teapot);
		testNode2->attachRenderable(gourd);
		testNode2->attachRenderable(roi);
		testNode3->attachRenderable(duck);
		testNode4->attachRenderable(gourd);
		testNode4->attachRenderable(duck);
		testNode4->attachRenderable(teapot);
		testNode4->attachRenderable(gourd);
		once = true;
	}

	static float wave = 0.0f;
	wave += 0.02f;
	float waveSin = sinf(wave);
	float waveCos = cosf(wave);
	float mulFactor = 1.2f;

	testNode1->setTranslation( waveSin * mulFactor,  waveCos * mulFactor, -waveSin * mulFactor);
	testNode2->setTranslation(-waveSin * mulFactor, -waveCos * mulFactor,  waveSin * mulFactor);
	testNode3->setTranslation( waveCos * mulFactor,  waveSin * mulFactor, -waveCos * mulFactor);
	testNode4->setTranslation( waveSin * mulFactor, -waveSin * mulFactor,  waveCos * mulFactor);
	/*
	testNode1->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode2->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode3->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	testNode4->setTranslation(bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f), bsMath::randomRange(-50.0f, 50.0f));
	
	const hkTransform& trans1 = testNode1->getDerivedTransformation();
	const hkTransform& trans2 = testNode2->getDerivedTransformation();
	const hkTransform& trans3 = testNode3->getDerivedTransformation();
	const hkTransform& trans4 = testNode4->getDerivedTransformation();
*/

	context->IASetInputLayout(vs->getInputLayout());
	context->VSSetShader(vs->getVertexShader(), nullptr, 0u);
	context->PSSetShader(ps->getPixelShader(), nullptr, 0u);
	
	mSceneGraph->drawAABBs(mDx11Renderer);

	mDx11Renderer->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderQueue rq(mDx11Renderer, mResourceManager->getShaderManager());
//	rq.addRenderables(renderables);
	rq.addSceneNode(testNode1);
	rq.addSceneNode(testNode2);
	rq.addSceneNode(testNode3);
	rq.addSceneNode(testNode4);
	rq.draw(mDx11Renderer);

	static auto textManager = mResourceManager->getTextManager();
	static auto snowman = textManager->createText(L"\u2603");
	snowman->setFontSize(192.0f);
//	snowman->setPosition(200.0f, 100.0f);
	snowman->setFlags(FW1_CENTER | FW1_VCENTER | FW1_RESTORESTATE);
	//snowman->addFlags(FW1_ALIASED);
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(mHwnd, &pt);
	snowman->setPosition((float)pt.x, (float)pt.y);
	//snowman->draw();

	static bsRenderStats stats;
	stats.setFps(1.0f/deltaTime * 1000.0f);
	stats.setFrameTime(deltaTime);

	static auto statsText = textManager->createText(L"");
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);
	//statsText->setPosition(0.5f, 0.5f);
	statsText->setText(stats.getStatsString());
	//statsText->addFlags(FW1_ALIASED);
	//statsText->draw();

	textManager->drawAllTexts();
	

	context->GSSetShader(nullptr, nullptr, 0u);
	context->HSSetShader(nullptr, nullptr, 0u);

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
			pi.mFieldOfView += 0.025f;
			mCamera->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mCamera->getProjectionInfo();
			pi.mFieldOfView -= 0.025f;
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
