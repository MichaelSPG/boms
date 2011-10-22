#pragma once


/*	Contains functions for saving and loading of meshes from disk.

	Define BS_SUPPORT_MESH_CREATION to enable creating bsSerializedMesh from
	other mesh formats. Note that this requires assimp to work.

	Define BS_MESH_SERIALIZER_EXTERNAL to enable using outside of the main engine.
	This results in StdAfx.h not being included (since it includes several other libraries),
	and log messages will be sent to an extern function matching the following signature:
	void bsMeshSerializerLogErrorMessage(const char*);
*/


#include <string>

#include <Windows.h>
#include <xnamath.h>

#include "bsVertexTypes.h"




/*	This struct contains everything used to describe a mesh; an array of index/vertex
	buffers (in case a mesh has submeshes), and min/max extents of all the vertices in
	all submeshes.

	There are functions to load/save this struct from/to disk, as well as a function to
	create it from a different mesh format (using Assimp).

	All of the mesh' dynamically allocated memory (index/vertex buffers) is allocated in
	a single contiguous memory block if using the functions below.
*/
struct bsSerializedMesh
{
	bsSerializedMesh()
		: vertexBuffers(nullptr)
		, indexBuffers(nullptr)
		, bufferCount(0)
		, boundingSphereCenterAndRadius(0.0f, 0.0f, 0.0f, -1.0f)//Invalid radius.
		, dynamicData(nullptr)
		, dynamicDataSize(0)
	{}

	bsSerializedMesh(bsSerializedMesh&& other)
	{
		memcpy(this, &other, sizeof(bsSerializedMesh));

		memset(&other, 0, sizeof(bsSerializedMesh));
	}

	bsSerializedMesh& operator=(bsSerializedMesh&& other)
	{
		memcpy(this, &other, sizeof(bsSerializedMesh));

		memset(&other, 0, sizeof(bsSerializedMesh));

		return *this;
	}

	~bsSerializedMesh()
	{
		free(dynamicData);
	}


	bool operator==(const bsSerializedMesh& other)
	{
		const int memCmpBoundingSphere = memcmp(&boundingSphereCenterAndRadius,
			&other.boundingSphereCenterAndRadius, sizeof(XMFLOAT3));

		const bool bufferCountAndExtentsEqual = bufferCount == other.bufferCount
			&& memCmpBoundingSphere == 0;

		if (!bufferCountAndExtentsEqual)
		{
			return false;
		}

		for (size_t i = 0; i < bufferCount; ++i)
		{
			//Compare vertex buffer sizes and contents.
			const bsVertexBuffer& currentVertexBuffer = vertexBuffers[i];
			if (currentVertexBuffer.vertexCount != other.vertexBuffers[i].vertexCount)
			{
				return false;
			}

			if (memcmp(currentVertexBuffer.vertices,
				other.vertexBuffers[i].vertices,
				sizeof(bsVertexNormalTex) * currentVertexBuffer.vertexCount) != 0)
			{
				return false;
			}

			//Compare index buffer sizes and contents.
			const bsIndexBuffer& currentIndexBuffer = indexBuffers[i];
			if (currentIndexBuffer.indexCount != other.indexBuffers[i].indexCount)
			{
				return false;
			}

			if (memcmp(currentIndexBuffer.indices,
				other.indexBuffers[i].indices,
				sizeof(unsigned int) * currentIndexBuffer.indexCount) != 0)
			{
				return false;
			}
		}

		return true;
	}


	//Arrays of vertex and index buffers for each submesh.
	//Both of these will always have the same size.
	bsVertexBuffer*	vertexBuffers;
	bsIndexBuffer*	indexBuffers;

	//Size of the two above buffers (not combined).
	unsigned int	bufferCount;

	//Center and radius of bounding sphere, radius in w.
	XMFLOAT4	boundingSphereCenterAndRadius;

	//Holds all the dynamically allocated memory in this mesh.
	void*	dynamicData;
	unsigned int dynamicDataSize;
};

/*	Loads a serialized mesh from disk into the output parameter.
	Returns true on successful load.
*/
bool bsLoadSerializedMesh(const std::string& fileName, bsSerializedMesh& meshOut);

/*	Loads a serialized mesh from memory into the output parameter.
	Returns true on successful load.
*/
bool bsLoadSerializedMeshFromMemory(const char* data, unsigned int dataSize,
	bsSerializedMesh& meshOut);

/*	Save the serialized mesh to disk with the given filename.
	Returns true on successful save.
*/
bool bsSaveSerializedMesh(const std::string& fileName, const bsSerializedMesh& mesh);

#ifdef BS_SUPPORT_MESH_CREATION
/*	Flags returned by the function below (bsCreateSerializedMesh).
	Can be used to determine what went wrong if the function did not return BS_MESH_SUCCESSS.
*/
enum bsCreateSerializedMeshFlags
{
	//All OK, output mesh is good to use.
	BS_MESH_SUCCESSS = 0,

	//Unspecified error, most likely failed to load the file.
	BS_MESH_GENERIC_ERROR = 1 << 0,

	//A memory allocation failed.
	BS_MESH_MEMORY_FAILURE = 1 << 5,

	//The input mesh contains invalid data or is empty.
	BS_MESH_INVALID_INPUT_MESH = 1 << 10,
};

#include <functional>

/*	A function with these parameters must be provided when calling bsCreateSerializedMesh.
	The function should calculate the minimal bounding sphere around all the points in the
	mesh, and return the sphere's center and radius as the last two parameters.
	The function should also return true if successful, or false if not successful.

	One option is using XNACollision's ComputeBoundingSphereFromPoints, which can be found
	in DXSDK\Samples\C++\Misc\Collision\xnacollision.h
*/
typedef std::function<bool(const bsSerializedMesh&, XMFLOAT3&, float&)> bsComputeBoundingSphereCallback;

/*	Loads a mesh from disk and converts it to the bsSerializedMesh format.
	Returns true on success.
*/
bsCreateSerializedMeshFlags bsCreateSerializedMesh(const std::string& fileName,
	const bsComputeBoundingSphereCallback& boundingSphereCallback, bsSerializedMesh& meshOut);
#endif // BS_SUPPORT_MESH_CREATION
