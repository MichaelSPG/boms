#include "bsMeshSerializer.h"

#include <fstream>
#include <sstream>
#include <float.h>

#ifdef BS_SUPPORT_MESH_CREATION
#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>


bool parseData(const aiMesh* mesh, std::vector<VertexNormalTex>& verticesOut,
	std::vector<unsigned int>& indicesOut, XMFLOAT3& minExtentsOut, XMFLOAT3& maxExtentsOut);
#endif // BS_SUPPORT_MESH_CREATION

#include "bsLog.h"


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
#ifdef BS_SUPPORT_MESH_CREATION
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

unsigned int calculateTotalVertexCount(const bsSerializedMesh& mesh)
{
	unsigned int sum = 0;
	std::for_each(mesh.vertices.cbegin(), mesh.vertices.cend(),
		[&](const std::vector<VertexNormalTex>& vertices)
	{
		sum += vertices.size();
	});

	return sum;
}

inline bool approximatelyEqual(float number1, float number2, float epsilon = 0.00001f)
{
	return (fabsf(number1 - number2) < epsilon);
}

//Compare two vectors with an epsilon, returning true if they are approximately equal
inline bool approximatelyEqual(const XMFLOAT3& vec1, const XMFLOAT3& vec2, float epsilon = 0.00001f)
{
	return approximatelyEqual(vec1.x, vec2.x, epsilon)
		&& approximatelyEqual(vec1.y, vec2.y, epsilon)
		&& approximatelyEqual(vec1.z, vec2.z, epsilon);
}


bool bsLoadSerializedMesh(const std::string& fileName, bsSerializedMesh& meshOut)
{
	std::ifstream inStream(fileName, std::ios::binary);
	try
	{
		//Load it
		boost::archive::binary_iarchive inArchive(inStream);
		inArchive >> meshOut;
	}
	catch (const std::exception& e)
	{
		//Uh oh, probably old version or something.
		std::string message("bsMeshSerializer::load: An exception occured while loading '");
		message += fileName + "'. Message: " + e.what() + "\nMaybe you tried to load an "
			"unsupported file (only .bsm is supported)";
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		inStream.close();

		return false;
	}
	inStream.close();

	return true;
}

bool bsSaveSerializedMesh(const std::string& fileName, const bsSerializedMesh& mesh)
{
	std::ofstream outStream(fileName, std::ios::binary);
	if (!outStream.is_open())
	{
		std::string errorMessage("Failed to open \'");
		errorMessage.append(fileName);
		errorMessage.append("\' for writing, aborting saving");
		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		return false;
	}

	try
	{
		std::stringstream ss;
		ss << "Saving mesh with " << mesh.vertices.size() << " submesh(es) to \'"
			<< fileName << "\'";
		bsLog::logMessage(ss.str().c_str(), pantheios::SEV_INFORMATIONAL);

		boost::archive::binary_oarchive outArchive(outStream);
		outArchive << mesh;

		outStream.close();
	}
	catch (const std::exception& e)
	{
		outStream.close();

		std::string errorMessage("An error occured while saving the mesh to \'");
		errorMessage.append(fileName);
		errorMessage.append("\', aborting. ");
		errorMessage.append(e.what());

		bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

		return false;
	}

	std::string message("Successfully saved the mesh to \'");
	message.append(fileName);
	message.append("\'");
	bsLog::logMessage(message.c_str(), pantheios::SEV_INFORMATIONAL);

	return true;
}

