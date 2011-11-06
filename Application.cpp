#include "StdAfx.h"

#include "Application.h"

#include <algorithm>
#include <functional>
#include <fstream>

#include <Common/Base/Algorithm/PseudoRandom/hkPseudoRandomGenerator.h>
#include <Physics/Utilities/Dynamics/Keyframe/hkpKeyFrameUtility.h>
#include <Physics/Dynamics/hkpDynamics.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>

#include "bsLog.h"
#include "bsScene.h"
#include "bsCamera.h"
#include "bsRenderStats.h"
#include "bsRenderQueue.h"
#include "bsScrollingText2D.h"
#include "bsTextManager.h"
#include "bsStringUtils.h"
#include "bsWindow.h"
#include "bsHavokManager.h"
#include "bsCore.h"
#include "bsEntity.h"
#include "bsLineRenderer.h"
#include "bsResourceManager.h"
#include "bsLight.h"
#include "bsDx11Renderer.h"
#include "bsPrimitiveCreator.h"
#include "bsRayCastUtil.h"
#include "bsSmoothCameraMovement.h"
#include "bsText3D.h"

#include "bsDeferredRenderer.h"

#include "bsAssert.h"
#include "bsScene.h"

#include "bsCharacterController.h"
#include "bsRandomNumberGenerator.h"
#include "bsGeometryUtils.h"


bsSmoothCameraMovement* camMov;

Application::Application(HINSTANCE hInstance, int showCmd, const int windowWidth,
	const int windowHeight)
	: mInputManager(nullptr)
	, mKeyboard(nullptr)
	, mMouse(nullptr)
	, mCameraSpeed(1.0f)
	, mFreeCamMode(false)
{
	w = a = s = d = space = c = shift = false;
	rightMouseDown = leftMouseDown = mQuit = false;

	bsCoreCInfo coreCInfo;
	coreCInfo.hInstance = hInstance;
	coreCInfo.showCmd = showCmd;
	mCore = new bsCore(coreCInfo);

	bsLog::log("Initializing OIS");

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


	bsWindow* window = mCore->getWindow();
	
	float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	mCore->getDx11Renderer()->setRenderTargetClearColor(clearColor);

	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	
	mScene = new bsScene(mCore->getDx11Renderer(), mCore->getHavokManager(), coreCInfo);

	mDeferredRenderer = new bsDeferredRenderer(mCore->getDx11Renderer(),
		mScene->getCamera(), mCore->getResourceManager()->getShaderManager(),
		window, mCore->getRenderQueue());
	//Register the text manager with the rendering system so that it'll draw texts
	mDeferredRenderer->registerEndOfRenderCallback(std::bind(&bsTextManager::drawAllTexts, textManager));

	mCore->setRenderSystem(mDeferredRenderer);
	mCore->getRenderQueue()->setCamera(mScene->getCamera());

	mPrimCreator = new bsPrimitiveCreator(*mCore->getResourceManager()->getMeshCache());

	bsLog::log("Initialization completed successfully");

	

	//////////////////////////////////////////////////////////////////////////


	bsCharacterControllerCInfo cInfo;
	cInfo.world = mScene->getPhysicsWorld();
	cInfo.maxRunSpeed = 20.0f;
	cInfo.mass = 25.0f;
	cInfo.jumpHeight = 2.5f;
	cInfo.airSpeed = 50.0f;
	cInfo.gravityStrength = 2.5f;
	mCharacterController = new bsCharacterController(cInfo);


	characterControllerEntity = new bsEntity();
	//bsEntity* camEntity = mScene->getCamera()->getEntity()->getOwner();
	bsEntity& camEntity = *mScene->getCamera()->getEntity();
	camEntity.mTransform.setLocalPosition(XMVectorSet(0.0f, cInfo.eyeHeight, 0.0f, 0.0f));
	mScene->addEntity(*characterControllerEntity);
	camEntity.mTransform.setParentTransform(&characterControllerEntity->mTransform);

	characterControllerEntity->mTransform.setPosition(XMVectorSet(2.8f, 3.5f, -5.6f, 0.0f));
	characterControllerEntity->attachRigidBody(*mCharacterController->getRigidBody());

	//camEntity->setLocalRotation(XMVectorSet(-0.004, -0.973f, 0.23f, 0.015f));


	createMeshes();
	//createSomeLights();
	createTexts();
	createEntities();
	//createWalls(100.0f, 100.0f, 25.0f);
	createKeyframedRb();
	createLines();
	createFactory();


	bsEntity* box = mPrimCreator->createBox(XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f));
	box->mTransform.setLocalPosition(XMVectorSet(2.5f, 2.0f, 0.0f, 0.0f));
	box->mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);

	bsEntity* sphere = mPrimCreator->createSphere(0.5f);
	sphere->mTransform.setLocalPosition(XMVectorSet(5.0f, 1.0f, 0.0f, 0.0f));
	sphere->mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);


	mScene->addEntity(*box);
	mScene->addEntity(*sphere);

	mScene->removeEntity(*box);
	mScene->removeEntity(*sphere);

	mScene->addEntity(*box);
	mScene->addEntity(*sphere);


	//plane->setLocalRotation(hkQuaternion(hkVector4(1.0f, 0.0f, 0.0f), 0.1f));
	/*
	plane->getEntity().mRigidBody->getWorld()->lock();
	plane->getEntity().mRigidBody->setLocalRotation(hkQuaternion::getIdentity());
	plane->getEntity().mRigidBody->getWorld()->unlock();
	*/
	/*
	bsEntity* test = new bsEntity();
	test->getEntity().attach(mCore->getResourceManager()->getMeshCache()->getMesh("bun_zipper.bsm"));
	test->setLocalScaleUniform(15.0f);
	test->setLocalPosition(hkVector4(10.0f, 10.0f, 10.0f));
	
	mScene->addEntity(test);
	*/

	const float camMoveSpeed = 10.0f;
	camMov = new bsSmoothCameraMovement(camEntity.mTransform, camMoveSpeed, camMoveSpeed,
		camMoveSpeed);

	{
		bsEntity* lightEntity = new bsEntity();
		bsPointLightCInfo ci;
		ci.color = XMFLOAT3(1.0f, 0.0f, 1.0f);
		ci.radius = 10.0f;
		ci.intensity = 0.75f;
		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), ci);

		lightEntity->attachLight(*light);
		mScene->addEntity(*lightEntity);

		entities["light"] = lightEntity;
	}

	{
		D3D11_SAMPLER_DESC samDesc;
		memset(&samDesc, 0, sizeof(samDesc));

		samDesc.MipLODBias = 0.0f;
		samDesc.BorderColor[0] = samDesc.BorderColor[1] = samDesc.BorderColor[2] = samDesc.BorderColor[3] = 0;

		samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samDesc.MaxAnisotropy = 4;
		samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samDesc.MaxLOD = 0.0f;
		samDesc.MinLOD = 0.0f;

		ID3D11SamplerState* sampler = nullptr;
		mCore->getDx11Renderer()->getDevice()->CreateSamplerState(&samDesc, &sampler);

		mCore->getDx11Renderer()->getDeviceContext()->PSSetSamplers(3, 1, &sampler);
	}
}

