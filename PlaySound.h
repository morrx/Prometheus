#pragma once
#include "HookIncludes.h"
typedef void(__thiscall* play_sound_t)(void*, const char*);

void IsReadyaccept()
{
	using IsReadyFn = void(__cdecl*)();
	static auto fnIsReady = (void(__cdecl*)(void))U::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 08 56 8B 35 ? ? ? ? 57 83 BE");

	reinterpret_cast<IsReadyFn>(fnIsReady)();
}

void __stdcall hkPlaySound(const char* szFileName)
{
	static auto ofunc = hooks::surface.get_original<play_sound_t>(82);
	//Call original PlaySound
	ofunc(g_Surface, szFileName);

	if (g_Engine->IsInGame()) return;

	if (g_Options.Misc.AutoAccept)
	{
		if (!strstr(szFileName, "!UI/competitive_accept_beep.wav"))
		{
			IsReadyaccept();

			FLASHWINFO flash;
			flash.cbSize = sizeof(FLASHWINFO);
			flash.hwnd = window;
			flash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			flash.uCount = 0;
			flash.dwTimeout = 0;
			FlashWindowEx(&flash);
		}
	}
}