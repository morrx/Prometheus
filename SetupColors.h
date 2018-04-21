#pragma once

#include "Global.h"

//Colors
void SetupColors()
{
	//ESP

	Global::ColorsForPicker1.push_back(ColorP("Team - Esp", g_Options.Colors.TeamESP));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Esp", g_Options.Colors.EnemyESP));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - FillBox", g_Options.Colors.fill_color_enemy));
	Global::ColorsForPicker1.push_back(ColorP("Team - FillBox", g_Options.Colors.fill_color_team));


	//CHAMS
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Chams_Visible", g_Options.Colors.EnemyChamsVis));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Chams_Invisible", g_Options.Colors.EnemyChamsNVis));
	Global::ColorsForPicker1.push_back(ColorP("Team - Chams_Visible", g_Options.Colors.TeamChamsVis));
	Global::ColorsForPicker1.push_back(ColorP("Team - Chams_Invisible", g_Options.Colors.TeamChamsNVis));



	//GLOW
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Glow", g_Options.Colors.EnemyGlow));
	Global::ColorsForPicker1.push_back(ColorP("Team - Glow", g_Options.Colors.TeamGlow));
	Global::ColorsForPicker1.push_back(ColorP("Weapons - Glow", g_Options.Colors.glow_weapon));
	Global::ColorsForPicker1.push_back(ColorP("C4 - Glow", g_Options.Colors.glow_c4));


	Global::ColorsForPicker1.push_back(ColorP("DroppedGuns_Color", g_Options.Colors.droppedguns));
	Global::ColorsForPicker1.push_back(ColorP("Hands_Color", g_Options.Colors.HandsColor));
	Global::ColorsForPicker1.push_back(ColorP("Hitmarker_Color", g_Options.Colors.hitmarker_color));
	Global::ColorsForPicker1.push_back(ColorP("DynamicLight_Color", g_Options.Colors.dlight_color));
	Global::ColorsForPicker1.push_back(ColorP("Skeleton_Color", g_Options.Colors.color_skeleton));
	Global::ColorsForPicker1.push_back(ColorP("BackTrackHistory_Color", g_Options.Colors.backtrackdots_color));
	Global::ColorsForPicker1.push_back(ColorP("RecoilCrosshair_Color", g_Options.Colors.color_recoil));
	Global::ColorsForPicker1.push_back(ColorP("SpreadCrosshair_Color", g_Options.Colors.color_spread));
	Global::ColorsForPicker1.push_back(ColorP("GrenadePrediction_Line_Color", g_Options.Colors.color_grenadeprediction));
	Global::ColorsForPicker1.push_back(ColorP("GrenadePrediction_Circle_Color", g_Options.Colors.color_grenadeprediction_circle));
	Global::ColorsForPicker1.push_back(ColorP("BulletTrace_Color", g_Options.Colors.BulletTraceColor));
	Global::ColorsForPicker1.push_back(ColorP("AimLine_Color", g_Options.Colors.AimLineColor));
	Global::ColorsForPicker1.push_back(ColorP("DemageIndicator_Color", g_Options.Colors.damageindicator));
	Global::ColorsForPicker1.push_back(ColorP("BeamTracers_Color", g_Options.Colors.flTracers));
	Global::ColorsForPicker1.push_back(ColorP("AmbientLight_Color", g_Options.Colors.ambientlightcolor));
}