Application::~Application()
{
	bsLog::logMessage("Shutting down", bsLog::SEV_NOTICE);

	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputSystem(mInputManager);
	
	delete camMov;

	delete mCharacterController;

	delete mPrimCreator;

	delete mScene;
	delete mDeferredRenderer;
	delete mCore;
}

bool goingUp = true;

void Application::update(float deltaTime)
{
	mKeyboard->capture();
	mMouse->capture();
	
	bsCamera* camera = mScene->getCamera();
	bsEntity& camEntity = *camera->getEntity();
	
	
	float right = a ? (-1.0f) : (d ? 1.0f : 0.0f);
	float forward = s ? (-1.0f) : (w ? 1.0f : 0.0f);
	float up = c ? (-1.0f) : (space ? 1.0f : 0.0f);
	
	//camMov->update(forward, right, up, deltaTime);

	mRenderStats.setFps(1.0f / deltaTime * 1000.0f);
	mRenderStats.setFrameTime(deltaTime);

	const XMMATRIX& camTransform = camEntity.mTransform.getTransform();
	const XMVECTOR dir = XMVectorSet(camTransform._13, camTransform._23, camTransform._33, 0.0f);

	if (mFreeCamMode)
	{
		camMov->update(forward, right, up, deltaTime);
	}
	else
	{
		const float speedMultiplier = 0.5f;

		mCharacterController->step(16.66667f, forward * speedMultiplier, -right * speedMultiplier, space,
			bsMath::toHK(dir));
	}


	//Update texts
	mCore->getResourceManager()->getTextManager()->updateTexts(deltaTime);

	mTexts["stats"]->setText(mRenderStats.getStatsString());
	mTexts["frameStats"]->setText(mDeferredRenderer->getRenderQueue()->getFrameStats()
		.getFrameStatsStringWide());

	XMFLOAT4A camPos;
	XMStoreFloat4A(&camPos, camEntity.mTransform.getPosition());
	XMFLOAT4A camRot;
	XMStoreFloat4A(&camRot, camEntity.mTransform.getRotation());
	
	wchar_t cameraInfoBuffer[1000] = { 0 };
	swprintf_s(cameraInfoBuffer, 1000, L"Cam pos: x: %.3f, y: %.3f, z: %.3f\n"
		L"Cam rot: x: %.3f, y: %.3f, z: %.3f, w: %.3f", camPos.x, camPos.y, camPos.z,
		camRot.x, camRot.y, camRot.z, camRot.w);
	
	mTexts["camPos"]->setText(cameraInfoBuffer);


	if (leftMouseDown)
	{
		bsEntity& lineEntity = *entities["line"];
		bsLineRenderer& line = *lineEntity.getLineRenderer();
		const float rayLength = 100.0f;

		hkpWorldRayCastOutput output;
		XMVECTOR destinationOut, originOut;
		output = camera->screenPointToWorldRay(XMFLOAT2((float)mMouse->getMouseState().X.abs,
			(float)mMouse->getMouseState().Y.abs), rayLength, destinationOut, originOut);

		if (output.hasHit())
		{
			XMVECTOR hitPoint = XMVectorLerp(originOut, destinationOut, output.m_hitFraction);

			XMFLOAT3 points[2];
			//XMStoreFloat3(points, camPos);
			XMStoreFloat3(points + 1, hitPoint);

			XMFLOAT3 normal;
			output.m_normal.store<3, HK_IO_NATIVE_ALIGNED>(&normal.x);
			XMVECTOR normalFromHitPoint = XMLoadFloat3(&normal);
			normalFromHitPoint = XMVectorAdd(normalFromHitPoint, hitPoint);
			XMStoreFloat3(points, normalFromHitPoint);

			{
				XMStoreFloat3(points, originOut);
				
				XMStoreFloat3(points + 1, hitPoint);
				//XMStoreFloat3(points + 1, destinationOut);

				//XMStoreFloat3(points, destinationOut);
				//++points[0].y;
			}
			

			//XMStoreFloat3(points, destination2);
			//XMStoreFloat3(points + 1, origin);

			line.clear();
			line.setColor(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
			line.addPoints(points, 2);
			{
				XMStoreFloat3(points, hitPoint);
				XMStoreFloat3(points + 1, normalFromHitPoint);
				++points[1].y;
				line.addPoints(points, 2);
			}
			line.build(mCore->getDx11Renderer());

			hkpRigidBody* rb = hkpGetRigidBody(output.m_rootCollidable);
			rb->getWorld()->markForWrite();
			if (rb->getMotionType() != hkpMotion::MOTION_FIXED)
			{
				XMVECTOR impulse = XMVectorSubtract(destinationOut, originOut);
				impulse = XMLoadFloat3(&normal);
				//impulse = XMVector3Normalize(impulse);
				impulse = XMVectorScale(impulse, -10.0f * rb->getMass());
				//rb->applyPointImpulse(bsMath::toHK(impulse), bsMath::toHK(hitPoint));
				rb->applyForce(0.0166666666666f, bsMath::toHK(impulse), bsMath::toHK(hitPoint));
			}
			rb->getWorld()->unmarkForWrite();

			entities["light"]->mTransform.setPosition(XMVectorAdd(hitPoint, XMLoadFloat3(&normal)));

			
			bsEntity& textEntity = *entities["text"];
			textEntity.mTransform.setPosition(XMLoadFloat3(points + 1));
			bsText3D& text = *textEntity.getTextRenderer();

			wchar_t buf[200] = { 0 };
			//swprintf_s(buf, L"mass: %.1f", liftRigidBody->getMass());
			swprintf_s(buf, L"[%3.3f, %3.3f, %3.3f]", points[1].x, points[1].y, points[1].z);
			text.setText(buf);

			static XMVECTOR current = XMVectorZero(), previous = XMVectorZero();
			previous = XMLoadFloat3(&points[1]);

			if (!XMVector3NearEqual(current, previous, XMVectorReplicate(0.25f)))
			{
				std::wfstream fs("lights.txt", std::ios::app);
				fs << buf << std::endl;
			}

			current = XMLoadFloat3(&points[1]);
		}
	}

	//Keyframed RB
	
	{
		static float accumulatedDt = mCurve->getEnd();
		const float speed = (1.0f / mCurve->getEnd()) * 50000.0f;
		if (!mKeyboard->isKeyDown(OIS::KC_G))
			accumulatedDt -= 0.01666667f * speed;

		while (accumulatedDt < 0.0f)
		{
			accumulatedDt += mCurve->getEnd();
		}
		
		const float invDeltaTimeSecs = 1.0f / 0.016666667f;

		hkpRigidBody& liftRigidBody = *entities["lift"]->mRigidBody;
		liftRigidBody.getWorld()->markForWrite();
		hkQuaternion nextRotation = liftRigidBody.getRotation();
		nextRotation.mul(hkQuaternion(hkVector4(0.0f, 1.0f, 0.0f), 0.0025f));
		nextRotation.setIdentity();

		hkVector4 nextPosition = liftRigidBody.getPosition();
		{
			mCurve->getPoint(accumulatedDt, nextPosition);
		}
		const hkVector4 delta(0.0f, 0.02f, 0.0f);
		if (goingUp)
		{
			nextPosition.add(delta);
			if (nextPosition.getComponent<1>().getReal() > 10.0f)
			{
				goingUp = false;
			}
		}
		else
		{
			nextPosition.sub(delta);
			if (nextPosition.getComponent<1>().getReal() < 0.1f)
			{
				goingUp = true;
			}
		}

		hkpKeyFrameUtility::applyHardKeyFrame(nextPosition, nextRotation, invDeltaTimeSecs,
			&liftRigidBody);
		liftRigidBody.getWorld()->unmarkForWrite();
	}

	mScene->update(deltaTime);

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
			auto pi = mScene->getCamera()->getProjectionInfo();
			pi.mFieldOfView += 0.25f;
			mScene->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_SUBTRACT:
		{
			auto pi = mScene->getCamera()->getProjectionInfo();
			pi.mFieldOfView -= 0.25f;
			mScene->getCamera()->setProjectionInfo(pi);
		}
		break;

	case OIS::KC_V:
		mCore->getDx11Renderer()->setVsyncEnabled(!mCore->getDx11Renderer()->getVsyncEnabled());
		break;

	case OIS::KC_1:
		mCameraSpeed = 0.25f;
		break;

	case OIS::KC_2:
		mCameraSpeed = 0.5f;
		break;

	case OIS::KC_3:
		mCameraSpeed = 1.0f;
		break;

	case OIS::KC_4:
		mCameraSpeed = 2.0f;
		break;

	case OIS::KC_5:
		mCameraSpeed = 4.0f;
		break;

	case OIS::KC_T:
		mCore->getResourceManager()->getTextManager()->toggleVisibility();
		break;

	case OIS::KC_X:
		createSpheres(25);
		break;

	case OIS::KC_N:
		createBoxes(true, 25);
		break;

	case OIS::KC_B:
		createBoxes(false, 25);
		break;

	case OIS::KC_R:
		{
			hkPseudoRandomGenerator rng(GetTickCount());
			hkVector4 p;
			rng.getRandomVector11(p);
			hkQuaternion q;
			rng.getRandomRotation(q);


			entities["line"]->mTransform.setPosition(bsMath::toXM(p));
			entities["line"]->mTransform.setLocalRotation(bsMath::toXM(q));
		}
		break;

	case OIS::KC_F:
		toggleFreeCam();
		break;

	case OIS::KC_P:
		mScene->setStepPhysicsEnabled(!mScene->isStepPhysicsEnabled());
		break;

	case OIS::KC_Y:
		mCore->getRenderQueue()->setUseInstancing(true);
		break;

	case OIS::KC_U:
		mCore->getRenderQueue()->setUseInstancing(false);
		break;
	}

	return true;
}

