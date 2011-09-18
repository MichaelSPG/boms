#include "StdAfx.h"

#include "bsMeshSerializer.h"

/*	File info:
Byte		Description
1-3			bytes: File format identification (chars).
4			Version information (char).
5-8			Amount of buffers (uint).
9-12		Total memory size of the mesh' dynamic memory, not including the
				bsSerializedMesh struct itself
13-16		Unused.

17-...		bsVertexBuffers * buffer count (defined by bytes 5-8).
...-...		bsIndexBuffers * buffer count.

Last 24		AABB, min extents and max extents (in that order) (2 * XMFLOAT3).


bsVertexBuffer
{
1-4			Amount of following vertices (uint).
5-...		(Vertex amount * sizeof(bsVertexNormalTex)) of bsVertexNormalTex*.
}

bsIndexBuffer
{
1-4			Amount of following indices (uint).
5-...		(Index amount * sizeof(unsigned int)) of unsigned int*.
}
*/

//Version info is embedded into
const char kSerializerVersion = 0;

#include <fstream>
#include <sstream>
#include <float.h>
#include <stdio.h>

//TODO: Remove these
#include <cassert>

#include "bsLog.h"
#include "bsLinearHeapAllocator.h"
#include "bsMath.h"

#ifdef BS_SUPPORT_MESH_CREATION
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>

bsCreateSerializedMeshFlags parseData(const aiMesh* mesh, bsVertexBuffer& verticesOut,
	bsIndexBuffer& indicesOut, XMFLOAT3& minExtentsOut, XMFLOAT3& maxExtentsOut,
	bsLinearHeapAllocator& allocator);

//Copies contents of aiVec to xmF3
inline XMFLOAT3 aiVector3ToXMFloat3(const aiVector3D& aiVec)
{
	return XMFLOAT3(aiVec.x, aiVec.y, aiVec.z);
}

//Returns a vec2 with x and y from the aiVec
inline XMFLOAT2 aiVector3ToXMFloat2TexCoord(const aiVector3D& aiVec)
{
	return XMFLOAT2(aiVec.x, aiVec.y);
}
#endif // BS_SUPPORT_MESH_CREATION


//Returns a vector3 with the smallest individual elements from v1 and v2.
inline XMFLOAT3 getMinExtents(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	XMFLOAT3 smallest;
	smallest.x = min(v1.x, v2.x);
	smallest.y = min(v1.y, v2.y);
	smallest.z = min(v1.z, v2.z);

	return smallest;
}

//Returns a vector3 with the largest individual elements from v1 and v2.
inline XMFLOAT3 getMaxExtents(const XMFLOAT3& v1, const XMFLOAT3& v2)
{
	XMFLOAT3 largest;
	largest.x = max(v1.x, v2.x);
	largest.y = max(v1.y, v2.y);
	largest.z = max(v1.z, v2.z);

	return largest;
}

