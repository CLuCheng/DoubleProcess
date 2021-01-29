// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN

namespace agora {
	namespace win32 {
		HMODULE g_hInstDll;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		agora::win32::g_hInstDll = hModule;
		DisableThreadLibraryCalls(hModule);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

