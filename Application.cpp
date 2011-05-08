#include "Application.h"

#include "bsConfig.h"

#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>
//#include <boost/bind/mem_fn.hpp>

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>

#include "bsLog.h"
#include "bsTimer.h"
#include "bsSceneGraph.h"
#include "bsCamera.h"
#include "bsRenderQueue.h"
#include "bsRenderStats.h"
#include "bsColorUtil.h"
#include "bsTextBox.h"
#include "bsTextManager.h"
#include "bsStringUtils.h"
#include "bsWindow.h"
#include "bsHavokManager.h"
#include "bsCore.h"
#include "bsSceneNode.h"
#include "bsLine3D.h"
#include "bsGeometryUtils.h"
#include "bsRenderTarget.h"
#include "bsFullScreenQuad.h"

#include "bsDeferredRenderer.h"


Application::Application(HINSTANCE hInstance, int showCmd, const int windowWidth,
	const int windowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
{
	w = a = s = d = space = c = shift = moveDuck = resetDuck = false;
	rightMouseDown = leftMouseDown = mQuit = pause = false;

	bsCoreCInfo coreCInfo;
	coreCInfo.hInstance = hInstance;
	coreCInfo.showCmd = showCmd;
	mCore = new bsCore(coreCInfo);

	bsLog::logMessage("Initializing OIS");

	//OIS initialization
	OIS::ParamList paramList;
	paramList.insert(OIS::ParamList::value_type("WINDOW",
		bsStringUtils::toString((unsigned long)mCore->getWindow()->getHwnd())));
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND"))); 
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE"))); 
	paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NOWINKEY")));

	mInputManager = OIS::InputManager::createInputSystem(paramList);

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

	mMouse->getMouseState().width = windowWidth;
	mMouse->getMouseState().height = windowHeight;

	mKeyboard->setEventCallback(this);
	mMouse->setEventCallback(this);


	RECT rect;
	GetWindowRect(mCore->getWindow()->getHwnd(), &rect);
	int x = rect.right - rect.left;
	int y = rect.bottom - rect.top;

	bsWindow* window = mCore->getWindow();

	for (unsigned int i = 0; i < 2; ++i)
	{
		mRenderTargets.push_back(new bsRenderTarget(window->getWindowWidth(),
			window->getWindowHeight(), mCore->getDx11Renderer()->getDevice()));
	}
	
	//mCore->getDx11Renderer()->setRenderTargets(&mRenderTarget, 1);
	bsRenderTarget* rts[2];
	rts[0] = mRenderTargets[0];
	rts[1] = mRenderTargets[1];
	
	mCore->getDx11Renderer()->setRenderTargets(rts, 2);

	//////////////////////////////////////////////////////////////////////////
	mFullScreenQuad = new bsFullScreenQuad(mCore->getDx11Renderer()->getDevice());

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;

	D3D11_INPUT_ELEMENT_DESC inputDesc;
	memset(&inputDesc, 0, sizeof(inputDesc));
	inputDesc.SemanticName = "POSITION";
	inputDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputLayout.push_back(inputDesc);

	inputDesc.SemanticName = "TEXCOORD";
	inputDesc.AlignedByteOffset = 12;
	inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(inputDesc);

	mFullscreenVS = mCore->getResourceManager()->getShaderManager()->getVertexShader("Merger.fx", inputLayout);
	mFullscreenPS = mCore->getResourceManager()->getShaderManager()->getPixelShader("Merger.fx");

	
	mDeferredRenderer = new bsDeferredRenderer(mCore->getDx11Renderer(),
		mCore->getSceneGraph()->getCamera(), mCore->getResourceManager()->getShaderManager(),
		window);

	mCore->setRenderSystem(mDeferredRenderer);

	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	//Register the text manager with the rendering system so that it'll get drawn.
	mDeferredRenderer->registerEndOfRenderCallback(boost::bind(&bsTextManager::drawAllTexts, textManager));

	//////////////////////////////////////////////////////////////////////////
	

	bsLog::logMessage("Initialization completed successfully", pantheios::SEV_NOTICE);
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", pantheios::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);
}

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	bsCamera* camera = mCore->getSceneGraph()->getCamera();

	if (w)
	{
		camera->translate(0.0f, 0.0f, shift ? 6.0f : 1.2f);
	}
	if (s)
	{
		camera->translate(0.0f, 0.0f, shift ? -6.0f : -1.2f);
	}
	if (a)
	{
		camera->translate(shift ? -6.0f : -1.2f, 0.0f, 0.0f);
	}
	if (d)
	{
		camera->translate(shift ? 6.0f : 1.2f, 0.0f, 0.0f);
	}
	if (space)
	{
		camera->translate(0.0f, shift ? 6.0f : 1.2f, 0.0f);
	}
	if (c)
	{
		camera->translate(0.0f, shift ? -6.0f : -1.2f, 0.0f);
	}
	camera->update();
	

	static std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayout;
	D3D11_INPUT_ELEMENT_DESC d;
	d.SemanticName = "POSITION";
	d.SemanticIndex = 0;
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.InputSlot = 0;
	d.AlignedByteOffset = 0;
	d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	d.InstanceDataStepRate = 0;
	inputLayout.push_back(d);

	d.SemanticName = "NORMAL";
	d.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d.AlignedByteOffset = 12;
	inputLayout.push_back(d);

	d.SemanticName = "TEXCOORD";
	d.AlignedByteOffset = 24;
	d.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayout.push_back(d);

	bsResourceManager* resourceManager = mCore->getResourceManager();
	static auto vs = resourceManager->getShaderManager()->getVertexShader("HLSL_Basic.fx",
		inputLayout);
	static auto ps = resourceManager->getShaderManager()->getPixelShader("HLSL_Basic.fx");


	static auto context = mCore->getDx11Renderer()->getDeviceContext();
	static bsMeshManager* meshManager = resourceManager->getMeshManager();

	static auto roi = meshManager->getMesh("roi.bsm");
	static auto duck = meshManager->getMesh("duck.bsm");
	static auto teapot = meshManager->getMesh("teapot.bsm");
	static auto gourd = meshManager->getMesh("gourd.bsm");
	static auto greeble = meshManager->getMesh("greeble.bsm");

	static auto cubeWithOffset = meshManager->getMesh("cubeWithOffset.bsm");

	bsSceneGraph* sceneGraph = mCore->getSceneGraph();
	static bsSceneNode* testNode1 = sceneGraph->createSceneNode(hkVector4(-70.0f, 0.0f, 0.0f));
	static bsSceneNode* testNode2 = sceneGraph->createSceneNode(hkVector4(-25.0f, 0.0f, 0.0f));
	static bsSceneNode* testNode3 = sceneGraph->createSceneNode(hkVector4( 25.0f, 0.0f, 0.0f));
	static bsSceneNode* testNode4 = sceneGraph->createSceneNode(hkVector4( 70.0f, 0.0f, 0.0f));

	static bsSceneNode* derivedNode1 = testNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode2 = derivedNode1->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	static bsSceneNode* derivedNode3 = derivedNode2->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f));
	

	static std::vector<bsSceneNode*> derivedNodes;
	bsSceneNode*& previousNode = derivedNode1;

	static bool derNodesOnce = false;
	if (!derNodesOnce)
	{
		for (unsigned int i = 0; i < 10; ++i)
		{
			derivedNodes.push_back(previousNode->createChildSceneNode(hkVector4(50.0f, 0.0f, 0.0f)));
			previousNode = derivedNodes.back();
		}

		derNodesOnce = true;
	}


	static std::vector<bsSceneNode*> sceneNodes;
	static bool createNodes = false;

	if (createNodes)
	{
		const unsigned int iters = 10;
		const float factor = 50.0f;

		for (unsigned int i = 0; i < iters; ++i)
		{
			for (unsigned int j = 0; j < iters; ++j)
			{
				float x = (i * factor) - ((iters * 0.5f) * factor);
				float z = (j * factor) - ((iters * 0.5f) * factor);

				bsSceneNode* node = sceneGraph->createSceneNode(hkVector4(x, 0.0f, z));
				node->attachRenderable(duck);
				sceneNodes.push_back(node);
			}
		}

		createNodes = false;
	}

	static std::shared_ptr<bsLine3D> line3d = std::make_shared<bsLine3D>(XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f));

	static bool once = false;
	if (!once)
	{
		bool disable = false;
		if (!disable)
		{
			for (unsigned int i = 0; i < derivedNodes.size(); ++i)
			{
				derivedNodes[i]->attachRenderable((i & 1) == 0 ? duck : greeble);
			}
		}
		
		once = true;
	}

	static float wave = 0.0f;
	wave += 0.02f;
	float waveSin = sinf(wave);
	float waveCos = cosf(wave);
	float mulFactor = 1.0f;

	if (moveDuck)
	{
		testNode1->setPosition(hkVector4(waveSin * mulFactor,  waveCos * mulFactor, -waveSin * mulFactor));

		hkRotation rot;
		rot.setAxisAngle(hkVector4(0.0f, 1.0f, 0.0f), wave);
	}
	if (resetDuck)
	{
		auto& dt = const_cast<hkVector4&>(testNode1->getDerivedTranslation());
		dt.setAll3(0.0f);
		testNode1->setPosition(hkVector4(0.0f, 0.0f, 0.0f));
	}
	
	static float rotWave = 0.0f;
	rotWave += 0.1f;
	waveSin = sinf(rotWave);
	waveCos = cosf(rotWave);
	hkQuaternion rot(hkVector4(0.0f, 1.0f, 0.0f), waveSin);
	rot.mul(hkQuaternion(hkVector4(1.0f, 0.0f, 0.0f), waveCos));

	static hkVector4 unit(0.0f, 1.0f, 0.0f);
	hkVector4 angle(waveSin, waveCos, -waveSin);
	angle.normalize3();

	static float increaseForever = 0.0f;
	increaseForever += 0.01f;
	hkQuaternion q2(unit, increaseForever);

	if (!pause)
	{
		for (unsigned int i = 0; i < derivedNodes.size(); ++i)
		{
			derivedNodes[i]->setPosition(hkVector4(sinf(increaseForever) * 100.0f, 0.0f, 0.0f));
			if ((i & 3) == 0)
			{
				continue;
			}

			derivedNodes[i]->setRotation(hkQuaternion(unit, (float)(increaseForever * (i * 0.2f))));
		}
	}

	
	static bsTextManager* textManager = resourceManager->getTextManager();

	textManager->updateTexts(deltaTime);

	static bsRenderStats stats;
	stats.setFps(1.0f / deltaTime * 1000.0f);
	stats.setFrameTime(deltaTime);

	static auto statsText = textManager->createText(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);
	statsText->setText(stats.getStatsString());

	static std::shared_ptr<bsTextBox> textBox = textManager->createTextBox(4500.0f, 10);
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);

	static auto frameStats = textManager->createText(L"");
	frameStats->setPosition(2.5f, 450.0f);
	frameStats->addFlags(FW1_RESTORESTATE);
	frameStats->setColor(0xff00ff00);
	frameStats->setFontSize(14.0f);
	
	frameStats->setText(mDeferredRenderer->getRenderQueue()->getFrameStats().getFrameStatsWideString());
	
	static bool logCallBackOnce = false;
	if (!logCallBackOnce)
	{
		//No implicit conversion from const char* to std::string, must use functor/lambda
		auto lmbd = [=](const char* text)
		{
			textBox->addTextLine(text);
		};
		bsLog::addCallback(lmbd);
		logCallBackOnce = true;
	}

