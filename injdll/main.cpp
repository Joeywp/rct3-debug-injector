#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>

//#include "detours.h"

//#pragma comment(lib, "detours.lib")

HINSTANCE hins;
#pragma data_seg(".shared")
HHOOK hkb = NULL;
BOOL control_down = false;
BOOL shift_down = false;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

#define MONEY_DELTA 0x0061CBF0
#define STALL_ROW_COLUMN 0x00765790

/*
Soaked:
RenderSpline: 0x00537B20
SplineType: 

Wild:
RenderSpline: 0x0059A170
SplineType: 0x0059A190

*/

unsigned long GetTargetThreadIdFromWindow(char *className, char *windowName)
{
	HWND targetWnd;
	//HANDLE hProcess;
	unsigned long processID = 0;

	targetWnd = FindWindowA(className, windowName);
	return GetWindowThreadProcessId(targetWnd, &processID);
} 

namespace RCT3	
{
	namespace SaveManager
	{
		namespace Park
		{
			void __stdcall Save()
			{
				typedef void (__stdcall *pFunctionAddress)();
				pFunctionAddress funcProto = (pFunctionAddress)(0x0062BA60);
				funcProto();
			}
		}
	}
	namespace GUI
	{
		namespace Windows
		{
			void __stdcall Open(char* name)
			{
				typedef void (__stdcall *pFunctionAddress)(char*);
				pFunctionAddress funcProto = (pFunctionAddress)(0x00BCF850);
				funcProto(name);
			}

			void __stdcall TestPopup()
			{
				typedef void* (__stdcall *pFunctionAddress)(wchar_t*);
				pFunctionAddress funcProto = (pFunctionAddress)(0x0068BE70);
				funcProto(L"AAAATest.dat");

				/*typedef void (__stdcall *pFunctionAddress)(char*, void*, int);
				pFunctionAddress funcProto = (pFunctionAddress)(0x004040F0);
				funcProto((char*)0x00F39344, (void*)0x01303CFC, 1);*/

				/*__asm {
					PUSH 1
					PUSH 0x01303CFC 
					PUSH 0x00F39344  
					LEA ECX,[ESP+38]
					CALL 0x004040F0 
				};*/

				/*CPU Disasm
Address   Hex dump          Command                                  Comments
0062B152  |.  6A 01         PUSH 1                                   ; Arg3 = 1
0062B154  |.  68 FC3C3001   PUSH 01303CFC                            ; Arg2 = RCT3plus.1303CFC
0062B159      68 4493F300   PUSH 00F39344                            ; ASCII "RideOpModeDialog_LowFriction_TT"
0062B15E  |.  8D4C24 20     LEA ECX,[LOCAL.9]
0062B162  |.  E8 898FDDFF   CALL 004040F0
0062B167  |.  8B70 08       MOV ESI,DWORD PTR DS:[EAX+8]
0062B16A  |.  85F6          TEST ESI,ESI
0062B16C  |.  C64424 38 01  MOV BYTE PTR SS:[LOCAL.0],1*/

			}
		}
	}
	namespace Debug
	{
		namespace Weather
		{
			void __stdcall NextType()
			{
				typedef void (__stdcall *pFunctionAddress)();
				pFunctionAddress funcProto = (pFunctionAddress)(0x009EA300);
				funcProto();
			}

			void __stdcall Thunder(int a)
			{
				//009EA840
				typedef void (__stdcall *pFunctionAddress)(int a);
				pFunctionAddress funcProto = (pFunctionAddress)(0x009EA840);
				funcProto(a);
			}
		}
		namespace Track
		{
			void __stdcall SwitchRendermode()
			{
				typedef void (__stdcall *pFunctionAddress)();
				pFunctionAddress funcProto = (pFunctionAddress)(0x0059A170);
				funcProto();
			}

			void __stdcall SwitchRenderstate()
			{
				typedef void (__stdcall *pFunctionAddress)();
				pFunctionAddress funcProto = (pFunctionAddress)(0x0059A190);
				funcProto();
			}

			void __stdcall SwitchCategory()
			{
				typedef void (__stdcall *pFunctionAddress)();
				pFunctionAddress funcProto = (pFunctionAddress)(0x0059A1B0);
				funcProto();
			}
		}
	}
}

//Money (in cents), Category (17 = Scenery?, 0 = Rides?), Unknown pointer (always 0)
void (__stdcall *SplineState)();
void SplineState_Hook()
{
	OutputDebugStringA("SplineTest()");
	SplineState();
}

