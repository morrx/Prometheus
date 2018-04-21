#pragma once
#include <set>
#include <map>
#include <unordered_map>


struct skinInfo
{
	int seed = -1;
	int paintkit;
	std::string tagName;
};


//Color Feature for Color Changer
struct ColorP
{
public:
	const char* Name;
	float* Ccolor;

	ColorP(const char* name, float* color)
	{
		this->Name = name;
		this->Ccolor = color;
	}
};

struct Variables
{
	Variables()
	{

	}

	struct Ragebot_s
	{
		bool	MainSwitch;
		bool 	Enabled;
		bool 	AutoFire;
		float 	FOV;
		bool 	Silent;
		bool	AutoPistol;
		int		KeyPress;
		bool	AimStep;

		//AA Builder
		bool	BuilderAAs;
		bool	Jitter;
		bool	FJitter;
		bool	LBYBreaker;
		float	BuilderReal;
		float	BuilderFake;
		float	BuilderPitch;
		float	JitterRange;
		float	FJitterRange;


		bool	EnabledAntiAim;
		int		SubAATabs;
		bool	PreAAs;
		int		Pitch;
		int		YawTrue;
		int		YawFake;
		int		YawTrueMove;
		float	PitchAdder;
		float	YawTrueAdder;
		float	YawFakeAdder;
		float	YawFakeMove;
		bool	AtTarget;
		bool	Edge;
		bool	KnifeAA;
		bool	FakeLag;
		int		FakeLagAmt;


		bool	FriendlyFire;
		int		Hitbox;
		int		Hitscan;
		float	Pointscale;
		bool	Multipoint;
		float	Multipoints;

		bool	AntiRecoil;
		bool	AutoWall;
		bool	AutoStop;
		bool	AutoCrouch;
		bool	AutoScope;
		float	MinimumDamageSniper;
		float	MinimumDamageRifle;
		float	MinimumDamagePistol;
		float	MinimumDamageHeavy;
		float	MinimumDamageSmg;
		float	MinimumDamageRevolver;
		bool	Hitchance;
		float	HitchanceSniper;
		float	HitchancePistol;
		float	HitchanceRifle;
		float	HitchanceHeavy;
		float	HitchanceSmgs;
		float	HitchanceRevolver;
		bool	Resolver;
		bool	FakeLagFix;
		bool	PosAdjust;
		bool	playerlist;
		int		BAIMkey;
		float	bruteAfterX;
		bool	MovingAA;
	} Ragebot;

	

	struct
	{
		bool Legitbotrun;


		int MainKey = 1;
		float MainSmooth = 1;
		float Mainfov;
		float main_random_smooth;
		float main_recoil_min;
		float main_recoil_max;
		float main_randomized_angle;



		int PistolKey = 6;
		float Pistolfov;
		float PistolSmooth = 1;;
		float pistol_random_smooth;
		float pistol_recoil_min;
		float pistol_recoil_max;
		float pistol_randomized_angle;



		int SniperKey = 6;
		float Sniperfov;
		float SniperSmooth = 1;
		float sniper_random_smooth;
		float sniper_recoil_min;
		float sniper_recoil_max;
		float sniper_randomized_angle;


		int smg_Key = 6;
		float smg_fov;
		float smg_Smooth = 1;
		float smg_random_smooth;
		float smg_recoil_min;
		float smg_recoil_max;
		float smg_randomized_angle;

		int heavy_wp_Key = 6;
		float heavy_wp_fov;
		float heavy_wp_Smooth;
		float heavy_wp_random_smooth;
		float heavy_wp_recoil_min;
		float heavy_wp_recoil_max;
		float heavy_wp_randomized_angle;

		struct
		{
			bool Headwpmain;
			bool Neckwpmain;
			bool Chestwpmain;
			bool Stomachwpmain;
			bool multiboneswpmain;
		}FilterMainWeapons;

		struct
		{
			bool Headwppis;
			bool Neckwppis;
			bool Chestwppis;
			bool Stomachwppis;
			bool multiboneswppis;
		}FilterPistolWeapons;


		struct
		{
			bool HeadwpSnip;
			bool NeckwpSnip;
			bool ChestwpSnip;
			bool StomachwpSnip;
			bool multiboneswpSnip;

		}FilterSniperWeapons;


		struct
		{
			bool Headwpsmg;
			bool Neckwpsmg;
			bool Chestwpsmg;
			bool Stomachwpsmg;
			bool multiboneswpsmg;

		}FiltersmgWeapons;


		struct
		{
			bool Headwphv;
			bool Neckwphv;
			bool Chestwphv;
			bool Stomachwphv;
			bool multibonewphv;

		}FilterheavyWeapons;


		struct
		{
			bool Enabled;
			float Delay;
			int Key = 6;
			float hitchance;
			struct
			{
				bool Head;
				bool Arms;
				bool Chest;
				bool Stomach;
				bool Legs;
			} Filter;

		} Triggerbot;


	}LegitBot;

	struct
	{
		bool backtrackenable;
		bool backtrackhistory;
		int  backtrackticks = 1;


	}Backtrack;





	struct
	{
		bool Enabled;
		bool TeamESP;
		bool Box;
		int BoxType;
		bool fill;
		float esp_fill_amount;
		int healthtype;
		bool health;
		bool armor;