void Application::toggleFreeCam()
{
	bsEntity& camEntity = *mScene->getCamera()->getEntity();
	//const XMVECTOR camPosition = camEntity->getPosition();
	const XMVECTOR camRotation = camEntity.mTransform.getRotation();

	if (!camEntity.mTransform.getParentTransform())
	{
		//In free cam mode.
		camEntity.mTransform.setPosition(XMVectorZero());
		camEntity.mTransform.setLocalPosition(XMVectorSet(0.0f, 1.1f, 0.0f, 0.0f));
		//camEntity->setLocalRotation(XMQuaternionIdentity());

		camEntity.mTransform.setParentTransform(&characterControllerEntity->mTransform);

		mFreeCamMode = false;
	}
	else
	{
		//Attached to controller.
		camEntity.mTransform.setParentTransform(nullptr);
		//camEntity->setPosition(camPosition);
		camEntity.mTransform.setLocalRotation(camRotation);

		mFreeCamMode = true;
	}
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


	return true;
}

XMFLOAT3 yawPitchRoll = XMFLOAT3(0.0f, 0.0f, 0.0f);

bool Application::mouseMoved(const OIS::MouseEvent& arg)
{
	bsCamera* camera = mScene->getCamera();
	bsEntity& camEntity = *camera->getEntity();
	const XMVECTOR& cameraRot = camEntity.mTransform.getRotation();

	if (rightMouseDown)
	{
		float rotationX = (float)arg.state.X.rel;
		float rotationY = (float)arg.state.Y.rel;

		const float sensitivity = 0.005f;

		//camera->rotateX(-rotationX * sensitivity);
		//camera->rotateY(-rotationY * sensitivity);

		//const XMVECTOR cameraXAxis = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), cameraRot);
		//const XMVECTOR cameraYAxis = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), cameraRot);

		//const XMVECTOR rotX = XMQuaternionRotationAxis(cameraXAxis, XMConvertToRadians(rotationX * sensitivity));
		//const XMVECTOR rotY = XMQuaternionRotationAxis(cameraYAxis, XMConvertToRadians(rotationY * sensitivity));

		const XMVECTOR rotX = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(rotationY * sensitivity));
		const XMVECTOR rotY = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(rotationX * sensitivity));

		XMVECTOR finalRot = XMQuaternionMultiply(rotX, rotY);
		finalRot = XMQuaternionMultiply(finalRot, cameraRot);

		finalRot = XMQuaternionMultiply(cameraRot, XMQuaternionRotationNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(rotationX * sensitivity)));
		
		yawPitchRoll.x += rotationX * sensitivity;
		yawPitchRoll.y += rotationY * sensitivity;

		finalRot = XMQuaternionRotationRollPitchYaw(yawPitchRoll.y, yawPitchRoll.x, 0.0f);
		
		camEntity.mTransform.setLocalRotation(XMQuaternionInverse(finalRot));
	}


	float rotationZ = bsMath::clamp(-5.0f, 5.0f, (float)arg.state.Z.rel);
	if (mFreeCamMode)
	{
		XMVECTOR scroll = XMVectorSet(0.0f, 0.0f, rotationZ, 0.0f);
		scroll = XMVector3InverseRotate(scroll, cameraRot);
		scroll = XMVectorAdd(scroll, camEntity.mTransform.getPosition());
		camEntity.mTransform.setPosition(scroll);
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

	case OIS::MB_Middle:
		entities["line"]->getLineRenderer()->clear();
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

void Application::createSomeLights()
{
	std::vector<bsEntity*> entities_(5);
	XMFLOAT3 lightColors[5] = 
	{
		XMFLOAT3(1.0f, 1.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(1.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < entities_.size(); ++i)
	{
		bsPointLightCInfo ci;
		ci.color = lightColors[i];
		ci.intensity = 1.0f;
		ci.radius = 25.0f;
		bsLight* light = new bsLight(bsLight::LT_POINT,
			mCore->getResourceManager()->getMeshCache(), ci);

		entities_[i] = new bsEntity();
		entities_[i]->attachLight(*light);
	}
	//auto torus = mCore->getResourceManager()->getMeshCache()->getMesh("torus_knot.bsm");
	//entities_[0]->attachRenderable(torus);

	entities_[0]->mTransform.setPosition(XMVectorSet(3.0f, 50.0f, 3.0f, 0.0f));
	/*
	entities_[0]->getEntity().detach<bsLight*>();
	bsPointLightCInfo ci;
	ci.color = lightColors[0];
	ci.intensity = 1.0f;
	ci.radius = 15.0f;
	entities_[0]->getEntity().attach(new bsLight(bsLight::LT_POINT,
	mCore->getResourceManager()->getMeshCache(), ci));
	*/
	entities_[0]->mTransform.setPosition(XMVectorSet(  0.0f, 50.5f,  0.0f, 0.0f));
	entities_[1]->mTransform.setPosition(XMVectorSet(  0.0f, 7.50f,  25.0f, 0.0f));
	entities_[2]->mTransform.setPosition(XMVectorSet(  0.0f, 7.50f, -25.0f, 0.0f));
	entities_[3]->mTransform.setPosition(XMVectorSet( 25.0f, 7.50f,   0.0f, 0.0f));
	entities_[4]->mTransform.setPosition(XMVectorSet(-25.0f, 7.50f,   0.0f, 0.0f));

	for (size_t i = 0; i < entities_.size(); ++i)
	{
		mScene->addEntity(*entities_[i]);
	}

	//Random position lights
	{
		hkPseudoRandomGenerator rng(GetTickCount());
		const float offsetPositive = 80.0f;
		const float offsetNegative = -5.0f;
		const float maxHeight = 15.0f;
		for (unsigned int i = 0; i < 100; ++i)
		{
			bsEntity* lightEntity = new bsEntity();
			lightEntity->mTransform.setPosition(XMVectorSet(
				rng.getRandRange(offsetNegative, offsetPositive), rng.getRandRange(2.0f, maxHeight),
				rng.getRandRange(offsetNegative, offsetPositive), 1.0f));

			bsPointLightCInfo plci;
			plci.color = XMFLOAT3(rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f));
			plci.radius = rng.getRandRange(2.0f, 25.0f);
			plci.intensity = 1.0f;

			bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(),
				plci);
			lightEntity->attachLight(*light);
			mScene->addEntity(*lightEntity);
		}
	}
}

void Application::createMeshes()
{
	bsMeshCache* meshCache = mCore->getResourceManager()->getMeshCache();

	mMeshes.insert(std::make_pair("teapot", meshCache->getMesh("teapot.bsm")));
	mMeshes.insert(std::make_pair("gourd", meshCache->getMesh("gourd.bsm")));
	mMeshes.insert(std::make_pair("sphere", meshCache->getMesh("sphere_1m_d.bsm")));
	mMeshes.insert(std::make_pair("greeble", meshCache->getMesh("greeble_town_small.bsm")));
	mMeshes.insert(std::make_pair("plane", meshCache->getMesh("plane_1m.bsm")));
	mMeshes["cube"] = meshCache->getMesh("unit_cube.bsm");
}

void Application::createTexts()
{
	bsTextManager* textManager = mCore->getResourceManager()->getTextManager();

	auto statsText = textManager->createText2D(L"");
	statsText->setPosition(2.5f, 2.5f);
	statsText->setColor(0xff0000ff);
	statsText->setFontSize(16.0f);
	statsText->addFlags(FW1_RESTORESTATE);

	
	//Scrolling text
	std::shared_ptr<bsScrollingText2D> textBox(textManager->createScrollingText2D(4500.0f, 10));
	textBox->getText()->setPosition(10.0f, 350.0f);
	textBox->getText()->addFlags(FW1_BOTTOM | FW1_RESTORESTATE);
	textBox->getText()->setFontSize(14.0f);
	bsLog::addCallback([textBox](const char* message)
	{
		textBox->addTextLine(message);
	});

	auto frameStats = textManager->createText2D(L"");
	frameStats->setPosition(2.5f, 450.0f);
	frameStats->addFlags(FW1_RESTORESTATE);
	frameStats->setColor(0xff00ff00);
	frameStats->setFontSize(14.0f);

	auto camPos = textManager->createText2D(L"");
	camPos->setPosition(850.0f, 2.5f);
	camPos->addFlags(FW1_RESTORESTATE);
	camPos->setColor(0xffffffff);
	camPos->setFontSize(14.0f);

	auto crosshair = textManager->createText2D(L".");
	crosshair->setPosition((float)mCore->getWindow()->getWindowWidth() * 0.5f,
		((float)mCore->getWindow()->getWindowHeight() * 0.5f) - 10.0f);
	crosshair->addFlags(FW1_RESTORESTATE | FW1_CENTER);
	crosshair->setColor(0xffcccccc);
	crosshair->setFontSize(12.0f);


	mTexts.insert(std::make_pair("stats", statsText));
	//mTexts.insert(std::make_pair("scrolling", textBox));
	mTexts.insert(std::make_pair("frameStats", frameStats));
	mTexts.insert(std::make_pair("camPos", camPos));
	mTexts.insert(std::make_pair("crosshair", crosshair));
}

#include <Common/Base/Reflection/hkClass.h>
#include <Common/Base/System/Io/Reader/hkStreamReader.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Serialize/Util/hkNativePackfileUtils.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Common/Serialize/Util/hkStructureLayout.h>
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Common/Serialize/Util/hkSerializeUtil.h>

void Application::createEntities()
{
	//Sphere
	{
		const XMVECTOR spherePosition = XMVectorSet(12.5f, 5.0f, 5.0f, 0.0f);

		bsEntity* sphereEntity = new bsEntity();
		sphereEntity->attachMesh(mMeshes["sphere"]);

		hkpRigidBodyCinfo ci;
		ci.m_shape = new hkpSphereShape(0.5f);
		ci.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;
		ci.m_position = bsMath::toHK(spherePosition);
		//ci.m_friction = 0.0f;
		//ci.m_restitution = 1.01f;
		//ci.m_linearDamping;
		ci.m_mass = (4.0f / 3.0f) * XM_PI * (0.5f * 0.5f * 0.5f);
		hkpRigidBody* sphereRb = new hkpRigidBody(ci);
		ci.m_shape->removeReference();
		sphereEntity->attachRigidBody(*sphereRb);

		bsPointLightCInfo plci;
		plci.color = XMFLOAT3(1.0f, 1.0f, 1.0f);
		plci.intensity = 1.0f;
		plci.radius = 10.0f;
		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), plci);
		sphereEntity->attachLight(*light);
	
		mScene->addEntity(*sphereEntity);
		entities.insert(std::make_pair("sphere", sphereEntity));

		sphereEntity->mTransform.setPosition(spherePosition);
	}
	
	//Ground
	{
		//bsEntity* ground = mPrimCreator->createPlane(XMVectorSet(50.0f, 1.0f, 50.0f, 0.0f));
		//mScene->addEntity(ground);
		//entities["ground"] = ground;
	}


	bsEntity* lineEntity = new bsEntity();
	bsLineRenderer* line3D = new bsLineRenderer(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	lineEntity->attachLineRenderer(*line3D);
	entities.insert(std::make_pair("line", lineEntity));
	mScene->addEntity(*lineEntity);

	bsEntity* textEntity = new bsEntity();
	bsText3D* text3D = new bsText3D(mCore->getDx11Renderer()->getDeviceContext(),
		mCore->getDx11Renderer()->getDevice(), mCore->getResourceManager()->getTextManager()->getFw1Factory());
	textEntity->attachTextRenderer(*text3D);
	entities["text"] = textEntity;
	mScene->addEntity(*textEntity);
}

