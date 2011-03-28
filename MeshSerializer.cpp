#include "MeshSerializer.h"

#include <fstream>
#include <sstream>

#include "Log.h"


bool MeshSerializer::load(const std::string& meshName, SerializedMesh& mesh)
{
	std::ifstream inStream(meshName, std::ios::binary);
	try
	{
		boost::archive::binary_iarchive inArchive(inStream);
		inArchive >> mesh;
	}
	catch (const std::exception& e)
	{
		std::string message("MeshSerializer::load: An exception occured while loading '");
		message += meshName + "'. Message: " + e.what() + "\nMaybe you tried to load an "
			"unsupported file (only .bsm is supported)";
		Log::logMessage(message.c_str(), pantheios::SEV_ERROR);

		return false;
	}
	return true;
}