bool bsLoadSerializedMesh(const std::string& fileName, bsSerializedMesh& meshOut)
{
#pragma warning(push)
#pragma warning (disable : 4996)//warning C4996: 'fopen' was declared deprecated
	FILE* file = fopen(fileName.c_str(), "rb");
#pragma warning(pop)
	assert(file != nullptr);

	char header[16];
	size_t read = fread(header, 16, 1, file);
	assert(read == 1);

	//Verify that the header is correct.
	if (memcmp(header, "bsm", 3) != 0)
	{
		std::string errorMessage("Bad file header in \'");
		errorMessage.append(fileName);
		errorMessage.append("\'");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		fclose(file);

		return false;
	}
	if (header[3] != kSerializerVersion)
	{
		std::string errorMessage("\'");
		errorMessage.append(fileName);
		errorMessage.append("\' was created with an old version of the serializer.");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		fclose(file);

		return false;
	}

	unsigned int bufferCount;
	memcpy(&bufferCount, header + 4, sizeof(unsigned int));

	unsigned int totalDynamicMemorySize;
	memcpy(&totalDynamicMemorySize, header + 8, sizeof(unsigned int));

	//Allocate enough memory to hold all the dynamic data used by the mesh.
	//Does not include the min/max extents to not waste 24 bytes of memory.
	char* const dataBuffer = static_cast<char*>(malloc(totalDynamicMemorySize));
	char* head = dataBuffer;

	read = fread(dataBuffer, totalDynamicMemorySize, 1, file);

	if (read != 1)
	{
		std::string errorMessage("Failed to read \'");
		errorMessage.append(fileName);
		errorMessage.append("\'");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		fclose(file);

		return false;
	}

	//Assign the vertex/index buffer pointers. This is all that is required to load the
	//dynamic memory because of the data layout used in the file.
	bsSerializedMesh meshToLoad;
	meshToLoad.bufferCount = bufferCount;
	meshToLoad.vertexBuffers = reinterpret_cast<bsVertexBuffer*>(head);
	head += sizeof(bsVertexBuffer) * bufferCount;
	assert(head < dataBuffer + totalDynamicMemorySize);

	meshToLoad.indexBuffers = reinterpret_cast<bsIndexBuffer*>(head);
	head += sizeof(bsIndexBuffer) * bufferCount;
	assert(head < dataBuffer + totalDynamicMemorySize);

	for (size_t i = 0; i < meshToLoad.bufferCount; ++i)
	{
		//Assign vertices.
		bsVertexBuffer& currentVertexBuffer = meshToLoad.vertexBuffers[i];

		currentVertexBuffer.vertices = reinterpret_cast<bsVertexNormalTex*>(head);
		head += sizeof(bsVertexNormalTex) * currentVertexBuffer.vertexCount;
		assert(head < dataBuffer + totalDynamicMemorySize);

		//Assign indices.
		bsIndexBuffer& currentIndexBuffer = meshToLoad.indexBuffers[i];

		currentIndexBuffer.indices = reinterpret_cast<unsigned int*>(head);
		head += sizeof(unsigned int) * currentIndexBuffer.indexCount;
	}
	assert(head == dataBuffer + totalDynamicMemorySize);

	//Read min/max extents into a buffer.
	char minMaxExtents[sizeof(XMFLOAT3) * 2];
	read = fread(minMaxExtents, sizeof(XMFLOAT3) * 2, 1, file);
	if (read != 1)
	{
		std::string errorMessage("Failed to load min/max extents for \'");
		errorMessage.append(fileName);
		errorMessage.append("\'");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		fclose(file);

		return false;
	}

	//Copy min/max extents.
	memcpy(&meshToLoad.minExtents, minMaxExtents, sizeof(XMFLOAT3));
	memcpy(&meshToLoad.maxExtents, minMaxExtents + sizeof(XMFLOAT3), sizeof(XMFLOAT3));

	meshOut = std::move(meshToLoad);

	meshOut.dynamicData = dataBuffer;
	meshOut.dynamicDataSize = totalDynamicMemorySize;

	//Not freeing dataBuffer, the mesh now owns it.

	fclose(file);

	return true;
}

