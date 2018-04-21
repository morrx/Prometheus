#include "ESP.h"
#include "Interfaces.h"
#include "Render.h"
#include <ctime>
#include <iostream>
#include <algorithm>
#include "GrenadePrediction.h"
#include "LagComp.h"
#include "Autowall.h"
#include "flashlight.h"


bool PrecacheModel(const char* szModelName)
{
	auto m_pModelPrecacheTable = g_ClientStringTableContainer->FindTable("modelprecache");

	if (m_pModelPrecacheTable)
	{
		g_ModelInfo->FindOrLoadModel(szModelName);
		int idx = m_pModelPrecacheTable->AddString(false, szModelName);
		if (idx == INVALID_STRING_INDEX)
			return false;
	}
	return true;
}



namespace ColorAlpha
{
	VisualsStruct ESP_ctx;
	RECT bbox;
}

visuals::visuals()
{
	BombCarrier = nullptr;
}

int width = 0;
int height = 0;
bool done = false;
float damage;
char bombdamagestringdead[24];
char bombdamagestringalive[24];


void visuals::OnPaintTraverse(C_BaseEntity* local)
{
	WorldModes();
	NightMode();
	DoAsusWalls();
	renderBeams();
	FlashlightRun(local);
	grenade_prediction::instance().Paint();

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	if (g_Options.Visuals.RecoilCrosshair && g_Options.Visuals.Enabled)
	{
		g_Engine->GetScreenSize(width, height);
		if (local && local->IsAlive())
		{
			static Vector ViewAngles;
			g_Engine->GetViewAngles(ViewAngles);
			ViewAngles += local->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

			Vector fowardVec;
			AngleVectors(ViewAngles, &fowardVec);
			fowardVec *= 10000;

			Vector start = local->GetOrigin() + local->GetViewOffset();
			Vector end = start + fowardVec, endScreen;

			if (g_Render->WorldToScreen(end, endScreen) && local->IsAlive())
			{
				g_Render->Line(endScreen.x - 4, endScreen.y, endScreen.x + 4, endScreen.y, Color(int(g_Options.Colors.color_recoil[0] * 255.f), int(g_Options.Colors.color_recoil[1] * 255.f), int(g_Options.Colors.color_recoil[2] * 255.f)));
				g_Render->Line(endScreen.x, endScreen.y - 4, endScreen.x, endScreen.y + 4, Color(int(g_Options.Colors.color_recoil[0] * 255.f), int(g_Options.Colors.color_recoil[1] * 255.f), int(g_Options.Colors.color_recoil[2] * 255.f)));
			}
		}
	}

	if (g_Options.Visuals.SpreadCrosshair && g_Options.Visuals.Enabled)
	{
		g_Engine->GetScreenSize(width, height);
		if (local  && local->IsAlive())
		{
			static Vector ViewAngles;
			g_Engine->GetViewAngles(ViewAngles);
			ViewAngles += local->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

			static Vector fowardVec;
			AngleVectors(ViewAngles, &fowardVec);
			fowardVec *= 10000;

			// Get ray start / end
			Vector start = local->GetOrigin() + local->GetViewOffset();
			Vector end = start + fowardVec, endScreen;

			CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(local->GetActiveWeaponHandle());
			float cone = pWeapon->GetSpread() + pWeapon->GetInaccuracy();
			if (cone > 0.0f)
			{
				if (cone < 0.01f) cone = 0.01f;
				float size = (cone * height) * 0.7f;
				Color color(int(g_Options.Colors.color_spread[0] * 255.f), int(g_Options.Colors.color_spread[1] * 255.f), int(g_Options.Colors.color_spread[2] * 255.f));

				if (g_Render->WorldToScreen(end, endScreen))
				{
					g_Render->OutlineCircle(endScreen.x, endScreen.y, (int)size, 48, color);
				}

			}

		}
	}


	if (g_Options.Misc.Hitmarker)
	{
		Hitmarker();
	}

	if (g_Options.Visuals.angleLines && pLocal->IsAlive() && g_Options.Visuals.Enabled)
	{
		DrawAngles();
	}
	if (g_Options.Visuals.DrawAwall && pLocal->IsAlive() && g_Options.Visuals.Enabled)
	{
		DrawAwall();
	}

	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
	{

		C_BaseEntity *entity = g_EntityList->GetClientEntity(i);
		if (entity == nullptr)
			continue;
		if (entity == local)
			continue;
		if (entity->IsDormant())
			continue;

		player_info_t pinfo;
		Vector pos, pos3D;
		pos3D = entity->GetOrigin();


		int owner = 0;
		ClientClass* cClass = (ClientClass*)entity->GetClientClass();

		if (!g_Render->WorldToScreen(pos3D, pos))
			continue;


		if (g_Options.Visuals.Droppedguns && g_Options.Visuals.Enabled &&  cClass->m_ClassID != (int)ClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)ClassID::CDEagle || cClass->m_ClassID == (int)ClassID::CAK47)))
		{
			RenderWeapon(entity, Color(int(g_Options.Colors.droppedguns[0] * 255.f), int(g_Options.Colors.droppedguns[1] * 255.f), int(g_Options.Colors.droppedguns[2] * 255.f)));
		}
	


		if (g_Options.Visuals.C4World && g_Options.Visuals.Enabled)
		{
			if (cClass->m_ClassID == (int)ClassID::CPlantedC4)
				DrawBombPlanted(entity, local);
		}
		if (cClass->m_ClassID == (int)ClassID::CC4)
		{
			DrawBomb(entity, cClass);
		}


		if (g_Options.Visuals.Grenades && g_Options.Visuals.Enabled)
		{

			if (!g_Render->WorldToScreen(pos3D, pos))
				continue;

			Color GrenadeColor = Color(0, 0, 0, 0);
			char* szModelName = "";
			if (strstr(cClass->m_pNetworkName, XorStr("Projectile")))
			{
				const model_t* pModel = entity->GetModel();
				if (!pModel)
					return;


				const studiohdr_t* pHdr = g_ModelInfo->GetStudiomodel(pModel);
				if (!pHdr)
					return;

				if (!strstr(pHdr->name, XorStr("thrown")) && !strstr(pHdr->name, XorStr("dropped")))
					return;



				IMaterial* mats[32];
				g_ModelInfo->GetModelMaterials(pModel, pHdr->numtextures, mats);

				for (int i = 0; i < pHdr->numtextures; i++)
				{
					IMaterial* mat = mats[i];
					if (!mat)
						continue;

					if (strstr(mat->GetName(), "flashbang"))
					{
						GrenadeColor = Color(255, 255, 0, 255);
						szModelName = "Flashbang";
					}
					else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
					{
						szModelName = "Grenade";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "smoke"))
					{
						szModelName = "Smoke";
						GrenadeColor = Color(0, 255, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "decoy"))
					{
						szModelName = "Decoy";
						GrenadeColor = Color(0, 255, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "incendiary"))
					{
						szModelName = "Incendiary";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "molotov"))
					{
						szModelName = "Molotov";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
				}
				g_Render->DrawString2(g_Render->font.ESP, (int)pos.x, (int)pos.y, GrenadeColor, FONT_CENTER, "%s", szModelName);
			}
		}

		if (g_Engine->GetPlayerInfo(i, &pinfo) && entity->IsAlive())
		{

			if (g_Options.Backtrack.backtrackenable)
			{

				if (local->IsAlive())
				{
					for (int t = 0; t < g_Options.Backtrack.backtrackticks; ++t)
					{
						Vector screenbacktrack[64][12];

						if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > local->GetSimulationTime())
						{
							if (g_Render->WorldToScreen(headPositions[i][t].hitboxPos, screenbacktrack[i][t]))
							{
								if (g_Options.Backtrack.backtrackhistory)
								{
									g_Surface->DrawSetColor(Color(int(g_Options.Colors.backtrackdots_color[0] * 255.f), int(g_Options.Colors.backtrackdots_color[1] * 255.f), int(g_Options.Colors.backtrackdots_color[2] * 255.f)));
									g_Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);
								}
							}
						}
					}
				}
				else
				{
					memset(&headPositions[0][0], 0, sizeof(headPositions));
				}
			}
			if (g_Options.Ragebot.PosAdjust)
			{
				if (local->IsAlive())
				{
					for (int t = 0; t < 12; ++t)
					{
						Vector screenbacktrack[64][12];

						if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > local->GetSimulationTime())
						{
							if (g_Render->WorldToScreen(headPositions[i][t].hitboxPos, screenbacktrack[i][t]))
							{

								g_Surface->DrawSetColor(Color(int(g_Options.Colors.backtrackdots_color[0] * 255.f), int(g_Options.Colors.backtrackdots_color[1] * 255.f), int(g_Options.Colors.backtrackdots_color[2] * 255.f)));
								g_Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

							}
						}
					}
				}
				else
				{
					memset(&headPositions[0][0], 0, sizeof(headPositions));
				}
			}
			if (g_Options.Ragebot.FakeLagFix)
			{
				if (local->IsAlive())
				{
					Vector screenbacktrack[64];

					if (backtracking->records[i].tick_count + 12 > g_Globals->tickcount)
					{
						if (g_Render->WorldToScreen(backtracking->records[i].headPosition, screenbacktrack[i]))
						{

							g_Surface->DrawSetColor(Color(int(g_Options.Colors.backtrackdots_color[0] * 255.f), int(g_Options.Colors.backtrackdots_color[1] * 255.f), int(g_Options.Colors.backtrackdots_color[2] * 255.f)));
							g_Surface->DrawLine(screenbacktrack[i].x, screenbacktrack[i].y, screenbacktrack[i].x + 2, screenbacktrack[i].y + 2);

						}
					}
				}
				else
				{
					memset(&backtracking->records[0], 0, sizeof(backtracking->records));
				}
			}
			custommodels(entity);

			if (!g_Render->WorldToScreen(pos3D, pos))
				continue;

			Color clr = entity->GetTeamNum() == local->GetTeamNum() ?
				Color(g_Options.Colors.TeamESP[0] * 255, g_Options.Colors.TeamESP[1] * 255, g_Options.Colors.TeamESP[2] * 255, 255) :
				Color(g_Options.Colors.EnemyESP[0] * 255, g_Options.Colors.EnemyESP[1] * 255, g_Options.Colors.EnemyESP[2] * 255, 255);
			if (entity->GetTeamNum() == local->GetTeamNum() && !g_Options.Visuals.TeamESP)
				continue;
			if (!entity->IsAlive())
				continue;

			bool PVS = false;
			RECT rect = DynamicBox(entity, PVS, local);

			DrawInfo(rect, entity, local);

			if (g_Options.Visuals.Box && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.BoxType)
				{
				case 0:
					DrawBox(rect, clr);
					break;
				case 1:
					DrawCorners(rect, clr);
					break;
				case 2:
					ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetOrigin(), clr);
					break;
				}
			}
			if (g_Options.Visuals.fill && g_Options.Visuals.Enabled)
			{

				if (entity->GetTeamNum() == local->GetTeamNum())
				{
					g_Surface->DrawSetColor(Color(int(g_Options.Colors.fill_color_team[0] * 255.f), int(g_Options.Colors.fill_color_team[1] * 255.f), int(g_Options.Colors.fill_color_team[2] * 255.f), g_Options.Visuals.esp_fill_amount));
					ColorAlpha::ESP_ctx.clr_fill.SetAlpha(g_Options.Visuals.esp_fill_amount * 255.f);
					RenderFill(rect);
				}
				else if (entity->GetTeamNum() != local->GetTeamNum())
				{
					ColorAlpha::ESP_ctx.clr_fill.SetAlpha(g_Options.Visuals.esp_fill_amount * 255.f);
					g_Surface->DrawSetColor(Color(int(g_Options.Colors.fill_color_enemy[0] * 255.f), int(g_Options.Colors.fill_color_enemy[1] * 255.f), int(g_Options.Colors.fill_color_enemy[2] * 255.f), g_Options.Visuals.esp_fill_amount));

					RenderFill(rect);
				}
			}
			if (g_Options.Visuals.AimLine && g_Options.Visuals.Enabled)
			{
				DrawSnapLine(Vector(rect.left + ((rect.right - rect.left) / 2), rect.bottom, 0), Color(int(g_Options.Colors.AimLineColor[0] * 255.f), int(g_Options.Colors.AimLineColor[1] * 255.f), int(g_Options.Colors.AimLineColor[2] * 255.f)));
			}
			if (g_Options.Visuals.skeletonenbl && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.skeletonopts)
				{
				case 0:
					Skeleton(entity, Color(int(g_Options.Colors.color_skeleton[0] * 255.f), int(g_Options.Colors.color_skeleton[1] * 255.f), int(g_Options.Colors.color_skeleton[2] * 255.f)));
					break;
				case 1:
					HealthSkeleton(entity);
					break;
				case 2:

					drawskeletonrainbow(entity);
					break;
				}
			}
			if (g_Options.Visuals.health && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.healthtype)
				{
				case 0: break;
				case 1:
					DrawHealth(rect, entity);
					break;
				case 2:
					edgyHealthBar(rect, entity);
					break;
				}
			}
			if (g_Options.Visuals.armor && g_Options.Visuals.Enabled)
			{
				armorbar(rect, entity);
			}
			if (g_Options.Visuals.barrel && g_Options.Visuals.Enabled)
			{
				BulletTrace(entity, Color(int(g_Options.Colors.BulletTraceColor[0] * 255.f), int(g_Options.Colors.BulletTraceColor[1] * 255.f), int(g_Options.Colors.BulletTraceColor[2] * 255.f)));
			}
			if (g_Options.Misc.radaringame)
			{
				Radar(entity);
			}
		
		}
	}
}

