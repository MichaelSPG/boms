#ifndef BS_MESH_SERIALIZER_H
#define BS_MESH_SERIALIZER_H

#include "bsConfig.h"

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


/*	Structure containing a serialized mesh' vertices and indices,
	as well as min and max extents used for AABB creation.
	
	The structure uses boost's serialization functionality to quickly load and save the
	contents to and from files.
*/
struct bsSerializedMesh
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

	//Vertex and index buffers for each mesh.
	std::vector<std::vector<VertexNormalTex>>	vertices;
	std::vector<std::vector<unsigned int>>		indices;

	//For AABB.
	std::vector<XMFLOAT3>	minExtents;
	std::vector<XMFLOAT3>	maxExtents;
};

//Functions for serialization of structures used by the serialized mesh structure.
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

}// namespace serialization
}// namespace boost

/*	This mesh serializer uses boost's serialization functionality,
	specifically the binary archive for better load times.
	
	This version supports loading only.
*/
class bsMeshSerializer
{
	friend class bsMeshManager;

	/*	Loads a file and puts its contents in the mesh parameter.
		Returns true on succes.
	*/
	bool load(const std::string& meshName, bsSerializedMesh& mesh);
};

#endif // BS_MESH_SERIALIZER_H
