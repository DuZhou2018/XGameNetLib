#ifndef OMP_LIB_NAME
#  error "Macro OMP_LIB_NAME not set (internal error)"
#endif

// check ths supported complilers

// set OMP_LIB_RTL_OPT for c/cpp run-time library
#if defined(_MT) && defined(_DLL)
#  define OMP_LIB_RTL_OPT "dynamic-rtl"
#else
#	if defined(_MT)
#		define OMP_LIB_RTL_OPT "static-rtl"
#	else
#		define OMP_LIB_RTL_OPT "static-rtl"
#	endif
#endif

// set OMP_LIB_DEBUG_OPT
#ifdef _DEBUG
#  define OMP_LIB_DEBUG_OPT "debug"
#else
#  define OMP_LIB_DEBUG_OPT "release"
#endif

//
// now include the lib:
//
#if defined(OMP_LIB_NAME) \
      && defined(OMP_LIB_RTL_OPT) \
      && defined(OMP_LIB_DEBUG_OPT)
#else
#  error "some required macros where not defined (internal logic error)."
#endif

#if defined _MSC_VER	
#	 pragma comment(lib, OMP_LIB_NAME "_" OMP_LIB_RTL_OPT "_" OMP_LIB_DEBUG_OPT ".lib")
#elif defined __GNUC__
//#	import <lzo2_static-rtl_release.lib>
#else
#	 error "Unknown compiler, only support for msvc and gcc."
#endif

#  ifdef OMP_LIB_DIAGNOSTIC
#	pragma message ("Linking to lib file: " OMP_LIB_NAME "_" OMP_LIB_RTL_OPT "_" OMP_LIB_DEBUG_OPT ".lib")
#endif

//
// finally undef any macros we may have set:
//
#if defined(OMP_LIB_NAME)
#  undef OMP_LIB_NAME
#endif
#if defined(OMP_LIB_RTL_OPT)
#  undef OMP_LIB_RTL_OPT
#endif
#if defined(OMP_LIB_DEBUG_OPT)
#  undef OMP_LIB_DEBUG_OPT
#endif