void visuals::DrawBombPlanted(C_BaseEntity* entity, C_BaseEntity* local)
{
	BombCarrier = nullptr;

	float damage;
	char bombdamagestringdead[24];
	char bombdamagestringalive[24];

	Vector vOrig; Vector vScreen;
	vOrig = entity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)entity;
	float flBlow = Bomb->GetC4BlowTime();
	float lifetime = flBlow - (g_Globals->interval_per_tick * local->GetTickBase());
	if (g_Render->WorldToScreen(vOrig, vScreen))
	{
		if (local->IsAlive())
		{
			float flDistance = local->GetEyePosition().DistTo(entity->GetEyePosition());
			float a = 450.7f;
			float b = 75.68f;
			float c = 789.2f;
			float d = ((flDistance - b) / c);
			float flDamage = a*exp(-d * d);

			damage = float((std::max)((int)ceilf(CSGO_Armor(flDamage, local->ArmorValue())), 0));

			sprintf_s(bombdamagestringdead, sizeof(bombdamagestringdead) - 1, "Health left: 0 :(");
			sprintf_s(bombdamagestringalive, sizeof(bombdamagestringalive) - 1, "Health left: %.0f", local->GetHealth() - damage);
			if (lifetime > -2.f)
			{
				if (damage >= local->GetHealth())
				{
					g_Render->Text((int)vScreen.x, int(vScreen.y + 10), Color(250, 42, 42, 255), g_Render->font.Defuse, bombdamagestringdead);
				}
				else if (local->GetHealth() > damage)
				{
					g_Render->Text((int)vScreen.x, int(vScreen.y + 10), Color(0, 255, 0, 255), g_Render->font.Defuse, bombdamagestringalive);
				}
			}
		}
		char buffer[64];
		if (lifetime > 0.01f && !Bomb->IsBombDefused())
		{
			sprintf_s(buffer, "Bomb: %.1f", lifetime);
			g_Render->Text((int)vScreen.x, (int)vScreen.y, Color(250, 42, 42, 255), g_Render->font.ESP, buffer);
		}

	}

	g_Engine->GetScreenSize(width, height);
	int halfX = width / 2;
	int halfY = height / 2;


	if (Bomb->GetBombDefuser() > 0)
	{
		float countdown = Bomb->GetC4DefuseCountDown() - (local->GetTickBase() * g_Globals->interval_per_tick);
		if (countdown > 0.01f)
		{
			if (lifetime > countdown)
			{
				char defuseTimeString[24];
				sprintf_s(defuseTimeString, sizeof(defuseTimeString) - 1, "Defusing: %.1f", countdown);
				g_Render->Text(halfX - 50, halfY + 200, Color(0, 255, 0, 255), g_Render->font.Defuse, defuseTimeString);
			}
			else
			{
				g_Render->Text(halfX - 50, halfY + 200, Color(255, 0, 0, 255), g_Render->font.Defuse, "No time! Get OUT!");
			}
		}
	}
}