bool bsLoadSerializedMeshFromMemory(const char* data, unsigned int dataSize,
	bsSerializedMesh& meshOut)
{
	char header[16];
	memcpy(header, data, 16);

	//Verify that the header is correct.
	if (memcmp(header, "bsm", 3) != 0)
	{
		bsLog::logMessage("Encountered a bad file header when loading mesh from memory",
			pantheios::SEV_ERROR);

		return false;
	}
	if (header[3] != kSerializerVersion)
	{
		bsLog::logMessage("Tried to load a mesh which was created with an old "
			"version of the mesh serializer.", pantheios::SEV_ERROR);

		return false;
	}

	unsigned int bufferCount;
	memcpy(&bufferCount, header + 4, sizeof(unsigned int));

	unsigned int totalDynamicMemorySize;
	memcpy(&totalDynamicMemorySize, header + 8, sizeof(unsigned int));

	//Size of dynamic memory + file header + AABB extents must be same as file size.
	assert(totalDynamicMemorySize + 16 + 24 == dataSize);
	if (totalDynamicMemorySize + 16 + 24 != dataSize)
	{
		bsLog::logMessage("Memory size mismatch when trying to load a mesh from memory",
			pantheios::SEV_ERROR);

		return false;
	}

	//Allocate enough memory to hold all the dynamic data used by the mesh.
	//Does not include the min/max extents to not waste 24 bytes of memory.
	char* const dataBuffer = static_cast<char*>(malloc(totalDynamicMemorySize));
	memcpy(dataBuffer, data + 16, totalDynamicMemorySize);
	char* head = dataBuffer;


	//Assign the vertex/index buffer pointers. This is all that is required to load the
	//dynamic memory because of the data layout used in the file.
	bsSerializedMesh meshToLoad;
	meshToLoad.bufferCount = bufferCount;
	meshToLoad.vertexBuffers = reinterpret_cast<bsVertexBuffer*>(head);
	head += sizeof(bsVertexBuffer) * bufferCount;
	assert(head < dataBuffer + totalDynamicMemorySize);

	meshToLoad.indexBuffers = reinterpret_cast<bsIndexBuffer*>(head);
	head += sizeof(bsIndexBuffer) * bufferCount;
	assert(head < dataBuffer + totalDynamicMemorySize);

	for (size_t i = 0; i < meshToLoad.bufferCount; ++i)
	{
		//Assign vertices.
		bsVertexBuffer& currentVertexBuffer = meshToLoad.vertexBuffers[i];

		currentVertexBuffer.vertices = reinterpret_cast<bsVertexNormalTex*>(head);
		head += sizeof(bsVertexNormalTex) * currentVertexBuffer.vertexCount;
		assert(head < dataBuffer + totalDynamicMemorySize);

		//Assign indices.
		bsIndexBuffer& currentIndexBuffer = meshToLoad.indexBuffers[i];

		currentIndexBuffer.indices = reinterpret_cast<unsigned int*>(head);
		head += sizeof(unsigned int) * currentIndexBuffer.indexCount;
	}
	assert(head == dataBuffer + totalDynamicMemorySize);

	//Read min/max extents into a buffer.
	char minMaxExtents[sizeof(XMFLOAT3) * 2];
	//Offset for start of extents is header + dynamic memory size.
	memcpy(minMaxExtents, data + totalDynamicMemorySize + 16, sizeof(XMFLOAT3) * 2);

	//Copy min/max extents.
	memcpy(&meshToLoad.minExtents, minMaxExtents, sizeof(XMFLOAT3));
	memcpy(&meshToLoad.maxExtents, minMaxExtents + sizeof(XMFLOAT3), sizeof(XMFLOAT3));

	meshOut = std::move(meshToLoad);

	meshOut.dynamicData = dataBuffer;
	meshOut.dynamicDataSize = totalDynamicMemorySize;

	//Not freeing dataBuffer, the mesh now owns it.

	return true;
}

