#ifndef _HQ_BASELIB_UTILITY_H_
#define _HQ_BASELIB_UTILITY_H_

#include <string>
#include <algorithm>
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>

namespace hq{

typedef unsigned long long	uint64;
typedef unsigned int		uint32;
typedef unsigned short		uint16;
typedef unsigned char		uint8;

typedef long long	int64;
typedef int			int32;
typedef short		int16;

#if 0
typedef const char* LPCSTR, * PCSTR;
typedef char* LPSTR, * PSTR;
#endif

#ifdef WIN32
#include <windows.h>
#ifdef OS_UWP
#include "BaseType_WinRT.h"
#endif
#else

#include <unistd.h>
//#include <sys/atomic.h>
#define sprintf_s snprintf
typedef void* PVOID;
typedef int					INT;
typedef int					INT32;
typedef int					INT32_t;
typedef int					BOOL;
typedef int* PBOOL;
typedef int* LPBOOL;
typedef unsigned int		UINT;
typedef unsigned int		UINT32;

#if !(defined OS_IOS)&&!(defined OS_LINUX_QIYI)
//typedef unsigned int		size_t;
#endif
typedef char				CHAR;
typedef short				SHORT;
typedef long				LONG;
typedef unsigned int		DWORD;
typedef unsigned int		ULONG;
typedef long long			INT64;
typedef long long			__int64;
typedef long long			LONGLONG;
typedef unsigned long long	UINT64;
typedef unsigned long long	DDWORD;
typedef unsigned long long	ULONGLONG;
typedef unsigned char		BYTE;
typedef unsigned char		UCHAR;
typedef unsigned short		WORD;
typedef unsigned short		USHORT;
typedef float				FLOAT;
typedef FLOAT* PFLOAT;
typedef BYTE* PBYTE;
typedef BYTE* LPBYTE;
typedef int* PINT;
typedef int* LPINT;
typedef WORD* PWORD;
typedef WORD* LPWORD;
typedef long* LPLONG;
typedef DWORD* PDWORD;
typedef DWORD* LPDWORD;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef void* HANDLE;
typedef void* HMODULE;
typedef wchar_t             WCHAR;
typedef WCHAR* PWCHAR;
typedef WCHAR* LPWCH;
typedef WCHAR* PWCH;
typedef ULONG* PULONG;
typedef void* HRSRC;
typedef const WCHAR* LPCWSTR;

// use for crypt
typedef unsigned int ALG_ID;

typedef unsigned long ULONG_PTR;
typedef unsigned long* PULONG_PTR;

typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTKEY;
typedef ULONG_PTR HCRYPTHASH;

typedef const char* LPCSTR;
typedef void				VOID;
#ifndef _UNICODE
typedef const char* LPCTSTR;
typedef char* LPTSTR;
typedef char                TCHAR;
#else
typedef const wchar_t* LPCTSTR;
typedef wchar_t* LPTSTR;
typedef wchar_t             TCHAR;
#endif	// _UNICODE

#ifdef OS_IOS
typedef HANDLE	            HKEY;
typedef	struct sockaddr_in	SOCKADDR_IN;
typedef	struct sockaddr* PSOCKADDR;
#define HMODULE                 HANDLE
#endif
#endif

#ifdef BASELIB_EXPORTS
#define baselib_declspec __declspec(dllexport)
#else..
#define baselib_declspec __declspec(dllimport)
#endif

#define LOG_WARN(msg)  LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), msg)
#define LOG_DEBUG(msg)  LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), msg)
#define LOG_ERROR(msg)  LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), msg)
#define LOG_FATAL(msg)  LOG4CPLUS_FATAL(log4cplus::Logger::getRoot(), msg)

namespace utility {

baselib_declspec std::string getCurrentDate();
baselib_declspec void logInit(const std::string& log_config);

}
}

#endif ///< _HQ_BASELIB_UTILITY_H_
