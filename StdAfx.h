#pragma once

//Std lib
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <deque>

//C std lib
#include <stdlib.h>

#include <Windows.h>

//DirectX
#include <d3d11.h>
#include <D3DX11.h>
#include <xnamath.h>

//Havok

#include <Common/Base/hkBase.h>
#include <Common/Base/Container/Array/hkArray.h>

#include <Common/Base/Math/hkMath.h>
#include <Physics/Dynamics/hkpDynamics.h>

#include <Physics/Collide/hkpCollide.h>
#include <Physics/Collide/Agent/ConvexAgent/SphereBox/hkpSphereBoxAgent.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>

#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>

#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>


//FW1
#include <FW1FontWrapper.h>

//OIS
#include <OIS.h>

//TBB
#include <tbb/tbb_thread.h>