void visuals::DrawBomb(C_BaseEntity* entity, ClientClass* cClass)
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)entity;
	Vector vOrig; Vector vScreen;
	vOrig = entity->GetOrigin();
	bool adopted = true;
	auto parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		C_BaseEntity* pParentEnt = (g_EntityList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}
	if (g_Options.Visuals.C4World)
	{
		if (adopted)
		{
			if (g_Render->WorldToScreen(vOrig, vScreen))
			{
				g_Render->Text((int)vScreen.x, (int)vScreen.y, Color(112, 20, 20, 255), g_Render->font.ESP, "Bomb");
			}
		}
	}


}

void visuals::edgyHealthBar(RECT rect, C_BaseEntity* pEntity)
{

	float top = 1.4;
	float right = 0;
	float left = 0;


	float HealthValue = pEntity->GetHealth();
	int iHealthValue = HealthValue;
	int Red = 255 - (HealthValue * 2.00);
	int Green = HealthValue * 2.00;
	float flBoxes = std::ceil(pEntity->GetHealth() / 10.f);

	float height = (rect.bottom - rect.top) * (HealthValue / 100);
	float height2 = (rect.bottom - rect.top) * (100 / 100);
	float flHeight = height2 / 10.f;


	g_Render->DrawRect(rect.left - 5, rect.top - 1, rect.left - 1, rect.bottom + 1, Color(0, 0, 0, 150));
	g_Render->DrawRect(rect.left - 4, rect.bottom - height, rect.left - 2, rect.bottom, Color(Red, Green, 0, 255));

	for (int i = 0; i < 10; i++)
		g_Render->Line(rect.left - 5, rect.top + i * flHeight, rect.left - 2, rect.top + i * flHeight, Color(0, 0, 0, 255));

}