#ifdef BS_SUPPORT_MESH_CREATION
bool bsSerializeMesh(const std::string& fileName, bsSerializedMesh& meshOut)
{
	Assimp::Importer importer;
	const unsigned int flags = aiProcess_Triangulate
		| aiProcessPreset_TargetRealtime_MaxQuality
		| aiProcess_ConvertToLeftHanded; //Need this one for D3D.

	const aiScene* scene = importer.ReadFile(fileName.c_str(), flags);
	if (scene == nullptr)
	{
		std::cout << "Error loading scene: " << importer.GetErrorString() << "\n";

		return false;
	}

	const unsigned int meshCount = scene->mNumMeshes;
	if (meshCount == 0)
	{
		std::cout << '\'' << fileName << "' contains no meshes";

		return false;
	}

	bsSerializedMesh mesh;
	mesh.vertices.resize(meshCount);
	mesh.indices.resize(meshCount);

	XMFLOAT3 minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	
	for (unsigned int i = 0u; i < meshCount; ++i)
	{
		XMFLOAT3 tempMinExtents(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 tempMaxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		if (!parseData(scene->mMeshes[i], mesh.vertices[i], mesh.indices[i],
			tempMinExtents, tempMaxExtents))
		{
			//Parsing failed, return now rather than generate an invalid mesh.
			return false;
		}

		minExtents = getMinExtents(minExtents, tempMinExtents);
		maxExtents = getMaxExtents(maxExtents, tempMaxExtents);
	}

	mesh.minExtents = minExtents;
	mesh.maxExtents = maxExtents;

	meshOut = mesh;

	return true;
}

bool parseData(const aiMesh* mesh, std::vector<VertexNormalTex>& verticesOut,
	std::vector<unsigned int>& indicesOut, XMFLOAT3& minExtentsOut, XMFLOAT3& maxExtentsOut)
{
	const unsigned int numFaces = mesh->mNumFaces;
	for (unsigned int i = 0u; i < numFaces; ++i)
	{
		const aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indicesOut.push_back(face.mIndices[j]);
		}
	}

	const bool meshHasNormals = mesh->HasNormals();
	const bool meshHasTexCoords = mesh->HasTextureCoords(0);

	//Min/max extents of the mesh
	XMFLOAT3 minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	verticesOut.reserve(mesh->mNumVertices);

	//Copy data from aiMesh into the vertex/index buffers.
	for (unsigned int i = 0u; i < mesh->mNumVertices; ++i)
	{
		VertexNormalTex vertex;
		vertex.position = aiVector3ToXMFloat3(mesh->mVertices[i]);

		//If the aiMesh has normals, use them, otherwise use invalid dummy normals.
		vertex.normal = meshHasNormals ? aiVector3ToXMFloat3(mesh->mNormals[i])
			: XMFLOAT3(0.0f, 0.0f, 0.0f);
		
		//Same for texture coords.
		vertex.textureCoord = meshHasTexCoords ? aiVector3ToXMFloat2TexCoord
			(mesh->mTextureCoords[0][i]) : XMFLOAT2(0.0f, 0.0f);

		verticesOut.push_back(vertex);

		//Store min/max positions for AABB generation.
		minExtents = getMinExtents(minExtents, vertex.position);
		maxExtents = getMaxExtents(maxExtents, vertex.position);
	}

	//Make sure something has been loaded.
	if (verticesOut.size() == 0 || verticesOut.size())
	{
		std::cout << "No vertices were loaded in a submesh\n";
	}

	//Comparison is OK here since it's not checking the result of a calculation, but if
	//an assignment has been made after the construction of the variable.
	if ((minExtents.x == FLT_MAX) || (minExtents.y == FLT_MAX)
		|| (minExtents.z == FLT_MAX))
	{
		std::cout << "Warning: Min extents were never updated in a submesh\n";

		return false;
	}

	if ((maxExtents.x == -FLT_MAX) || (maxExtents.y == -FLT_MAX)
		|| (maxExtents.z == -FLT_MAX))
	{
		std::cout << "Warning: Max extents were never updated in a submesh\n";

		return false;
	}

	if (verticesOut.size() == 0 || indicesOut.size() == 0)
	{
		std::cout << "Warning: A mesh contains no vertices or indices\n";
	}

	minExtentsOut = minExtents;
	maxExtentsOut = maxExtents;

	return (verticesOut.size() > 0) && (indicesOut.size() > 0);
}
#endif // BS_SUPPORT_MESH_CREATION
