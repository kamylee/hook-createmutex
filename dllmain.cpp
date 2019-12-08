// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <tchar.h>
#include <STDIO.H>
#include <TCHAR.H>

FARPROC	addrOpenMutexW = NULL;
FARPROC addrCreateMutexW = NULL;

void hookAddr(DWORD dest, DWORD src);
bool __stdcall IsName(wchar_t* name);
void OpenMutexWHook();
void CreateMutexWHook();
void CLoaseMutexHandle();

void hookAddr(DWORD dest, DWORD src)
{
	DWORD oldProcect = 0;
	VirtualProtect((LPVOID)dest, 5, PAGE_READWRITE, &oldProcect);
	*(BYTE*)dest = 0xE9;
	*(DWORD*)(dest + 1) = src - dest - 0x5;
	VirtualProtect((LPVOID)dest, 5, oldProcect, &oldProcect);
}

__declspec(naked) void OpenMutexWHook()
{
	__asm
	{
		pushad
		push [esp+0x20+0xC]
		call IsName
		test eax,eax
		je PASS
		mov dword ptr ss:[esp+0x20+0xC],0
	PASS:
		popad
		push ebp
		mov ebp,esp
		push addrOpenMutexW
		add [esp],5
		retn
	}
}

__declspec(naked) void CreateMutexWHook()
{
	__asm
	{
		pushad
		push[esp + 0x20 + 0xC]
		call IsName
		test eax, eax
		je PASS
		popad
		retn 0xC
	PASS :
		popad
		push ebp
		mov ebp, esp
		push addrOpenMutexW
		add[esp], 5
		retn
	}
}

bool __stdcall IsName(wchar_t* name)
{
	return wcscmp(name, L"_WeChat_App_Instance_Identity_Mutex_Name") == 0 ? true : false;
}



void InitWindow(HMODULE hModule);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitWindow, hModule, 0, NULL);

		HMODULE module = GetModuleHandleA("KERNELBASE.dll");
		addrOpenMutexW = GetProcAddress(module, "OpenMutexW");
		addrCreateMutexW = GetProcAddress(module, "CreateMutexW");

		//hookAddr((DWORD)addrOpenMutexW, (DWORD)OpenMutexWHook);
		MessageBox(NULL, L"start hook createmutex", L"tiop", 0);
		hookAddr((DWORD)addrCreateMutexW, (DWORD)CreateMutexWHook);

		break;
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//获取模块基址
DWORD getModuleAddress()
{
	return (DWORD)LoadLibrary(L"WeChatWin.dll");
}

void InitWindow(HMODULE hModule)
{
	DWORD dwWeChatWinAddr = 0;
	DWORD dwWeChatWinAddr1 = 0;
	while (true)
	{
		dwWeChatWinAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll");
		dwWeChatWinAddr1 = /*(DWORD)LoadLibrary(L"WeChatWin.dll")*/getModuleAddress();
		/*wchar_t info[100] = { 0 };
		swprintf_s(info, L"WeChatWinAddr 1: %02X  WeChatWinAddr 2: %02X", dwWeChatWinAddr, dwWeChatWinAddr1);
		MessageBox(NULL, info, L"test1", MB_OK);*/
		if (dwWeChatWinAddr )
		{
			break;
		}
		if ( dwWeChatWinAddr1)
		{
			dwWeChatWinAddr = dwWeChatWinAddr1;
			break;
		}
		Sleep(100);
	}
	//MessageBox(NULL, L"in dll main 2", L"test1", MB_OK);
	DWORD mutexAddre = dwWeChatWinAddr + 0x11F2F18;		

	wchar_t info[100] = { 0 };
	swprintf_s(info, L"mutex str:%02X  content:%s", mutexAddre, (wchar_t*)mutexAddre);
	MessageBox(NULL, info, L"test1", MB_OK);

		
	wchar_t strMutexInWx[42] = { 0 };
	//swprintf_s(strMutexInWx, L"_WeChat_App_%d", GetTickCount());
	swprintf_s(strMutexInWx, L"_WeChat_App_Instance_Identity_Mutex_1111"); 
	//修改代码段属性
	DWORD dwOldAttr = 0;
	VirtualProtect((LPVOID)mutexAddre, sizeof(strMutexInWx), PAGE_EXECUTE_READWRITE, &dwOldAttr);
	//VirtualProtect((LPVOID)mutexAddre, 40, PAGE_EXECUTE_READWRITE, &dwOldAttr);
	//Patch
	memcpy((LPVOID)mutexAddre, strMutexInWx, sizeof(strMutexInWx));
	//memcpy((LPVOID)mutexAddre, strMutexInWx, 40);
	//恢复属性
	VirtualProtect((LPVOID)mutexAddre, 5, dwOldAttr, &dwOldAttr);

	memset(info, 0, 100);
	swprintf_s(info, L"mutex str modified:%02X  content:%s", mutexAddre, (wchar_t*)mutexAddre);
	MessageBox(NULL, info, L"test1", MB_OK);
}