void visuals::armorbar(RECT rect, C_BaseEntity* pEntity)
{
	float ArmorValue = pEntity->ArmorValue();
	int iArmorValue = ArmorValue;
	int red = 255 - (ArmorValue * 2.0);
	int blue = ArmorValue * 2.0;

	float height = (rect.right - rect.left) * (ArmorValue / 100);


	g_Render->DrawRect(rect.left - 1, rect.bottom + 1, rect.right + 1, rect.bottom + 5, Color(10, 10, 10, 165));
	g_Render->DrawRect(rect.left, rect.bottom + 2, rect.left + height, rect.bottom + 4, Color(red, blue, blue, 255));

	int Armor = pEntity->ArmorValue();
}

void visuals::DrawBox(RECT rect, Color Col)
{
	g_Render->DrawOutlinedRect(rect.left - 1, rect.top - 1, rect.right - rect.left + 2, rect.bottom - rect.top + 2, Color(0, 0, 0, 150));
	g_Render->DrawOutlinedRect(rect.left + 1, rect.top + 1, rect.right - rect.left - 2, rect.bottom - rect.top - 2, Color(0, 0, 0, 125));
	g_Render->DrawOutlinedRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, Col);
}

void visuals::RenderFill(RECT rect)
{
	g_Surface->DrawFilledRect(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2);
}

void visuals::DrawCorners(RECT rect, Color Col)
{
	int x1, y1, x2, y2, w, h;
	x1 = rect.left;
	y1 = rect.top;
	x2 = rect.right;
	y2 = rect.bottom;

	w = x2;
	h = y2;

	int Line_Size = (y1 - h) / 6;
	int Line_Size2 = (y1 - h) / 6;

	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;
	Col.GetColor(red, green, blue, alpha);
	g_Surface->DrawSetColor(red, green, blue, alpha);

	//top inwards
	g_Surface->DrawLine(w, y1, w + Line_Size, y1);
	g_Surface->DrawLine(x1, y1, x1 - Line_Size, y1);

	//top downwards
	g_Surface->DrawLine(x1, y1, x1, y1 - Line_Size);
	g_Surface->DrawLine(w, y1, w, y1 - Line_Size);

	//bottom inwards
	g_Surface->DrawLine(x1, h, x1 - Line_Size, h);
	g_Surface->DrawLine(w, h, w + Line_Size, h);

	//bottom upwards
	g_Surface->DrawLine(x1, h, x1, h + Line_Size);
	g_Surface->DrawLine(w, h, w, h + Line_Size);

	//outlines

	g_Surface->DrawSetColor(0, 0, 0, 200);

	//top inwards
	g_Surface->DrawLine(w, y1 - 1, w + Line_Size, y1 - 1);
	g_Surface->DrawLine(x1, y1 - 1, x1 - Line_Size, y1 - 1);
	//inlines
	g_Surface->DrawLine(w - 1, y1 + 1, w + Line_Size, y1 + 1);
	g_Surface->DrawLine(x1 + 1, y1 + 1, x1 - Line_Size, y1 + 1);

	// top downwards
	g_Surface->DrawLine(x1 - 1, y1 - 1, x1 - 1, y1 - Line_Size);
	g_Surface->DrawLine(w + 1, y1 - 1, w + 1, y1 - Line_Size);
	//inlines
	g_Surface->DrawLine(x1 + 1, y1, x1 + 1, y1 - Line_Size);
	g_Surface->DrawLine(w - 1, y1, w - 1, y1 - Line_Size);

	//bottom inwards
	g_Surface->DrawLine(x1, h + 1, x1 - Line_Size, h + 1);
	g_Surface->DrawLine(w, h + 1, w + Line_Size, h + 1);
	//inlines
	g_Surface->DrawLine(x1 + 1, h - 1, x1 - Line_Size, h - 1);
	g_Surface->DrawLine(w - 1, h - 1, w + Line_Size, h - 1);

	//bottom upwards
	g_Surface->DrawLine(x1 - 1, h + 1, x1 - 1, h + Line_Size);
	g_Surface->DrawLine(w + 1, h + 1, w + 1, h + Line_Size);
	//inlines
	g_Surface->DrawLine(x1 + 1, h, x1 + 1, h + Line_Size);
	g_Surface->DrawLine(w - 1, h, w - 1, h + Line_Size);
}