bool bsSaveSerializedMesh(const std::string& fileName, const bsSerializedMesh& mesh)
{
	assert(!fileName.empty());
	assert(mesh.bufferCount > 0);
	assert(mesh.vertexBuffers[0].vertexCount > 0);
	assert(mesh.indexBuffers[0].indexCount > 0);
	assert(mesh.dynamicDataSize > 0);
	assert(mesh.dynamicData != nullptr);

	//Check if the mesh seems valid.
	if (mesh.bufferCount == 0 || mesh.vertexBuffers[0].vertexCount == 0
		|| mesh.indexBuffers[0].indexCount == 0 || mesh.dynamicDataSize == 0
		|| mesh.dynamicData == nullptr)
	{
		return false;
	}

#pragma warning(push)
#pragma warning (disable : 4996)//warning C4996: 'fopen' was declared deprecated
	FILE* file = fopen(fileName.c_str(), "wb");
#pragma warning(pop)
	assert(file != nullptr);

	//File header.
	char header[16] = {'b', 's', 'm', kSerializerVersion };
	memcpy(header + 4, &mesh.bufferCount, sizeof(mesh.bufferCount));

	const unsigned int bufferMemorySize = mesh.dynamicDataSize;
	memcpy(header + 8, &bufferMemorySize, sizeof(bufferMemorySize));
	//4 unused bytes.
	memset(header + 12, 0xFFFFFFFF, sizeof(unsigned int));

	//Total amount of size needed for the whole file, includes non-dynamic data.
	const unsigned int totalBufferSize = bufferMemorySize + sizeof(mesh.minExtents)
		+ sizeof(mesh.maxExtents) + sizeof(header);
	
	//This represents all the data to write to the file.
	char* const dataBuffer = static_cast<char*>(malloc(totalBufferSize));
	char* head = dataBuffer;//Start of data buffer.

	memcpy(head, header, sizeof(header));
	head += sizeof(header);
	assert(head < dataBuffer + totalBufferSize);

	//Since all the dynamic data of the mesh is contained in a single buffer, copying this
	//buffer is all that is required, don't need to touch vertex/index buffers manually.
	memcpy(head, mesh.dynamicData, bufferMemorySize);
	head += bufferMemorySize;
	assert(head < dataBuffer + totalBufferSize);

	memcpy(head, &mesh.minExtents, sizeof(mesh.minExtents));
	head += sizeof(mesh.minExtents);
	assert(head < dataBuffer + totalBufferSize);

	memcpy(head, &mesh.maxExtents, sizeof(mesh.maxExtents));
	head += sizeof(mesh.maxExtents);
	assert(head == dataBuffer + totalBufferSize);

	//Write the entire buffer to the file.
	size_t write = fwrite(dataBuffer, totalBufferSize, 1, file);
	assert(write == 1);

	fclose(file);

	free(dataBuffer);

	return true;
}

#ifdef BS_SUPPORT_MESH_CREATION

unsigned int getAiMeshIndexCount(const aiMesh& mesh)
{
	const unsigned int numFaces = mesh.mNumFaces;
	unsigned int totalIndexCount = 0;

	//Find total index count.
	for (unsigned int i = 0u; i < numFaces; ++i)
	{
		totalIndexCount += mesh.mFaces[i].mNumIndices;
	}

	return totalIndexCount;
}

unsigned int getAiMeshVertexCount(const aiMesh& mesh)
{
	return mesh.mNumVertices;
}

/*	Calculates and returns the amount of bytes required by a bsSerializedMesh to store
	all the meshes in this scene.
*/
unsigned int getAiMeshBufferMemSize(const aiScene* scene)
{
	const unsigned int meshCount = scene->mNumMeshes;

	if (meshCount == 0)
	{
		return 0;
	}

	unsigned int totalVertexCount = 0;
	unsigned int totalIndexCount = 0;

	for (size_t i = 0; i < meshCount; ++i)
	{
		const aiMesh& currentMesh = *scene->mMeshes[i];
		totalVertexCount += getAiMeshVertexCount(currentMesh);
		totalIndexCount += getAiMeshIndexCount(currentMesh);
	}

	const unsigned int vertexBufferSize = sizeof(bsVertexNormalTex) * totalVertexCount;
	const unsigned int indexBufferSize = sizeof(unsigned int) * totalIndexCount;
	const unsigned int vertexBufferExtraSize = sizeof(bsVertexBuffer) * meshCount;
	const unsigned int indexBufferExtraSize = sizeof(bsIndexBuffer) * meshCount;

	return vertexBufferSize + indexBufferSize + vertexBufferExtraSize + indexBufferExtraSize;
}

