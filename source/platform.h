

#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#define PLATFORM_GL14
#define PLATFORM_WIN
#endif

#if __APPLE__

#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define PLATFORM_GL14
#define PLATFORM_MAC
#endif
#if TARGET_OS_IPHONE
#define PLATFORM_IOS
#define PLATFORM_IPHONE
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif
#if TARGET_OS_IPAD
#define PLATFORM_IOS
#define PLATFORM_IPAD
#define PLATFORM_MOBILE
#define PLATFORM_GLES20
#undef PLATFORM_GL14
#endif

#endif

#if defined( __GNUC__ )
//#define PLATFORM_LINUX
#endif
#if defined( __linux__ )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif
#if defined ( __linux )
#define PLATFORM_LINUX
#define PLATFORM_GL14
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#ifdef PLATFORM_WIN
#include <initguid.h>
#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <ws2bth.h>
#include <windows.h>
//#include <mmsystem.h>
//#include <commdlg.h>
//#include <dirent.h>
#include "../../../libs/win/dirent-1.20.1/include/dirent.h"
#endif

#ifdef PLATFORM_LINUX
/* POSIX! getpid(), readlink() */
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
//file listing dirent
#include <dirent.h>
//htonl
#include <arpa/inet.h>
#include <sys/time.h>
//bt
#include <stdlib.h>
#include <sys/socket.h>
#include <bluetooth.h>
#include <hci.h>
#include <hci_lib.h>
#include <rfcomm.h>
#include <l2cap.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
#include <sys/types.h>
#include <sys/dir.h>
//htonl
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#if defined(PLATFORM_IOS)
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>	//mkdir
#endif

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>
//#include <unordered_multimap>
#include <fstream>
#include <iostream>
#include <math.h>
#ifndef PLATFORM_WIN
//already included in sdl2
#include <stdint.h>
#endif
#include <limits.h>
//#include <unordered_map>

#ifdef PLATFORM_WIN
#include <jpeglib.h>
#include <png.h>
//#include <zip.h>
#endif

#ifdef PLATFORM_LINUX
#include <jpeglib.h>
#include <png.h>
#endif
//#define NO_SDL_GLEXT

#ifdef PLATFORM_MAC

#if 0
// https://trac.macports.org/ticket/42710
#ifndef FALSE            /* in case these macros already exist */
#define FALSE   0        /* values of boolean */
#endif
#ifndef TRUE
#define TRUE    1
#endif
#define HAVE_BOOLEAN

#endif

#ifdef PLATFORM_IOS
/*
 Use User Header Search Paths !
(Or else jpeglib.h from system folders will be used, version mismatch)
*/
#include "jpeglib.h"
#include "png.h"

#else

#include <jpeglib.h>
#include <png.h>
//#include <zip.h>
#endif

#endif

#ifndef MATCHMAKER
#ifdef PLATFORM_WIN
#include <GL/glew.h>
#endif
#endif

#ifndef MATCHMAKER
#ifdef PLATFORM_LINUX
#define GL_GLEXT_PROTOTYPES
//#include <GL/xglew.h>
#include <GL/glew.h>
#endif
#endif

#if defined(PLATFORM_LINUX) && !defined(PLATFORM_MAC)
//#include <GL/gl.h>
//#include <GL/glu.h>
#endif

//#define GL_GLEXT_PROTOTYPES

#if 1

#if defined(PLATFORM_LINUX) && !defined(PLATFORM_MAC)
#include <SDL2/SDL.h>
#ifndef MATCHMAKER
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_mixer.h>
//#include <GL/glut.h>
//#include <SDL_ttf.h>
#endif
//#include <SDL2/SDL_net.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
#ifndef MATCHMAKER
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
#endif

#ifdef PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#endif

#if defined(PLATFORM_MAC) && !defined(PLATFORM_IOS)
//#include <GL/xglew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
//#include <SDL2/SDL_net.h>
//#include <SDL2/SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#endif

#if defined(PLATFORM_IOS)
//#include <GL/xglew.h>
#include "SDL.h"
#include "SDL_opengles2.h"
//#include <SDL2/SDL_net.h>
//#include <SDL2/SDL_mixer.h>
#include "SDL_net.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#import <CoreMotion/CoreMotion.h>
#endif

#ifdef PLATFORM_WIN
#include <GL/wglew.h>
#include <SDL.h>
#ifndef MATCHMAKER
#include <SDL_opengl.h>
//#include <SDL_mixer.h>
//#include <SDL_ttf.h>
#endif
//#include <SDL_net.h>
#endif

#endif

#if defined(PLATFORM_LINUX) && !defined(PLATFORM_MAC)
//#include <GL/xglew.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_mixer.h>
//#include <SDL_net.h>
//#include <SDL_mixer.h>
//#include <SDL_ttf.h>
#endif

#ifdef PLATFORM_WIN
#ifndef MATCHMAKER
#include <gl/glaux.h>
#endif
#endif

#if 0
#ifdef PLATFORM_WIN
#include "drdump/CrashRpt.h"

extern crash_rpt::CrashRpt g_crashRpt;
#endif
#endif

#ifdef PLATFORM_WIN
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_mixer.lib")
#pragma comment(lib, "SDL2_net.lib")
//#pragma comment(lib, "SDL.lib")
//#pragma comment(lib, "SDLmain.lib")
//#pragma comment(lib, "lib32/assimp.lib")
//#pragma comment(lib, "assimp_release-dll_win32/assimp.lib")
//#pragma comment(lib, "assimp.lib")
//#pragma comment(lib, "assimp-vc110-mt.lib")
#pragma comment(lib, "assimp-vc90-mt.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "libpng15.lib")
//#pragma comment(lib, "zlib.lib")
//#pragma comment(lib, "zipstatic.lib")
#endif

#ifndef BW_MAX_PATH
#define BW_MAX_PATH 300
#endif

#ifndef PLATFORM_WIN
#define SOCKET int32_t
typedef unsigned char byte;
typedef uint32_t UINT;
typedef int16_t WORD;
#define _isnan(x) isnan(x)
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define ERROR 0
#define APIENTRY
#endif

#ifdef PLATFORM_WIN
#define _isnan(x) (x!=x)
#endif

#ifdef PLATFORM_MAC
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif

#ifdef PLATFORM_WIN
#define stricmp _stricmp
#endif

/*
 #ifndef _isnan
 int32_t _isnan(double x) { return x != x; }
 #endif
 */

//#ifndef MATCHMAKER
extern SDL_Window *g_window;
extern SDL_Renderer* g_renderer;
extern SDL_GLContext g_glcontext;
//#endif

//#define GLDEBUG
//#define DEBUGLOG

#define CHECKGLERROR() CheckGLError(__FILE__,__LINE__)

#ifndef GLDEBUG
#define CheckGLError(a,b); (void)0;
#endif

#ifdef PLATFORM_IOS
#include "SDL_syswm.h"
#import "AdToAppSDK.h"
#import "AdToAppView.h"
#import "ATALog.h"

#include "../../../../copy/aaa004_iso/libs/ios/SDL-2.0.4-9901/src/video/uikit/SDL_uikitopenglview.h"
#include "../../../../copy/aaa004_iso/libs/ios/SDL-2.0.4-9901/src/video/uikit/SDL_uikitopengles.h"
#include "../../../../copy/aaa004_iso_libs/ios/SDL-2.0.4-9901/src/video/uikit/SDL_uikitviewcontroller.h"
#endif

#endif // #define LIBRARY_H