void visuals::ThreeDBox(Vector minin, Vector maxin, Vector pos, Color Col)
{
	Vector min = minin + pos;
	Vector max = maxin + pos;

	Vector corners[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(min.x, min.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(max.x, max.y, max.z),
		Vector(max.x, min.y, max.z) };


	int edges[12][2] = { { 0, 1 },{ 1, 2 },{ 2, 3 },{ 3, 0 },{ 4, 5 },{ 5, 6 },{ 6, 7 },{ 7, 4 },{ 0, 4 },{ 1, 5 },{ 2, 6 },{ 3, 7 }, };

	for (const auto edge : edges)
	{
		Vector p1, p2;
		if (!g_Render->WorldToScreen(corners[edge[0]], p1) || !g_Render->WorldToScreen(corners[edge[1]], p2))
			return;
		int red = 0;
		int green = 0;
		int blue = 0;
		int alpha = 0;
		Col.GetColor(red, green, blue, alpha);
		g_Surface->DrawSetColor(red, green, blue, alpha);
		g_Surface->DrawLine(p1.x, p1.y, p2.x, p2.y);
	}
}

void visuals::DrawInfo(RECT rect, C_BaseEntity* pPlayer, C_BaseEntity* local)
{
	player_info_t info;
	static class Text
	{
	public:
		std::string text;
		int side;
		int Font;
		Color color;

		Text(std::string text, int side, int Font, Color color) : text(text), side(side), Font(Font), color(color)
		{
		}
	};
	std::vector< Text > texts;
	if (pPlayer->GetClientClass()->m_ClassID == 83 || pPlayer->GetClientClass()->m_ClassID == 84)
	{
		if (g_Options.Visuals.Hostage && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Hostage*", 0, g_Render->font.ESP, Color(255, 255, 255, 255)));
		}
	}
	else if (g_Engine->GetPlayerInfo(pPlayer->GetIndex(), &info))
	{
		if (pPlayer == BombCarrier && g_Options.Visuals.BombCarrier &&  g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Bomb Carrier*", 0, g_Render->font.ESP, Color(255, 220, 220, 255)));
		}

		if (g_Options.Visuals.Flashed && pPlayer->IsFlashed() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Flashed*", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.IsHasDefuser && pPlayer->hasDefuser() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Defuser*", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.IsDefusing &&  pPlayer->IsDefusing() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("Defusing", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.Name && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(info.name, 0, g_Render->font.ESP, Color(255, 255, 255, 255)));
		}

		if (g_Options.Visuals.resolveMode && g_Options.Visuals.Enabled)
		{
			if (Globals::resolvemode == 1)
			{
				texts.push_back(Text(std::string("Resolver: Bruteforce"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 2)
			{
				texts.push_back(Text(std::string("Resolver: Predicting LBY"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 3)
			{
				texts.push_back(Text(std::string("Resolver: LBY Update"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 4)
			{
				texts.push_back(Text(std::string("Resolver: Legit Player"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
		}


		if (g_Options.Visuals.Distance && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(std::to_string(flGetDistance(local->GetOrigin(), pPlayer->GetOrigin())) + std::string("FT"), 2, g_Render->font.ESP, Color(255, 255, 255, 255)));
		}


		CBaseCombatWeapon* weapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pPlayer->GetActiveWeaponHandle());
		if (weapon)
		{
			if (g_Options.Visuals.Weapon && weapon && g_Options.Visuals.Enabled)
			{
				texts.push_back(Text(weapon->GetGunName() + std::string("[") + std::to_string(weapon->ammo()) + std::string("|") + std::to_string(weapon->ammo2()) + std::string("]"), 2, g_Render->font.ESP, Color(255, 255, 255, 255)));
			}
		}

		if (g_Options.Visuals.Scoped && pPlayer->IsScoped() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Scoped*", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.Reloading && weapon->IsInReload() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("*Reloading*", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

	
		if (g_Options.Visuals.Money && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(std::string("$") + std::to_string(pPlayer->iAccount()), 1, g_Render->font.ESP, Color(71, 132, 60, 255)));
		}

	}


	int middle = ((rect.right - rect.left) / 2) + rect.left;
	int Top[3] = { rect.top,rect.top, rect.bottom };
	for (auto text : texts)
	{
		RECT nameSize = g_Render->GetTextSize(text.Font, (char*)text.text.c_str());
		switch (text.side)
		{
		case 0:
			Top[0] -= nameSize.bottom + 1;
			g_Render->DrawString2(text.Font, middle, Top[0] + 8, text.color, FONT_CENTER, (char*)text.text.c_str());
			break;
		case 1:
			g_Render->DrawString2(text.Font, rect.right + 2, Top[1] + 8, text.color, FONT_LEFT, (char*)text.text.c_str());
			Top[1] += nameSize.bottom - 4;
			break;
		case 2:
			g_Render->DrawString2(text.Font, middle, Top[2] + 8, text.color, FONT_CENTER, (char*)text.text.c_str());
			Top[2] += nameSize.bottom - 4;
			break;
		}
	}
}

void visuals::Radar(C_BaseEntity* entity)
{
	DWORD m_bSpotted = offsetz.DT_BaseEntity.m_bSpotted;
	*(char*)((DWORD)(entity)+m_bSpotted) = 1;
}

void visuals::DrawAwall()
{
	int MidX;
	int MidY;
	g_Engine->GetScreenSize(MidX, MidY);

	int damage;
	if (CanWallbang(damage))
	{
		g_Render->OutlineCircle(MidX / 2, MidY / 2, 10, 10, Color(0, 255, 0));
		g_Render->Textf(MidX / 2, MidY / 2 + 6, Color(255, 255, 255, 255), g_Render->font.ESP, "DMG: %1i", damage);
	}
	else
	{
		g_Render->OutlineCircle(MidX / 2, MidY / 2, 10, 10, Color(255, 0, 0));
		g_Render->Textf(MidX / 2, MidY / 2 + 6, Color(255, 255, 255, 255), g_Render->font.ESP, "DMG: 0");
	}
}

void visuals::DrawAngles()
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = pLocal;


	AngleVectors(QAngle(0, Globals::RealAngle, 0), &forward);
	src3D = pLocal->GetOrigin();
	dst3D = src3D + (forward * 45.f);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;

	g_Render->Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));
	g_Render->Text(dst.x, dst.y, Color(0, 255, 0, 255), g_Render->font.ESP, "REAL");

	AngleVectors(QAngle(0, Globals::FakeAngle, 0), &forward);
	dst3D = src3D + (forward * 45.f);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;

	g_Render->Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));
	g_Render->Text(dst.x, dst.y, Color(255, 0, 0, 255), g_Render->font.ESP, "FAKE");

}

RECT visuals::DynamicBox(C_BaseEntity* pPlayer, bool& PVS, C_BaseEntity* local)
{
	Vector trans = pPlayer->GetOrigin();

	Vector min;
	Vector max;

	min = pPlayer->GetCollideable()->OBBMins();
	max = pPlayer->GetCollideable()->OBBMaxs();

	Vector pointList[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector Distance = pointList[0] - pointList[1];
	int dst = Distance.Length();
	dst /= 1.3f;
	Vector angs;
	CalcAngle(trans, local->GetEyePosition(), angs);

	Vector all[8];
	angs.y += 45;
	for (int i = 0; i < 4; i++)
	{
		AngleVectors(angs, &all[i]);
		all[i] *= dst;
		all[i + 4] = all[i];
		all[i].z = max.z;
		all[i + 4].z = min.z;
		VectorAdd(all[i], trans, all[i]);
		VectorAdd(all[i + 4], trans, all[i + 4]);
		angs.y += 90;
	}

	Vector flb, brt, blb, frt, frb, brb, blt, flt;
	PVS = true;

	if (!g_DebugOverlay->ScreenPosition(all[3], flb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[0], blb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[2], frb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[6], blt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[5], brt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[4], frt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[1], brb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[7], flt))
		PVS = false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for (int i = 0; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
	}
	RECT rect;
	rect.left = left;
	rect.bottom = bottom;
	rect.right = right;
	rect.top = top;

	return rect;
}

bool visuals::GetBBox(C_BaseEntity* entity, visuals::ESPBox &result)
{

	// Variables
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;


	vOrigin = entity->GetAbsOrigin2();
	min = entity->collisionProperty()->GetMins() + vOrigin;
	max = entity->collisionProperty()->GetMaxs() + vOrigin;

	// Points of a 3d bounding box
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	// Get screen positions
	if (!g_Render->WorldToScreen(points[3], flb) || !g_Render->WorldToScreen(points[5], brt)
		|| !g_Render->WorldToScreen(points[0], blb) || !g_Render->WorldToScreen(points[4], frt)
		|| !g_Render->WorldToScreen(points[2], frb) || !g_Render->WorldToScreen(points[1], brb)
		|| !g_Render->WorldToScreen(points[6], blt) || !g_Render->WorldToScreen(points[7], flt))
		return false;

	// Put them in an array (maybe start them off in one later for speed?)
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	// Init this shit
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	// Find the bounding corners for our box
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	RECT rect;
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	// Width / height
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;
	return true;
}

void visuals::RenderWeapon(C_BaseEntity* entity,Color color)
{
	
	if (entity)
	{
		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)entity;

		auto owner = pWeapon->GetOwnerHandle();

		if (owner > -1)
			return;

		Vector pos3D = entity->GetAbsOrigin2();

		if (pos3D.x == 0.0f && pos3D.y == 0.0f && pos3D.z == 0.0f)
			return;

		Vector pos;

	        if (!g_Render->WorldToScreen(pos3D, pos))
			return;

		int weaponID = pWeapon->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		auto weaponName = ItemDefinitionIndexToString(weaponID);


		g_Render->Text(pos.x, pos.y, color, g_Render->font.DroppedGuns, weaponName);
	}
}

void visuals::BulletTrace(C_BaseEntity* pEntity, Color color)
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	Vector eyes = *pEntity->GetEyeAngles();

	AngleVectors(eyes, &forward);
	filter.pSkip = pEntity;
	src3D = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	dst3D = src3D + (forward * g_Options.Visuals.barrelL);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;


	g_Render->Line(src.x, src.y, dst.x, dst.y, color);
	g_Render->DrawOutlinedRect(dst.x - 3, dst.y - 3, 6, 6, color);
};

void visuals::Skeleton(C_BaseEntity* pEntity, Color Col)
{

	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			g_DebugOverlay->ScreenPosition(vParent, sParent);
			g_DebugOverlay->ScreenPosition(vChild, sChild);
			g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Col);
		}
	}
}

void visuals::HealthSkeleton(C_BaseEntity* pEntity)
{

	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			if (pEntity->GetHealth() > 85)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(250, 250, 250, 255));

			}
			else if (pEntity->GetHealth() < 85 & pEntity->GetHealth() > 60)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(20, 250, 140, 255));
			}
			else if (pEntity->GetHealth() < 60 & pEntity->GetHealth() > 35)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(252, 150, 40, 255));
			}
			else if (pEntity->GetHealth() < 60 & pEntity->GetHealth() > 30)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(252, 150, 40, 255));
			}
			else  if (pEntity->GetHealth() < 30 & pEntity->GetHealth() > 0)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(252, 1, 80, 255));
			}
		}
	}
}

