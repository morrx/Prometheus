#pragma once
#include "SDK.h"
#include "singleton.hpp"
#include "Color.h"
#include "MathFunctions.h"
#include "bulletshow.h"

class visuals
	:public singleton<visuals>
{
public:
	visuals();
	void OnPaintTraverse(C_BaseEntity* local);

	struct ESPBox
	{
		int x, y, w, h;
	};
	std::vector<cbullet_tracer_info>logs;
	void renderBeams();
	void custommodels(C_BaseEntity * entity);
private:
	C_BaseEntity *BombCarrier;
	void DrawBombPlanted(C_BaseEntity * entity, C_BaseEntity * local);
	void DrawBomb(C_BaseEntity * entity, ClientClass * cClass);
	void edgyHealthBar(RECT rect, C_BaseEntity * pEntity);
	void armorbar(RECT rect, C_BaseEntity * pEntity);
	void DrawBox(RECT rect, Color Col);
	void RenderFill(RECT rect);
	void DrawCorners(RECT rect, Color Col);
	void ThreeDBox(Vector minin, Vector maxin, Vector pos, Color Col);
	void DrawInfo(RECT rect, C_BaseEntity* pPlayer, C_BaseEntity* local);
	bool GetBBox(C_BaseEntity * entity, visuals::ESPBox & result);
	void RenderWeapon(C_BaseEntity * entity, Color col);
	void BulletTrace(C_BaseEntity* pEntity, Color color);
	void Skeleton(C_BaseEntity* pEntity, Color Color);
	void HealthSkeleton(C_BaseEntity * pEntity);
	void drawskeletonrainbow(C_BaseEntity * pEntity);
	void DrawSnapLine(Vector to, Color clr);
	void DrawHealth(RECT rect, C_BaseEntity* pPlayer);
	void DrawAngles();
	void Radar(C_BaseEntity * entity);
	void DrawAwall();
	RECT DynamicBox(C_BaseEntity* pPlayer, bool& PVS, C_BaseEntity* local);
	void DLight(C_BaseEntity* local, C_BaseEntity* entity);
	void Hitmarker();
	void WorldModes();
	void NightMode();
	void DoAsusWalls();


};




inline int flGetDistance(Vector from, Vector to)
{
	Vector angle;
	angle.x = to.x - from.x;	angle.y = to.y - from.y;	angle.z = to.z - from.z;

	return floor(sqrt(angle.x*angle.x + angle.y*angle.y + angle.z*angle.z) * 0.0254f);
}

inline float CSGO_Armor(float flDamage, int ArmorValue)
{
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}


namespace ColorAlpha
{
	struct VisualsStruct
	{
		Color2 clr_fill;
		C_BaseEntity* player;
	};
	
	extern VisualsStruct ESP_ctx;
	extern Color2 clr_fill;
}





char* const itemNames[] =
{
	
	"KNIFE", //0 - default
	"DEAGLE",
	"ELITE",
	"FIVESEVEN",
	"GLOCK",
	"none",
	"none",
	"AK-47",
	"AUG",
	"AWP",

	"FAMAS", //10
	"G3SG1",
	"none",
	"GALIL",
	"M249",
	"none",
	"M4A4",
	"MAC-10",
	"none",
	"P90",

	"none", //20
	"none",
	"none",
	"none",
	"UMP-45",
	"XM1014",
	"BIZON",
	"MAG-7",
	"NEGEV",
	"SAWED-OFF",

	"TEC-9", //30
	"Zeus-x27",
	"P2000",
	"MP-7",
	"MP-9",
	"NOVA",
	"P250",
	"none",
	"SCAR-20",
	"SG-556",

	"SSG-08", //40
	"KNIFE",
	"KNIFE",
	"FLASH",
	"NADE",
	"SMOKE",
	"MOLOTOV",
	"DECOY",
	"INCENDIARY",
	"C4",

	"none", //50
	"none",
	"none",
	"none",
	"none",
	"none",
	"none",
	"none",
	"none",
	"KNIFE",

	"M4A1", //60
	"USP",
	"none",
	"CZ-75",
	"REVOLVER"
};




inline const char* ItemDefinitionIndexToString(int index)
{
	if (index < 0 || index > 64)
		index = 0;

	return itemNames[index];
}