void Application::createSpheres(unsigned int count)
{
	hkPseudoRandomGenerator rng(GetTickCount());
	const hkVector4 minPos(-25.0f,  2.5f, -25.0f);
	const hkVector4 maxPos( 25.0f, 15.0f,  25.0f);

	std::vector<bsEntity*> spheres;
	spheres.reserve(count);

	hkVector4 pos;
	float radius;

	bsPointLightCInfo ci;

	for (size_t i = 0; i < count; ++i)
	{
		rng.getRandomVectorRange(minPos, maxPos, pos);
		radius = rng.getRandRange(0.2f, 2.0f);
		const float volume = (4.0f / 3.0f) * XM_PI * (radius * radius * radius);

#if 1
		bsEntity* sphereEntity = new bsEntity();
		sphereEntity->attachMesh(mMeshes["sphere"]);

		hkpMassProperties massProps;
		hkInertiaTensorComputer::computeSphereVolumeMassProperties(radius, volume, massProps);

		hkpRigidBodyCinfo rbci;
		rbci.setMassProperties(massProps);
		rbci.m_shape = new hkpSphereShape(radius);
		rbci.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;
		rbci.m_position = pos;
		//rbci.m_mass = volume;
		hkpRigidBody* rb = new hkpRigidBody(rbci);
		sphereEntity->attachRigidBody(*rb);

		mScene->addEntity(*sphereEntity);

		sphereEntity->mTransform.setLocalScaleUniform(radius * 2.0f);
#endif
		//bsEntity* sphere = mPrimCreator->createSphere(radius);
		sphereEntity->mTransform.setLocalPosition(bsMath::toXM(pos));

		/*
		//Detach mesh+attach light

		sphere->getEntity().detach<std::shared_ptr<bsMesh>>();

		ci.color = XMFLOAT3(rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f), rng.getRandRange(0.0f, 1.0f));
		ci.radius = rng.getRandRange(1.0f, 15.0f);
		bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(), ci);
		sphere->getEntity().attach(light);
		*/

		//mScene->addEntity(sphere);
	}
}

