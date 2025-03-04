#include "INiceNet.h"
#include "INiceLog.h"
#include "NiceNetBase64.h"
using namespace peony::net;
//#include <boost/version.hpp>

//for other use
//ifdef USE_PEYONENET_DLL use dll,or use static
#ifndef PEYONENET_DLL_CONFIG
#define PEYONENET_DLL_CONFIG

#ifdef _WIN32
    #ifdef PEYONENET_DLL
        #define PEYONENE_API __declspec(dllexport)
    #else
        #define PEYONENE_API 
    #endif
#endif // _WIN32

    #define LIBNAME "PeonyNet"

    #ifdef USE_PEYONENET_DLL
        #define LIBLINKKIND "dynamic"
    #else
        #define LIBLINKKIND "static"
    #endif // USE_PEYONENET_DLL

    #ifdef _DEBUG
        //#define LIBKIND "debug"
        #if defined(_MT) && defined(_DLL)
        //use MDD
            //#pragma message("exe--debug--MDD")
            #define LIBCCKIND "mdd"
        #else
        //use MTD 
            //#pragma message("exe--debug--MTD")
            #define LIBCCKIND "mtd"
        #endif

    #else
        //#define LIBKIND "release"
        #if defined(_MT) && defined(_DLL)
        //use MDD
            #define LIBCCKIND "md"
        #else
        //use MTD
            #define LIBCCKIND "mt"
        #endif

    #endif // _DEBUG

	#ifdef STLPORT
		#define LIB_STLPORTZZ "stlport"
	#else
		#define LIB_STLPORTZZ "wstl"
	#endif // STLPORT

	#define BOOSTVERZHT ".lib"
	#define LIB_PEYONENET_LINK_NAME    LIBNAME "_" LIBLINKKIND "_" LIBCCKIND "_" LIB_STLPORTZZ BOOSTVERZHT
    
    #if defined _MSC_VER
        #ifndef PEYONENET_DLL
            #pragma comment(lib, LIB_PEYONENET_LINK_NAME)
        #endif
    #elif defined __GNUC__
		   //#pragma message("gun not set linker lib")
    #endif

#undef LIBNAME
#undef LIB_STLPORTZZ
#undef LIBLINKKIND
#undef LIBCCKIND
#undef LIB_PEYONENET_LINK_NAME

#endif //PEYONENET_DLL_CONFIG
