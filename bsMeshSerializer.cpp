#include "bsMeshSerializer.h"

#include <fstream>

#include "bsLog.h"


bool bsLoadSerializedMesh(const std::string& meshName, bsSerializedMesh& meshOut)
{
	std::ifstream inStream(meshName, std::ios::binary);
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
		message += meshName + "'. Message: " + e.what() + "\nMaybe you tried to load an "
			"unsupported file (only .bsm is supported)";
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		inStream.close();

		return false;
	}
	inStream.close();

	return true;
}