void Application::createBoxes(bool staticBoxes, unsigned int count)
{
	hkPseudoRandomGenerator rng(GetTickCount());
	const hkVector4 minPos(-25.0f,  2.5f, -25.0f);
	const hkVector4 maxPos( 25.0f, 15.0f,  25.0f);

	auto mesh = mCore->getResourceManager()->getMeshCache()->getMesh("unit_cube.bsm");
	//auto mesh = mCore->getResourceManager()->getMeshCache()->getMesh("teapot.bsm");

	XMFLOAT3 boundingSphereCenter;
	XMStoreFloat3(&boundingSphereCenter, mesh->getBoundingSphere().positionAndRadius);
	const float bsradius = mesh->getBoundingSphere().getRadius();
	float boundingSphereRadius = bsradius;


	hkVector4 pos;
	XMFLOAT4A scale;

	const hkpMotion::MotionType motion = staticBoxes ? hkpMotion::MOTION_FIXED
		: hkpMotion::MOTION_BOX_INERTIA;

	for (size_t i = 0; i < count; ++i)
	{
		rng.getRandomVectorRange(minPos, maxPos, pos);
		scale.x = rng.getRandRange(0.5f, 5.0f);
		scale.y = rng.getRandRange(0.5f, 5.0f);
		scale.z = rng.getRandRange(0.5f, 5.0f);
		scale.x = scale.y = scale.z;


		const float maxScale = max(max(scale.x, scale.y), scale.z);
		boundingSphereRadius = bsradius * maxScale;

		const float volume = scale.x * scale.y * scale.z;

#if 1
		bsEntity* boxEntity = new bsEntity();
		boxEntity->attachMesh(mesh);

		hkVector4 halfExtents(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
		const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
			hkConvexShapeDefaultRadius);
		halfExtents.sub(convexRadius);

		hkpMassProperties massProps;
		hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume, massProps);

		hkpRigidBodyCinfo rbci;
		rbci.setMassProperties(massProps);
		rbci.m_shape = new hkpBoxShape(halfExtents);
		rbci.m_motionType = motion;
		rbci.m_position = pos;

		hkpRigidBody* rb = new hkpRigidBody(rbci);
		boxEntity->attachRigidBody(*rb);

		mScene->addEntity(*boxEntity);

		boxEntity->mTransform.setLocalScale(XMLoadFloat4A(&scale));
		boxEntity->mTransform.setPosition(bsMath::toXM(pos));
#endif

		bsLineRenderer* lineRenderer = new bsLineRenderer(XMFLOAT4(0.25f, 1.0f, 0.25f, 1.0f));

		bsGeometry::createLinesFromSphere(boundingSphereCenter, boundingSphereRadius,
			24, lineRenderer);
		lineRenderer->build(mCore->getDx11Renderer());

		boxEntity->attachLineRenderer(*lineRenderer);

		
		/*
		bsEntity* boxEntity = mPrimCreator->createBox(XMVectorScale(XMLoadFloat4A(&scale), 0.5f));
		boxEntity->setLocalPosition(bsMath::toXM(pos));
		boxEntity->getEntity().mRigidBody->setMotionType(motion);
		mScene->addEntity(boxEntity);
		*/
	}
}