bsCreateSerializedMeshFlags bsCreateSerializedMesh(const std::string& fileName,
	bsSerializedMesh& meshOut)
{
	Assimp::Importer importer;
	const unsigned int flags = aiProcess_Triangulate
		| aiProcessPreset_TargetRealtime_MaxQuality
		| aiProcess_ConvertToLeftHanded; //Need this one for D3D.

	const aiScene* scene = importer.ReadFile(fileName.c_str(), flags);
	if (scene == nullptr)
	{
		std::string errorMessage("Error loading scene: ");
		errorMessage.append(importer.GetErrorString());
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		return BS_MESH_GENERIC_ERROR;
	}

	const unsigned int meshCount = scene->mNumMeshes;
	if (meshCount == 0)
	{
		std::string errorMessage("\'");
		errorMessage.append(fileName);
		errorMessage.append("\' contains no meshes");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		return BS_MESH_INVALID_INPUT_MESH;
	}


	const size_t meshMemSize = getAiMeshBufferMemSize(scene);
	//Has enough memory to hold the entire bsSerializedMesh.
	bsLinearHeapAllocator allocator(meshMemSize);

	bsSerializedMesh mesh;
	mesh.vertexBuffers = allocator.allocate<bsVertexBuffer>(meshCount);
	mesh.indexBuffers = allocator.allocate<bsIndexBuffer>(meshCount);

	if (mesh.vertexBuffers == nullptr || mesh.indexBuffers == nullptr)
	{
		return BS_MESH_MEMORY_FAILURE;
	}

	mesh.bufferCount = meshCount;

	//Combined min/max extents of all submeshes.
	XMFLOAT3 minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	
	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		//Min/max extents of this submesh.
		XMFLOAT3 tempMinExtents(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 tempMaxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		const bsCreateSerializedMeshFlags parseFlags = parseData(scene->mMeshes[i],
			mesh.vertexBuffers[i], mesh.indexBuffers[i],
			tempMinExtents, tempMaxExtents, allocator);
		if (parseFlags != BS_MESH_SUCCESSS)
		{
			//Parsing failed, return now rather than generate an invalid mesh.
			return parseFlags;
		}

		minExtents = getMinExtents(minExtents, tempMinExtents);
		maxExtents = getMaxExtents(maxExtents, tempMaxExtents);
	}

	mesh.minExtents = minExtents;
	mesh.maxExtents = maxExtents;

	meshOut = std::move(mesh);

	//Take ownership of the data the mesh uses to prevent it from going out of scope
	//when the allocator does.
	meshOut.dynamicData = allocator.takeOwnershipOfAllocatedMemory();
	meshOut.dynamicDataSize = meshMemSize;

	return BS_MESH_SUCCESSS;
}

/*	Copies all the indices described in the aiMesh into the provided index buffer.
*/
bsCreateSerializedMeshFlags copyIndices(const aiMesh* mesh, bsIndexBuffer& indexBufferOut,
	bsLinearHeapAllocator& allocator)
{
	/*	This function iterates through the indices twice, once to find out how many
		indices there are, making it possible to allocate memory only once, and then
		another time to actually copy the indices.
	*/

	const unsigned int numFaces = mesh->mNumFaces;
	unsigned int totalIndexCount = 0;

	//Find total index count.
	for (unsigned int i = 0u; i < numFaces; ++i)
	{
		totalIndexCount += mesh->mFaces[i].mNumIndices;
	}

	indexBufferOut.indexCount = totalIndexCount;
	//Allocate the memory needed, then loop through the indices again and copy them.
	indexBufferOut.indices = allocator.allocate<unsigned int>(totalIndexCount);
	assert(indexBufferOut.indices != nullptr);
	if (indexBufferOut.indices == nullptr)
	{
		return BS_MESH_MEMORY_FAILURE;
	}

	unsigned int currentIndex = 0;

	//Copy indices.
	for (unsigned int i = 0u; i < numFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];

		memcpy(indexBufferOut.indices + currentIndex, face.mIndices,
			face.mNumIndices * sizeof(unsigned int));
		currentIndex += face.mNumIndices;
	}

	return BS_MESH_SUCCESSS;
}

