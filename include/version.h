#ifndef VERSION_H
#define VERSION_H

#define OS_NAME "OpenWare"
#define OS_CODENAME "Genesis"

#define OS_VERSION_MAJOR 0
#define OS_VERSION_MINOR 1
#define OS_VERSION_PATCH 1

// Build number - increment manually or via build script
#define OS_BUILD 1001 
#define OS_BUILD_DATE __DATE__
#define OS_BUILD_TIME __TIME__

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define OS_VERSION_STRING STR(OS_VERSION_MAJOR) "." STR(OS_VERSION_MINOR) "." STR(OS_VERSION_PATCH)
#define OS_FULL_VERSION_STRING OS_NAME " v" OS_VERSION_STRING " (" OS_CODENAME ") Build " STR(OS_BUILD)

#endif // VERSION_H