void Application::createWalls(float offsetFromCenter, float length, float height)
{
	const XMVECTOR northPos = XMVectorSet(0.0f, height * 0.5f, offsetFromCenter * 0.5f, 0.0f);
	const XMVECTOR eastPos = XMVectorSet(offsetFromCenter * 0.5f, height * 0.5f, 0.0f, 0.0f);
	const XMVECTOR southPos = XMVectorSet(0.0f, height * 0.5f, -offsetFromCenter * 0.5f, 0.0f);
	const XMVECTOR westPos = XMVectorSet(-offsetFromCenter * 0.5f, height * 0.5f, 0.0f, 0.0f);

	
	bsEntity* northEntity = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsEntity* eastEntity = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsEntity* southEntity = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	bsEntity* westEntity = mPrimCreator->createBox(XMVectorSet(length * 0.5f, height * 0.5f, 5.0f * 0.5f, 1.0f));
	northEntity->mTransform.getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	eastEntity->mTransform.getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	southEntity->mTransform.getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);
	westEntity->mTransform.getEntity().mRigidBody->setMotionType(hkpMotion::MOTION_FIXED);

	northEntity->mTransform.setLocalPosition(northPos);
	eastEntity->mTransform.setLocalPosition(eastPos);
	southEntity->mTransform.setLocalPosition(southPos);
	westEntity->mTransform.setLocalPosition(westPos);



	XMVECTOR scale = XMVectorSet(length, height, 5.0f, 1.0f);
	northEntity->mTransform.setLocalScale(scale);
	eastEntity->mTransform.setLocalScale(scale);
	southEntity->mTransform.setLocalScale(scale);
	westEntity->mTransform.setLocalScale(scale);

	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(90.0f));
	eastEntity->mTransform.setLocalRotation(rotation);

	rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(180.0f));
	southEntity->mTransform.setLocalRotation(rotation);

	rotation = XMQuaternionRotationNormal(up, XMConvertToRadians(270.0f));
	westEntity->mTransform.setLocalRotation(rotation);


	mScene->addEntity(*northEntity);
	mScene->addEntity(*eastEntity);
	mScene->addEntity(*southEntity);
	mScene->addEntity(*westEntity);
}