long (__stdcall *TestFunc)(char *a);
long TestFunc_Hook(char *a)
{
	char buffer [50];
	sprintf_s(buffer, "TestFunc(%s)", a);
	OutputDebugStringA(buffer);
	return TestFunc(a);
}

void __stdcall MoneyFunc(int param1, int param2, char*& param3)
{
	typedef void (__stdcall *pFunctionAddress)(int, int, char*&);
	pFunctionAddress pMySecretFunction = (pFunctionAddress)(0x0061CBF0);
	pMySecretFunction(param1, param2, param3);
}

LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(
	int nCode, 
	WPARAM wParam, 
	LPARAM lParam)
{
	/*if ((lParam & (1 << 31)) && (lParam & (1 << 30))) { 
	if(nCode == HC_ACTION)
	{
	if(wParam == VK_F5)
	{
	RCT3::Debug::SwitchSplinerendermode();
	}
	}
	}*/
	//Repeated hooks
	if(nCode == HC_ACTION)
	{
		BOOL up = !(lParam & 0xC0000000);
		if (wParam == VK_CONTROL)
		{
			OutputDebugStringA("Key: Controlstate changed!");
			control_down = up;
		} else if (wParam == VK_SHIFT)
		{
			OutputDebugStringA("Key: Shiftstate changed!");
			shift_down = up;
		}
	}
	if ((lParam & 0x80000000) || (lParam & 0x40000000))
		return CallNextHookEx( hkb, nCode, wParam, lParam );
	//Non-repeated hooks
	if(nCode == HC_ACTION)
	{
		BOOL up = !(lParam & 0xC0000000);
		if(wParam == VK_F2)
		{
			OutputDebugStringA("Key: F2 pressed!");
			if(control_down && !shift_down)
				RCT3::Debug::Track::SwitchRenderstate();
			else if(shift_down && !control_down)
				RCT3::Debug::Track::SwitchCategory();
			else
				RCT3::Debug::Track::SwitchRendermode();
		} else if (wParam == VK_F4)
		{
			RCT3::GUI::Windows::TestPopup();
			//RCT3::Debug::Weather::Thunder(3);
		} else if (wParam == VK_F3)
		{
			OutputDebugStringA("Key: F3 pressed!");
			int *fps_active = (int*)0x1103C64;
			*fps_active = !*fps_active;
		} else if (wParam == VK_F1)
		{
			OutputDebugStringA("Key: F1 pressed!");
			int *wireframe_mode = (int*)0x1103C80;
			*wireframe_mode += 1;
			if(control_down)
				*wireframe_mode = 0;
			if(wireframe_mode[0] > 8)
				*wireframe_mode = 0;
		} else if (wParam == VK_F6)
		{
			OutputDebugStringA("Key: F6 pressed!");
			RCT3::GUI::Windows::Open("SaveTrackPopup");
		} else if (wParam == VK_F7)
		{
			OutputDebugStringA("Key: F7 pressed!");
			RCT3::SaveManager::Park::Save();
		}
	}
	return CallNextHookEx( hkb, nCode, wParam, lParam );
}

void MainLoop()
{
	OutputDebugStringA("Mainloop DLL called");
	unsigned long thid = GetTargetThreadIdFromWindow("Main", "RCT3");
	hkb=SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, hins, thid);
	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{

	}
	OutputDebugStringA("Mainloop DLL ending now");
	return;
}

//BOOL APIENTRY DllMain(HANDLE hModule
BOOL WINAPI DllMain( HINSTANCE hModule,	DWORD ul_reason_for_call, LPVOID lpReserved)
{
	hins = hModule;
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:		
		DisableThreadLibraryCalls(hModule);

		//SplineState = (void (__stdcall*)())DetourFunction((PBYTE)0x0059A170, (PBYTE)SplineState_Hook);

		CreateThread(NULL, 0, (unsigned long(__stdcall*)(void*))MainLoop, NULL, 0, NULL);
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		//DetourRemove((PBYTE)0x0059A170, (PBYTE)SplineState);
		OutputDebugStringA("DLL Detaching..");
		/*MessageBox(
		NULL,
		(LPCWSTR)L"DLL detached :(",
		(LPCWSTR)L"Info",
		MB_ICONWARNING | MB_DEFBUTTON2
		);*/
		UnhookWindowsHookEx(hkb);
		break;
	}
	return TRUE;
}