		bool Money;
		bool Name;
		bool skeletonenbl;
		int skeletonopts;
		bool Weapon;
		bool AimLine;
		bool angleLines;
		bool barrel;
		int barrelL;
		bool DrawAwall;
		bool LBYIndicator;
		bool RageDraw;
		bool C4World;
		bool resolveMode;
		bool Droppedguns;
		bool noscopeborder;



         //Info Enemy
		bool BombCarrier;
		bool Hostage;
		bool Flashed;
		bool Distance;
		bool Scoped;
		bool Reloading;
		bool IsHasDefuser;
		bool IsDefusing;

		//GRENADE
		bool Grenades;
		bool GrenadePrediction;




		//CHAMS
		bool Chams;
		int matierial;
		bool Teamchams;
		int champlayeralpha = 100;
		bool chamsHp;
		bool XQZ;
		int Hands;
		int chamswphands;

		//CROSSHAIR
		bool RecoilCrosshair;
		bool SpreadCrosshair;

	

		//REMOVERS
		bool NoVisualRecoil;
		bool NoFlash;
		bool NoSmoke;


		//VIEWMODEL CHANGERS
		bool viewmodelChanger_enabled;
		bool FOVChanger_enabled;
		float FOVChanger;
		float viewmodelChanger = 68;

		//WORLD
		bool nightMode;
		bool postprocess;
		bool snowmode;
		bool lsdmode;
		bool chromemode;
		bool asuswalls;
		bool minecraftmode;
		bool ambientlight;

		//ThirdPerson
		int TPKey;
		bool transparency;
		bool offtp; 

		//GLOW
		bool GlowEnable;
		bool GlowPlayerEnable;
		bool GlowEnemy;
		bool GlowTeam;
		float EnemyAlpha = 255;
		float TeamAlpha = 255;
		bool GlowWeaponsEnable;
		bool GlowC4Enable;
		float C4GlowAlpha = 255;
		float WeaponsGlowAlpha = 255;


		//OTHERS
	    bool demageidicator;
		bool bulletshow;
		float flTracersDuration;
		float flTracersWidth;

	


	} Visuals;
	struct
	{
		bool antiuntrusted = true;
		bool syncclantag;
		bool namePrometheus;
		bool Bhop;
		int spammer;
		bool spammeron;
		bool AutoStrafe;
		bool SpecList;
		bool ServerRankRevealAll;
		bool Watermark = true;
		bool AutoAccept;
		bool Hitmarker = false;
		int Hitsound = 0;
		bool afkbot;
		bool bFreeCam;
		int iFreeCamKey;
		float flFreeCamSpeed;
		bool inventoryalwayson;
		bool eventlogs;
		bool eventlogs2;
		int flashlight;
		bool radaringame;
		bool radarwindow;
		int radrsize = 200;
		float radralpha = 1.f;
		float radrzoom = 2.f;
		bool bBlockbot;
		int iBlockbotKey;
		int customodelst;
		int customodelsct;

	} Misc;
	struct
	{
		bool Enabled;
		int knifemodel;
		int glove;
		int gloveskin;

		struct
		{
			bool ChangerEnabled = false;
			int ChangerSkin = 0;
		} W[519]; // CHANGE HERE

		std::unordered_map<std::string, std::set<std::string>> weaponSkins;
		std::unordered_map<std::string, skinInfo> skinMap;
		std::unordered_map<std::string, std::string> skinNames;
		std::vector<std::string> weaponNames;
	} Skinchanger;

	struct
	{
		bool Opened = false;
		int 	Key;
		int		ConfigFile = 0;
		int		Theme = 0;
		int currentWeapon;
	} Menu;

	struct
	{//need more

		float TeamESP[3] = { 0, 1.f, 0 };
		float EnemyESP[3] = { 1.f, 0, 0 };
		float EnemyChamsVis[3] = { 1.f, 0, 0 };
		float EnemyChamsNVis[3] = { 1.f, 0, 0 };
		float TeamChamsVis[3] = { 0, 1.f, 0 };
		float TeamChamsNVis[3] = { 0, 1.f, 0 };
		float hitmarker_color[3] = { 0.f, 0.f, 0.f };
		float backtrackdots_color[3] = { 0.f, 0.f, 0.f };
		float dlight_color[3] = { 0.f, 0.f, 0.f };
		float color_skeleton[3] = { 0.f, 0.f, 0.f };
		float color_grenadeprediction[3] = { 0.f, 0.f, 0.f };
		float color_grenadeprediction_circle[3] = { 0.f, 0.f, 0.f };
		float color_recoil[3] = { 0.f, 0.f, 0.f };
		float color_spread[3] = { 0.f, 0.f, 0.f };
		float EnemyGlow[3] = { 0.f, 0.f, 0.f };
		float TeamGlow[3] = { 0.f, 0.f, 0.f };
		float HandsColor[3] = { 0.f, 0.f, 0.f };
		float AimLineColor[3] = { 0.f, 0.f, 0.f };
		float BulletTraceColor[3] = { 0.f, 0.f, 0.f };
		float fill_color_enemy[3] = { 1.f, 1.f, 0.0f};
		float fill_color_team[3] = { 0.f, 0.2f, 1.f};
		float glow_weapon[3] = { 0.f, 0.f, 0.f };
		float glow_c4[3] = { 0.f, 0.f, 0.f };
    	float damageindicator[3] = { 0.f,0.f,0.f };
		float droppedguns [3] = { 0.f,0.f,0.f };
		float flTracers[3] = { 1.f, 0.f, 0.f };
		float ambientlightcolor[3];

	}Colors;
};

extern Variables g_Options;