void Application::createKeyframedRb()
{
	const XMVECTOR boxSize = XMVectorSet(5.0f, 0.1f, 5.0f, 0.0f);
	const XMVECTOR position = XMVectorSet(10.0f, 0.1f, 0.0f, 0.0f);

	bsEntity* keyframedEntity = new bsEntity();
	keyframedEntity->mTransform.setPosition(position);

	keyframedEntity->attachMesh(mMeshes["cube"]);
	keyframedEntity->mTransform.setLocalScale(boxSize);

	float volume = XMVectorGetX(boxSize) * XMVectorGetY(boxSize) * XMVectorGetZ(boxSize);
	hkVector4 halfExtents(bsMath::toHK(boxSize));
	halfExtents.mul(hkSimdReal::convert(0.5f));

	hkpMassProperties massProps;
	hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume, massProps);

	hkpRigidBodyCinfo ci;
	ci.setMassProperties(massProps);
	ci.m_shape = new hkpBoxShape(bsMath::toHK(XMVectorScale(boxSize, 0.5f)));
	ci.m_motionType = hkpMotion::MOTION_KEYFRAMED;
	ci.m_position = bsMath::toHK(position);

	hkpRigidBody* rb = new hkpRigidBody(ci);
	keyframedEntity->attachRigidBody(*rb);



	XMFLOAT4 bounding;
	XMStoreFloat4(&bounding, keyframedEntity->getMesh()->getBoundingSphere().positionAndRadius);

	bsLineRenderer* lineRenderer = new bsLineRenderer(XMFLOAT4(0.25f, 1.0f, 0.25f, 1.0f));
	bsGeometry::createLinesFromSphere(XMFLOAT3(bounding.x, bounding.y, bounding.z), bounding.w * 5.0f,
		24, lineRenderer);
	lineRenderer->build(mCore->getDx11Renderer());
	keyframedEntity->attachLineRenderer(*lineRenderer);




	mScene->addEntity(*keyframedEntity);

	entities["lift"] = keyframedEntity;

	float j = 1.0f;
	for (size_t i = 0; i < 10; ++i, j += 1.0f)
	{
		//const float x = (i % 2 == 0) ? 0.1f : -0.1f;
		//const float z = (i % 4 == 0) ? 0.1f : -0.1f;

		halfExtents.set(0.5f, 0.5f, 0.5f);
		const hkVector4 convexRadius(hkConvexShapeDefaultRadius, hkConvexShapeDefaultRadius,
			hkConvexShapeDefaultRadius);
		halfExtents.sub(convexRadius);

		volume = (halfExtents.getComponent<0>().getReal() * 2.0f) *
			(halfExtents.getComponent<1>().getReal() * 2.0f) * (halfExtents.getComponent<2>().getReal() * 2.0f);

		hkInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, volume * 100.0f, massProps);

		hkpRigidBodyCinfo ci;
		ci.setMassProperties(massProps);
		ci.m_shape = new hkpBoxShape(halfExtents);
		ci.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
		//ci.m_mass = 10.0f;
		hkpRigidBody* rb = new hkpRigidBody(ci);

		bsEntity* boxEntity = new bsEntity();
		boxEntity->attachRigidBody(*rb);
		boxEntity->attachMesh(mMeshes["cube"]);

		//const XMVECTOR boxPosition = XMVectorAdd(position, XMVectorSet(x, (j + 1.1001f) * 1.01f, z, 0.0f));
		const XMVECTOR boxPosition = XMVectorAdd(position, XMVectorSet(0.0f, j * 1.0001f, 0.0f, 0.0f));
		boxEntity->mTransform.setPosition(boxPosition);
		//boxEntity->setLocalScale(XMVectorSet(2.1f, 2.1f, 2.1f, 0.0f));


		XMFLOAT4 bounding;
		XMStoreFloat4(&bounding, boxEntity->getMesh()->getBoundingSphere().positionAndRadius);

		bsLineRenderer* lineRenderer = new bsLineRenderer(XMFLOAT4(0.25f, 1.0f, 0.25f, 1.0f));
		bsGeometry::createLinesFromSphere(XMFLOAT3(bounding.x, bounding.y, bounding.z), bounding.w,
			24, lineRenderer);
		lineRenderer->build(mCore->getDx11Renderer());
		boxEntity->attachLineRenderer(*lineRenderer);

		mScene->addEntity(*boxEntity);
	}
}

