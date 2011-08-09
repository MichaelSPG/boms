#pragma once

/*	Configuration used to change functionality of certain objects, or error checking.
*/


#ifdef _DEBUG
#define BS_DEBUG
#endif // _DEBUG



//Disables all logging to file.
//#define BS_DISABLE_LOGGING

//Disables all callbacks from logging. Has no effect if BS_DISABLE_LOGGING is defined.
//#define BS_DISABLE_LOG_CALLBACKS