void visuals::drawskeletonrainbow(C_BaseEntity* pEntity)
{

	static float rainbow;
	rainbow += 0.005f;
	if (rainbow > 1.f) rainbow = 0.f;

	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			g_DebugOverlay->ScreenPosition(vParent, sParent);
			g_DebugOverlay->ScreenPosition(vChild, sChild);
			g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color::FromHSB(rainbow, 1.f, 1.f));
		}
	}

}

void visuals::DrawSnapLine(Vector to, Color clr)
{
	int width = 0;
	int height = 0;
	g_Engine->GetScreenSize(width, height);
	Vector From((width / 2), height - 1, 0);
	g_Render->Line(From.x, From.y, to.x, to.y, clr);
}

void visuals::DrawHealth(RECT rect, C_BaseEntity* pPlayer)
{
	float HealthValue = pPlayer->GetHealth();

	float HealthValue2 = pPlayer->GetHealth();
	if (HealthValue2 > 100)
		HealthValue2 = 100;

	char hp[256];
	sprintf(hp, "%.0f", HealthValue);
	int Red = 255 - (HealthValue2 * 2.55);
	int Green = HealthValue2 * 2.55;
	float height = (rect.bottom - rect.top) * (HealthValue2 / 100);
	g_Render->GradientH(rect.left - 9, rect.top - 1, 6, rect.bottom - rect.top + 2, Color(0, 0, 0, 150), Color(0, 0, 0, 150));
	g_Render->GradientH(rect.left - 8, rect.bottom - height, 4, height, Color(Red, Green, 0, 255), Color(Red, Green, 0, 255));
	g_Render->DrawOutlinedRect(rect.left - 9, rect.top - 1, 6, rect.bottom - rect.top + 2, Color(0, 0, 0, 255));
	g_Render->DrawString2(g_Render->font.ESP, rect.left - 6, rect.bottom - height + 1, Color(255, 255, 255, 255), FONT_CENTER, hp);
}