void Application::createLines()
{
	mCurve = new hkpLinearParametricCurve();
	mCurve->setSmoothingFactor(0.5f);

	const unsigned int pointCount = 1500;
	const unsigned int extraPointCount = 100;
	const float pointIncr = 1.0f / pointCount;

	const float twists = 3.0f;
	const float height = 15.0f;
	const float radius = 10.0f;
	hkVector4 point;

	float f = 1.0f;
	for (unsigned int t = 0; t < pointCount; ++t, f -= pointIncr)
	{
		const float angle = f * XM_PI * 2.0f * twists;
		point.set(cosf(angle) * radius * (1.0f + f), f * height, sinf(angle) * radius * (1.0f + f));

		mCurve->addPoint(point);
	}

	{
		hkVector4 startPoint, endPoint, currentPosition;
		mCurve->getPoint(mCurve->getStart(), startPoint);
		mCurve->getPoint(mCurve->getEnd(), endPoint);

		const float extraPointIncr = 1.0f / extraPointCount;
		f = 0.0f;
		for (unsigned int i = 0; i < extraPointCount; ++i, f += extraPointIncr)
		{
			currentPosition.setInterpolate(endPoint, startPoint, hkSimdReal::convert(f));
			mCurve->addPoint(currentPosition);
		}
	}
	
	//Make closed.
		f = 1.0f;
	const float angle = f * XM_PI * 2.0f * twists;
	point.set(cosf(angle) * radius * (1.0f + f), f * height, sinf(angle) * radius * (1.0f + f));
	mCurve->addPoint(point);

	mCurve->setClosedLoop(true);


	std::vector<XMFLOAT3> pointList;
	pointList.reserve((pointCount * 2) + 2 + extraPointCount);

	XMFLOAT3 p;
	hkVector4 pointOnCurve;
	f = 0.0f;
	for (unsigned int i = 0; i < pointCount + extraPointCount; ++i, f += 1.0f)
	{
		mCurve->getPoint(f, pointOnCurve);
		pointOnCurve.store<3, HK_IO_NATIVE_ALIGNED>(&p.x);
		pointList.push_back(p);

		mCurve->getPoint(f + 1.0f, pointOnCurve);
		pointOnCurve.store<3, HK_IO_NATIVE_ALIGNED>(&p.x);
		pointList.push_back(p);
	}

	bsEntity* lineEntity = new bsEntity();
	bsLineRenderer* line = new bsLineRenderer(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
	line->addPoints(&pointList[0], pointList.size());
	line->build(mCore->getDx11Renderer());
	lineEntity->attachLineRenderer(*line);
	mScene->addEntity(*lineEntity);
	entities["mCurve"] = lineEntity;
}

bsEntity* Application::createLightAtPosition(const XMVECTOR& position, float radius, const XMFLOAT3& color)
{
	bsEntity* entity = new bsEntity();

	bsPointLightCInfo plci;
	plci.color = color;
	plci.intensity = 1.0f;
	plci.radius = radius;
	
	bsLight* light = new bsLight(bsLight::LT_POINT, mCore->getResourceManager()->getMeshCache(),
		plci);
	entity->attachLight(*light);

	entity->mTransform.setPosition(position);

	return entity;
}

void Application::createFactory()
{
	bsEntity* greebleEntity = new bsEntity();
	//greebleEntity.attach(mMeshes["greeble"]);
	//greebleEntity.attach(mCore->getResourceManager()->getMeshCache()->getMesh("greeble_town.bsm"));
	greebleEntity->attachMesh(mCore->getResourceManager()->getMeshCache()->getMesh("factory.bsm"));
	//greebleEntity.attach(mCore->getResourceManager()->getMeshCache()->getMesh("arrow.bsm"));
	greebleEntity->mTransform.setPosition(XMVectorSet(0.0f, 0.1f, 0.0f, 0.0f));

	entities.insert(std::make_pair("greeble", greebleEntity));
	{
		const std::string fileName = mCore->getResourceManager()->getFileSystem()->
			getPathFromFilename("factory.hkx");
		//getPathFromFilename("arrow.hkx");

		hkResource* loadedData;
		hkSerializeUtil::ErrorDetails loadError;
		loadedData = hkSerializeUtil::load(fileName.c_str(), &loadError);
		BS_ASSERT(loadedData);

		hkRootLevelContainer* container = loadedData->getContents<hkRootLevelContainer>();
		BS_ASSERT(container);

		hkpPhysicsData* physicsData = static_cast<hkpPhysicsData*>(container->
			findObjectByType(hkpPhysicsDataClass.getName()));
		BS_ASSERT(physicsData);

		const hkArray<hkpPhysicsSystem*>& physicsSystems = physicsData->getPhysicsSystems();
		BS_ASSERT(physicsSystems.getSize() == 1);
		BS_ASSERT(physicsSystems[0]->getRigidBodies().getSize() == 1);

		hkpRigidBody* rb = physicsSystems[0]->getRigidBodies()[0];
		//rb->setPosition(hkVector4(0.0f, 0.1f, 0.0f));
		//hkQuaternion q = rb->getRotation();
		//rb->setRotation(hkQuaternion::getIdentity());

		greebleEntity->attachRigidBody(*rb);
		//greebleEntity->setPosition(XMVectorSplatOne());

		//loadedData->removeReference();
	}
	mScene->addEntity(*greebleEntity);

	createFactoryLights();
}

void Application::createFactoryLights()
{
	std::vector<bsEntity*> factoryLightEntities;
	factoryLightEntities.reserve(50);

	const XMFLOAT3 pole1Color(1.0f, 1.0f, 0.0f);
	const float pole1Radius = 10.0f;

	//Light poles, outside
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(-3.888f, 7.357f, 20.353f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(-3.758f, 7.425f, -5.118f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(11.155f, 7.290f, -5.153f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(26.460f, 7.338f, -5.140f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(41.830f, 7.248f, -5.163f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(55.885f, 7.562f, -6.136f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(54.399f, 7.354f, 7.460f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(56.222f, 7.353f, 20.534f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(41.958f, 7.337f, 20.358f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(26.807f, 7.357f, 20.353f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(11.326f, 7.291f, 20.369f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(8.721f, 8.152f, 10.135f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(23.830f, 8.142f, 10.134f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(39.133f, 8.137f, 10.133f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(40.390f, 8.401f, 5.729f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(23.978f, 8.196f, 4.693f, 0.0f), pole1Radius, pole1Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(8.772f, 8.181f, 4.695f, 0.0f), pole1Radius, pole1Color));


	const XMFLOAT3 pole2Color(1.0f, 1.0f, 0.0f);
	const float pole2Radius = 10.0f;

	//Light poles, other side
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(6.613f, 7.173f, 59.080f, 0.0f), pole2Radius, pole2Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(6.562f, 7.429f, 65.674f, 0.0f), pole2Radius, pole2Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(18.676f, 7.471f, 66.730f, 0.0f), pole2Radius, pole2Color));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(19.777f, 7.268f, 59.104f, 0.0f), pole2Radius, pole2Color));


	const XMFLOAT3 insideColor(1.0f, 0.75f, 0.0f);
	const float insideRadius = 5.0f;

	//Lights inside factory
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(32.394f, 2.540f, 32.108f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(6.613f, 7.173f, 59.080f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(6.562f, 7.429f, 65.674f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(18.676f, 7.471f, 66.730f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(19.777f, 7.268f, 59.104f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(13.338f, 2.540f, 27.841f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(20.105f, 2.540f, 34.408f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(27.829f, 2.540f, 37.567f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(32.822f, 2.540f, 43.459f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(27.967f, 2.540f, 46.547f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(20.151f, 2.540f, 49.886f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(32.885f, 2.540f, 43.716f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(45.068f, 3.683f, 37.157f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(45.059f, 3.683f, 41.054f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(45.071f, 3.683f, 45.268f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(36.378f, 2.540f, 53.706f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(41.332f, 2.540f, 53.683f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(46.456f, 2.540f, 53.727f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(51.567f, 2.540f, 53.837f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(47.540f, 2.540f, 47.603f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(52.665f, 2.540f, 47.512f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(54.069f, 2.540f, 60.130f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(54.779f, 2.540f, 29.036f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(67.384f, 3.059f, 54.627f, 0.0f), insideRadius, insideColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(58.841f, 3.063f, 47.061f, 0.0f), insideRadius, insideColor));


	const XMFLOAT3 underColor(1.0f, 0.0f, 0.0f);
	const float underRadius = 7.5f;

	//Underground
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(49.232f, -2.032f, 57.773f, 0.0f), underRadius, underColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(49.286f, -2.032f, 67.051f, 0.0f), underRadius, underColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(33.145f, -0.762f, 67.122f, 0.0f), underRadius, underColor));
	factoryLightEntities.push_back(createLightAtPosition(XMVectorSet(33.100f, -0.762f, 57.669f, 0.0f), underRadius, underColor));




	const XMVECTOR offset = XMVectorSet(0.0f, -0.05f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < factoryLightEntities.size(); ++i)
	{
		factoryLightEntities[i]->mTransform.translate(offset);
		mScene->addEntity(*factoryLightEntities[i]);
	}
}