//	context->GSSetShader(nullptr, nullptr, 0);
//	context->HSSetShader(nullptr, nullptr, 0);




	if (!mCore->update(deltaTime))
	{
		mQuit = true;
	}
}

bool Application::keyPressed(const OIS::KeyEvent& arg)
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
	if (arg.key == OIS::KC_LSHIFT)
	{
		shift = true;
	}

	switch (arg.key)
	{
	case OIS::KC_ADD:
		{
			auto pi = mCore->getSceneGraph()->getCamera()->getProjectionInfo();
			pi.mFieldOfView += 0.25f;
			mCore->getSceneGraph()->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mCore->getSceneGraph()->getCamera()->getProjectionInfo();
			pi.mFieldOfView -= 0.25f;
			mCore->getSceneGraph()->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_M:
		moveDuck = true;
		break;

	case OIS::KC_R:
		resetDuck = true;
		break;

	case OIS::KC_X:
		mCore->getSceneGraph()->getCamera()->rotateAboutAxis(hkVector4(0.0f, 1.0f, 0.0f), 22.5f);
		break;

	case OIS::KC_V:
		mCore->getDx11Renderer()->setVsyncEnabled(!mCore->getDx11Renderer()->getVsyncEnabled());

	case OIS::KC_P:
		pause = !pause;
		break;
	}


	return true;
}

bool Application::keyReleased(const OIS::KeyEvent& arg)
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

	switch(arg.key)
	{
	case OIS::KC_M:
		moveDuck = false;
		break;

	case OIS::KC_R:
		resetDuck = false;
		break;
	}

	return true;
}

bool Application::mouseMoved(const OIS::MouseEvent& arg)
{
	if (leftMouseDown)
	{
		bsCamera* camera = mCore->getSceneGraph()->getCamera();

		camera->rotateX(-(float)arg.state.X.rel * 0.01f);
		camera->rotateY(-(float)arg.state.Y.rel * 0.01f);
	}
	
	return true;
}

bool Application::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	switch (id)
	{
	case OIS::MB_Left:
		leftMouseDown = true;
		break;

	case OIS::MB_Right:
		rightMouseDown = true;
		break;
	}
	
	return true;
}

bool Application::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	switch (id)
	{
	case OIS::MB_Left:
		leftMouseDown = false;
		break;

	case OIS::MB_Right:
		rightMouseDown = false;
		break;
	}

	return true;
}
