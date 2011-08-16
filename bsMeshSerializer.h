#pragma once


/*	Contains functions for saving and loading of meshes from disk.
	Define BS_SUPPORT_MESH_CREATION to enable creating bsSerializedMesh from
	other mesh formats. Note that this requires assimp to work.
*/

#include <string>
#include <vector>

#include <Windows.h>
#include <xnamath.h>

#pragma warning(push)//boost header warnings
#pragma warning(disable:4244)//'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4310)//cast truncates constant value
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#pragma warning(pop)

#include "bsVertexTypes.h"


#ifdef BS_SUPPORT_MESH_CREATION
struct aiMesh;
#endif //BS_SUPPORT_MESH_CREATION


/*	Structure containing a serialized mesh' vertices and indices,
	as well as min and max extents used for AABB creation.
	
	The structure uses boost's binary serialization functionality to quickly load and
	save the contents from and to files.
*/
class bsSerializedMesh
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & vertices;
		ar & indices;
		ar & minExtents;
		ar & maxExtents;
	}

public:
	bsSerializedMesh()
		: minExtents(XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX))
		, maxExtents(XMFLOAT3(FLT_MIN, FLT_MIN, FLT_MIN))
	{}


	//Vertex and index buffers for each submesh.
	std::vector<std::vector<VertexNormalTex>>	vertices;
	std::vector<std::vector<unsigned int>>		indices;
	
	//For AABB.
	XMFLOAT3	minExtents;
	XMFLOAT3	maxExtents;
};

namespace boost
{
namespace serialization
{
template<class Archive>
void serialize(Archive& ar, VertexNormalTex& vnt, const unsigned int)
{
	ar & vnt.position;
	ar & vnt.normal;
	ar & vnt.textureCoord;
}

template<class Archive>
void serialize(Archive& ar, XMFLOAT3& xmf3, const unsigned int)
{
	ar & xmf3.x;
	ar & xmf3.y;
	ar & xmf3.z;
}

template<class Archive>
void serialize(Archive& ar, XMFLOAT2& xmf2, const unsigned int)
{
	ar & xmf2.x;
	ar & xmf2.y;
}

}//namespace serialization
}//namespace boost


/*	Loads a serialized mesh from disk into the output parameter.
	Returns true on successful load.
*/
bool bsLoadSerializedMesh(const std::string& fileName, bsSerializedMesh& meshOut);

/*	Save the serialized mesh to disk with the given filename.
	Returns true on successful save.
*/
bool bsSaveSerializedMesh(const std::string& fileName, const bsSerializedMesh& mesh);

#ifdef BS_SUPPORT_MESH_CREATION
/*	Loads a mesh from disk and converts it to the bsSerializedMesh format.
	Returns true on success.
*/
bool bsSerializeMesh(const std::string& fileName, bsSerializedMesh& meshOut);
#endif // BS_SUPPORT_MESH_CREATION