void visuals::Hitmarker()
{
	if (G::hitmarkeralpha < 0.f)
		G::hitmarkeralpha = 0.f;
	else if (G::hitmarkeralpha > 0.f)
		G::hitmarkeralpha -= 0.01f;

	int W, H;
	g_Engine->GetScreenSize(W, H);

	if (G::hitmarkeralpha > 0.f)
	{
		g_Render->Line(W / 2 - 5, H / 2 - 5, W / 2 - 3, H / 2 - 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 - 5, H / 2 + 5, W / 2 - 3, H / 2 + 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 + 5, H / 2 - 5, W / 2 + 3, H / 2 - 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 + 5, H / 2 + 5, W / 2 + 3, H / 2 + 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));

	}
}

void visuals::DLight(C_BaseEntity *local, C_BaseEntity* entity)
{
	player_info_t pinfo;
	if (local && entity && entity != local)
	{
		if (g_Engine->GetPlayerInfo(entity->GetIndex(), &pinfo) && entity->IsAlive() && !entity->IsDormant())
		{
			if (local->GetTeamNum() != entity->GetTeamNum())
			{
				dlight_t* pElight = g_Dlight->CL_AllocElight(entity->GetIndex());
				pElight->origin = entity->GetOrigin() + Vector(0.0f, 0.0f, 35.0f);
				pElight->radius = 300.0f;
				pElight->color.b = 36;
				pElight->color.g = 224;
				pElight->color.r = 41;
				pElight->die = g_Globals->curtime + 0.05f;
				pElight->decay = pElight->radius / 5.0f;
				pElight->key = entity->GetIndex();

				dlight_t* pDlight = g_Dlight->CL_AllocDlight(entity->GetIndex());
				pDlight->origin = entity->GetOrigin();
				pDlight->radius = 300.0f;
				pDlight->color.b = 36;
				pDlight->color.g = 224;
				pDlight->color.r = 41;
				pDlight->die = g_Globals->curtime + 0.05f;
				pDlight->decay = pDlight->radius; // / 5.0f;
				pDlight->key = entity->GetIndex();
			}
		}
	}
}

void visuals::WorldModes()
{
	static auto postprocess_enable = g_CVar->FindVar("mat_postprocess_enable");
	static auto postprocess_enable_s = new SpoofedConvar(postprocess_enable);


	if (g_Options.Visuals.postprocess && g_Options.Visuals.Enabled)
	{
		postprocess_enable_s->SetInt(0);
	}
	else
	{
		postprocess_enable_s->SetInt(1);
	}


	static auto snowmode = g_CVar->FindVar("mat_drawgray");
	static auto snowmode_s = new SpoofedConvar(snowmode);

	if (g_Options.Visuals.snowmode && g_Options.Visuals.Enabled)
	{
		snowmode_s->SetInt(1);
	}
	else
	{
		snowmode_s->SetInt(0);
	}


	static auto lsdmode_enabled = g_CVar->FindVar("mat_showmiplevels");
	static auto  lsdmode_enabled_s = new SpoofedConvar(lsdmode_enabled);


	if (g_Options.Visuals.lsdmode && g_Options.Visuals.Enabled)
	{

		lsdmode_enabled_s->SetInt(1);
	}
	else
	{
		lsdmode_enabled_s->SetInt(0);
	}


	static auto chrome_enabled = g_CVar->FindVar("r_showenvcubemap");
	static auto  chrome_enabled_s = new SpoofedConvar(chrome_enabled);


	if (g_Options.Visuals.chromemode && g_Options.Visuals.Enabled)
	{
		chrome_enabled_s->SetInt(1);
	}
	else
	{
		chrome_enabled_s->SetInt(0);
	}


	static auto minecraftmode = g_CVar->FindVar("mat_showlowresimage");
	static auto  minecraftmode_s = new SpoofedConvar(minecraftmode);


	if (g_Options.Visuals.minecraftmode && g_Options.Visuals.Enabled)
	{
		minecraftmode_s->SetInt(1);
	}
	else
	{
		minecraftmode_s->SetInt(0);
	}


	if (g_Options.Visuals.ambientlight && g_Options.Visuals.Enabled)
	{

	
	
		float AmbientRedAmount = g_Options.Colors.ambientlightcolor[0];
		float AmbientGreenAmount = g_Options.Colors.ambientlightcolor[1];
		float AmbientBlueAmount = g_Options.Colors.ambientlightcolor[2];



		ConVar* AmbientRedCvar = g_CVar->FindVar("mat_ambient_light_r");
		*(float*)((DWORD)&AmbientRedCvar->fnChangeCallback + 0xC) = NULL;
		AmbientRedCvar->SetValue(AmbientRedAmount);

		ConVar* AmbientGreenCvar = g_CVar->FindVar("mat_ambient_light_g");
		*(float*)((DWORD)&AmbientGreenCvar->fnChangeCallback + 0xC) = NULL;
		AmbientGreenCvar->SetValue(AmbientGreenAmount);

		ConVar* AmbientBlueCvar = g_CVar->FindVar("mat_ambient_light_b");
		*(float*)((DWORD)&AmbientBlueCvar->fnChangeCallback + 0xC) = NULL;
		AmbientBlueCvar->SetValue(AmbientBlueAmount);
	}
}

