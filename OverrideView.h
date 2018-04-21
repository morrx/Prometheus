#pragma once
#include "HookIncludes.h"


typedef void(__thiscall *override_view_t)(void* _this, CViewSetup* setup);
typedef float(__thiscall *get_fov_t)(void*);


float __fastcall hkGetViewModelFOV(void* ecx, void* edx)
{
	static auto ofunc = hooks::clientmode.get_original<get_fov_t>(35);
	if (g_Options.Visuals.viewmodelChanger_enabled && g_Options.Visuals.Enabled)
	{

		float viewmodelFOV = ofunc(ecx);
		return g_Options.Visuals.viewmodelChanger;
	}

	return ofunc(ecx);
}


void thridperson()
{
	static bool thridperson = false;

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	static size_t lastTime = 0;

	if (GetAsyncKeyState(g_Options.Visuals.TPKey) && g_Options.Visuals.Enabled)
	{
		if (GetTickCount() > lastTime)
		{
			thridperson = !thridperson;

			if (g_Options.Visuals.offtp)
			{
				thridperson = false;
			}

			lastTime = GetTickCount() + 650;
		}
	}
	g_Input->m_fCameraInThirdPerson =  thridperson && pLocal && pLocal->IsAlive();
}

void __fastcall hkOverrideView(void* _this, void* _edx, CViewSetup* setup)
{
	static auto ofunc = hooks::clientmode.get_original<override_view_t>(18);
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	if (pLocal && g_Engine->IsInGame())
	{
		if (g_Options.Visuals.NoVisualRecoil)
		{
			setup->angles.x -= ((pLocal->localPlayerExclusive()->GetViewPunchAngle().x * 2) * 0.45);
			setup->angles.y -= ((pLocal->localPlayerExclusive()->GetViewPunchAngle().y * 2) * 0.45);
		}
		if (!pLocal->IsScoped())
		{
			if (g_Options.Visuals.FOVChanger_enabled && g_Options.Visuals.Enabled)
			{
				setup->fov += g_Options.Visuals.FOVChanger;
			}
		}
		

		thridperson();
	
		if (g_Options.Misc.bFreeCam)
		{
			static Vector newOrigin;

			int freecamkey = g_Options.Misc.iFreeCamKey;
			float freecamspeed = g_Options.Misc.flFreeCamSpeed;


			if (!GetAsyncKeyState(freecamkey))
			{
				newOrigin = setup->origin;
			}
			if (GetAsyncKeyState(freecamkey))
			{

				unsigned int fSpeed = freecamspeed; //5.f;
				if (GetAsyncKeyState(VK_CONTROL))
					fSpeed = fSpeed * 0.45;
				if (GetAsyncKeyState(VK_SHIFT))
					fSpeed = fSpeed * 1.65;
				if (GetAsyncKeyState(0x57)) // W		
				{
					newOrigin += setup->angles.Forward() * fSpeed;
				}
				if (GetAsyncKeyState(0x41)) // A		
				{
					newOrigin += setup->angles.Right() * fSpeed;
				}
				if (GetAsyncKeyState(0x44)) // D		
				{
					newOrigin -= setup->angles.Right() * fSpeed;
				}
				if (GetAsyncKeyState(0x53)) // S		
				{
					newOrigin -= setup->angles.Forward() * fSpeed;

				}
				if (GetAsyncKeyState(VK_SPACE)) // Space		
				{
					newOrigin += setup->angles.Up() * fSpeed;
				}
				setup->origin = newOrigin;
			}
		}
	}
	grenade_prediction::instance().View(setup);
	ofunc(_this, setup);
}