/*	Copies all the vertices described in the aiMesh into the provided vertex buffer.
	Also calculates the min/max extents of the vertices.
*/
bsCreateSerializedMeshFlags copyVertices(const aiMesh* mesh, bsVertexBuffer& vertexBufferOut,
	bsLinearHeapAllocator& allocator, XMFLOAT3& minExtentsOut, XMFLOAT3& maxExtentsOut)
{
	const bool meshHasNormals = mesh->HasNormals();
	const bool meshHasTexCoords = mesh->HasTextureCoords(0);

	vertexBufferOut.vertexCount = mesh->mNumVertices;
	//Allocate enough space for all the vertices.
	vertexBufferOut.vertices = allocator.allocate<bsVertexNormalTex>(mesh->mNumVertices);
	assert(vertexBufferOut.vertices != nullptr && "Failed to allocate memory while"
		"creating a mesh");
	if (vertexBufferOut.vertices == nullptr)
	{
		return BS_MESH_MEMORY_FAILURE;
	}

	//Copy data from aiMesh into the vertex buffers.
	bsVertexNormalTex vertex;

	for (unsigned int i = 0u; i < mesh->mNumVertices; ++i)
	{
		vertex.position = aiVector3ToXMFloat3(mesh->mVertices[i]);

		//If the aiMesh has normals, use them, otherwise use invalid dummy normals.
		vertex.normal = meshHasNormals ? aiVector3ToXMFloat3(mesh->mNormals[i])
			: XMFLOAT3(0.0f, 0.0f, 0.0f);

		//Same for texture coords.
		vertex.textureCoord = meshHasTexCoords ? aiVector3ToXMFloat2TexCoord
			(mesh->mTextureCoords[0][i]) : XMFLOAT2(0.0f, 0.0f);

		vertexBufferOut.vertices[i] = vertex;

		//Store min/max positions for AABB generation.
		minExtentsOut = getMinExtents(minExtentsOut, vertex.position);
		maxExtentsOut = getMaxExtents(maxExtentsOut, vertex.position);
	}

	return BS_MESH_SUCCESSS;
}

/*	Parameters:
	const aiMesh* mesh:				A mesh describing a single mesh.
	bsVertexBuffer& verticesOut:	A vertex buffer whose data will be created by this function.
	bsIndexBuffer& indicesOut:		An index buffer whose data will be created by this function.
	XMFLOAT3& minExtentsOut:		The minimum extents of the output vertex buffer.
	XMFLOAT3& maxExtentsOut:		The maximum extents of the output vertex buffer.
*/
bsCreateSerializedMeshFlags parseData(const aiMesh* mesh, bsVertexBuffer& verticesOut,
	bsIndexBuffer& indicesOut, XMFLOAT3& minExtentsOut, XMFLOAT3& maxExtentsOut,
	bsLinearHeapAllocator& allocator)
{
	//Min/max extents of the mesh. Initialize to inverse of maximum size,
	//which is completely invalid. Makes it easier to check for errors.
	XMFLOAT3 minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	bsCreateSerializedMeshFlags flags =
		copyVertices(mesh, verticesOut, allocator, minExtents, maxExtents);

	if (flags != BS_MESH_SUCCESSS)
	{
		return flags;
	}
	flags = copyIndices(mesh, indicesOut, allocator);
	if (flags != BS_MESH_SUCCESSS)
	{
		return flags;
	}

	//Comparison is OK here since it's not checking the result of a calculation, but if
	//an assignment has been made after the construction of the variable.
	if ((minExtents.x == FLT_MAX) || (minExtents.y == FLT_MAX)
		|| (minExtents.z == FLT_MAX))
	{
		bsLog::logMessage("Error: Min extents were never updated in a submesh");

		return BS_MESH_INVALID_INPUT_MESH;
	}

	if ((maxExtents.x == -FLT_MAX) || (maxExtents.y == -FLT_MAX)
		|| (maxExtents.z == -FLT_MAX))
	{
		bsLog::logMessage("Error: Max extents were never updated in a submesh");

		return BS_MESH_INVALID_INPUT_MESH;
	}

	minExtentsOut = minExtents;
	maxExtentsOut = maxExtents;

	if (verticesOut.vertexCount == 0 || indicesOut.indexCount == 0)
	{
		bsLog::logMessage("A mesh contains no vertices or indices", pantheios::SEV_ERROR);
		return BS_MESH_INVALID_INPUT_MESH;
	}

	return BS_MESH_SUCCESSS;
}

#endif // BS_SUPPORT_MESH_CREATION