void visuals::NightMode()
{
	if (g_Options.Visuals.nightMode)
	{
		if (!done)
		{
			static auto sv_skyname = g_CVar->FindVar("sv_skyname");
			static ConVar* r_DrawSpecificStaticProp = g_CVar->FindVar("r_DrawSpecificStaticProp");
			r_DrawSpecificStaticProp->SetValue(1);
			sv_skyname->SetValue("sky_csgo_night02");

			for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
			{
				IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{
					pMaterial->ColorModulate(0.10, 0.10, 0.10);
				}
				if (strstr(group, "StaticProp"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}

				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				done = true;
			}

		}
	}
	else
	{
		if (done)
		{
			for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
			{
				IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "StaticProp"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
			}
			done = false;
		}
	}
}

void visuals::DoAsusWalls()
{
	static bool bPerformed = false, bLastSetting;

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());


	if (!pLocal || !g_Engine->IsConnected() || !g_Engine->IsInGame())
		return;

	if (!bPerformed)
	{
		for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
		{
			IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;


			const char* group = pMaterial->GetTextureGroupName();
			const char* name = pMaterial->GetName();



			if (strstr(group, "World textures"))
			{
				if (bLastSetting)
				{
					pMaterial->AlphaModulate(0.6);
				}	
				else
				{
					pMaterial->AlphaModulate(1.00);
				}

			}


		}
		bPerformed = true;
	}
	if (bLastSetting != g_Options.Visuals.asuswalls)
	{
		bLastSetting = g_Options.Visuals.asuswalls;
		bPerformed = false;
	}

	junk3263761::doJunk();
}

void visuals::renderBeams()
{

	if (g_Options.Visuals.bulletshow && g_Options.Visuals.Enabled)
		return;

	auto local = static_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer()));
	if (!local)
		return;

	for (size_t i = 0; i < logs.size(); i++)
	{

		auto current = logs.at(i);

		current.color = Color(g_Options.Colors.flTracers);

		if (g_Options.Visuals.bulletshow && g_Options.Visuals.Enabled)
			g_DebugOverlay->AddLineOverlay(current.src, current.dst, current.color.r(), current.color.g(), current.color.g(), true, -1.f);


		g_DebugOverlay->AddBoxOverlay(current.dst, Vector(-2, -2, -2), Vector(2, 2, 2), Vector(0, 0, 0), current.color.r(),current.color.g(),current.color.g(), 127, -1.f);

		if (fabs(g_Globals->curtime - current.time) > 5.f)
			logs.erase(logs.begin() + i);
	}
}


void visuals::custommodels(C_BaseEntity* entity)
{
	if (entity->GetTeamNum() == TEAM_CS_CT)
	{
		switch (g_Options.Misc.customodelsct)
		{
		case 0: break;
		case 1:
			PrecacheModel("models/player/custom_player/kuristaja/trump/trump.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/kuristaja/trump/trump.mdl"));
			break;

		case 2:
			PrecacheModel("models/player/custom_player/xnet/girl/noire_swimwear/noire.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/xnet/girl/noire_swimwear/noire.mdl"));
			break;

		case 3:
			PrecacheModel("models/player/custom_player/bbs_93x_net_2017/momiji_inubashiri/update_2017_02_04/momiji_inubashiri.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/bbs_93x_net_2017/momiji_inubashiri/update_2017_02_04/momiji_inubashiri.mdl"));
			break;

		case 4:
			PrecacheModel("models/player/custom_player/kuristaja/banana_joe/banana_joe.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/kuristaja/banana_joe/banana_joe.mdl"));
			break;


		}
	}
	else if (entity->GetTeamNum() == TEAM_CS_T)
	{
		switch (g_Options.Misc.customodelst)
		{
		case 0: break;
		case 1:
			PrecacheModel("models/player/custom_player/kuristaja/trump/trump.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/kuristaja/trump/trump.mdl"));
			break;

		case 2:
			PrecacheModel("models/player/custom_player/xnet/girl/noire_swimwear/noire.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/xnet/girl/noire_swimwear/noire.mdl"));
			break;

		case 3:
			PrecacheModel("models/player/custom_player/bbs_93x_net_2017/momiji_inubashiri/update_2017_02_04/momiji_inubashiri.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/bbs_93x_net_2017/momiji_inubashiri/update_2017_02_04/momiji_inubashiri.mdl"));
			break;

		case 4:
			PrecacheModel("models/player/custom_player/kuristaja/banana_joe/banana_joe.mdl");
			entity->SetModelIndex(g_ModelInfo->GetModelIndex("models/player/custom_player/kuristaja/banana_joe/banana_joe.mdl"));
			break;


		}

	}
}