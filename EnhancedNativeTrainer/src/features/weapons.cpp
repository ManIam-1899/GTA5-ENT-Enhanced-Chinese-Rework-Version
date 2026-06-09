/*
这段代码的部分内容最初是 GTA V SCRIPT HOOK SDK 的一部分。
http://dev-c.com
(C) Alexander Blade 2015

现在它是 Enhanced Native Trainer 项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Sondai Smith 和其他贡献者 2015
*/

#include "script.h"
#include "fuel.h"
#include "hotkeys.h"
#include "peds_dont_like_weapons.h"
#include "prison_break.h"
#include "..\ui_support\menu_functions.h"
#include "weapons.h"
#include "..\io\config_io.h"
#include <ctime>

int activeLineIndexWeapon = 0;
int lastSelectedWeaponCategory = 0;
int lastSelectedWeapon = 0;

Ped equip_ped = -1;
bool refresh_w = false;

int vision_toggle = 0;

int arrest_secs = 0;

int weapDmgModIndex = 0;
int activeLineIndexCopArmed = 0;
int activeLineIndexPedAgainstWeapons = 0;
int activeLineIndexPowerPunchWeapons = 0;

// 引力榴弹消息提示
static bool shown_vacuum_message = false; 
// 重力枪消息提示 
static bool shown_gravitygun_message = false;
// 载具模型枪消息提示
static bool shown_vehiclemodelgun_message = false;
// 角色模型枪消息提示
static bool shown_pedmodelgun_message = false;

// 保存武器相关变量
bool requireRefreshOfWeaponSaveSlotMenu = false;
std::string activeSavedWeaponSlotName;
bool WeaponSaveSlotMenuInterrupt = false;
bool requireRefreshOfWeaponSaveSlots = false;
int lastKnownSavedWeaponCount = 0;
bool WeaponSaveMenuInterrupt = false;
int activeSavedWeaponIndex = -1;
bool requireRefreshOfWeaponSlotMenu = false;

// 自动给予所有武器的相关变量
bool featureGiveAllWeapons = false;
bool featureAddAllWeaponsAttachments = false;
int tick_allw, tick_firemode = 0;
int w_tick_secs_passed, w_tick_secs_curr = 0;
int tick_a_allw, w_a_tick_secs_curr = 0;
Ped oldplayerPed_W, oldplayerPed_A = -1;
bool PlayerUpdated_w, PlayerUpdated_a = true;
int tick_s_allw, ss_tick_secs_curr = 0;
Ped oldplayerPed_s = -1;
bool PlayerUpdated_s = true;

Hash temp_weapon = -1;

// 手电筒频闪功能
int WeapStrobeIndexN = 0;
bool WeapStrobeChanged = true;
bool f_strobe = false;

// 全局屏幕准星相关变量
bool featureWeaponsCrosshair = false; // 屏幕准星开关，默认关闭
int WeaponsCrosshairStyleIndex = 0; // 默认0 对应值1（实线）
bool WeaponsCrosshairStyleChanged = false;
int WeaponsCrosshairColorIndex = 0; // 默认0 白色
bool WeaponsCrosshairColorChanged = false;
int strb_c = 0;
float strobe_tick = 0.0;

// 载具模型枪和角色模型枪相关变量
bool featureVehicleModelGun = false;
bool featureVehicleModelGunUpdated = false;
bool featurePedModelGun = false;
bool featurePedModelGunUpdated = false;
int VehicleModelGunCategoryIndex = 0; // 默认选择小型汽车
bool VehicleModelGunCategoryChanged = true; // 用于配置加载
int VehicleModelGunSpeedIndex = 4; // 默认正常速度（索引4=200.0f）
bool VehicleModelGunSpeedChanged = true; // 用于配置加载
bool featureVehicleModelGunInvincible = false;

int PedModelGunCategoryIndex = 0; // 默认选择环境女性
bool PedModelGunCategoryChanged = true; // 用于配置加载
int PedModelGunSpeedIndex = 4; // 默认正常速度（索引4=200.0f）
bool PedModelGunSpeedChanged = true; // 用于配置加载
bool featurePedModelGunInvincible = false;

// 手电筒强度
int WeapFlashDistIndex = 0;
bool WeapFlashDistChanged = true;

bool featureWeaponInfiniteAmmo = false;
bool featureWeaponInfiniteParachutes = false, featureWeaponInfiniteParachutesUpdated = false;
bool featureWeaponNoParachutes = false, featureWeaponNoParachutesUpdated = false;
bool featureWeaponNoReload = false;
bool featureCopTakeWeapon = false;
bool featureWeaponFireAmmo = false;
bool featureWeaponExplosiveAmmo = false;
bool featureWeaponExplosiveMelee = false;
bool featureWeaponExplosiveGrenades = false;
bool featureWeaponVacuumGrenades = false;

bool featurePunchFists = true;
bool featurePunchMeleeWeapons = false;
bool featurePunchFireWeapons = false;

bool featureCopArmedWith = false;
bool featurePlayerMelee = true;
bool featureSwitchWeaponIfDanger = false;
bool featureArmyMelee = false;
bool featureDetainedIfNotMove = false;

int bullet_a = 0;
int bullet_tick = 0;

bool featureGravityGun = false;
bool featureFriendlyFire = false;
bool featureRapidFire = false;
bool featureDropWeapon = false;
bool featureDropWeaponOutAmmo = false;
bool featureCanDisarmNPC = false;
bool featurePedNoWeaponDrop = false;
bool featurePowerPunch = false;

// 警察武器
bool someonehasgunandshooting = false;
Ped shooting_criminal = -1;

int s_vacuum_secs_passed, s_vacuum_secs_curr, vacuum_seconds = 0;

Ped temp_nearest_ped = -1;
bool force_nearest_ped = false;

bool grav_target_locked = false;
Entity grav_entity = 0;
DWORD grav_partfx = 0;

DWORD featureWeaponVehShootLastTime = 0;

// 强力拳击
std::string result_p;
std::string lastPowerWeapon;
std::string lastCustomWeapon;
char* currWeaponCompHash;

int const SAVED_WEAPONS_COUNT = TOTAL_WEAPONS_COUNT;
int saved_weapon_model[SAVED_WEAPONS_COUNT];
bool saved_has_weapon[SAVED_WEAPONS_COUNT];
int saved_ammo[SAVED_WEAPONS_COUNT];
int saved_clip_ammo[SAVED_WEAPONS_COUNT];
int saved_weapon_tints[SAVED_WEAPONS_COUNT];
bool saved_weapon_mods[SAVED_WEAPONS_COUNT][MAX_MOD_SLOTS];
bool saved_parachute = false;
int saved_parachute_tint = 0;
int saved_armour = 0;

int tick_rap_allw, w_tick_rap_secs_passed, ss_tick_rap_secs_curr = 0;

//bool do_give_weapon(std::string modelName);

bool redrawWeaponMenuAfterEquipChange = false;

// 警察武器
const std::vector<std::string> WEAPONS_COPARMED_CAPTIONS{ "\"WEAPON_UNARMED\"", "\"WEAPON_NIGHTSTICK\"", "\"WEAPON_FLASHLIGHT\"", "\"WEAPON_KNIFE\"", "\"WEAPON_DAGGER\"", "\"WEAPON_HAMMER\"", "\"WEAPON_BAT\"", "\"WEAPON_GOLFCLUB\"", 
"\"WEAPON_CROWBAR\"", "\"WEAPON_POOLCUE\"", "\"WEAPON_WRENCH\"", "\"WEAPON_MACHETE\"", "\"WEAPON_BOTTLE\"", "\"WEAPON_PISTOL\"", "\"WEAPON_APPISTOL\"", "\"WEAPON_REVOLVER\"", "\"WEAPON_STUNGUN\"", "\"WEAPON_FLAREGUN\"",
"\"WEAPON_MACHINEPISTOL\"", "\"WEAPON_MARKSMANPISTOL\"", "\"WEAPON_MINISMG\"", "\"WEAPON_ASSAULTSMG\"", "\"WEAPON_ASSAULTRIFLE\"", "\"WEAPON_CARBINERIFLE\"", "\"WEAPON_ADVANCEDRIFLE\"", "\"WEAPON_COMPACTRIFLE\"", "\"WEAPON_HEAVYSHOTGUN\"", 
"\"WEAPON_DBSHOTGUN\"", "\"WEAPON_AUTOSHOTGUN\"", "\"WEAPON_MUSKET\"", "\"WEAPON_SAWNOFFSHOTGUN\"", "\"WEAPON_COMBATMG\"", "\"WEAPON_MINIGUN\"", "\"WEAPON_GUSENBERG\"", "\"WEAPON_SNIPERRIFLE\"", "\"WEAPON_HEAVYSNIPER\"", 
"\"WEAPON_GRENADELAUNCHER\"", "\"WEAPON_GRENADELAUNCHER_SMOKE\"", "\"WEAPON_RPG\"", "\"WEAPON_HOMINGLAUNCHER\"", "\"WEAPON_COMPACTLAUNCHER\"", "\"WEAPON_RAILGUN\"", "\"WEAPON_FIREWORK\"", "\"WEAPON_RAYPISTOL\"", "\"WEAPON_RAYCARBINE\"" };
int CopCurrArmedIndex = 1;
bool CopCurrArmedChanged = true;

// 载具武器
const std::vector<std::string> WEAPONS_VEHICLE_CAPTIONS{ "关", "\"WEAPON_RPG\"", "\"WEAPON_GRENADE\"", "\"WEAPON_MOLOTOV\"", "\"WEAPON_FIREWORK\"", "\"VEHICLE_WEAPON_PLAYER_BULLET\"", "\"VEHICLE_WEAPON_PLAYER_LAZER\"", 
"\"WEAPON_DBSHOTGUN\"", "\"WEAPON_GRENADELAUNCHER\"", "\"WEAPON_RAILGUN\"", "\"VEHICLE_WEAPON_MINE\"", "\"VEHICLE_WEAPON_MINE_KINETIC\"", "\"VEHICLE_WEAPON_MINE_EMP\"", "\"VEHICLE_WEAPON_MINE_SPIKE\"", 
"\"VEHICLE_WEAPON_MINE_SLICK\"", "\"VEHICLE_WEAPON_MINE_TAR\"", "\"WEAPON_PROXMINE\""/*, "\"WEAPON_FLAREGUN\"", "\"WEAPON_RAYPISTOL\""*/ };
int VehCurrWeaponIndex = 0;
bool VehCurrWeaponChanged = true;

// 警察通缉等级
const std::vector<std::string> WEAPONS_COPALARM_CAPTIONS{ "1 星", "2 星 或 以下", "3 星 或 以下", "4 星 或 以下", "5 星 或 以下", "始终总是" };
const int WEAPONS_COPALARM_VALUES[] = { 1, 2, 3, 4, 5, 6 };
int CopAlarmIndex = 1;
bool CopAlarmChanged = true;

// 快速射击速度
const std::vector<std::string> WEAPONS_RAPIDFIRE_CAPTIONS{ "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "默认" };
const int WEAPONS_RAPIDFIRE_VALUES[] = { 40, 35, 30, 25, 20, 15, 10, 5, -1 };
int RapidFireIndex = 8;
bool RapidFireChanged = true;

// 切换狙击步枪的视野模式
const std::vector<std::string> WEAPONS_SNIPERVISION_CAPTIONS{ "关", "启用快捷键", "开启夜视", "开启热成像" };
int SniperVisionIndex = 0;
bool SniperVisionChanged = true;

// 强力拳击强度
int PowerPunchIndex = 2;
bool PowerPunchChanged = true;

// 开火模式
const std::vector<std::string> WEAPONS_FIREMODE_CAPTIONS{ "默认", "手动 单点射", "半自动 5连发", "全自动 5连发" };
int WeaponsFireModeIndex = 0;
bool WeaponsFireModeChanged = true;

// 无准星
const std::vector<std::string> WEAPONS_NORETICLE_CAPTIONS{ "关", "始终总是", "仅限第一人称模式" };
int WeaponsNoReticle = 0;
bool WeaponsNoReticleChanged = true;

// 自动加载已保存的武器
const std::vector<std::string> WEAPONS_SAVED_LOAD_CAPTIONS{ "关", "添加到武器库", "仅限已保存武器" };
int WeaponsSavedLoad = 0;
bool WeaponsSavedLoadChanged = true;

/* 开始重力枪相关代码 */

// 获取重力枪实体坐标的新方法 —— 来自 ScripthookV.Net
Vector3 RotationToDirection(Vector3* rot){
	float radiansZ = rot->z * 0.0174532925f;
	float radiansX = rot->x * 0.0174532925f;
	float num = std::abs((float) std::cos((double) radiansX));
	Vector3 dir;

	dir.x = (float) ((double) ((float) (-(float) std::sin((double) radiansZ))) * (double) num);
	dir.y = (float) ((double) ((float) std::cos((double) radiansZ)) * (double) num);
	dir.z = (float) std::sin((double) radiansX);

	return dir;
}

Vector3 add(Vector3* vectorA, Vector3* vectorB){
	Vector3 result;

	result.x = vectorA->x;
	result.y = vectorA->y;
	result.z = vectorA->z;
	result.x += vectorB->x;
	result.y += vectorB->y;
	result.z += vectorB->z;

	return result;
}

Vector3 DistanceFromCam(float distance){
	Vector3 camPosition = CAM::GET_GAMEPLAY_CAM_COORD();
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(0);
	Vector3 dir = RotationToDirection(&rot);

	dir.x *= distance;
	dir.y *= distance;
	dir.z *= distance;

	Vector3 inworld = add(&camPosition, &dir);

	return inworld;
}

// 基于距离和相机旋转获取方向偏移量
Vector3 DirectionOffsetFromCam(float distance){
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(0);
	Vector3 dir = RotationToDirection(&rot);

	dir.x *= distance;
	dir.y *= distance;
	dir.z *= distance;

	return dir;
}

void VectorToFloat(Vector3 unk, float *Out){
	Out[0] = unk.x;
	Out[1] = unk.y;
	Out[2] = unk.z;
}

void RequestControlEntity(Entity entity) // 需要此功能以便我们可以拾取道具/行人。即使在单人模式（SP）中也需要，尽管这是一个网络（NETWORK）原生函数。
{
	int tick = 0;

	while(!NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(entity) && tick <= 12){
		NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(entity);
		tick++;
	}
}
/* 结束重力枪相关代码 */

void fire_mode_hotkey() {
	WeaponsFireModeIndex = WeaponsFireModeIndex + 1;
	if (WeaponsFireModeIndex > 3) WeaponsFireModeIndex = 0; // 1
	if (WeaponsFireModeIndex == 0) set_status_text("默认");
	if (WeaponsFireModeIndex == 1) set_status_text("手动 单点射");
	if (WeaponsFireModeIndex == 2) set_status_text("半自动 5连发");
	if (WeaponsFireModeIndex == 3) set_status_text("全自动 5连发");
}

void onchange_knuckle_appearance(int value, SelectFromListMenuItem* source){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_KNUCKLE");

	int i = 0;

	Hash hashToApply = 0;
	for each (std::string componentName in VALUES_ATTACH_KNUCKLES){
		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(playerPed, weapHash, componentHash);

		if(i == value){
			hashToApply = componentHash;
		}

		i++;
	}

	if(hashToApply != 0){
		WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, weapHash, hashToApply);
	}
}

int get_current_knuckle_appearance(){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_KNUCKLE");

	int i = 0;
	for each (std::string componentName in VALUES_ATTACH_KNUCKLES){
		if(i == 0){
			continue;
		}

		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		if(WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, componentHash)){
			return i;
		}

		i++;
	}

	return 0;
}

void onchange_switchblade_appearance(int value, SelectFromListMenuItem* source){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_SWITCHBLADE");

	int i = 0;

	Hash hashToApply = 0;
	for each (std::string componentName in VALUES_ATTACH_SWITCHBLADE){
		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(playerPed, weapHash, componentHash);

		if(i == value){
			hashToApply = componentHash;
		}

		i++;
	}

	if(hashToApply != 0){
		WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, weapHash, hashToApply);
	}
}

int get_current_switchblade_appearance(){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_SWITCHBLADE");

	int i = 0;
	for each (std::string componentName in VALUES_ATTACH_SWITCHBLADE){
		if(i == 0){
			continue;
		}

		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		if(WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, componentHash)){
			return i;
		}

		i++;
	}

	return 0;
}

void onchange_revolver_appearance(int value, SelectFromListMenuItem* source){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_REVOLVER");

	int i = 0;

	Hash hashToApply = 0;
	for each (std::string componentName in VALUES_ATTACH_REVOLVER){
		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(playerPed, weapHash, componentHash);

		if(i == value){
			hashToApply = componentHash;
		}

		i++;
	}

	if(hashToApply != 0){
		WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, weapHash, hashToApply);
	}
}

void give_all_weapons_hotkey() {
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	for (int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++){
		for (int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++){
			char *weaponName = (char*)VOV_WEAPON_VALUES[a].at(b).c_str();
			Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
			int clipMax = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weaponHash, true); clipMax = min(clipMax, 250);
			WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, clipMax * 2, false, false);
		}
	}
	// 降落伞
	WEAPON::GIVE_WEAPON_TO_PED(playerPed, PARACHUTE_ID, 1, false, false);
	PLAYER::SET_PLAYER_HAS_RESERVE_PARACHUTE(playerPed);

	set_status_text("所有的武器已成功添加！");
}

void add_all_weapons_attachments(Ped choice) {
	for (int a = 0; a < WEAPONTYPES_MOD.size(); a++) {
		for (int b = 0; b < VOV_WEAPONMOD_VALUES[a].size(); b++) {
			char *weaponName = (char *)WEAPONTYPES_MOD.at(a).c_str(), *compName = (char *)VOV_WEAPONMOD_VALUES[a].at(b).c_str();
			Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName), compHash = GAMEPLAY::GET_HASH_KEY(compName);
			if (!WEAPON::HAS_PED_GOT_WEAPON(choice, weaponHash, 0)) {
				break;
			}

			if (strcmp(weaponName, "WEAPON_SMG") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_ASSAULTRIFLE") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_CARBINERIFLE") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_HEAVYSNIPER") == 0) {
				break;
			}
			if (strcmp(weaponName, "WEAPON_COMBATPDW") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_COMPACTRIFLE") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_HEAVYSHOTGUN") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_MACHINEPISTOL") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_REVOLVER") == 0) {
				break;
			}
			if (strcmp(weaponName, "WEAPON_SPECIALCARBINE") == 0 && b == 0) {
				continue;
			}
			if (strcmp(weaponName, "WEAPON_SWITCHBLADE") == 0) {
				break;
			}

			if (WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(choice, weaponHash, compHash)) {
				continue;
			}

			WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(choice, weaponHash, compHash);
		}
	}

	for (int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++) {
		for (int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++) {
			char* weaponName = (char*)VOV_WEAPON_VALUES[a].at(b).c_str();
			Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
			if (WEAPON::HAS_PED_GOT_WEAPON(choice, weaponHash, FALSE)) {
				WEAPON::GIVE_WEAPON_TO_PED(choice, weaponHash, 10000, false, false);
			}
		}
	}

	set_status_text("所有武器装备的配件，\n已添加到现有武器中！");
}

void load_saved_weapons() {
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	ENTDatabase* database = get_database();
	std::vector<SavedWeaponDBRow*> savedWeapon = database->get_saved_weapon();

	if (NPC_RAGDOLL_VALUES[WeaponsSavedLoad] == 2) WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, false);

	WAIT(200);

	for each (SavedWeaponDBRow * sv in savedWeapon)
	{
		int clipMax = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, sv->weapon, true); clipMax = min(clipMax, 250);
		if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, sv->weapon, 0)) {
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, sv->weapon);
			WEAPON::GIVE_WEAPON_TO_PED(playerPed, sv->weapon, clipMax * 2, false, false);
		}
		else WEAPON::GIVE_WEAPON_TO_PED(playerPed, sv->weapon, clipMax * 2, false, false);

		if (sv->comp0 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp0);
		if (sv->comp1 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp1);
		if (sv->comp2 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp2);
		if (sv->comp3 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp3);
		if (sv->comp4 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp4);
		if (sv->comp5 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp5);
		if (sv->comp6 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, sv->weapon, sv->comp6);
		if (sv->w_tint != -1) WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, sv->weapon, sv->w_tint);

		int maxAmmo = 0;
		WEAPON::GET_MAX_AMMO(playerPed, sv->weapon, &maxAmmo);
		int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, sv->weapon, false);
		WEAPON::SET_AMMO_IN_CLIP(playerPed, sv->weapon, maxClipAmmo);
		WEAPON::SET_PED_AMMO(playerPed, sv->weapon, maxAmmo);

		set_status_text("已装备保存的武器！");
	}

	for (std::vector<SavedWeaponDBRow*>::iterator it = savedWeapon.begin(); it != savedWeapon.end(); ++it)
	{
		delete (*it);
	}
	savedWeapon.clear();
}

int get_current_revolver_appearance(){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weapHash = GAMEPLAY::GET_HASH_KEY("WEAPON_REVOLVER");

	int i = 0;
	for each (std::string componentName in VALUES_ATTACH_REVOLVER){
		if(i == 0){
			continue;
		}

		DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

		if(WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, componentHash)){
			return i;
		}

		i++;
	}

	return 0;
}

bool process_individual_weapon_menu(int weaponIndex){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	
	lastSelectedWeapon = weaponIndex;

	std::string label = VOV_WEAPON_CAPTIONS[lastSelectedWeaponCategory].at(weaponIndex);
	std::string label_caption = get_weapon_label_with_fallback(label);

	if(label_caption.compare("手枪 .50") == 0){
		label_caption = "手枪 .50"; // 菜单标题无法处理符号
	}

	std::string value = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(weaponIndex);
	std::vector<MenuItem<int>*> menuItems;

	std::string weaponValue = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(weaponIndex);
	char *weaponChar = (char*) weaponValue.c_str();
	int thisWeaponHash = GAMEPLAY::GET_HASH_KEY(weaponChar);
	bool isEquipped = (WEAPON::HAS_PED_GOT_WEAPON(playerPed, GAMEPLAY::GET_HASH_KEY(weaponChar), 0) ? true : false);

	WEAPON::SET_CURRENT_PED_WEAPON(playerPed, thisWeaponHash, true);

	FunctionDrivenToggleMenuItem<int> *equipItem = new FunctionDrivenToggleMenuItem<int>();
	std::stringstream ss;
	ss << "装备 " << label_caption << "?";
	equipItem->caption = ss.str();
	equipItem->value = 1;
	equipItem->getter_call = is_weapon_equipped;
	equipItem->setter_call = set_weapon_equipped;
	equipItem->extra_arguments.push_back(lastSelectedWeaponCategory);
	equipItem->extra_arguments.push_back(weaponIndex);
	menuItems.push_back(equipItem);

	if(isEquipped){

		Ped playerPed = PLAYER::PLAYER_PED_ID();
		std::string weaponValue = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
		char *weaponChar = (char*) weaponValue.c_str();
		int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);
		int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weapHash, false);

		if(maxClipAmmo > 0){
			MenuItem<int> *giveClipItem = new MenuItem<int>();
			giveClipItem->caption = "补充弹匣";
			giveClipItem->value = 2;
			giveClipItem->isLeaf = true;
			giveClipItem->onConfirmFunction = give_weapon_clip;
			menuItems.push_back(giveClipItem);

			MenuItem<int> *fillAmmoItem = new MenuItem<int>();
			fillAmmoItem->caption = "补充弹药";
			fillAmmoItem->value = 3;
			fillAmmoItem->isLeaf = true;
			fillAmmoItem->onConfirmFunction = fill_weapon_ammo;
			menuItems.push_back(fillAmmoItem);
		}

		int moddableIndex = -1;
		for(int i = 0; i < WEAPONTYPES_MOD.size(); i++){
			if(weaponValue.compare(WEAPONTYPES_MOD.at(i)) == 0){
				moddableIndex = i;
				break;
			}
		}

		if(moddableIndex != -1){
			std::vector<std::string> modCaptions = VOV_WEAPONMOD_CAPTIONS[moddableIndex];
			for(int i = 0; i < modCaptions.size(); i++){
				FunctionDrivenToggleMenuItem<int> *item = new FunctionDrivenToggleMenuItem<int>();
				std::string label_caption = modCaptions.at(i);
				item->caption = UI::_GET_LABEL_TEXT(&label_caption[0]);
				item->getter_call = is_weaponmod_equipped;
				item->setter_call = set_weaponmod_equipped;
				item->extra_arguments.push_back(lastSelectedWeaponCategory);
				item->extra_arguments.push_back(weaponIndex);
				item->extra_arguments.push_back(moddableIndex);
				item->extra_arguments.push_back(i);
				menuItems.push_back(item);
			}
		}

		if(strcmp(weaponChar, "WEAPON_KNUCKLE") == 0){
			SelectFromListMenuItem *listItem = new SelectFromListMenuItem(CAPTIONS_ATTACH_KNUCKLES, onchange_knuckle_appearance);
			listItem->wrap = false;
			listItem->caption = "选择皮肤";
			listItem->value = get_current_knuckle_appearance();
			menuItems.push_back(listItem);
		}

		if(strcmp(weaponChar, "WEAPON_SWITCHBLADE") == 0){
			SelectFromListMenuItem *listItem = new SelectFromListMenuItem(CAPTIONS_ATTACH_SWITCHBLADE, onchange_switchblade_appearance);
			listItem->wrap = false;
			listItem->caption = "选择皮肤";
			listItem->value = get_current_switchblade_appearance();
			menuItems.push_back(listItem);
		}

		if(strcmp(weaponChar, "WEAPON_REVOLVER") == 0){
			SelectFromListMenuItem *listItem = new SelectFromListMenuItem(CAPTIONS_ATTACH_REVOLVER, onchange_revolver_appearance);
			listItem->wrap = false;
			listItem->caption = "选择皮肤";
			listItem->value = get_current_revolver_appearance();
			menuItems.push_back(listItem);
		}

		int tintableIndex = -1;
		for(int i = 0; i < WEAPONTYPES_TINT.size(); i++){
			if(weaponValue.compare(WEAPONTYPES_TINT.at(i)) == 0){
				tintableIndex = i;
				break;
			}
		}

		if(tintableIndex != -1){
			MenuItem<int> *tintItem = new MenuItem<int>();
			tintItem->caption = "武器涂装颜色";
			tintItem->value = 4;
			tintItem->isLeaf = false;
			tintItem->onConfirmFunction = onconfirm_open_tint_menu;
			menuItems.push_back(tintItem);

			/* 返回空菜单 - 需要改进！

			MenuItem<int> *LiveryTintItem = new MenuItem<int>();
			LiveryTintItem->caption = "Weapon Livery Colours";
			LiveryTintItem->value = 5;
			LiveryTintItem->isLeaf = false;
			LiveryTintItem->onConfirmFunction = onconfirm_open_tint_menu_colour;
			menuItems.push_back(LiveryTintItem);*/
		}
	}

	draw_generic_menu<int>(menuItems, 0, label_caption, NULL, NULL, NULL, weapon_reequip_interrupt);

	return false;
}

bool weapon_reequip_interrupt(){
	return redrawWeaponMenuAfterEquipChange;
}

bool onconfirm_weapon_in_category(MenuItem<int> choice){
	do{
		redrawWeaponMenuAfterEquipChange = false;
		process_individual_weapon_menu(choice.value);
	}
	while(redrawWeaponMenuAfterEquipChange);

	return false;
}

bool process_weapons_in_category_menu(int category){
	lastSelectedWeaponCategory = category;
	std::vector<MenuItem<int>*> menuItems;

	Ped playerPed = PLAYER::PLAYER_PED_ID();
	int weaponSelectionIndex = 0;
	int current = WEAPON::GET_SELECTED_PED_WEAPON(playerPed);

	for(int i = 0; i < VOV_WEAPON_CAPTIONS[category].size(); i++){
		MenuItem<int> *item = new MenuItem<int>();
		std::string label = VOV_WEAPON_CAPTIONS[category].at(i);
		std::string label_caption = get_weapon_label_with_fallback(label);
		item->caption = label_caption;

		const char* value = VOV_WEAPON_VALUES[category].at(i).c_str();
		if(weaponSelectionIndex == 0 && GAMEPLAY::GET_HASH_KEY((char*) value) == current){
			weaponSelectionIndex = i;
		}

		item->value = i;
		item->isLeaf = false;
		menuItems.push_back(item);
	}

	return draw_generic_menu<int>(menuItems, &weaponSelectionIndex, MENU_WEAPON_CATEGORIES[category], onconfirm_weapon_in_category, NULL, NULL);
}

bool onconfirm_weaponlist_menu(MenuItem<int> choice){
	process_weapons_in_category_menu(choice.value);
	return false;
}

bool process_weaponlist_menu(){
	std::vector<MenuItem<int>*> menuItems;

	equip_ped = PLAYER::PLAYER_PED_ID();
	int weaponSelectionIndex = 0;
	int current = WEAPON::GET_SELECTED_PED_WEAPON(equip_ped);

	for(int i = 0; i < MENU_WEAPON_CATEGORIES.size(); i++){
		MenuItem<int> *item = new MenuItem<int>();
		item->caption = MENU_WEAPON_CATEGORIES[i];
		item->value = i;
		item->isLeaf = false;
		menuItems.push_back(item);

		if(weaponSelectionIndex == 0){
			for(int j = 0; j < VOV_WEAPON_VALUES[i].size(); j++){
				const char* value = VOV_WEAPON_VALUES[i].at(j).c_str();
				if(GAMEPLAY::GET_HASH_KEY((char*) value) == current){
					weaponSelectionIndex = i;
					break;
				}
			}
		}
	}

	return draw_generic_menu<int>(menuItems, &weaponSelectionIndex, "武器类型", onconfirm_weaponlist_menu, NULL, NULL);
}

void onchange_cop_armed_index(int value, SelectFromListMenuItem* source){ 
	CopCurrArmedIndex = value;
	CopCurrArmedChanged = true;
}

void onchange_cop_alarm_index(int value, SelectFromListMenuItem* source){
	CopAlarmIndex = value;
	CopAlarmChanged = true;
}

bool onconfirm_coparmed_menu(MenuItem<int> choice)
{
	return false;
}

void process_copweapon_menu(){
	const std::string caption = "警察武器选项";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	ToggleMenuItem<int>* toggleItem;

	int i = 0;

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureCopArmedWith;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "当玩家未持武器仅持近战";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePlayerMelee;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "警察遭到射击则用枪反击";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureSwitchWeaponIfDanger;
	menuItems.push_back(toggleItem);

	// 使用本地化标题替换警察装备的模型名显示，保持索引与值不变
	std::vector<std::string> copLocalized = localize_weapon_models(WEAPONS_COPARMED_CAPTIONS);
	listItem = new SelectFromListMenuItem(copLocalized, onchange_cop_armed_index);
	listItem->wrap = false;
	listItem->caption = "警察的装备";
	listItem->value = CopCurrArmedIndex;
	menuItems.push_back(listItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "包括军队";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureArmyMelee;
	menuItems.push_back(toggleItem);
	
	listItem = new SelectFromListMenuItem(WEAPONS_COPALARM_CAPTIONS, onchange_cop_alarm_index);
	listItem->wrap = false;
	listItem->caption = "当在";
	listItem->value = CopAlarmIndex;
	menuItems.push_back(listItem);
	
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "不移动则被逮捕";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureDetainedIfNotMove;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexCopArmed, caption, onconfirm_coparmed_menu, NULL, NULL);
}

void onchange_chance_police_calling_index(int value, SelectFromListMenuItem* source){
	ChancePoliceCallingIndex = value;
	ChancePoliceCallingChanged = true;
}

void onchange_chance_attacking_you_index(int value, SelectFromListMenuItem* source){
	ChanceAttackingYouIndex = value;
	ChanceAttackingYouChanged = true;
}

void onchange_sniper_vision_modifier(int value, SelectFromListMenuItem* source){
	SniperVisionIndex = value;
	SniperVisionChanged = true;
}

void onchange_power_punch_index(int value, SelectFromListMenuItem* source) {
	PowerPunchIndex = value;
	PowerPunchChanged = true;
}

void onchange_weapons_firemode_modifier(int value, SelectFromListMenuItem* source) {
	WeaponsFireModeIndex = value;
	WeaponsFireModeChanged = true;
}

void onchange_weapons_rapidfire_modifier(int value, SelectFromListMenuItem* source) {
	RapidFireIndex = value;
	RapidFireChanged = true;
}

void onchange_weap_strobe_index(int value, SelectFromListMenuItem* source) {
	WeapStrobeIndexN = value;
	WeapStrobeChanged = true;
}

void onchange_weap_flashdist_index(int value, SelectFromListMenuItem* source) {
	WeapFlashDistIndex = value;
	WeapFlashDistChanged = true;
}

void onchange_vehicle_weapon_modifier(int value, SelectFromListMenuItem* source) {
	VehCurrWeaponIndex = value;
	VehCurrWeaponChanged = true;
}

void onchange_weapon_no_reticle_modifier(int value, SelectFromListMenuItem* source) {
	WeaponsNoReticle = value;
	WeaponsNoReticleChanged = true;
}

void onchange_weapon_load_saved_modifier(int value, SelectFromListMenuItem* source) {
	WeaponsSavedLoad = value;
	WeaponsSavedLoadChanged = true;
}

///////////////////////////////// 切换狙击步枪的瞄准镜模式 /////////////////////////////////
void sniper_vision_toggle()
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	if ((WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_SNIPERRIFLE") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER") ||
		WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_REMOTESNIPER") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER_MK2") ||
		WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_MARKSMANRIFLE") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_MARKSMANRIFLE_MK2")) &&
		PED::GET_PED_CONFIG_FLAG(playerPed, 78, 1) && !PED::GET_PED_CONFIG_FLAG(playerPed, 58, 1) && !SCRIPT::HAS_SCRIPT_LOADED("carsteal2"))
	{
		if (WORLD_GRAVITY_LEVEL_VALUES[SniperVisionIndex] == 1) {
			vision_toggle = vision_toggle + 1;
			if (vision_toggle == 3) vision_toggle = 0;

			if (vision_toggle == 0) {
				GRAPHICS::SET_NIGHTVISION(false);
				GRAPHICS::SET_SEETHROUGH(false);
			}
			if (vision_toggle == 1) {
				GRAPHICS::SET_NIGHTVISION(true);
				GRAPHICS::SET_SEETHROUGH(false);
			}
			if (vision_toggle == 2) {
				GRAPHICS::SET_NIGHTVISION(false);
				GRAPHICS::SET_SEETHROUGH(true);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////

bool onconfirm_pedagainstweapons_menu(MenuItem<int> choice)
{
	return false;
}

void process_pedagainstweapons_menu(){
	const std::string caption = "行人不喜欢武器";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	ToggleMenuItem<int>* toggleItem;

	int i = 0;

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePedAgainstWeapons;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "包括近战武器";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureAgainstMeleeWeapons;
	menuItems.push_back(toggleItem);
	
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "行人攻击";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePedAgainst;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "司机攻击";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureDriverAgainst;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "警察攻击";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePoliceAgainst;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(WEAPONS_CHANCEPOLICECALLING_CAPTIONS, onchange_chance_police_calling_index);
	listItem->wrap = false;
	listItem->caption = "看见枪报警的概率";
	listItem->value = ChancePoliceCallingIndex;
	menuItems.push_back(listItem);

	listItem = new SelectFromListMenuItem(WEAPONS_CHANCEPOLICECALLING_CAPTIONS, onchange_chance_attacking_you_index);
	listItem->wrap = false;
	listItem->caption = "攻击你的概率";
	listItem->value = ChanceAttackingYouIndex;
	menuItems.push_back(listItem);
		
	draw_generic_menu<int>(menuItems, &activeLineIndexPedAgainstWeapons, caption, onconfirm_pedagainstweapons_menu, NULL, NULL);
}

// 已保存的武器
bool spawn_saved_weapon(int slot, std::string caption)
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	ENTDatabase* database = get_database();
	std::vector<SavedWeaponDBRow*> savedWeapons = database->get_saved_weapon(slot);
	SavedWeaponDBRow* savedWeapon = savedWeapons.at(0);
	
	int clipMax = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, savedWeapon->weapon, true); clipMax = min(clipMax, 250);
	if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, savedWeapon->weapon, 0)) {
		WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, savedWeapon->weapon);
		WEAPON::GIVE_WEAPON_TO_PED(playerPed, savedWeapon->weapon, clipMax * 2, false, true);
	} else WEAPON::GIVE_WEAPON_TO_PED(playerPed, savedWeapon->weapon, clipMax * 2, false, true);
		
	if (savedWeapon->comp0 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp0);
	if (savedWeapon->comp1 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp1);
	if (savedWeapon->comp2 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp2);
	if (savedWeapon->comp3 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp3);
	if (savedWeapon->comp4 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp4);
	if (savedWeapon->comp5 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp5);
	if (savedWeapon->comp6 != -1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, savedWeapon->weapon, savedWeapon->comp6);
	if (savedWeapon->w_tint != -1) WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, savedWeapon->weapon, savedWeapon->w_tint);

	WEAPON::SET_CURRENT_PED_WEAPON(playerPed, savedWeapon->weapon, 1);
	
	// 给予所有已装备武器的弹药
	for (int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++) {
		for (int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++) {
			char* weaponName = (char*)VOV_WEAPON_VALUES[a].at(b).c_str();
			Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
			if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, FALSE)) {
				WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, 10000, false, false);
			}
		}
	}
	//
	int maxAmmo = 0;
	WEAPON::GET_MAX_AMMO(playerPed, savedWeapon->weapon, &maxAmmo);
	int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, savedWeapon->weapon, false);

	WEAPON::SET_AMMO_IN_CLIP(playerPed, savedWeapon->weapon, maxClipAmmo);
	WEAPON::SET_PED_AMMO(playerPed, savedWeapon->weapon, maxAmmo);

	WEAPON::SET_PED_CURRENT_WEAPON_VISIBLE(playerPed, true, false, 1, 1);
	
	for (std::vector<SavedWeaponDBRow*>::iterator it = savedWeapons.begin(); it != savedWeapons.end(); ++it)
	{
		delete (*it);
	}
	savedWeapons.clear();

	return false;
}

void save_current_weapon(int slot)
{
	BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID());
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	if (!WEAPON::IS_PED_ARMED(playerPed, 7) && WEAPON::GET_SELECTED_PED_WEAPON(playerPed) != GAMEPLAY::GET_HASH_KEY("WEAPON_PETROLCAN")) CONTROLS::_SET_CONTROL_NORMAL(0, 37, 1);

	if (bPlayerExists)
	{
		std::ostringstream ss;
		if (slot != -1)
		{
			ss << activeSavedWeaponSlotName;
		}
		else
		{
			ss << "新建武器存档 " << (lastKnownSavedWeaponCount + 1);
		}

		keyboard_on_screen_already = true;
		curr_message = "输入保存名称"; // 保存当前武器
		auto existingText = ss.str();
		std::string result = show_keyboard("手动输入名称", (char*)existingText.c_str());
		if (!result.empty())
		{
			ENTDatabase* database = get_database();

			if (database->save_weapon(playerPed, result, slot))
			{
				activeSavedWeaponSlotName = result;
				set_status_text("武器保存成功了！");
			}
			else
			{
				set_status_text("武器保存失败了！");
			}
		}
	}
}

bool weapon_save_menu_interrupt()
{
	if (WeaponSaveMenuInterrupt)
	{
		WeaponSaveMenuInterrupt = false;
		return true;
	}
	return false;
}

bool weapon_save_slot_menu_interrupt()
{
	if (WeaponSaveSlotMenuInterrupt)
	{
		WeaponSaveSlotMenuInterrupt = false;
		return true;
	}
	return false;
}

bool onconfirm_weapon_save_slot_menu(MenuItem<int> choice)
{
	switch (choice.value)
	{
	case 1: //生成
		spawn_saved_weapon(activeSavedWeaponIndex, activeSavedWeaponSlotName);
		break;
	case 2: //覆盖
	{
		save_current_weapon(activeSavedWeaponIndex);
		requireRefreshOfWeaponSaveSlots = true;
		requireRefreshOfWeaponSlotMenu = true;
		WeaponSaveSlotMenuInterrupt = true;
		WeaponSaveMenuInterrupt = true;
	}
	break;
	case 3: //重命名
	{
		keyboard_on_screen_already = true;
		curr_message = "输入新的名称"; // 重命名已保存的武器
		std::string result = show_keyboard("手动输入名称", (char*)activeSavedWeaponSlotName.c_str());
		if (!result.empty())
		{
			ENTDatabase* database = get_database();
			database->rename_saved_weapon(result, activeSavedWeaponIndex);
			activeSavedWeaponSlotName = result;
		}
		requireRefreshOfWeaponSaveSlots = true;
		requireRefreshOfWeaponSlotMenu = true;
		WeaponSaveSlotMenuInterrupt = true;
		WeaponSaveMenuInterrupt = true;
	}
	break;
	case 4: //删除
	{
		ENTDatabase* database = get_database();
		database->delete_saved_weapon(activeSavedWeaponIndex);
		requireRefreshOfWeaponSaveSlotMenu = false;
		requireRefreshOfWeaponSaveSlots = true;
		WeaponSaveSlotMenuInterrupt = true;
		WeaponSaveMenuInterrupt = true;
	}
	break;
	}
	return false;
}

bool onconfirm_weapon_save_menu(MenuItem<int> choice)
{
	if (choice.value == -1)
	{
		save_current_weapon(-1);
		requireRefreshOfWeaponSaveSlots = true;
		WeaponSaveMenuInterrupt = true;
		return false;
	}

	activeSavedWeaponIndex = choice.value;
	activeSavedWeaponSlotName = choice.caption;
	return process_weapon_save_slot_menu(choice.value);
}

bool process_saveweapon_menu()
{
	do
	{
		WeaponSaveMenuInterrupt = false;
		requireRefreshOfWeaponSaveSlotMenu = false;
		requireRefreshOfWeaponSaveSlots = false;

		ENTDatabase* database = get_database();
		std::vector<SavedWeaponDBRow*> savedWeapon = database->get_saved_weapon();

		lastKnownSavedWeaponCount = savedWeapon.size();

		std::vector<MenuItem<int>*> menuItems;

		MenuItem<int>* item = new MenuItem<int>();
		item->isLeaf = true;
		item->value = -1;
		item->caption = "创建新的武器存档";
		menuItems.push_back(item);

		for each (SavedWeaponDBRow * sv in savedWeapon)
		{
			MenuItem<int>* item = new MenuItem<int>();
			item->isLeaf = false;
			item->value = sv->rowID;
			item->caption = sv->saveName;
			menuItems.push_back(item);
		}

		draw_generic_menu<int>(menuItems, 0, "保存的武器", onconfirm_weapon_save_menu, NULL, NULL, weapon_save_menu_interrupt);

		for (std::vector<SavedWeaponDBRow*>::iterator it = savedWeapon.begin(); it != savedWeapon.end(); ++it)
		{
			delete (*it);
		}
		savedWeapon.clear();
	} while (requireRefreshOfWeaponSaveSlots);

	return false;
}

bool process_weapon_save_slot_menu(int slot)
{
	do
	{
		WeaponSaveSlotMenuInterrupt = false;
		requireRefreshOfWeaponSaveSlotMenu = false;

		std::vector<MenuItem<int>*> menuItems;

		MenuItem<int>* item = new MenuItem<int>();
		item->isLeaf = true;
		item->value = 1;
		item->caption = "装备";
		menuItems.push_back(item);

		item = new MenuItem<int>();
		item->isLeaf = true;
		item->value = 2;
		item->caption = "用当前内容覆盖";
		menuItems.push_back(item);

		item = new MenuItem<int>();
		item->isLeaf = true;
		item->value = 3;
		item->caption = "重命名";
		menuItems.push_back(item);

		item = new MenuItem<int>();
		item->isLeaf = true;
		item->value = 4;
		item->caption = "删除";
		menuItems.push_back(item);

		draw_generic_menu<int>(menuItems, 0, activeSavedWeaponSlotName, onconfirm_weapon_save_slot_menu, NULL, NULL, weapon_save_slot_menu_interrupt);
	} while (requireRefreshOfWeaponSaveSlotMenu);
	return false;
}
// 保存武器结束

bool onconfirm_weapon_menu(MenuItem<int> choice){
	// 公共变量
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	
	switch(activeLineIndexWeapon){
		case 0:
			give_all_weapons_hotkey();
			break;
		case 2:
			WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, false);
			set_status_text("所有的武器已成功移除！");
			break;
		case 3:
			add_all_weapons_attachments(playerPed);
			break;
		case 5:
			for(int a = 0; a < WEAPONTYPES_MOD.size(); a++){
				for(int b = 0; b < VOV_WEAPONMOD_VALUES[a].size(); b++){
					char *weaponName = (char *) WEAPONTYPES_MOD.at(a).c_str(), *compName = (char *) VOV_WEAPONMOD_VALUES[a].at(b).c_str();
					Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName), compHash = GAMEPLAY::GET_HASH_KEY(compName);
					if(!WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, 0)){
						break;
					}

					if(strcmp(weaponName, "WEAPON_REVOLVER") == 0){
						break;
					}
					if(strcmp(weaponName, "WEAPON_SWITCHBLADE") == 0){
						break;
					}

					if(!WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weaponHash, compHash)){
						continue;
					}

					WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(playerPed, GAMEPLAY::GET_HASH_KEY(weaponName), GAMEPLAY::GET_HASH_KEY(compName));
				}
			}

			for(int a = 0; a < WEAPONTYPES_TINT.size(); a++){
				char *weaponName = (char *) WEAPONTYPES_TINT.at(a).c_str();
				Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
				if(!WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, 0)){
					continue;
				}

				WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, weaponHash, VALUES_TINT.at(0));
			}

			set_status_text("所有武器的配件和涂装, \n已从现有的武器中移除！");
			break;
		case 6:
			if (process_saveweapon_menu()) return false;
			break;
		case 8:
			for(int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++){
				for(int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++){
					char *weaponName = (char*) VOV_WEAPON_VALUES[a].at(b).c_str();
					Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
					if(WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, FALSE)){
						WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, 10000, false, false);
					}
				}
			}

			if(WEAPON::HAS_PED_GOT_WEAPON(playerPed, PARACHUTE_ID, FALSE)){
				PLAYER::SET_PLAYER_HAS_RESERVE_PARACHUTE(player);
			}

			set_status_text("所有弹药已补满！");
			break;
		case 9:
			for(int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++){
				for(int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++){
					char *weaponName = (char *) VOV_WEAPON_VALUES[a].at(b).c_str();
					WEAPON::SET_PED_AMMO(playerPed, GAMEPLAY::GET_HASH_KEY(weaponName), 0);
				}
			}

			// 降落伞
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, PARACHUTE_ID);

			set_status_text("所有弹药已清空！");
			break;
		case 10:
			process_weaponlist_menu();
			break;
		case 11:
		{
			keyboard_on_screen_already = true;
			curr_message = "输入武器的模型名称（例如：weapon_microsmg）"; // 装备武器
			std::string result = show_keyboard("手动输入名称", (char *) lastCustomWeapon.c_str());
			if(!result.empty()){
				result = trim(result);
				lastCustomWeapon = result;
				Hash weaponHash = GAMEPLAY::GET_HASH_KEY((char *) result.c_str());
				std::ostringstream ss;
				if(WEAPON::IS_WEAPON_VALID(weaponHash)){
					WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, 250, false, false);
					ss << result << " 已添加成功！";
				}
				else{
					ss << "~r~错误: 无法找到此武器 \"" << result << "\"";
				}
				set_status_text(ss.str());
			}
			break;
		}
		case 17:
			WEAPON::GIVE_WEAPON_TO_PED(playerPed, PARACHUTE_ID, 1, false, false);
			PLAYER::SET_PLAYER_HAS_RESERVE_PARACHUTE(player);

			set_status_text("降落伞已添加！");
			break;
		case 18:
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, PARACHUTE_ID);

			set_status_text("降落伞已移除！");
			break;
		case 27:
			process_copweapon_menu();
			break;
		case 28:
			process_pedagainstweapons_menu();
			break;
		case 39: // 屏幕准星显示
			process_weapons_crosshair_menu();
			break;
		case 42: // 丢弃当前装备武器
		{
			if (WEAPON::IS_PED_ARMED(playerPed, 7)) {
				Hash currentWeapon = WEAPON::GET_SELECTED_PED_WEAPON(playerPed);
				Hash unarmedHash = GAMEPLAY::GET_HASH_KEY("WEAPON_UNARMED");
				if (currentWeapon != unarmedHash) {
					int ammo = WEAPON::GET_AMMO_IN_PED_WEAPON(playerPed, currentWeapon);
					if (ammo < 0) ammo = 0;
					// 丢弃当前武器（包括特殊道具等），携带完整弹药
					WEAPON::SET_PED_DROPS_INVENTORY_WEAPON(playerPed, currentWeapon, 0.0f, -0.5f, 0.0f, ammo);
					// 可选等待，确保游戏生成掉落物
					WAIT(100);
					set_status_text("武器已丢弃！可以再次拾取！");
				}
			} else {
				set_status_text("~r~当前没有装备武器！");
			}
			break;
		}
		case 43: // 载具模型枪菜单
			process_vehicle_model_gun_menu();
			break;
		case 44: // 角色模型枪菜单
			process_ped_model_gun_menu();
			break;
		//case 36:
		//	if (AIMBOT_INCLUDED) process_aimbot_esp_menu();
		//	break;
	default:
		break;
	}
	return false;
}

bool process_weapon_menu(){
	int i = 0;

	const std::string caption = "武器选项";
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem* listItem;

	MenuItem<int> *item = new MenuItem<int>();
	item->caption = "添加所有武器装备";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "自动添加所有武器装备";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureGiveAllWeapons;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "移除所有武器装备";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "添加所有武器的配件";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "自动添加所有武器的配件";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureAddAllWeaponsAttachments;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "移除所有武器的配件和涂装";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "保存的武器";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	listItem = new SelectFromListMenuItem(WEAPONS_SAVED_LOAD_CAPTIONS, onchange_weapon_load_saved_modifier);
	listItem->wrap = false;
	listItem->caption = "装备保存武器";
	listItem->value = WeaponsSavedLoad;
	menuItems.push_back(listItem);

	item = new MenuItem<int>();
	item->caption = "补充所有弹药";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "清空所有弹药";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "单个武器添加";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "自定义添加武器";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	listItem = new SelectFromListMenuItem(WEAP_DMG_CAPTIONS, onchange_weap_dmg_modifier);
	listItem->wrap = false;
	listItem->caption = "武器伤害倍数";
	listItem->value = weapDmgModIndex;
	menuItems.push_back(listItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "无限弹药";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponInfiniteAmmo;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "无需换弹";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponNoReload;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(WEAPONS_NORETICLE_CAPTIONS, onchange_weapon_no_reticle_modifier);
	listItem->wrap = false;
	listItem->caption = "不显示准星";
	listItem->value = WeaponsNoReticle;
	menuItems.push_back(listItem);
	
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "死亡/被捕时丢弃武器";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureCopTakeWeapon;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "添加降落伞";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "移除降落伞";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "无限降落伞";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponInfiniteParachutes;
	toggleItem->toggleValueUpdated = &featureWeaponInfiniteParachutesUpdated;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "没有降落伞";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponNoParachutes;
	toggleItem->toggleValueUpdated = &featureWeaponNoParachutesUpdated;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "火焰弹药";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponFireAmmo;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "爆炸弹药";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponExplosiveAmmo;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "爆炸近战";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponExplosiveMelee;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "超级连环爆炸手雷";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponExplosiveGrenades;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "引力榴弹枪";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponVacuumGrenades;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	// 使用本地化标题替换载具武器的模型名显示，但保持索引与值不变
	std::vector<std::string> vehLocalized = localize_weapon_models(WEAPONS_VEHICLE_CAPTIONS);
	listItem = new SelectFromListMenuItem(vehLocalized, onchange_vehicle_weapon_modifier);
	listItem->wrap = false;
	listItem->caption = "载具装备武器";
	listItem->value = VehCurrWeaponIndex;
	menuItems.push_back(listItem);

	item = new MenuItem<int>();
	item->caption = "警察武器";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "行人不喜欢武器";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "重力枪";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureGravityGun;
	toggleItem->toggleValueUpdated = NULL;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(WEAPONS_SNIPERVISION_CAPTIONS, onchange_sniper_vision_modifier);
	listItem->wrap = false;
	listItem->caption = "切换狙击步枪开镜视角";
	listItem->value = SniperVisionIndex;
	menuItems.push_back(listItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "友军伤害";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureFriendlyFire;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "手部中弹时丢弃武器";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureDropWeapon;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "射击以解除NPC的武装";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureCanDisarmNPC;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "禁止拾取丢弃的武器";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePedNoWeaponDrop;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "快速射击";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureRapidFire;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(WEAPONS_RAPIDFIRE_CAPTIONS, onchange_weapons_rapidfire_modifier);
	listItem->wrap = false;
	listItem->caption = "快速射击速度";
	listItem->value = RapidFireIndex;
	menuItems.push_back(listItem);

	listItem = new SelectFromListMenuItem(WEAPONS_FIREMODE_CAPTIONS, onchange_weapons_firemode_modifier);
	listItem->wrap = false;
	listItem->caption = "射击模式";
	listItem->value = WeaponsFireModeIndex;
	menuItems.push_back(listItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "弹药用尽时丢弃武器";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureDropWeaponOutAmmo;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "屏幕显示准星";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	listItem = new SelectFromListMenuItem(FUEL_COLOURS_R_CAPTIONS, onchange_weap_strobe_index);
	listItem->wrap = false;
	listItem->caption = "手电筒闪烁";
	listItem->value = WeapStrobeIndexN;
	menuItems.push_back(listItem);

	listItem = new SelectFromListMenuItem(WEAP_DMG_CAPTIONS, onchange_weap_flashdist_index);
	listItem->wrap = false;
	listItem->caption = "手电筒亮度";
	listItem->value = WeapFlashDistIndex;
	menuItems.push_back(listItem);

	// 丢弃当前装备的武器（触发式功能选项）
	item = new MenuItem<int>();
	item->caption = "丢弃当前装备的武器";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	// 载具模型枪菜单
	item = new MenuItem<int>();
	item->caption = "载具模型枪";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	// 角色模型枪菜单
	item = new MenuItem<int>();
	item->caption = "角色模型枪";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	//if (AIMBOT_INCLUDED) {
	//	item = new MenuItem<int>();
	//	item->caption = "Aimbot ESP";
	//	item->value = i++;
	//	item->isLeaf = false;
	//	menuItems.push_back(item);
	//}

	return draw_generic_menu<int>(menuItems, &activeLineIndexWeapon, caption, onconfirm_weapon_menu, NULL, NULL);
}

void reset_weapon_globals(){
	activeLineIndexWeapon = 0;

	weapDmgModIndex = 0;

	CopCurrArmedIndex = 1;
	VehCurrWeaponIndex = 0;
	CopAlarmIndex = 1;

	WeaponsNoReticle = 0;
	WeaponsSavedLoad = 0;

	ChancePoliceCallingIndex = 5;
	ChanceAttackingYouIndex = 1;
	SniperVisionIndex = 0;
	PowerPunchIndex = 2;
	WeaponsFireModeIndex = 0;
	RapidFireIndex = 8;
	WeapStrobeIndexN = 0;
	WeapFlashDistIndex = 0;
	
	featureWeaponsCrosshair = false; // 屏幕准星默认关闭
	WeaponsCrosshairStyleIndex = 0; // 默认0 对应实线（1）
	WeaponsCrosshairColorIndex = 0; // 默认0 对应白色
	
	// 重置载具模型枪和角色模型枪相关变量
	featureVehicleModelGun = false;
	featureVehicleModelGunUpdated = false;
	featurePedModelGun = false;
	featurePedModelGunUpdated = false;
	VehicleModelGunCategoryIndex = 0;
	VehicleModelGunCategoryChanged = true;
	VehicleModelGunSpeedIndex = 4; // 默认正常速度（索引4=200.0f）
	VehicleModelGunSpeedChanged = true;
	featureVehicleModelGunInvincible = false;
	PedModelGunCategoryIndex = 0;
	PedModelGunCategoryChanged = true;
	PedModelGunSpeedIndex = 4; // 默认正常速度（索引4=200.0f）
	PedModelGunSpeedChanged = true;
	featurePedModelGunInvincible = false;

	activeLineIndexCopArmed = 0;
	activeLineIndexPedAgainstWeapons = 0;
	activeLineIndexPowerPunchWeapons = 0;
	
	featurePedAgainst = 
	featureDriverAgainst =
	featurePoliceAgainst =
	featurePunchFists =
	featurePlayerMelee = true;
	
	featureWeaponInfiniteAmmo =
		featureWeaponInfiniteParachutes =
		featureWeaponInfiniteParachutesUpdated =
		featureWeaponNoParachutes =
		featureWeaponNoParachutesUpdated =
		featureWeaponNoReload =
		featureCopTakeWeapon =
		featureWeaponFireAmmo =
		featureWeaponExplosiveAmmo =
		featureWeaponExplosiveMelee =
		featureWeaponExplosiveGrenades =
		featureWeaponVacuumGrenades =
		featureGiveAllWeapons =
		featureAddAllWeaponsAttachments =
		featureCopArmedWith =
		featureArmyMelee =
		featureDetainedIfNotMove =
		featurePedAgainstWeapons = 
		featureAgainstMeleeWeapons =
		featureFriendlyFire =
		featureRapidFire =
		featureDropWeapon = 
		featureDropWeaponOutAmmo =
		featureCanDisarmNPC =
		featurePedNoWeaponDrop =
		featurePowerPunch =
		featureSwitchWeaponIfDanger =
		featurePunchMeleeWeapons =
		featurePunchFireWeapons =
		featureGravityGun = false;

	shown_vacuum_message = false;
	shown_gravitygun_message = false;
}

void update_weapon_features(BOOL bPlayerExists, Player player){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	// 武器伤害修改器
	if(bPlayerExists){
		// 如果为默认值，则无需每帧设置
		if (!SCRIPT::HAS_SCRIPT_LOADED("wardrobe_sp")) {
			PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(player, WEAP_DMG_FLOAT[weapDmgModIndex]);
			PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(player, WEAP_DMG_FLOAT[weapDmgModIndex], 1); // R* 修改了这个原生函数。现在它最后需要一个布尔值。
			PLAYER::SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(player, WEAP_DMG_FLOAT[weapDmgModIndex]);
		}
	}

	// 车载武器
	if (VehCurrWeaponIndex > 0 && PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0)) { // WEAPONS_VEHICLE_VALUES[VehCurrWeaponIndex] > 0
		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		bool bSelect = IsKeyDown(KeyConfig::KEY_VEH_ROCKETS) || (CONTROLS::IS_CONTROL_PRESSED(2, controller_binds["KEY_VEH_ROCKETS"].first) && CONTROLS::IS_CONTROL_PRESSED(2, controller_binds["KEY_VEH_ROCKETS"].second)) || (CONTROLS::IS_CONTROL_PRESSED(2, 69) && !CONTROLS::IS_CONTROL_PRESSED(2, 70));

		if (bSelect && featureWeaponVehShootLastTime + 150 < GetTickCount() && PLAYER::IS_PLAYER_CONTROL_ON(player)) { // 150
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			Vector3 v0, v1;
			GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);

			char *currWeapon_v = new char[WEAPONS_VEHICLE_CAPTIONS[VehCurrWeaponIndex].length() + 1];
			strcpy(currWeapon_v, WEAPONS_VEHICLE_CAPTIONS[VehCurrWeaponIndex].c_str());
			Hash weaponAssetRocket = GAMEPLAY::GET_HASH_KEY(currWeapon_v);
			if (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket)) {
				WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
				while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket)) {
					WAIT(0);
				}
			}

			if (weaponAssetRocket != 1508567460 && weaponAssetRocket != 1007245390 && weaponAssetRocket != 1776356704 && weaponAssetRocket != 3647840364 && weaponAssetRocket != 1459276487 && weaponAssetRocket != 4097936288 && weaponAssetRocket != 2874559379) {
				Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.25f), v1.y + 1.25f, 0.1f);
				Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.25f), v1.y + 1.25f, 0.1f);
				Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y + 100.0f, 0.1f);
				Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y + 100.0f, 0.1f);
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z, coords0to.x, coords0to.y, coords0to.z, 250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z, coords1to.x, coords1to.y, coords1to.z, 250, 1, weaponAssetRocket, playerPed, 1, 0, -1.0);
			}
			else {
				Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.15f), -(v1.y + 0.25f), 0.1f);
				Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.15f), -(v1.y + 0.25f), 0.1f);
				Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y - 10.0f, 0.1f);
				Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y - 10.0f, 0.1f);
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z, coords0to.x, coords0to.y, coords0to.z, 25, 1, weaponAssetRocket, playerPed, 1, 0, -1.0); // 250
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z, coords1to.x, coords1to.y, coords1to.z, 25, 1, weaponAssetRocket, playerPed, 1, 0, -1.0); // 250
			}
			featureWeaponVehShootLastTime = GetTickCount();
		}
	}

	if (is_hotkey_held_drop_mine() && PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0) && PLAYER::IS_PLAYER_CONTROL_ON(player)) {
		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(veh), &v0, &v1);
		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -(v1.x + 0.15f), -(v1.y + 0.25f), 0.1f);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, (v1.x + 0.15f), -(v1.y + 0.25f), 0.1f);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, -v1.x, v1.y - 10.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(veh, v1.x, v1.y - 10.0f, 0.1f);
		Hash weaponAssetRocket = -1;
		if (GetKeyState('1') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE_KINETIC");
		if (GetKeyState('2') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE_SPIKE");
		if (GetKeyState('3') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE_EMP");
		if (GetKeyState('4') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE");
		if (GetKeyState('5') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE_SLICK");
		if (GetKeyState('6') & 0x8000) weaponAssetRocket = GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_MINE_TAR");

		if (weaponAssetRocket != -1 && !WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket)) {
			WEAPON::REQUEST_WEAPON_ASSET(weaponAssetRocket, 31, 0);
			while (!WEAPON::HAS_WEAPON_ASSET_LOADED(weaponAssetRocket)) WAIT(0);
		}
		if (featureWeaponVehShootLastTime + 75 < GetTickCount()) { // 150
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z, coords0to.x, coords0to.y, coords0to.z, 25, 1, weaponAssetRocket, PLAYER::PLAYER_PED_ID(), 1, 0, -1.0); // 250
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z, coords1to.x, coords1to.y, coords1to.z, 25, 1, weaponAssetRocket, PLAYER::PLAYER_PED_ID(), 1, 0, -1.0); // 250
			featureWeaponVehShootLastTime = GetTickCount();
		}
	}
	
	// 武器
	if(featureWeaponFireAmmo){
		if(bPlayerExists){
			GAMEPLAY::SET_FIRE_AMMO_THIS_FRAME(player);
		}
	}
	if(featureWeaponExplosiveAmmo){
		if(bPlayerExists){
			GAMEPLAY::SET_EXPLOSIVE_AMMO_THIS_FRAME(player);
		}
	}
	if(featureWeaponExplosiveMelee){
		if(bPlayerExists)
			GAMEPLAY::SET_EXPLOSIVE_MELEE_THIS_FRAME(player);
	}

	// 超级爆炸手榴弹 && 吸附手榴弹
	if (featureWeaponExplosiveGrenades || featureWeaponVacuumGrenades) {
		const int array_g = 1024;
		Object objects_g[array_g];
		int count_g = worldGetAllObjects(objects_g, array_g);
		for (int i = 0; i < count_g; i++) {
			// 超级爆炸手榴弹
			if (featureWeaponExplosiveGrenades) {
				Hash grenade = ENTITY::GET_ENTITY_MODEL(objects_g[i]);
				if ((grenade == 0x1152354B || grenade == 0x741FD3C4)) {
					Vector3 gr_cor = ENTITY::GET_ENTITY_COORDS(objects_g[i], TRUE);
					Vector3 me_cor = ENTITY::GET_ENTITY_COORDS(playerPed, TRUE);
					float dist = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(me_cor.x, me_cor.y, me_cor.z, gr_cor.x, gr_cor.y, gr_cor.z, TRUE);
					if (ENTITY::IS_ENTITY_IN_AIR(objects_g[i]) && dist > 10.0 && dist < 16.0) FIRE::ADD_EXPLOSION(gr_cor.x, gr_cor.y, gr_cor.z, ExplosionTypeGrenadeL, 3.0, rand() % 15 == 0, false, 0.0); // rand() % 3 == 0
					if (ENTITY::IS_ENTITY_IN_AIR(objects_g[i]) && dist > 15.0 && dist < 99.0) FIRE::ADD_EXPLOSION(gr_cor.x, gr_cor.y, gr_cor.z, ExplosionTypeGrenadeL, 35.0, rand() % 15 == 0, false, 0.0); // dist < 159.0 // 15.0 
					if (!ENTITY::IS_ENTITY_IN_AIR(objects_g[i]) && dist > 10.0 && dist < 16.0) FIRE::ADD_EXPLOSION(gr_cor.x, gr_cor.y, gr_cor.z, ExplosionTypeGrenadeL, 5.0, rand() % 15 == 0, false, 0.0);
					if (!ENTITY::IS_ENTITY_IN_AIR(objects_g[i]) && dist > 15.0 && dist < 99.0) FIRE::ADD_EXPLOSION(gr_cor.x, gr_cor.y, gr_cor.z, ExplosionTypeGrenadeL, 35.0, rand() % 15 == 0, false, 0.0);
				}
			}
			// 吸附手榴弹
			if (featureWeaponVacuumGrenades) {
				if (vacuum_seconds < 30) {
					s_vacuum_secs_passed = clock() / CLOCKS_PER_SEC;
					if (((clock() / CLOCKS_PER_SEC) - s_vacuum_secs_curr) != 0) {
						vacuum_seconds = vacuum_seconds + 1;
						s_vacuum_secs_curr = s_vacuum_secs_passed;
					}
				}
				if (!shown_vacuum_message) {// 移除武器检查
					set_status_text("~y~已装备 ~q~引力 ~g~榴弹发射器！");
					shown_vacuum_message = true;// 限制显示次数
				}
				Vector3 obj_cor = ENTITY::GET_ENTITY_COORDS(playerPed, TRUE);
				float c_x, c_y, c_z = 0.0;
				Hash grenade = ENTITY::GET_ENTITY_MODEL(objects_g[i]);
				if (/*grenade == 0x1152354B || */grenade == 0x741FD3C4) {
					Vector3 gr_cor = ENTITY::GET_ENTITY_COORDS(objects_g[i], TRUE);
					Vector3 me_cor = ENTITY::GET_ENTITY_COORDS(playerPed, TRUE);
					float dist = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(me_cor.x, me_cor.y, me_cor.z, gr_cor.x, gr_cor.y, gr_cor.z, TRUE);
					if (dist > 199) OBJECT::DELETE_OBJECT(&objects_g[i]);
					if (/*ENTITY::IS_ENTITY_IN_AIR(objects_g[i]) && */dist > 1.0 && dist < 200) {
						const int arrSize_bl = 1024;
						Ped surr_p_peds[arrSize_bl];
						int count_surr_p_peds = worldGetAllPeds(surr_p_peds, arrSize_bl);
						c_x, c_y, c_z = 0.0;
						for (int j = 0; j < count_surr_p_peds; j++) {
							obj_cor = ENTITY::GET_ENTITY_COORDS(surr_p_peds[j], TRUE);
							if (obj_cor.x > gr_cor.x) c_x = -1.5; // 0.5
							else c_x = 1.5;
							if (obj_cor.y > gr_cor.y) c_y = -1.5;
							else c_y = 1.5;
							if (obj_cor.z > gr_cor.z) c_z = -0.5;
							else c_z = 0.5;
							if (surr_p_peds[j] != PLAYER::PLAYER_PED_ID() && surr_p_peds[j] != objects_g[i]) {
								if (!PED::IS_PED_RAGDOLL(surr_p_peds[j])) {
									AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(surr_p_peds[j], true);
									PED::SET_PED_CAN_RAGDOLL(surr_p_peds[j], true);
									PED::SET_PED_TO_RAGDOLL(surr_p_peds[j], 1500, 1500, 1, true, true, false);
								}
								ENTITY::APPLY_FORCE_TO_ENTITY(surr_p_peds[j], 1, c_x, c_y, c_z, 0, 0, 0, true, false, true, true, true, true);
								float dist_center = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(obj_cor.x, obj_cor.y, obj_cor.z, gr_cor.x, gr_cor.y, gr_cor.z, TRUE);
								if (dist_center < 95) { // 10, 15, 55
									ENTITY::SET_ENTITY_MAX_SPEED(surr_p_peds[j], 20); // 10
								}
							}
						}
						Vehicle surr_vehicles[arrSize_bl];
						int count_surr_v = worldGetAllVehicles(surr_vehicles, arrSize_bl);
						c_x, c_y, c_z = 0.0;
						for (int j = 0; j < count_surr_v; j++) {
							obj_cor = ENTITY::GET_ENTITY_COORDS(surr_vehicles[j], TRUE);
							if (obj_cor.x > gr_cor.x) c_x = -1.5;
							else c_x = 1.5;
							if (obj_cor.y > gr_cor.y) c_y = -1.5;
							else c_y = 1.5;
							if (obj_cor.z > gr_cor.z) c_z = -0.5;
							else c_z = 0.5;
							if (!VEHICLE::_IS_VEHICLE_DAMAGED(surr_vehicles[j])) VEHICLE::SET_VEHICLE_DAMAGE(surr_vehicles[j], obj_cor.x, obj_cor.y, obj_cor.z, 1000, 1000, 1);
							if (surr_vehicles[j] != PED::GET_VEHICLE_PED_IS_USING(playerPed) && surr_vehicles[j] != objects_g[i]) {
								ENTITY::APPLY_FORCE_TO_ENTITY(surr_vehicles[j], 1, c_x, c_y, c_z, 0, 0, 0, true, false, true, true, true, true);
								float dist_center = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(obj_cor.x, obj_cor.y, obj_cor.z, gr_cor.x, gr_cor.y, gr_cor.z, TRUE);
								if (dist_center < 95) { // 10, 15, 55
									ENTITY::SET_ENTITY_MAX_SPEED(surr_vehicles[j], 20); // 10
								}
							}
						}
						Object surr_objects[arrSize_bl];
						int count_surr_o = worldGetAllObjects(surr_objects, arrSize_bl);
						c_x, c_y, c_z = 0.0;
						for (int j = 0; j < count_surr_o; j++) {
							obj_cor = ENTITY::GET_ENTITY_COORDS(surr_objects[j], TRUE);
							if (obj_cor.x > gr_cor.x) c_x = -1.5;
							else c_x = 1.5;
							if (obj_cor.y > gr_cor.y) c_y = -1.5;
							else c_y = 1.5;
							if (obj_cor.z > gr_cor.z) c_z = -0.5;
							else c_z = 0.5;
							if (surr_objects[j] != objects_g[i]) {
								ENTITY::APPLY_FORCE_TO_ENTITY(surr_objects[j], 1, c_x, c_y, c_z, 0, 0, 0, true, false, true, true, true, true);
								float dist_center = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(obj_cor.x, obj_cor.y, obj_cor.z, gr_cor.x, gr_cor.y, gr_cor.z, TRUE);
								if (dist_center < 95) { // 10, 15, 55
									ENTITY::SET_ENTITY_MAX_SPEED(surr_objects[j], 20); // 10
								}
							}
						}
					}
				} // 手榴弹结束
			} // 吸附手榴弹结束
		} // 循环结束
	} else {  // 添加else分支
		if (!featureWeaponVacuumGrenades) {
			shown_vacuum_message = false;// 重置标记
		}
	}

	// 无限弹药
	if(bPlayerExists && featureWeaponInfiniteAmmo){
		for(int i = 0; i < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); i++){
			for(int j = 0; j < VOV_WEAPON_VALUES[i].size(); j++){
				char *weaponName = (char*) VOV_WEAPON_VALUES[i].at(j).c_str();
				Hash weapon = GAMEPLAY::GET_HASH_KEY(weaponName);

				if(WEAPON::IS_WEAPON_VALID(weapon) && WEAPON::HAS_PED_GOT_WEAPON(playerPed, weapon, 0)){
					int ammo;

					if(WEAPON::GET_MAX_AMMO(playerPed, weapon, &ammo)){
						WEAPON::SET_PED_AMMO(playerPed, weapon, ammo);
					}
				}
			}
		}
	}

	// 无限降落伞
	if(featureWeaponInfiniteParachutesUpdated){
		if(featureWeaponInfiniteParachutes){
			featureWeaponNoParachutes = false;
		}
		featureWeaponInfiniteParachutesUpdated = false;
	}
	if(bPlayerExists && featureWeaponInfiniteParachutes && detained == false && in_prison == false && super_jump_no_parachute == false){
		int pState = PED::GET_PED_PARACHUTE_STATE(playerPed);
		// 未持武器或坠落状态 - 不要尝试给已经在使用降落伞的玩家再添加一个，否则会导致游戏崩溃
		if(pState == -1 || pState == 3){
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, PARACHUTE_ID, 1, 0);
		}
	}

	// 无降落伞
	if(featureWeaponNoParachutesUpdated){
		if(featureWeaponNoParachutes){
			featureWeaponInfiniteParachutes = false;
		}
		featureWeaponNoParachutesUpdated = false;
	}
	if(bPlayerExists && featureWeaponNoParachutes){
		int pState = PED::GET_PED_PARACHUTE_STATE(playerPed);
		if((pState == -1 || pState == 3) && WEAPON::HAS_PED_GOT_WEAPON(playerPed, PARACHUTE_ID, FALSE)){
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, PARACHUTE_ID);
		}
	}

	// 武器无需换弹
	if(bPlayerExists){
		WEAPON::SET_PED_INFINITE_AMMO_CLIP(playerPed, featureWeaponNoReload);
	}
	
	// 无准星
	if (NPC_RAGDOLL_VALUES[WeaponsNoReticle] > 0) {
		Vehicle cur_v = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		if (NPC_RAGDOLL_VALUES[WeaponsNoReticle] == 1 || (NPC_RAGDOLL_VALUES[WeaponsNoReticle] == 2 && !PED::IS_PED_IN_ANY_VEHICLE(playerPed, true) && CAM::_0xEE778F8C7E1142E2(0) == 4) ||
			(NPC_RAGDOLL_VALUES[WeaponsNoReticle] == 2 && PED::IS_PED_IN_ANY_VEHICLE(playerPed, true) && ((VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(1) == 4) ||
				(VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(2) == 4) || (VEHICLE::IS_THIS_MODEL_A_BOAT(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(3) == 4) ||
			(VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(4) == 4) || 
				((ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE") || ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE2")) && CAM::_0xEE778F8C7E1142E2(5) == 4) || 
				(VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(6) == 4)))) {
			bool sniper_rifle = false;

			switch (WEAPON::GET_SELECTED_PED_WEAPON(playerPed))
			{
				case RAGE_JOAAT("WEAPON_SNIPERRIFLE"):
				case RAGE_JOAAT("WEAPON_HEAVYSNIPER"):
				case RAGE_JOAAT("WEAPON_REMOTESNIPER"):
				case RAGE_JOAAT("WEAPON_HEAVYSNIPER_MK2"):
				case RAGE_JOAAT("WEAPON_MARKSMANRIFLE"):
				case RAGE_JOAAT("WEAPON_MARKSMANRIFLE_MK2"):
					sniper_rifle = true;
			}
		
			if (sniper_rifle == false) UI::HIDE_HUD_COMPONENT_THIS_FRAME(14);
		}
	}
	
	// 如果手部被击中则掉落武器
	if (featureDropWeapon) {
		Vector3 coords_myfinger_p = PED::GET_PED_BONE_COORDS(playerPed, 64016, 0, 0, 0); // 右手手指骨骼
		if (WEAPON::HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON(playerPed, 0, 2) && GAMEPLAY::HAS_BULLET_IMPACTED_IN_AREA(coords_myfinger_p.x, coords_myfinger_p.y, coords_myfinger_p.z, 0.25, 0, 0) && WEAPON::IS_PED_ARMED(playerPed, 7)) { // 0.2
			Hash curr_w = WEAPON::GET_SELECTED_PED_WEAPON(playerPed);
			WEAPON::SET_AMMO_IN_CLIP(playerPed, curr_w, 0);
			Vector3 p_coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(playerPed, 10.0f, 10.0f, 0.0f);
			Object temp_w = WEAPON::GET_WEAPON_OBJECT_FROM_PED(playerPed, 1);
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, curr_w);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&temp_w);
			OBJECT::DELETE_OBJECT(&temp_w);
			PED::CLEAR_PED_LAST_DAMAGE_BONE(playerPed);
			ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(playerPed);
		}
	}

	// 弹药耗尽时掉落武器
	if (featureDropWeaponOutAmmo && WEAPON::IS_PED_ARMED(playerPed, 7) && WEAPON::IS_PED_ARMED(playerPed, 6) && WEAPON::GET_SELECTED_PED_WEAPON(playerPed) != GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN")) {
		if (WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed)) == 1) temp_weapon = WEAPON::GET_SELECTED_PED_WEAPON(playerPed);
		if (WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed)) < 1) {
			WEAPON::SET_CURRENT_PED_WEAPON(playerPed, WEAPON::GET_SELECTED_PED_WEAPON(playerPed), true);
			Object temp_w = WEAPON::GET_WEAPON_OBJECT_FROM_PED(playerPed, 1);
			WEAPON::SET_PED_DROPS_WEAPON(playerPed);
			WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, WEAPON::GET_SELECTED_PED_WEAPON(playerPed));
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&temp_w);
			OBJECT::DELETE_OBJECT(&temp_w);
		}
	}
	if (featureDropWeaponOutAmmo && WEAPON::HAS_PED_GOT_WEAPON(playerPed, temp_weapon, false) && WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), temp_weapon) < 1) {
		WEAPON::SET_CURRENT_PED_WEAPON(playerPed, temp_weapon, true);
		Object temp_w = WEAPON::GET_WEAPON_OBJECT_FROM_PED(playerPed, 1);
		WEAPON::SET_PED_DROPS_WEAPON(playerPed);
		WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, temp_weapon);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&temp_w);
		OBJECT::DELETE_OBJECT(&temp_w);
	}

	// 射击以解除NPC武装 && 无法拾取掉落的武器
	if (featureCanDisarmNPC || featurePedNoWeaponDrop) {
		const int arrSize2 = 1024;
		Ped a_npcs[arrSize2];
		int count_npcs = worldGetAllPeds(a_npcs, arrSize2);
		for (int i = 0; i < count_npcs; i++) {
			// 射击以解除NPC武装
			if (featureCanDisarmNPC) {
				if (a_npcs[i] != playerPed) {
					Vector3 coords_finger_p = PED::GET_PED_BONE_COORDS(a_npcs[i], 64016, 0, 0, 0); // 右手手指骨骼
					if (WEAPON::HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON(a_npcs[i], 0, 2) && GAMEPLAY::HAS_BULLET_IMPACTED_IN_AREA(coords_finger_p.x, coords_finger_p.y, coords_finger_p.z, 0.5/*0.4*/, 0, 0) && WEAPON::IS_PED_ARMED(a_npcs[i], 7)) {
						Hash curr_w = WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]);
						if (!featurePedNoWeaponDrop) {
							Vector3 p_coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(a_npcs[i], 10.0f, 10.0f, 0.0f);
							WEAPON::SET_PED_DROPS_INVENTORY_WEAPON(a_npcs[i], curr_w, p_coords.x, p_coords.y, p_coords.z, 1);
							WEAPON::REMOVE_WEAPON_FROM_PED(a_npcs[i], curr_w);
						}
						if (featurePedNoWeaponDrop) {
							Object temp_w = WEAPON::GET_WEAPON_OBJECT_FROM_PED(a_npcs[i], 1);
							WEAPON::REMOVE_WEAPON_FROM_PED(a_npcs[i], curr_w);
							ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&temp_w);
							OBJECT::DELETE_OBJECT(&temp_w);
						}
						PED::CLEAR_PED_LAST_DAMAGE_BONE(a_npcs[i]);
						ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(a_npcs[i]);
					}
				}
			}
			// 无法拾取掉落的武器
			if (featurePedNoWeaponDrop) {
				if (!PED::IS_PED_DEAD_OR_DYING(a_npcs[i], true) && a_npcs[i] != playerPed) WEAPON::SET_PED_DROPS_WEAPONS_WHEN_DEAD(a_npcs[i], false);
				
				if (ENTITY::GET_ENTITY_MODEL(a_npcs[i]) == GAMEPLAY::GET_HASH_KEY((char*)"mp_f_freemode_01") || ENTITY::GET_ENTITY_MODEL(a_npcs[i]) == GAMEPLAY::GET_HASH_KEY((char*)"mp_m_freemode_01") ||
					PED::GET_PED_TYPE(a_npcs[i]) == 6 || PED::GET_PED_TYPE(a_npcs[i]) == 27 || PED::GET_PED_TYPE(a_npcs[i]) == 29) PED::SET_PED_CONFIG_FLAG(a_npcs[i], 281, true);

				if (a_npcs[i] != playerPed && (PED::IS_PED_HURT(a_npcs[i]) || PED::IS_PED_INJURED(a_npcs[i]) || AI::IS_PED_IN_WRITHE(a_npcs[i]) || PED::IS_PED_FATALLY_INJURED(a_npcs[i]))) {
					Hash curr_w = WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]);
					Object temp_w = WEAPON::GET_WEAPON_OBJECT_FROM_PED(a_npcs[i], 1);
					WEAPON::REMOVE_WEAPON_FROM_PED(a_npcs[i], curr_w);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&temp_w);
					OBJECT::DELETE_OBJECT(&temp_w);
				}
			}
		} // 循环结束
	}

	// 警察武器
	if (featureCopArmedWith && !PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
		Hash curr_weapon = WEAPON::GET_SELECTED_PED_WEAPON(playerPed);
		Hash Weapon_Type = WEAPON::GET_WEAPONTYPE_GROUP(curr_weapon);
		char* currWeapon = new char[WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex].length() + 1];
		strcpy(currWeapon, WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex].c_str());
		Hash Cop_Weapon = GAMEPLAY::GET_HASH_KEY(currWeapon);
		if (PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID()) <= WEAPONS_COPALARM_VALUES[CopAlarmIndex] || WEAPONS_COPALARM_VALUES[CopAlarmIndex] > 5) {
			const int arrSize2 = 1024;
			Ped a_npcs[arrSize2];
			int count_npcs = worldGetAllPeds(a_npcs, arrSize2);
			for (int i = 0; i < count_npcs; i++) {
				if (temp_ped == -1 && a_npcs[i] != PLAYER::PLAYER_PED_ID() && (PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID()) <= WEAPONS_COPALARM_VALUES[CopAlarmIndex] || WEAPONS_COPALARM_VALUES[CopAlarmIndex] > 5)) {
					if (featureSwitchWeaponIfDanger && (WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_UNARMED\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_NIGHTSTICK\"" ||
						WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_FLASHLIGHT\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_KNIFE\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_DAGGER\"" ||
						WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_HAMMER\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_BAT\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_GOLFCLUB\"" ||
						WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_CROWBAR\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_POOLCUE\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_WRENCH\"" ||
						WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_MACHETE\"" || WEAPONS_COPARMED_CAPTIONS[CopCurrArmedIndex] == "\"WEAPON_BOTTLE\"")) {
						if (!PED::IS_PED_DEAD_OR_DYING(a_npcs[i], true) && PED::GET_PED_TYPE(a_npcs[i]) != 6 && PED::GET_PED_TYPE(a_npcs[i]) != 27 && PED::GET_PED_TYPE(a_npcs[i]) != 29 && PED::IS_PED_SHOOTING(a_npcs[i]) &&
							a_npcs[i] != playerPed) {
							Vector3 coords_mebullet = ENTITY::GET_ENTITY_COORDS(playerPed, true);
							if (WEAPON::IS_PED_ARMED(a_npcs[i], 7) && WEAPON::IS_PED_ARMED(a_npcs[i], 6) && !PED::IS_PED_SHOOTING(playerPed) && (GAMEPLAY::HAS_BULLET_IMPACTED_IN_AREA(coords_mebullet.x, coords_mebullet.y, coords_mebullet.z, 400.0, 0, 0))) {
								if (someonehasgunandshooting == false) {
									shooting_criminal = a_npcs[i];
									someonehasgunandshooting = true;
								}
							}
						}
						if ((PED::IS_PED_DEAD_OR_DYING(shooting_criminal, true) || !ENTITY::DOES_ENTITY_EXIST(shooting_criminal)) && someonehasgunandshooting == true) someonehasgunandshooting = false;
					}
					else someonehasgunandshooting = false;
					if (featurePlayerMelee && (Weapon_Type == 3566412244/*近战武器*/ || Weapon_Type == 2685387236/*徒手*/) && someonehasgunandshooting == false) {
						if ((PED::GET_PED_TYPE(a_npcs[i]) == 6 || PED::GET_PED_TYPE(a_npcs[i]) == 27) && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) != Cop_Weapon)
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], Cop_Weapon, 999, false, true);
						if (featureArmyMelee && PED::GET_PED_TYPE(a_npcs[i]) == 29 && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) != Cop_Weapon) 
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], Cop_Weapon, 999, false, true);
					}
					if (featurePlayerMelee && Weapon_Type != 3566412244 && Weapon_Type != 2685387236) {
						if ((PED::GET_PED_TYPE(a_npcs[i]) == 6 || PED::GET_PED_TYPE(a_npcs[i]) == 27) && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) == GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN"))
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 999, false, true);
						if (featureArmyMelee && PED::GET_PED_TYPE(a_npcs[i]) == 29 && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) == GAMEPLAY::GET_HASH_KEY("WEAPON_STUNGUN")) 
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 999, false, true);
					}
					if (!featurePlayerMelee && someonehasgunandshooting == false) {
						if ((PED::GET_PED_TYPE(a_npcs[i]) == 6 || PED::GET_PED_TYPE(a_npcs[i]) == 27) && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) != Cop_Weapon) 
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], Cop_Weapon, 999, false, true);
						if (featureArmyMelee && PED::GET_PED_TYPE(a_npcs[i]) == 29 && !PED::IS_PED_GROUP_MEMBER(a_npcs[i], myENTGroup) && WEAPON::GET_SELECTED_PED_WEAPON(a_npcs[i]) != Cop_Weapon) 
							WEAPON::GIVE_WEAPON_TO_PED(a_npcs[i], Cop_Weapon, 999, false, true);
					}
				}
				// 逮捕模式
				if (featureDetainedIfNotMove && a_npcs[i] != PLAYER::PLAYER_PED_ID() && (PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID()) == 1 || PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID()) == 2) && AI::IS_PED_STILL(PLAYER::PLAYER_PED_ID())) {
					s_vacuum_secs_passed = clock() / CLOCKS_PER_SEC;
					if (((clock() / CLOCKS_PER_SEC) - s_vacuum_secs_curr) != 0) {
						arrest_secs = arrest_secs + 1;
						s_vacuum_secs_curr = s_vacuum_secs_passed;
					}
					if (arrest_secs > 5 && arrest_secs < 100) { // 10 && 15
						find_nearest_ped();
						if (PED::GET_PED_TYPE(temp_ped) == 6 || PED::GET_PED_TYPE(temp_ped) == 27) {
							PLAYER::SET_MAX_WANTED_LEVEL(5);
							PLAYER::SET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID(), 1, 0);
							PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(PLAYER::PLAYER_ID(), 0);
							WEAPON::REMOVE_ALL_PED_WEAPONS(temp_ped, false);
							WEAPON::GIVE_WEAPON_TO_PED(temp_ped, GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 999, false, true);
							AI::TASK_ARREST_PED(temp_ped, PLAYER::PLAYER_PED_ID());
							arrest_secs = 500;
						}
					}
				}
				if (featureDetainedIfNotMove && !AI::IS_PED_STILL(PLAYER::PLAYER_PED_ID())) {
					arrest_secs = 0;
					temp_ped = -1;
				}
			} // 循环结束
		} // 条件判断结束
	}

	//// <--- 行人不喜欢武器 ////
	peds_dont_like_weapons(); 

	// 强力拳击
	if (featurePowerPunch && !PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
		Vector3 CamRot = ENTITY::GET_ENTITY_ROTATION(playerPed, 2);
		long long int p_force = -1; 
		const int arrSize_punch = 1024;
		Ped surr_p_peds[arrSize_punch];
		int count_surr_p_peds = worldGetAllPeds(surr_p_peds, arrSize_punch);
		Vehicle surr_vehicles[arrSize_punch];
		int count_surr_v = worldGetAllVehicles(surr_vehicles, arrSize_punch);
		Object surr_objects[arrSize_punch];
		int count_surr_o = worldGetAllObjects(surr_objects, arrSize_punch);

		if (WEAPONS_POWERPUNCH_VALUES[PowerPunchIndex] != 55) p_force = WEAPONS_POWERPUNCH_VALUES[PowerPunchIndex];
		if (WEAPONS_POWERPUNCH_VALUES[PowerPunchIndex] == 55 && !lastPowerWeapon.empty()) {
			std::string::size_type sz;
			p_force = std::stoll(lastPowerWeapon, &sz);
		}
		
		if (CONTROLS::IS_CONTROL_PRESSED(2, 24) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 140) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 141)) {
			force_nearest_ped = true;
			if (PED::IS_PED_IN_MELEE_COMBAT(PED::GET_MELEE_TARGET_FOR_PED(playerPed))) {
				WAIT(500);
				AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(PED::GET_MELEE_TARGET_FOR_PED(playerPed), true);
				AI::CLEAR_PED_TASKS_IMMEDIATELY(PED::GET_MELEE_TARGET_FOR_PED(playerPed));
			}
		}
		
		bool cur_weapon_e = false;
		bool cur_weapon_peds = false;
		bool cur_weapon_e_peds = false;
		
		if (featurePunchFists && !WEAPON::IS_PED_ARMED(playerPed, 7)) cur_weapon_e = true;
		if (featurePunchMeleeWeapons && !WEAPON::IS_PED_ARMED(playerPed, 6) && WEAPON::IS_PED_ARMED(playerPed, 7)) cur_weapon_e = true;
		if (featurePunchFireWeapons && WEAPON::IS_PED_ARMED(playerPed, 7) && WEAPON::IS_PED_ARMED(playerPed, 6)) cur_weapon_e = true;

		float rad = 2 * 3.14 * (CamRot.z / 360);
		float v_x = -(sin(rad) * p_force * 10);
		float v_y = (cos(rad) * p_force * 10);
		float v_z = p_force * (CamRot.x * 0.2);

		if (force_nearest_ped == true && cur_weapon_e == true/* && PEDS_POWERPUNCH_VALUES[PedsPowerPunchIndex] < 2*/) {
			PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(player, 1000.0);
			PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(player, 1000.0, 1);
			PLAYER::SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(playerPed, 1000.0);

			for (int i = 0; i < count_surr_p_peds; i++) {
				if (surr_p_peds[i] != playerPed && !PED::IS_PED_IN_MELEE_COMBAT(surr_p_peds[i])) { 
					if (!WEAPON::IS_PED_ARMED(playerPed, 7)) AI::CLEAR_PED_SECONDARY_TASK(surr_p_peds[i]);
					if (ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY(surr_p_peds[i], playerPed, 1)) {
						temp_nearest_ped = surr_p_peds[i];
					}
				}
			} // peds 相关代码块结束
			for (int i = 0; i < count_surr_o; i++) {
				if (ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY(surr_objects[i], playerPed, 1)) {
					ENTITY::APPLY_FORCE_TO_ENTITY(surr_objects[i], 1, v_x, v_y, v_z, 0, 0, 0, true, false, true, true, true, true);
					force_nearest_ped = false;
					PED::CLEAR_PED_LAST_DAMAGE_BONE(surr_objects[i]);
					ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(surr_objects[i]);
				}
			} // objects 相关代码块结束
			for (int i = 0; i < count_surr_v; i++) {
				if (ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY(surr_vehicles[i], playerPed, 1)) {
					ENTITY::APPLY_FORCE_TO_ENTITY(surr_vehicles[i], 1, v_x, v_y, v_z, 0, 0, 0, true, false, true, true, true, true);
					force_nearest_ped = false;
					PED::CLEAR_PED_LAST_DAMAGE_BONE(surr_vehicles[i]);
					ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(surr_vehicles[i]);
				}
			} // vehicles 相关代码块结束
		}
		if (temp_nearest_ped != -1) {
			AUDIO::PLAY_SOUND_FROM_ENTITY(-1, "FIB3A_LAND_FROM_HEIGHT_MASTER", PLAYER::PLAYER_PED_ID(), 0, 0, 0);
			PED::RESET_PED_MOVEMENT_CLIPSET(temp_nearest_ped, 0.0);
			PED::SET_PED_CAN_RAGDOLL(temp_nearest_ped, true);
			PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(temp_nearest_ped, true);
			PED::SET_PED_RAGDOLL_FORCE_FALL(temp_nearest_ped);
			AI::CLEAR_PED_SECONDARY_TASK(temp_nearest_ped);
			AI::CLEAR_PED_TASKS(temp_nearest_ped);
			AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(temp_nearest_ped, true);
			AI::CLEAR_PED_TASKS_IMMEDIATELY(temp_nearest_ped);
			ENTITY::APPLY_FORCE_TO_ENTITY(temp_nearest_ped, 1, v_x, v_y, v_z, 0, 0, 0, true, false, true, true, true, true);
			force_nearest_ped = false;
			PED::CLEAR_PED_LAST_DAMAGE_BONE(temp_nearest_ped);
			ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(temp_nearest_ped);
			temp_nearest_ped = -1;
		}
		for (int i = 0; i < count_surr_p_peds; i++) {
			PED::CLEAR_PED_LAST_DAMAGE_BONE(surr_p_peds[i]);
			ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(surr_p_peds[i]);
		}
		for (int i = 0; i < count_surr_v; i++) {
			PED::CLEAR_PED_LAST_DAMAGE_BONE(surr_vehicles[i]);
			ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(surr_vehicles[i]);
		}
		for (int i = 0; i < count_surr_o; i++) {
			PED::CLEAR_PED_LAST_DAMAGE_BONE(surr_objects[i]);
			ENTITY::CLEAR_ENTITY_LAST_DAMAGE_ENTITY(surr_objects[i]);
		}
	}

	// 死亡/被捕时失去武器
	if (featureCopTakeWeapon) {
		if ((time_since_d > 100 && time_since_d < 5000) || (time_since_a > 100 && time_since_a < 5000) || PLAYER::IS_PLAYER_BEING_ARRESTED(PLAYER::PLAYER_ID(), 1) || player_died == true) {
			WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, false);
			if (!ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()) && !PLAYER::IS_PLAYER_BEING_ARRESTED(PLAYER::PLAYER_ID(), 1) && detained == false) player_died = false;
		}
	}

	// 自动给予所有武器
	if (featureGiveAllWeapons && detained == false && in_prison == false && PED::IS_PED_HUMAN(playerPed) && !STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS()) {
		if (tick_allw < 100) {
			w_tick_secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / (CLOCKS_PER_SEC / 1000)) - w_tick_secs_curr) != 0) {
				tick_allw = tick_allw + 1;
				w_tick_secs_curr = w_tick_secs_passed;
			}
		}
		if (tick_allw > 50 && PlayerUpdated_w && !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID())) {
			WAIT(200);
			give_all_weapons_hotkey();
			oldplayerPed_W = playerPed;
			tick_allw = 0;
			PlayerUpdated_w = false; 
			if (detained == false && alert_level == 0) player_died = false;
		}
		if (((time_since_d > -1 && time_since_d < 2000) || playerPed != oldplayerPed_W || player_died == true || DLC2::GET_IS_LOADING_SCREEN_ACTIVE()) && PlayerUpdated_w == false) {
			PlayerUpdated_w = true;
			tick_allw = 0;
		}
	}
	
	// 自动添加所有武器配件
	if (featureAddAllWeaponsAttachments && detained == false && in_prison == false && PED::IS_PED_HUMAN(playerPed) && !STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS()) {
		if (tick_a_allw < 150) {
			w_tick_secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / (CLOCKS_PER_SEC / 1000)) - w_a_tick_secs_curr) != 0) {
				tick_a_allw = tick_a_allw + 1;
				w_a_tick_secs_curr = w_tick_secs_passed;
			}
		}
		if (tick_a_allw > 100 && PlayerUpdated_a && !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID())) {
			WAIT(200);
			add_all_weapons_attachments(playerPed);
			oldplayerPed_A = playerPed;
			tick_a_allw = 0;
			PlayerUpdated_a = false;
			if (detained == false && alert_level == 0) player_died = false;
		}
		if (((time_since_d > -1 && time_since_d < 2000) || playerPed != oldplayerPed_A || player_died == true || DLC2::GET_IS_LOADING_SCREEN_ACTIVE()) && PlayerUpdated_a == false) {
			PlayerUpdated_a = true;
			tick_a_allw = 0;
		}
	}

	// 装备已保存的武器
	if (NPC_RAGDOLL_VALUES[WeaponsSavedLoad] > 0 && detained == false && in_prison == false && PED::IS_PED_HUMAN(playerPed) && !CUTSCENE::IS_CUTSCENE_PLAYING() && GAMEPLAY::GET_MISSION_FLAG() == 0 && !STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS()) {
		if (tick_s_allw < 100) {
			w_tick_secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / (CLOCKS_PER_SEC / 1000)) - ss_tick_secs_curr) != 0) {
				tick_s_allw = tick_s_allw + 1;
				ss_tick_secs_curr = w_tick_secs_passed;
			}
		}
		if (tick_s_allw > 60 && PlayerUpdated_s && !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID())) {
			WAIT(300); //WAIT(200);
			load_saved_weapons();
			for (int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++) { // 给予所有已装备武器的弹药
				for (int b = 0; b < VOV_WEAPON_VALUES[a].size(); b++) {
					char* weaponName = (char*)VOV_WEAPON_VALUES[a].at(b).c_str();
					Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
					if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, FALSE)) WEAPON::GIVE_WEAPON_TO_PED(playerPed, weaponHash, 10000, false, false);
				}
			}
			oldplayerPed_s = playerPed;
			tick_s_allw = 0;
			PlayerUpdated_s = false;
			if (detained == false && alert_level == 0) player_died = false;
		}
		if (((time_since_d > -1 && time_since_d < 2000) || playerPed != oldplayerPed_s || player_died == true || DLC2::GET_IS_LOADING_SCREEN_ACTIVE()) && PlayerUpdated_s == false) {
			PlayerUpdated_s = true;
			tick_s_allw = 0;
		}
	}

	// 未瞄准时禁用视觉效果
	if (WORLD_GRAVITY_LEVEL_VALUES[SniperVisionIndex] != 0 && !SCRIPT::HAS_SCRIPT_LOADED("carsteal2"))
	{
		if (!PED::GET_PED_CONFIG_FLAG(playerPed, 78, 1)) { 
			if (!featureNightVision && !featureThermalVision) {
				GRAPHICS::SET_NIGHTVISION(false);
				GRAPHICS::SET_SEETHROUGH(false);
			}
			if (featureNightVision && !featureThermalVision) {
				GRAPHICS::SET_SEETHROUGH(false);
				GRAPHICS::SET_NIGHTVISION(true);
			}
			if (!featureNightVision && featureThermalVision) {
				GRAPHICS::SET_NIGHTVISION(false);
				GRAPHICS::SET_SEETHROUGH(true);
			}
			if (featureNightVision && featureThermalVision) {
				GRAPHICS::SET_NIGHTVISION(true);
				GRAPHICS::SET_SEETHROUGH(true);
			}
			vision_toggle = 0;
		}
		if (WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_SNIPERRIFLE") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER") ||
			WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_REMOTESNIPER") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER_MK2") ||
			WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_MARKSMANRIFLE") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_MARKSMANRIFLE_MK2")) {
			if (WORLD_GRAVITY_LEVEL_VALUES[SniperVisionIndex] == 2 && !PED::GET_PED_CONFIG_FLAG(playerPed, 58, 1) && PED::GET_PED_CONFIG_FLAG(playerPed, 78, 1)) {
				GRAPHICS::SET_NIGHTVISION(true);
				GRAPHICS::SET_SEETHROUGH(false);
			}
			if (WORLD_GRAVITY_LEVEL_VALUES[SniperVisionIndex] == 3 && !PED::GET_PED_CONFIG_FLAG(playerPed, 58, 1) && PED::GET_PED_CONFIG_FLAG(playerPed, 78, 1)) {
				GRAPHICS::SET_NIGHTVISION(false);
				GRAPHICS::SET_SEETHROUGH(true);
			}
		}
	}

	// 友军伤害
	if (featureFriendlyFire) {
		NETWORK::NETWORK_SET_FRIENDLY_FIRE_OPTION(true);
		PED::SET_CAN_ATTACK_FRIENDLY(playerPed, true, false);
	}
	if (!featureFriendlyFire) {
		NETWORK::NETWORK_SET_FRIENDLY_FIRE_OPTION(false);
		PED::SET_CAN_ATTACK_FRIENDLY(playerPed, false, false);
	}

	// 快速射击
	if (featureRapidFire) {
		if ((CONTROLS::IS_CONTROL_PRESSED(2, 24) || (CONTROLS::IS_CONTROL_PRESSED(2, 24) && CONTROLS::IS_CONTROL_PRESSED(2, 25)))
			&& ENTITY::DOES_ENTITY_EXIST(playerPed) && !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()) && !PED::IS_PED_RELOADING(playerPed)) {
			if (WEAPONS_RAPIDFIRE_VALUES[RapidFireIndex] != -1 && tick_rap_allw < WEAPONS_RAPIDFIRE_VALUES[RapidFireIndex]) {
				w_tick_rap_secs_passed = clock() / CLOCKS_PER_SEC;
				if (((clock() / (CLOCKS_PER_SEC / 10)) - ss_tick_rap_secs_curr) != 0) {
					tick_rap_allw = tick_rap_allw + 1;
					ss_tick_rap_secs_curr = w_tick_rap_secs_passed;
				}
			}
			if (WEAPONS_RAPIDFIRE_VALUES[RapidFireIndex] == -1 || tick_rap_allw >= WEAPONS_RAPIDFIRE_VALUES[RapidFireIndex]) {
				Entity curr_w = WEAPON::GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed);
				Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(curr_w, 1);
				float Coord[3];
				Vector3 moveToPos = add(&myCoords, &DirectionOffsetFromCam(5.5f));
				VectorToFloat(moveToPos, Coord);
				if (WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_FIREWORK") || WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_RPG")) WEAPON::SET_PED_INFINITE_AMMO_CLIP(playerPed, true);
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(myCoords.x, myCoords.y, myCoords.z, Coord[0], Coord[1], Coord[2]/* + 0.5*/, 250, 1, WEAPON::GET_SELECTED_PED_WEAPON(playerPed), playerPed, 1, 0, -1.0);
				GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(myCoords.x, myCoords.y, myCoords.z, Coord[0], Coord[1], Coord[2]/* + 0.5*/, 250, 1, WEAPON::GET_SELECTED_PED_WEAPON(playerPed), playerPed, 1, 0, -1.0);
				tick_rap_allw = 0;
			}
		}
	}

	// 手电筒频闪
	if (FUEL_COLOURS_R_VALUES[WeapStrobeIndexN] > 0) {
		float tmp_s = FUEL_COLOURS_R_VALUES[WeapStrobeIndexN];
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(2, 54) && WEAPON::SET_WEAPON_SMOKEGRENADE_ASSIGNED(playerPed) && strb_c < 6) {
			f_strobe = true;
		}
		if (f_strobe == true) {
			strobe_tick = strobe_tick + 0.1;
			if (strobe_tick > (tmp_s / 100)) { // 0.9
				CONTROLS::_SET_CONTROL_NORMAL(0, 54, 1);
				strobe_tick = 0.0;
			}
		}
		if (CONTROLS::IS_CONTROL_PRESSED(2, 54)) strb_c = strb_c + 1;
		else strb_c = 0;
		if (strb_c > 5) {
			f_strobe = false;
			strobe_tick = 0.0;
		}
	}

	// 手电筒强度
	if (WEAP_DMG_FLOAT[WeapFlashDistIndex] > 1.0) {
		if ((!PED::IS_PED_RELOADING(playerPed) && WEAPON::SET_WEAPON_SMOKEGRENADE_ASSIGNED(playerPed)) || (WEAPON::GET_SELECTED_PED_WEAPON(playerPed) == GAMEPLAY::GET_HASH_KEY("WEAPON_FLASHLIGHT") && CONTROLS::IS_CONTROL_PRESSED(2, 25))) {
			Entity curr_w = WEAPON::GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed);
			Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(curr_w, 1);
				
			float Coord1[3];
			Vector3 moveToPos1 = add(&myCoords, &DirectionOffsetFromCam(1.0f));
			VectorToFloat(moveToPos1, Coord1);
		
			float Coord2[3];
			Vector3 moveToPos2 = add(&myCoords, &DirectionOffsetFromCam(5.5f));
			VectorToFloat(moveToPos2, Coord2);

			float dirVector_lr_x = Coord2[0] - Coord1[0];
			float dirVector_lr_y = Coord2[1] - Coord1[1];
			float dirVector_lr_z = Coord2[2] - Coord1[2];
			if (WEAP_DMG_FLOAT[WeapFlashDistIndex] < 1000.0) GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(Coord1[0], Coord1[1], Coord1[2], dirVector_lr_x, dirVector_lr_y, dirVector_lr_z, 255, 255, 255, WEAP_DMG_FLOAT[WeapFlashDistIndex] * 40.0, 1, 50, 19, 2.7, 10); // 21
			if (WEAP_DMG_FLOAT[WeapFlashDistIndex] == 1000.0) GRAPHICS::_DRAW_SPOT_LIGHT_WITH_SHADOW(Coord1[0], Coord1[1], Coord1[2], dirVector_lr_x, dirVector_lr_y, dirVector_lr_z, 255, 255, 255, 300 * 40.0, 1, 50, 19, 2.7, 10); // 21
		}
	}

	// 开火模式
	if (WORLD_GRAVITY_LEVEL_VALUES[WeaponsFireModeIndex] > 0) {
		CONTROLS::DISABLE_CONTROL_ACTION(2, 24, 1); // 攻击
		CONTROLS::DISABLE_CONTROL_ACTION(2, 257, 1); // 攻击 2
		CONTROLS::DISABLE_CONTROL_ACTION(2, 69, 1); // 载具 攻击
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, 24)) {
			if (featureWeaponInfiniteAmmo && PED::IS_PED_SHOOTING(playerPed)) bullet_tick = bullet_tick + 1;
			if (WORLD_GRAVITY_LEVEL_VALUES[WeaponsFireModeIndex] == 3 && (((bullet_a - WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed))) > 4) || bullet_tick > 4)) { // 连发自动模式
				w_tick_secs_passed = clock() / CLOCKS_PER_SEC;
				if (((clock() / (CLOCKS_PER_SEC / 1000)) - w_tick_secs_curr) != 0) {
					tick_firemode = tick_firemode + 1;
					w_tick_secs_curr = w_tick_secs_passed;
				}
				if (tick_firemode > 50) { // 90
					bullet_a = WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed));
					tick_firemode = 0;
					bullet_tick = 0;
				}
			}
			if ((WORLD_GRAVITY_LEVEL_VALUES[WeaponsFireModeIndex] == 1 && ((bullet_a - WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed))) < 1) && bullet_tick < 1) || // 1 - 单发模式
				(WORLD_GRAVITY_LEVEL_VALUES[WeaponsFireModeIndex] == 2 && ((bullet_a - WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed))) < 5) && bullet_tick < 5) || // 2 - 点射模式
				(WORLD_GRAVITY_LEVEL_VALUES[WeaponsFireModeIndex] == 3 && ((bullet_a - WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed))) < 5) && bullet_tick < 5)) { // 3 - 连发自动模式
				CONTROLS::ENABLE_CONTROL_ACTION(2, 24, 1); // 攻击
				CONTROLS::ENABLE_CONTROL_ACTION(2, 257, 1); // 攻击 2
				CONTROLS::ENABLE_CONTROL_ACTION(2, 69, 1); // 载具 攻击
			}
		}
		if (!CONTROLS::IS_DISABLED_CONTROL_PRESSED(2, 24) && !PED::GET_PED_CONFIG_FLAG(PLAYER::PLAYER_PED_ID(), 58, 1)) {
			bullet_a = WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), WEAPON::GET_SELECTED_PED_WEAPON(playerPed));
			tick_firemode = 0;
			bullet_tick = 0;
		}
	}
	
	// 重力枪
	if(bPlayerExists && featureGravityGun && GAMEPLAY::GET_MISSION_FLAG() == 0) {
		Ped tempPed;
		Hash tempWeap;

		if (!shown_gravitygun_message) {// 移除武器检查
			set_status_text("~y~已装备 ~q~重力 ~g~电击枪！");
			shown_gravitygun_message = true; // 限制显示次数
		}

		if(!grav_target_locked) PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER::PLAYER_ID(), &grav_entity);

		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(grav_entity, true, true);

		tempPed = PLAYER::PLAYER_ID();
		WEAPON::GET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), &tempWeap, 1);

		if((PLAYER::IS_PLAYER_FREE_AIMING(tempPed) || PLAYER::IS_PLAYER_TARGETTING_ANYTHING(tempPed)) && ENTITY::DOES_ENTITY_EXIST(grav_entity) && tempWeap == GAMEPLAY::GET_HASH_KEY("weapon_stungun")){
			Vector3 myCoords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
			float myCoordV[3];
			VectorToFloat(myCoords, myCoordV);

			if(!grav_target_locked){
				PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER::PLAYER_ID(), &grav_entity);
				grav_target_locked = true;
			}

			float Coord[3];

			Vector3 moveToPos = add(&myCoords, &DirectionOffsetFromCam(5.5f));
			VectorToFloat(moveToPos, Coord);

			/*This isn't mandatory, but makes it look nice
			if (!GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(grav_partfx))
			{
			STREAMING::REQUEST_PTFX_ASSET();
			if (STREAMING::HAS_PTFX_ASSET_LOADED())
			{
			grav_partfx = GRAPHICS::START_PARTICLE_FX_LOOPED_AT_COORD((char*)"scr_drug_traffic_flare_L", Coord[0], Coord[1], Coord[2], 0.0f, 0.0f, 0.0f, 0.5f, 0, 0, 0, 0);
			GRAPHICS::SET_PARTICLE_FX_LOOPED_COLOUR(grav_partfx, 1.0f, 0.84f, 0.0f, 0);
			}
			}*/

			RequestControlEntity(grav_entity); // 以便我们可以拾取行人/道具/载具
			
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(grav_entity, Coord[0], Coord[1], Coord[2], 0, 0, 0); // 这是导致道具消失的原因

			if(ENTITY::IS_ENTITY_A_VEHICLE(grav_entity)){
				ENTITY::SET_ENTITY_HEADING(grav_entity, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()) + 90.0f);
			}

			if(PED::IS_PED_SHOOTING(PLAYER::PLAYER_PED_ID())){
				//set_status_text_centre_screen("Throwing");
				//AUDIO::PLAY_SOUND_FROM_ENTITY(-1, (char*)"Foot_Swish", grav_entity, (char*)"docks_heist_finale_2a_sounds", 0, 0);

				ENTITY::SET_ENTITY_HEADING(grav_entity, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));

				ENTITY::APPLY_FORCE_TO_ENTITY(grav_entity, 1, 0.0f, 350.0f, 2.0f, 2.0f, 0.0f, 0.0f, 10, 1, 1, 1, 0, 1);
				// 保持锁定直到我们停止瞄准，但将实体设置为空
				grav_entity = NULL;
			}
		}
		if(!PLAYER::IS_PLAYER_FREE_AIMING(tempPed)){
			//set_status_text_centre_screen("Nothing");
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(grav_entity, true, true);

			grav_target_locked = false;
			grav_entity = NULL;
		}
		/*else if (GRAPHICS::DOES_PARTICLE_FX_LOOPED_EXIST(grav_partfx))
		{
		GRAPHICS::STOP_PARTICLE_FX_LOOPED(grav_partfx, 0);
		GRAPHICS::REMOVE_PARTICLE_FX(grav_partfx, 0);
		STREAMING::REMOVE_PTFX_ASSET();
		}*/
		
		//featureGravityGunUpdated = false;

		//set_status_text("Gravity gun: ~r~called");
	} else {// 添加else分支
		shown_gravitygun_message = false;  // 重置标记
	}
	
	// 全局屏幕准星显示（不包括自由相机模式）
	if (bPlayerExists && !freeCamActive) {
		draw_weapons_crosshair();
	}
	
	// 载具模型枪和角色模型枪的互斥逻辑（使用Updated标志避免菜单闪烁）
	// 注意：必须同时重置对方的Updated标志，否则菜单框架不知道值被外部修改，会导致闪烁
	if (featureVehicleModelGunUpdated) {
		if (featureVehicleModelGun && featurePedModelGun) {
			featurePedModelGun = false;
			featurePedModelGunUpdated = true; // ✅ 关键：通知菜单框架值已改变
			set_status_text("~r~已关闭 ~q~角色模型枪");
		}
		featureVehicleModelGunUpdated = false;
	}
	
	if (featurePedModelGunUpdated) {
		if (featurePedModelGun && featureVehicleModelGun) {
			featureVehicleModelGun = false;
			featureVehicleModelGunUpdated = true; // ✅ 关键：通知菜单框架值已改变
			set_status_text("~r~已关闭 ~p~载具模型枪");
		}
		featurePedModelGunUpdated = false;
	}
	
	// 载具模型枪开启提示
	if (featureVehicleModelGun) {
		if (!shown_vehiclemodelgun_message) {
			set_status_text("~y~已启用 ~p~载具模型枪！");
			shown_vehiclemodelgun_message = true;
		}
	} else {
		shown_vehiclemodelgun_message = false; // 关闭时重置标记
	}
	
	// 角色模型枪开启提示
	if (featurePedModelGun) {
		if (!shown_pedmodelgun_message) {
			set_status_text("~y~已启用 ~q~角色模型枪！");
			shown_pedmodelgun_message = true;
		}
	} else {
		shown_pedmodelgun_message = false; // 关闭时重置标记
	}
	
	// 载具模型枪和角色模型枪射击检测（参考MenyooSP实现）
	// 当玩家射击时触发，IS_PED_SHOOTING会在武器发射子弹瞬间返回true
	if (bPlayerExists && PED::IS_PED_SHOOTING(playerPed)) {
		if (featureVehicleModelGun) {
			fire_vehicle_model_gun();
		}
		if (featurePedModelGun) {
			fire_ped_model_gun();
		}
	}
}

void save_player_weapons(Ped playerPed){
	Player player = PLAYER::PLAYER_ID();

	int index = 0;
	for(int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++){
		for(int b = 0; b < VOV_WEAPON_VALUES[a].size() && index < SAVED_WEAPONS_COUNT; b++, index++){
			std::string weaponNameStr = VOV_WEAPON_VALUES[a].at(b);
			char *weaponName = (char*) weaponNameStr.c_str();
			Hash weaponHash = GAMEPLAY::GET_HASH_KEY(weaponName);
			saved_weapon_model[index] = weaponHash;
			if(WEAPON::HAS_PED_GOT_WEAPON(playerPed, weaponHash, 0)){
				saved_has_weapon[index] = true;
				saved_ammo[index] = WEAPON::GET_AMMO_IN_PED_WEAPON(playerPed, weaponHash);
				WEAPON::GET_AMMO_IN_CLIP(playerPed, weaponHash, &saved_clip_ammo[index]);

				for(int c = 0; c < WEAPONTYPES_TINT.size(); c++){
					if(weaponNameStr.compare(WEAPONTYPES_TINT.at(c)) == 0){
						saved_weapon_tints[index] = WEAPON::GET_PED_WEAPON_TINT_INDEX(playerPed, weaponHash);
						break;
					}
				}

				for(int c = 0; c < WEAPONTYPES_MOD.size(); c++){
					if(weaponNameStr.compare(WEAPONTYPES_MOD.at(c)) == 0){
						for(int d = 0; d < VOV_WEAPONMOD_VALUES[c].size(); d++){
							char *componentName = (char *) VOV_WEAPONMOD_VALUES[c].at(d).c_str();
							Hash componentHash = GAMEPLAY::GET_HASH_KEY(componentName);
							bool modEquipped = WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weaponHash, componentHash) ? true : false;
							saved_weapon_mods[index][d] = modEquipped;
						}
						break;
					}
				}
			} else {
				saved_has_weapon[index] = false;
			}
		}
	}

	if((saved_parachute = WEAPON::HAS_PED_GOT_WEAPON(playerPed, PARACHUTE_ID, 0) ? true : false)){
		PLAYER::GET_PLAYER_PARACHUTE_TINT_INDEX(player, &saved_parachute_tint);
	}
	saved_armour = PED::GET_PED_ARMOUR(playerPed);
}

void restore_player_weapons(Ped playerPed){
	Player player = PLAYER::PLAYER_ID();
	WEAPON::REMOVE_ALL_PED_WEAPONS(playerPed, false);

	int index = 0;
	for(int a = 0; a < sizeof(VOV_WEAPON_VALUES) / sizeof(VOV_WEAPON_VALUES[0]); a++){
		for(int b = 0; b < VOV_WEAPON_VALUES[a].size() && index < SAVED_WEAPONS_COUNT; b++, index++){
			if(saved_has_weapon[index]){
				WEAPON::GIVE_WEAPON_TO_PED(playerPed, saved_weapon_model[index], 1, false, false);
				WEAPON::SET_PED_AMMO(playerPed, saved_weapon_model[index], saved_ammo[index]);
				WEAPON::SET_AMMO_IN_CLIP(playerPed, saved_weapon_model[index], saved_clip_ammo[index]);

				for(int c = 0; c < WEAPONTYPES_TINT.size(); c++){
					if(VOV_WEAPON_VALUES[a].at(b).compare(WEAPONTYPES_TINT.at(c)) == 0){
						WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, saved_weapon_model[index], saved_weapon_tints[index]);
						break;
					}
				}

				for(int c = 0; c < WEAPONTYPES_MOD.size(); c++){
					if(VOV_WEAPON_VALUES[a].at(b).compare(WEAPONTYPES_MOD.at(c)) == 0){
						for(int d = 0; d < VOV_WEAPONMOD_VALUES[c].size(); d++){
							char *componentName = (char *) VOV_WEAPONMOD_VALUES[c].at(d).c_str();
							Hash componentHash = GAMEPLAY::GET_HASH_KEY(componentName);
							if(saved_weapon_mods[index][d]){
								WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, saved_weapon_model[index], componentHash);
							}
						}
						break;
					}
				}
			}
		}
	}

	if(saved_parachute){
		WEAPON::GIVE_WEAPON_TO_PED(playerPed, PARACHUTE_ID, 1, false, false);
		PLAYER::SET_PLAYER_PARACHUTE_TINT_INDEX(player, saved_parachute_tint);
	}

	PED::SET_PED_ARMOUR(playerPed, saved_armour);
}

bool is_weapon_equipped(std::vector<int> extras){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	std::string weaponValue = VOV_WEAPON_VALUES[extras.at(0)].at(extras.at(1));
	char *weaponChar = (char*) weaponValue.c_str();
	return (WEAPON::HAS_PED_GOT_WEAPON(playerPed, GAMEPLAY::GET_HASH_KEY(weaponChar), 0) ? true : false);
}

void set_weapon_equipped(bool equipped, std::vector<int> extras){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	std::string weaponValue = VOV_WEAPON_VALUES[extras.at(0)].at(extras.at(1));
	char *weaponChar = (char*) weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);
	if(equipped){
		WEAPON::GIVE_WEAPON_TO_PED(playerPed, weapHash, 1000, 0, 0);

		// 填满弹夹并额外携带一个备用弹夹
		int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weapHash, false);
		WEAPON::SET_PED_AMMO(playerPed, weapHash, maxClipAmmo);
		WEAPON::SET_AMMO_IN_CLIP(playerPed, weapHash, maxClipAmmo);
	}
	else{
		WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, GAMEPLAY::GET_HASH_KEY(weaponChar));
	}

	redrawWeaponMenuAfterEquipChange = true;
}

bool is_weaponmod_equipped(std::vector<int> extras){
	Ped playerPed = equip_ped;
	std::string weaponValue = VOV_WEAPON_VALUES[extras.at(0)].at(extras.at(1));
	
	if (!WEAPON::IS_PED_ARMED(PLAYER::PLAYER_PED_ID(), 7) && refresh_w == true) { // playerPed != PLAYER::PLAYER_PED_ID() && 
		WAIT(500);
		CONTROLS::_SET_CONTROL_NORMAL(0, 37, 1);
		refresh_w = false;
	}

	char *weaponChar = (char*) weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);

	const std::vector<std::string> MOD_VECTOR = VOV_WEAPONMOD_VALUES[extras.at(2)];
	std::string componentName = MOD_VECTOR.at(extras.at(3));
	DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

	return WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, componentHash) ? true : false;
}

void set_weaponmod_equipped(bool equipped, std::vector<int> extras){
	Ped playerPed = equip_ped;
	std::string weaponValue = VOV_WEAPON_VALUES[extras.at(0)].at(extras.at(1));
	
	if (playerPed != PLAYER::PLAYER_PED_ID()) {
		WAIT(500);
		CONTROLS::_SET_CONTROL_NORMAL(0, 37, 1);
		refresh_w = true;
	}
	
	char *weaponChar = (char*) weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);

	std::string componentName = VOV_WEAPONMOD_VALUES[extras.at(2)].at(extras.at(3));
	DWORD componentHash = GAMEPLAY::GET_HASH_KEY((char *) componentName.c_str());

	if(WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, componentHash)){
		WEAPON::REMOVE_WEAPON_COMPONENT_FROM_PED(playerPed, weapHash, componentHash);
	}
	else{
		WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, weapHash, componentHash);
		int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weapHash, false);
		WEAPON::SET_AMMO_IN_CLIP(playerPed, weapHash, maxClipAmmo);
	}
}

void give_weapon_clip(MenuItem<int> choice){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	std::string weaponValue = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	char *weaponChar = (char*) weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);

	int curAmmo = WEAPON::GET_AMMO_IN_PED_WEAPON(playerPed, weapHash);
	int curClipAmmo = 0;
	WEAPON::GET_AMMO_IN_CLIP(playerPed, weapHash, &curClipAmmo);
	int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weapHash, false);

	if(curClipAmmo < maxClipAmmo){
		set_status_text("弹匣补充完毕！");
		WEAPON::SET_AMMO_IN_CLIP(playerPed, weapHash, maxClipAmmo);
	}
	else{
		set_status_text("额外弹匣补充完毕！");
		WEAPON::SET_PED_AMMO(playerPed, weapHash, curAmmo + maxClipAmmo);
	}
}

void fill_weapon_ammo(MenuItem<int> choice){
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	std::string weaponValue = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	char *weaponChar = (char*) weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);

	int maxAmmo = 0;
	WEAPON::GET_MAX_AMMO(playerPed, weapHash, &maxAmmo);
	int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, weapHash, false);

	WEAPON::SET_AMMO_IN_CLIP(playerPed, weapHash, maxClipAmmo);
	WEAPON::SET_PED_AMMO(playerPed, weapHash, maxAmmo);

	set_status_text("弹药补充完毕！");
}

void fill_weapon_ammo_hotkey()
{
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Hash tempWep;
	int maxAmmo = 0;

	WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &tempWep, 1);
	WEAPON::GET_MAX_AMMO(playerPed, tempWep, &maxAmmo);

	int maxClipAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, tempWep, false);

	WEAPON::SET_AMMO_IN_CLIP(playerPed, tempWep, maxClipAmmo);
	WEAPON::SET_PED_AMMO(playerPed, tempWep, maxAmmo);

	set_status_text("弹药补充完毕！");
}

void onhighlight_weapon_mod_menu_tint(MenuItem<int> choice){
	onconfirm_weapon_mod_menu_tint(choice);
}

bool onconfirm_weapon_mod_menu_tint(MenuItem<int> choice){
	Ped playerPed = equip_ped; // PLAYER::PLAYER_PED_ID();
	std::string weaponName = "";
	if (equip_ped == PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	if (equip_ped != PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedBodWeaponCategory].at(lastSelectedBodWeapon);
	int weapHash = GAMEPLAY::GET_HASH_KEY((char*) weaponName.c_str());

	WEAPON::SET_PED_WEAPON_TINT_INDEX(playerPed, weapHash, choice.value);

	return true;
}

void onconfirm_open_tint_menu(MenuItem<int> choice) {
	int tintSelection = 0;

	std::string weaponValue = "";
	if (equip_ped == PLAYER::PLAYER_PED_ID()) weaponValue = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	if (equip_ped != PLAYER::PLAYER_PED_ID()) weaponValue = VOV_WEAPON_VALUES[lastSelectedBodWeaponCategory].at(lastSelectedBodWeapon);
	char *weaponChar = (char*)weaponValue.c_str();
	int weapHash = GAMEPLAY::GET_HASH_KEY(weaponChar);
	std::vector<MenuItem<int>*> menuItems;
		
	for (int i = 0; i < WEAPON::GET_WEAPON_TINT_COUNT(weapHash); i++) {
		MenuItem<int> *item = new MenuItem<int>();
		item->caption = UI::_GET_LABEL_TEXT((char*)("WCT_C_TINT_" + std::to_string(i)).c_str()); // CAPTIONS_TINT_MK2[i];
		item->value = i;
		menuItems.push_back(item);
	}

	/*
	for (int i = 0; i < WEAPONTYPES_TINT.size(); i++) {
		if (WEAPON::GET_PED_WEAPON_TINT_INDEX(playerPed, weapHash) == VALUES_TINT_MK2[i]) {
			tintSelection = i;
			break;
		}
	}
	else
	{
		for (int i = 0; i < VALUES_TINT.size(); i++) {
			MenuItem<int> *item = new MenuItem<int>();
			item->caption = CAPTIONS_TINT[i];
			item->value = VALUES_TINT[i];
			menuItems.push_back(item);
			}
		}
		*/

	draw_generic_menu<int>(menuItems, &tintSelection, "武器涂装颜色", onconfirm_weapon_mod_menu_tint, onhighlight_weapon_mod_menu_tint, NULL);
}

void onhighlight_weapon_mod_menu_tint_colour(MenuItem<int> choice) {
	onconfirm_weapon_mod_menu_tint_colour(choice);
}

bool onconfirm_weapon_mod_menu_tint_colour(MenuItem<int> choice) {
	Ped playerPed = equip_ped; 
	std::string weaponName = "";
	if (equip_ped == PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	if (equip_ped != PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedBodWeaponCategory].at(lastSelectedBodWeapon);
	int weapHash = GAMEPLAY::GET_HASH_KEY((char*)weaponName.c_str());

	WEAPON::SET_WEAPON_LIVERY_COLOR(playerPed, weapHash, GAMEPLAY::GET_HASH_KEY(currWeaponCompHash), choice.value);

	return true;
}

void onconfirm_open_tint_menu_colour(MenuItem<int> choice) {
	Ped playerPed = equip_ped;
	int tintColourSelection = 0;
	std::vector<MenuItem<int>*> menuItems;

	std::string weaponName = "";
	if (equip_ped == PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedWeaponCategory].at(lastSelectedWeapon);
	if (equip_ped != PLAYER::PLAYER_PED_ID()) weaponName = VOV_WEAPON_VALUES[lastSelectedBodWeaponCategory].at(lastSelectedBodWeapon);
	int weapHash = GAMEPLAY::GET_HASH_KEY((char*)weaponName.c_str());

	for each(char* MK2_wep in MK2_WEAPONS)
	{
		if (weapHash == GAMEPLAY::GET_HASH_KEY((char*)MK2_wep))
		{
			for each (char* MK2_wep_comp in MK2_WEAPONS_LIVERY_COMP)
			{
				if (WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, weapHash, GAMEPLAY::GET_HASH_KEY(MK2_wep_comp)))
				{
					currWeaponCompHash = MK2_wep_comp;

					for (int i = 0; i < WEAPON::GET_WEAPON_TINT_COUNT(weapHash); i++) {
						MenuItem<int> *item = new MenuItem<int>();
						item->caption = UI::_GET_LABEL_TEXT((char*)("WCT_C_TINT_" + std::to_string(i)).c_str()); //  CAPTIONS_TINT_MK2[i];
						item->value = i;
						menuItems.push_back(item);
					}
				}
			}

		}
		else
			set_status_text("应用武器涂装颜色时出错！");
	}

	draw_generic_menu<int>(menuItems, &tintColourSelection, "选择武器涂装颜色", onconfirm_weapon_mod_menu_tint_colour, onhighlight_weapon_mod_menu_tint_colour, NULL);
}

void add_weapon_feature_enablements(std::vector<FeatureEnabledLocalDefinition>* results){
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponExplosiveAmmo", &featureWeaponExplosiveAmmo});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponExplosiveMelee", &featureWeaponExplosiveMelee});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponExplosiveGrenades", &featureWeaponExplosiveGrenades});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponVacuumGrenades", &featureWeaponVacuumGrenades});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponFireAmmo", &featureWeaponFireAmmo});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponInfiniteAmmo", &featureWeaponInfiniteAmmo});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponInfiniteParachutes", &featureWeaponInfiniteParachutes, &featureWeaponInfiniteParachutesUpdated });
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponNoParachutes", &featureWeaponNoParachutes, &featureWeaponNoParachutesUpdated });
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponNoReload", &featureWeaponNoReload});
	results->push_back(FeatureEnabledLocalDefinition{"featureCopTakeWeapon", &featureCopTakeWeapon });
	results->push_back(FeatureEnabledLocalDefinition{"featureGravityGun", &featureGravityGun});
	results->push_back(FeatureEnabledLocalDefinition{"featureFriendlyFire", &featureFriendlyFire});
	results->push_back(FeatureEnabledLocalDefinition{"featureRapidFire", &featureRapidFire});
	results->push_back(FeatureEnabledLocalDefinition{"featureDropWeapon", &featureDropWeapon});
	results->push_back(FeatureEnabledLocalDefinition{"featureDropWeaponOutAmmo", &featureDropWeaponOutAmmo});
	results->push_back(FeatureEnabledLocalDefinition{"featureCanDisarmNPC", &featureCanDisarmNPC});
	results->push_back(FeatureEnabledLocalDefinition{"featurePedNoWeaponDrop", &featurePedNoWeaponDrop});
	results->push_back(FeatureEnabledLocalDefinition{"featurePowerPunch", &featurePowerPunch});
	results->push_back(FeatureEnabledLocalDefinition{"featureGiveAllWeapons", &featureGiveAllWeapons});
	results->push_back(FeatureEnabledLocalDefinition{"featureAddAllWeaponsAttachments", &featureAddAllWeaponsAttachments});
	results->push_back(FeatureEnabledLocalDefinition{"featureCopArmedWith", &featureCopArmedWith});
	results->push_back(FeatureEnabledLocalDefinition{"featurePedAgainstWeapons", &featurePedAgainstWeapons});
	results->push_back(FeatureEnabledLocalDefinition{"featureAgainstMeleeWeapons", &featureAgainstMeleeWeapons});
	results->push_back(FeatureEnabledLocalDefinition{"featurePedAgainst", &featurePedAgainst});
	results->push_back(FeatureEnabledLocalDefinition{"featurePunchFists", &featurePunchFists});
	results->push_back(FeatureEnabledLocalDefinition{"featurePunchMeleeWeapons", &featurePunchMeleeWeapons});
	results->push_back(FeatureEnabledLocalDefinition{"featurePunchFireWeapons", &featurePunchFireWeapons});
	results->push_back(FeatureEnabledLocalDefinition{"featureDriverAgainst", &featureDriverAgainst});
	results->push_back(FeatureEnabledLocalDefinition{"featurePoliceAgainst", &featurePoliceAgainst});
	results->push_back(FeatureEnabledLocalDefinition{"featurePlayerMelee", &featurePlayerMelee});
	results->push_back(FeatureEnabledLocalDefinition{"featureSwitchWeaponIfDanger", &featureSwitchWeaponIfDanger});
	results->push_back(FeatureEnabledLocalDefinition{"featureArmyMelee", &featureArmyMelee});
	results->push_back(FeatureEnabledLocalDefinition{"featureDetainedIfNotMove", &featureDetainedIfNotMove});
	results->push_back(FeatureEnabledLocalDefinition{"featureWeaponsCrosshair", &featureWeaponsCrosshair});
	results->push_back(FeatureEnabledLocalDefinition{"featureVehicleModelGun", &featureVehicleModelGun, &featureVehicleModelGunUpdated});
	results->push_back(FeatureEnabledLocalDefinition{"featurePedModelGun", &featurePedModelGun, &featurePedModelGunUpdated});
	results->push_back(FeatureEnabledLocalDefinition{"featureVehicleModelGunInvincible", &featureVehicleModelGunInvincible});
	results->push_back(FeatureEnabledLocalDefinition{"featurePedModelGunInvincible", &featurePedModelGunInvincible});
}

void add_weapon_feature_enablements2(std::vector<StringPairSettingDBRow>* results)
{
	results->push_back(StringPairSettingDBRow{ "CopCurrArmedIndex", std::to_string(CopCurrArmedIndex) });
	results->push_back(StringPairSettingDBRow{ "VehCurrWeaponIndex", std::to_string(VehCurrWeaponIndex) });
	results->push_back(StringPairSettingDBRow{ "WeaponsNoReticle", std::to_string(WeaponsNoReticle) });
	results->push_back(StringPairSettingDBRow{ "WeaponsSavedLoad", std::to_string(WeaponsSavedLoad) });
	results->push_back(StringPairSettingDBRow{ "CopAlarmIndex", std::to_string(CopAlarmIndex) });
	results->push_back(StringPairSettingDBRow{ "ChancePoliceCallingIndex", std::to_string(ChancePoliceCallingIndex) });
	results->push_back(StringPairSettingDBRow{ "ChanceAttackingYouIndex", std::to_string(ChanceAttackingYouIndex) });
	results->push_back(StringPairSettingDBRow{ "SniperVisionIndex", std::to_string(SniperVisionIndex) });
	results->push_back(StringPairSettingDBRow{ "PowerPunchIndex", std::to_string(PowerPunchIndex) });
	results->push_back(StringPairSettingDBRow{ "WeaponsFireModeIndex", std::to_string(WeaponsFireModeIndex) });
	results->push_back(StringPairSettingDBRow{ "RapidFireIndex", std::to_string(RapidFireIndex) });
	results->push_back(StringPairSettingDBRow{ "WeapStrobeIndexN", std::to_string(WeapStrobeIndexN) });
	results->push_back(StringPairSettingDBRow{ "WeapFlashDistIndex", std::to_string(WeapFlashDistIndex) });
	results->push_back(StringPairSettingDBRow{ "WeaponsCrosshairStyleIndex", std::to_string(WeaponsCrosshairStyleIndex) });
	results->push_back(StringPairSettingDBRow{ "WeaponsCrosshairColorIndex", std::to_string(WeaponsCrosshairColorIndex) });
	results->push_back(StringPairSettingDBRow{ "VehicleModelGunCategoryIndex", std::to_string(VehicleModelGunCategoryIndex) });
	results->push_back(StringPairSettingDBRow{ "VehicleModelGunSpeedIndex", std::to_string(VehicleModelGunSpeedIndex) });
	results->push_back(StringPairSettingDBRow{ "PedModelGunCategoryIndex", std::to_string(PedModelGunCategoryIndex) });
	results->push_back(StringPairSettingDBRow{ "PedModelGunSpeedIndex", std::to_string(PedModelGunSpeedIndex) });
}

void onchange_weap_dmg_modifier(int value, SelectFromListMenuItem* source){
	weapDmgModIndex = value;
}

void add_weapons_generic_settings(std::vector<StringPairSettingDBRow>* results){
	results->push_back(StringPairSettingDBRow{"lastCustomWeapon", lastCustomWeapon});
	results->push_back(StringPairSettingDBRow{"lastPowerWeapon", lastPowerWeapon});
	results->push_back(StringPairSettingDBRow{"weapDmgModIndex", std::to_string(weapDmgModIndex)});
}

void handle_generic_settings_weapons(std::vector<StringPairSettingDBRow>* settings){
	for(int i = 0; i < settings->size(); i++){
		StringPairSettingDBRow setting = settings->at(i);
		if(setting.name.compare("weapDmgModIndex") == 0){
			weapDmgModIndex = stoi(setting.value);
		}
		else if (setting.name.compare("CopCurrArmedIndex") == 0){
			CopCurrArmedIndex = stoi(setting.value);
		}
		else if (setting.name.compare("VehCurrWeaponIndex") == 0) {
			VehCurrWeaponIndex = stoi(setting.value);
		}
		else if (setting.name.compare("WeaponsNoReticle") == 0) {
			WeaponsNoReticle = stoi(setting.value);
		}
		else if (setting.name.compare("WeaponsSavedLoad") == 0) {
			WeaponsSavedLoad = stoi(setting.value);
		}
		else if (setting.name.compare("CopAlarmIndex") == 0){
			CopAlarmIndex = stoi(setting.value);
		}
		else if (setting.name.compare("ChancePoliceCallingIndex") == 0){
			ChancePoliceCallingIndex = stoi(setting.value);
		}
		else if (setting.name.compare("ChanceAttackingYouIndex") == 0){
			ChanceAttackingYouIndex = stoi(setting.value);
		}
		else if (setting.name.compare("SniperVisionIndex") == 0){
			SniperVisionIndex = stoi(setting.value);
		}
		else if (setting.name.compare("PowerPunchIndex") == 0) {
			PowerPunchIndex = stoi(setting.value);
		}
		else if (setting.name.compare("WeaponsFireModeIndex") == 0) {
			WeaponsFireModeIndex = stoi(setting.value);
		}
		else if (setting.name.compare("RapidFireIndex") == 0) {
			RapidFireIndex = stoi(setting.value);
		}
		else if (setting.name.compare("WeapStrobeIndexN") == 0) {
			WeapStrobeIndexN = stoi(setting.value);
		}
		else if (setting.name.compare("WeapFlashDistIndex") == 0) {
			WeapFlashDistIndex = stoi(setting.value);
		}
		else if (setting.name.compare("lastCustomWeapon") == 0) {
			lastCustomWeapon = setting.value;
		}
		else if (setting.name.compare("lastPowerWeapon") == 0) {
			lastPowerWeapon = setting.value;
		}
		else if (setting.name.compare("WeaponsCrosshairStyleIndex") == 0) {
			WeaponsCrosshairStyleIndex = stoi(setting.value);
			WeaponsCrosshairStyleChanged = true;
		}
		else if (setting.name.compare("WeaponsCrosshairColorIndex") == 0) {
			WeaponsCrosshairColorIndex = stoi(setting.value);
			WeaponsCrosshairColorChanged = true;
		}
		else if (setting.name.compare("VehicleModelGunCategoryIndex") == 0) {
			VehicleModelGunCategoryIndex = stoi(setting.value);
			VehicleModelGunCategoryChanged = true;
		}
		else if (setting.name.compare("VehicleModelGunSpeedIndex") == 0) {
			VehicleModelGunSpeedIndex = stoi(setting.value);
			VehicleModelGunSpeedChanged = true;
		}
		else if (setting.name.compare("PedModelGunCategoryIndex") == 0) {
			PedModelGunCategoryIndex = stoi(setting.value);
			PedModelGunCategoryChanged = true;
		}
		else if (setting.name.compare("PedModelGunSpeedIndex") == 0) {
			PedModelGunSpeedIndex = stoi(setting.value);
			PedModelGunSpeedChanged = true;
		}
	}
}

// 绘制全局屏幕准星
void draw_weapons_crosshair() {
	if (!featureWeaponsCrosshair) {
		return; // 屏幕准星功能未启用
	}

	// 中心点
	float centerX = 0.5f;
	float centerY = 0.5f;

	// 尺寸（像素）
	float lengthPx = 20.0f;//屏幕准星长度
	float thickPx = 2.0f;//屏幕准星厚度

	// 屏幕分辨率
	int screenW, screenH;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screenW, &screenH);

	// 换算尺寸
	float lengthX = lengthPx / (float)screenW;
	float lengthY = lengthPx / (float)screenH;
	float thickX = thickPx / (float)screenW;
	float thickY = thickPx / (float)screenH;

	// 颜色映射
	int r = 255, g = 255, b = 255, a = 255;
	switch (WeaponsCrosshairColorIndex) {
		case 1: r = 255; g = 0;   b = 0;   break; // 红色
		case 2: r = 255; g = 105; b = 180; break; // 粉红色
		case 3: r = 0;   g = 255; b = 0;   break; // 绿色
		case 4: r = 0;   g = 122; b = 255; break; // 蓝色
		case 5: r = 255; g = 242; b = 0;   break; // 黄色
		case 6: r = 255; g = 165; b = 0;   break; // 橙色
		case 7: r = 128; g = 0;   b = 128; break; // 紫色
		case 8: r = 0;   g = 0;   b = 0;   break; // 黑色
		case 9: r = 128; g = 128; b = 128; break; // 灰色
		default: break; // 白色（默认）
	}

	int styleValue = WEAPONS_CROSSHAIR_STYLE_VALUES[WeaponsCrosshairStyleIndex];

	if (styleValue == 1) {
		// 实线屏幕准星
		GRAPHICS::DRAW_RECT(centerX, centerY, lengthX, thickY, r, g, b, a); // 水平线
		GRAPHICS::DRAW_RECT(centerX, centerY, thickX, lengthY, r, g, b, a); // 垂直线
	} else if (styleValue == 2) {
		// 虚线屏幕准星
		int segments = 2;//虚线屏幕准星线段数量
		float gapPx = 8.0f;//虚线屏幕准星线段间距
		float totalGapPx = (segments - 1) * gapPx;
		float segLengthPx = (lengthPx - totalGapPx) / (float)segments;
		float segLenX = segLengthPx / (float)screenW;
		float segLenY = segLengthPx / (float)screenH;
		float gapX = gapPx / (float)screenW;
		float gapY = gapPx / (float)screenH;

		// 绘制水平虚线段
		for (int i = 0; i < segments; ++i) {
			float offsetX = (segLenX + gapX) * (i - (segments - 1) * 0.5f);
			GRAPHICS::DRAW_RECT(centerX + offsetX, centerY, segLenX, thickY, r, g, b, a);
		}
		// 绘制垂直虚线段
		for (int i = 0; i < segments; ++i) {
			float offsetY = (segLenY + gapY) * (i - (segments - 1) * 0.5f);
			GRAPHICS::DRAW_RECT(centerX, centerY + offsetY, thickX, segLenY, r, g, b, a);
		}
	} else if (styleValue == 3) {
		// 空心方块准星
		float boxSizePx = 10.0f; // 方块半边长（像素）- 可修改此值调整方块大小
		float boxThicknessPx = 2.0f; // 方块边框粗细（像素）- 可修改此值调整边框粗细
		
		// 计算方块尺寸
		float boxSizeX = boxSizePx / (float)screenW;
		float boxSizeY = boxSizePx / (float)screenH;
		float boxThickX = boxThicknessPx / (float)screenW;
		float boxThickY = boxThicknessPx / (float)screenH;
		
		// 绘制四条边框线（确保四个角重叠连接）
		// 上边（包含完整宽度加边框厚度，确保角落重叠）
		GRAPHICS::DRAW_RECT(centerX, centerY - boxSizeY, boxSizeX * 2.0f + boxThickX, boxThickY, r, g, b, a);
		// 下边（包含完整宽度加边框厚度，确保角落重叠）
		GRAPHICS::DRAW_RECT(centerX, centerY + boxSizeY, boxSizeX * 2.0f + boxThickX, boxThickY, r, g, b, a);
		// 左边（包含完整高度，与上下边重叠）
		GRAPHICS::DRAW_RECT(centerX - boxSizeX, centerY, boxThickX, boxSizeY * 2.0f, r, g, b, a);
		// 右边（包含完整高度，与上下边重叠）
		GRAPHICS::DRAW_RECT(centerX + boxSizeX, centerY, boxThickX, boxSizeY * 2.0f, r, g, b, a);
	} else if (styleValue == 4) {
		// 实心方块准星
		float boxSizePx = 1.5f; // 方块半边长（像素）- 可修改此值调整方块大小
		
		// 计算方块尺寸
		float boxSizeX = boxSizePx / (float)screenW;
		float boxSizeY = boxSizePx / (float)screenH;
		
		// 绘制实心方块（一个矩形）
		GRAPHICS::DRAW_RECT(centerX, centerY, boxSizeX * 2.0f, boxSizeY * 2.0f, r, g, b, a);
	}
}

// 屏幕准星样式回调函数
void onchange_weapons_crosshair_style_index(int value, SelectFromListMenuItem* source) {
	WeaponsCrosshairStyleIndex = value;
	WeaponsCrosshairStyleChanged = true;
}

// 屏幕准星颜色回调函数
void onchange_weapons_crosshair_color_index(int value, SelectFromListMenuItem* source) {
	WeaponsCrosshairColorIndex = value;
	WeaponsCrosshairColorChanged = true;
}

// 屏幕准星子菜单处理函数
bool process_weapons_crosshair_menu() {
	const std::string caption = "屏幕准星设置";
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem* listItem;
	ToggleMenuItem<int>* toggleItem;
	
	int i = 0;
	
	// 启用/关闭屏幕准星
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用屏幕准星";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureWeaponsCrosshair;
	menuItems.push_back(toggleItem);
	
	// 屏幕准星样式选择
	listItem = new SelectFromListMenuItem(WEAPONS_CROSSHAIR_STYLE_CAPTIONS, onchange_weapons_crosshair_style_index);
	listItem->wrap = false;
	listItem->caption = "屏幕准星样式";
	listItem->value = WeaponsCrosshairStyleIndex;
	menuItems.push_back(listItem);
	
	// 屏幕准星颜色选择
	listItem = new SelectFromListMenuItem(WEAPONS_CROSSHAIR_COLOR_CAPTIONS, onchange_weapons_crosshair_color_index);
	listItem->wrap = false;
	listItem->caption = "屏幕准星颜色";
	listItem->value = WeaponsCrosshairColorIndex;
	menuItems.push_back(listItem);
	
	return draw_generic_menu<int>(menuItems, &activeLineIndexWeapon, caption, NULL, NULL, NULL);
}

// 载具模型枪回调函数
void onchange_vehicle_model_gun_category_index(int value, SelectFromListMenuItem* source) {
	VehicleModelGunCategoryIndex = value;
	
	// 检测：当切换到特种车辆（索引11）且开启了永不通缉时，显示警告
	if (value == 11 && featurePlayerNeverWanted) {
		set_status_text("~r~警告: 选择特种车辆分类时！");
		set_status_text("~r~注意: 请不要开启永不通缉！");
	}
}

void onchange_vehicle_model_gun_speed_index(int value, SelectFromListMenuItem* source) {
	VehicleModelGunSpeedIndex = value;
}

// 角色模型枪回调函数
void onchange_ped_model_gun_category_index(int value, SelectFromListMenuItem* source) {
	PedModelGunCategoryIndex = value;
}

void onchange_ped_model_gun_speed_index(int value, SelectFromListMenuItem* source) {
	PedModelGunSpeedIndex = value;
}

// 载具模型枪菜单处理
bool process_vehicle_model_gun_menu() {
	const std::string caption = "载具模型枪设置";
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem* listItem;
	ToggleMenuItem<int>* toggleItem;
	
	int i = 0;
	
	// 启用/关闭载具模型枪
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用载具模型枪";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureVehicleModelGun;
	toggleItem->toggleValueUpdated = &featureVehicleModelGunUpdated;
	menuItems.push_back(toggleItem);
	
	// 载具分类选择
	listItem = new SelectFromListMenuItem(VEHICLE_MODEL_GUN_CATEGORIES, onchange_vehicle_model_gun_category_index);
	listItem->wrap = false;
	listItem->caption = "选择载具模型";
	listItem->value = VehicleModelGunCategoryIndex;
	menuItems.push_back(listItem);
	
	// 载具发射速度选择
	listItem = new SelectFromListMenuItem(MODEL_GUN_SPEED_CAPTIONS, onchange_vehicle_model_gun_speed_index);
	listItem->wrap = false;
	listItem->caption = "载具发射速度";
	listItem->value = VehicleModelGunSpeedIndex;
	menuItems.push_back(listItem);
	
	// 载具无敌开关
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "开启载具无敌";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureVehicleModelGunInvincible;
	menuItems.push_back(toggleItem);
	
	return draw_generic_menu<int>(menuItems, &activeLineIndexWeapon, caption, NULL, NULL, NULL);
}

// 角色模型枪菜单处理
bool process_ped_model_gun_menu() {
	const std::string caption = "角色模型枪设置";
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem* listItem;
	ToggleMenuItem<int>* toggleItem;
	
	int i = 0;
	
	// 启用/关闭角色模型枪
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用角色模型枪";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePedModelGun;
	toggleItem->toggleValueUpdated = &featurePedModelGunUpdated;
	menuItems.push_back(toggleItem);
	
	// 角色分类选择
	listItem = new SelectFromListMenuItem(PED_MODEL_GUN_CATEGORIES, onchange_ped_model_gun_category_index);
	listItem->wrap = false;
	listItem->caption = "选择角色模型";
	listItem->value = PedModelGunCategoryIndex;
	menuItems.push_back(listItem);
	
	// 角色发射速度选择
	listItem = new SelectFromListMenuItem(MODEL_GUN_SPEED_CAPTIONS, onchange_ped_model_gun_speed_index);
	listItem->wrap = false;
	listItem->caption = "角色发射速度";
	listItem->value = PedModelGunSpeedIndex;
	menuItems.push_back(listItem);
	
	// 角色无敌开关
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "开启角色无敌";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featurePedModelGunInvincible;
	menuItems.push_back(toggleItem);
	
	return draw_generic_menu<int>(menuItems, &activeLineIndexWeapon, caption, NULL, NULL, NULL);
}

// 获取随机载具Hash（根据NativeTrainerConfig.xml的分类）- 随机不重复模式
Hash get_random_vehicle_hash_by_category(int categoryIndex) {
	// 根据NativeTrainerConfig.xml的分类数据定义车辆模型
	static const std::vector<std::vector<std::string>> VEHICLE_MODELS_BY_CATEGORY = {
		// 小型汽车
		{"ASBO", "PANTO", "RHAPSODY", "WEEVIL", "CLUB", "ISSI3", "BRIOSO2", "BRIOSO3", "PRAIRIE", "BLISTA"},
		
		// 轿车
		{"PRIMO", "CINQUEMILA", "ASTEROPE2", "DEITY", "COG55", "SUPERD", "TAILGATER2", "MINIMUS", "SURGE", "FUGITIVE"},
		
		// SUV
		{"TOROS", "NOVAK", "GRESLEY", "CASTIGATOR", "VIVANITE", "JUBILEE", "IWAGEN", "CONTENDER", "REBLA", "XLS"},
		
		// 轿跑车
		{"KANJOSJ", "FELON", "WINDSOR", "F620", "JACKAL", "EXEMPLAR", "FR36", "ORACLE", "SENTINEL", "ZION"},
		
		// 肌肉车
		{"VIRGO", "BUFFALO4", "GAUNTLET", "DOMINATOR7", "DOMINATOR3", "SLAMVAN3", "SABREGT", "VIGERO2", "HERMES", "RUINER4"},
		
		// 经典跑车
		{"MANANA", "CASCO", "PIGALLE", "GT500", "PEYOTE", "DYNASTY", "JB700", "MAMBA", "FELTZER3", "COQUETTE5"},
		
		// 跑车
		{"JESTER", "DRIFTJESTER", "BANSHEE", "KURUMA", "ITALIGTO", "ITALIRSX", "PENUMBRA2", "TENF", "TENF2", "SEVEN70"},
		
		// 超级跑车
		{"IGNUS", "ZENTORNO", "REAPER", "BANSHEE2", "TURISMO3", "VOLTIC2", "NERO2", "ADDER", "THRAX", "PFISTER811"},
		
		// 摩托车
		{"POLICEB2", "DOUBLE", "AKUMA", "BATI2", "THRUST", "NEMESIS", "SOVEREIGN", "HAKUCHOU", "STRYDER", "BF400"},
		
		// 越野车
		{"INSURGENT2", "DLOADER", "KAMACHO", "MESA", "VERUS", "CARACARA2", "DRAUGUR", "PATRIOT3", "DUBSTA3", "BLAZER2"},
		
		// 开轮式
		{"FORMULA", "FORMULA2", "OPENWHEEL2", "OPENWHEEL1"},
		
		// 特种车
		{"FBI", "FIRETRUK", "POLDORADO", "POLGAUNTLET", "POLICE4", "PRANGER", "LGUARD", "POLICE", "SHERIFF2", "POLICET"},
		
		// 厢型车
		{"PARADISE", "YOUGA", "RUMPO3", "BISON", "YOUGA4", "SPEEDO2", "MINIVAN", "MINIVAN2", "BOBCATXL", "PONY"},
		
		// 自行车
		{"BMX", "SCORCHER", "CRUISER", "TRIBIKE", "INDUCTOR2", "TRIBIKE2", "TRIBIKE3", "INDUCTOR", "FIXTER"},
		
		// 直升机
		{"HUNTER", "VALKYRIE", "MAVERICK", "VOLATUS", "SWIFT", "BUZZARD", "FROGGER", "SAVAGE", "AKULA", "POLMAV"},
		
		// 飞机
		{"STRIKEFORCE", "TULA", "LAZER", "BOMBUSHKA", "CUBAN800", "ALKONOST", "TITAN", "DUSTER", "LUXOR", "BESRA"},
		
		// 船只
		{"SPEEDER", "TORO", "PATROLBOAT", "SEASHARK", "AVISA", "PREDATOR", "SUNTRAP", "TROPIC", "LONGFIN", "DINGHY"}
	};
	
	// 洗牌算法相关静态变量
	static int lastCategoryIndex = -1; // 上次使用的分类
	static int currentIndex = 0; // 当前使用的索引
	static std::vector<int> shuffledIndices; // 洗牌后的索引列表
	
	if (categoryIndex < 0 || categoryIndex >= VEHICLE_MODELS_BY_CATEGORY.size()) {
		categoryIndex = 0; // 默认到小型汽车分类
	}
	
	const auto& vehList = VEHICLE_MODELS_BY_CATEGORY[categoryIndex];
	if (vehList.empty()) {
		return GAMEPLAY::GET_HASH_KEY((char*)"ADDER"); // 默认车辆
	}
	
	// 检查是否需要重新洗牌（分类切换或遍历完成）
	if (lastCategoryIndex != categoryIndex || currentIndex >= shuffledIndices.size()) {
		// 初始化索引列表（0, 1, 2, ..., size-1）
		shuffledIndices.clear();
		for (int i = 0; i < vehList.size(); ++i) {
			shuffledIndices.push_back(i);
		}
		
		// Fisher-Yates 洗牌算法
		for (int i = shuffledIndices.size() - 1; i > 0; --i) {
			int j = rand() % (i + 1);
			std::swap(shuffledIndices[i], shuffledIndices[j]);
		}
		
		currentIndex = 0;
		lastCategoryIndex = categoryIndex;
	}
	
	// 获取当前模型并递增索引
	int modelIndex = shuffledIndices[currentIndex];
	currentIndex++;
	
	return GAMEPLAY::GET_HASH_KEY((char*)vehList[modelIndex].c_str());
}

// 获取随机角色Hash（根据ent-Peds-1.xml的分类）- 随机不重复模式
Hash get_random_ped_hash_by_category(int categoryIndex) {
	// 根据ent-Peds-1.xml的分类数据定义角色模型
	static const std::vector<std::vector<std::string>> PED_MODELS_BY_CATEGORY = {
		// 主角
		//{"player_zero", "player_one", "player_two", "p_franklin_02"},
		
		// 环境女性
		{"a_f_m_beach_01", "a_f_m_bevhills_01", "a_f_m_bodybuild_01", "a_f_m_downtown_01", "a_f_m_genbiker_01", "a_f_m_genstreet_01", "a_f_m_prolhost_01", "a_f_m_tourist_01", "a_f_m_trampbeac_01", "a_f_y_beach_02"},
		
		// 环境男性
		{"a_m_m_acult_01", "a_m_m_afriamer_01", "a_m_m_beach_01", "a_m_m_beach_02", "a_m_m_bevhills_01", "a_m_m_bevhills_02", "a_m_m_business_01", "a_m_m_eastsa_01", "a_m_m_eastsa_02", "a_m_m_farmer_01"},
		
		// 过场动画
		{"cs_amandatownley", "cs_andreas", "cs_ashley", "cs_bankman", "cs_barry", "cs_beverly", "cs_brad", "cs_carbuyer", "cs_casey", "cs_chengsr"},
		
		// 帮派女性
		{"g_f_importexport_01", "g_f_y_ballas_01", "g_f_y_families_01", "g_f_y_lost_01", "g_f_y_vagos_01", "g_f_m_undeadmage", "g_f_m_fooliganz_01"},
		
		// 帮派男性
		{"g_m_importexport_01", "g_m_m_armboss_01", "g_m_m_armgoon_01", "g_m_m_armlieut_01", "g_m_m_casrn_01", "g_m_m_chemwork_01", "g_m_m_chiboss_01", "g_m_m_chicold_01", "g_m_m_chigoon_01", "g_m_m_chigoon_02"},
		
		// 故事模式
		{"ig_abigail", "ig_acidlabcook", "ig_agatha", "ig_agent", "ig_agent_02", "ig_ahronward", "ig_amandatownley", "ig_andreas", "ig_armsmanufac_01", "ig_ary"},
		
		// 线上模式
		{"mp_f_bennymech_01", "mp_f_boatstaff_01", "mp_f_cardesign_01", "mp_f_chbar_01", "mp_f_cocaine_01", "mp_f_counterfeit_01", "mp_f_deadhooker", "mp_f_execpa_01", "mp_f_execpa_02", "mp_f_forgery_01"},
		
		// 场景女性
		{"s_f_m_autoshop_01", "s_f_m_fembarber", "s_f_m_maid_01", "s_f_m_retailstaff_01", "s_f_m_shop_high", "s_f_m_studioassist_01", "s_f_m_sweatshop_01", "s_f_m_warehouse_01", "s_f_y_airhostess_01", "s_f_y_bartender_01"},
		
		// 场景男性
		{"s_m_m_ammucountry", "s_m_m_armoured_01", "s_m_m_armoured_02", "s_m_m_autoshop_01", "s_m_m_autoshop_02", "s_m_m_bailoffice_01", "s_m_m_bouncer_01", "s_m_m_ccrew_01", "s_m_m_ccrew_02", "s_m_m_ccrew_03"},
		
		// 剧情场景女性
		{"u_f_y_comjane", "u_f_y_princess", "u_f_y_mistress", "u_f_o_carol", "u_f_o_eileen", "u_f_y_beth", "u_f_y_lauren", "u_f_y_taylor", "u_f_m_miranda", "u_f_m_miranda_02"},
		
		// 剧情场景男性
		{"u_m_y_mani", "u_m_y_chip", "u_m_y_ushi", "u_m_m_jesus_01", "u_m_y_prisoner_01", "u_m_m_blane", "u_m_m_curtis", "u_m_y_abner", "u_m_o_dean", "u_m_m_griff_01"},
		
		// 其他角色
		{"ig_furry", "hc_driver", "hc_gunman", "hc_hacker", "u_m_m_jewelsec_01", "u_m_m_jewelthief", "u_f_y_jewelass_01", "u_m_o_filmnoir", "ig_zombie_dj_01", "u_m_y_zombie_01"},
	
		// 动物
		{"a_c_cat_01", "a_c_chimp_02", "a_c_chop", "a_c_cow", "a_c_coyote", "a_c_deer", "a_c_husky", "a_c_pig", "a_c_rabbit_01", "a_c_rhesus"}
	};
	
	// 洗牌算法相关静态变量
	static int lastCategoryIndex = -1; // 上次使用的分类
	static int currentIndex = 0; // 当前使用的索引
	static std::vector<int> shuffledIndices; // 洗牌后的索引列表
	
	if (categoryIndex < 0 || categoryIndex >= PED_MODELS_BY_CATEGORY.size()) {
		categoryIndex = 0; // 默认到环境女性分类
	}
	
	const auto& pedList = PED_MODELS_BY_CATEGORY[categoryIndex];
	if (pedList.empty()) {
		return GAMEPLAY::GET_HASH_KEY((char*)"a_f_y_beach_02"); // 默认角色
	}
	
	// 检查是否需要重新洗牌（分类切换或遍历完成）
	if (lastCategoryIndex != categoryIndex || currentIndex >= shuffledIndices.size()) {
		// 初始化索引列表（0, 1, 2, ..., size-1）
		shuffledIndices.clear();
		for (int i = 0; i < pedList.size(); ++i) {
			shuffledIndices.push_back(i);
		}
		
		// Fisher-Yates 洗牌算法
		for (int i = shuffledIndices.size() - 1; i > 0; --i) {
			int j = rand() % (i + 1);
			std::swap(shuffledIndices[i], shuffledIndices[j]);
		}
		
		currentIndex = 0;
		lastCategoryIndex = categoryIndex;
	}
	
	// 获取当前模型并递增索引
	int modelIndex = shuffledIndices[currentIndex];
	currentIndex++;
	
	return GAMEPLAY::GET_HASH_KEY((char*)pedList[modelIndex].c_str());
}

// 辅助函数：将角度转换为弧度
Vector3 DegreeToRadian(Vector3 angles) {
	Vector3 result;
	result.x = angles.x * 0.0174532925199433f;
	result.y = angles.y * 0.0174532925199433f;
	result.z = angles.z * 0.0174532925199433f;
	return result;
}

// 辅助函数：根据相机计算指定距离的坐标（参考MenyooSP实现）
Vector3 get_coords_from_gameplay_cam(float distance) {
	Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	Vector3 camCoord = CAM::GET_GAMEPLAY_CAM_COORD();
	
	// 将旋转角度转换为弧度
	Vector3 rotRad = DegreeToRadian(camRot);
	
	// 根据相机旋转和距离计算新坐标
	rotRad.y = distance * cos(rotRad.x);
	camCoord.x = camCoord.x + rotRad.y * sin(rotRad.z * -1.0f);
	camCoord.y = camCoord.y + rotRad.y * cos(rotRad.z * -1.0f);
	camCoord.z = camCoord.z + distance * sin(rotRad.x);
	
	return camCoord;
}

// 载具模型枪发射函数
void fire_vehicle_model_gun() {
	if (!featureVehicleModelGun) return;
	
	// 射速限制：防止高射速武器（如加特林）导致崩溃
	static DWORD lastFireTime = 0;
	const DWORD MIN_FIRE_INTERVAL = 150; // 最小发射间隔（毫秒），约每秒6-7发
	
	DWORD currentTime = GetTickCount();
	if (currentTime - lastFireTime < MIN_FIRE_INTERVAL) {
		return; // 未到达最小间隔，跳过本次发射
	}
	lastFireTime = currentTime;
	
	static Hash nextVehicleHash = 0; // 正在加载的模型
	static Hash readyVehicleHash = 0; // 已加载准备发射的模型

	// 1. 预加载逻辑（确保始终有一个模型在后台加载）
	if (nextVehicleHash == 0) {
		nextVehicleHash = get_random_vehicle_hash_by_category(VehicleModelGunCategoryIndex);
		
		if (!STREAMING::IS_MODEL_IN_CDIMAGE(nextVehicleHash) || !STREAMING::IS_MODEL_A_VEHICLE(nextVehicleHash)) {
			nextVehicleHash = GAMEPLAY::GET_HASH_KEY((char*)"ADDER");
		}
		STREAMING::REQUEST_MODEL(nextVehicleHash);
	}
	else if (STREAMING::HAS_MODEL_LOADED(nextVehicleHash)) {
		// 新模型加载完成，更新到ready状态
		// 如果旧的ready模型存在且不同，释放它
		if (readyVehicleHash != 0 && readyVehicleHash != nextVehicleHash) {
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(readyVehicleHash);
		}
		readyVehicleHash = nextVehicleHash;
		nextVehicleHash = 0; // 重置next，以便下一帧加载新模型
	}

	// 2. 发射逻辑（使用已加载好的ready模型，无需等待）
	if (readyVehicleHash != 0 && STREAMING::HAS_MODEL_LOADED(readyVehicleHash)) {
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
		Vector3 camPos = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		
		Vector3 minDim, maxDim;
		GAMEPLAY::GET_MODEL_DIMENSIONS(readyVehicleHash, &minDim, &maxDim);
		
		// 计算生成位置（根据分类调整距离，避免大型载具撞到玩家）
		float camToPlayerDist = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(camPos.x, camPos.y, camPos.z, playerPos.x, playerPos.y, playerPos.z, true);
		float extraDistance = 1.5f; // 默认额外距离
		
		// 为大型飞行器增加额外生成距离，避免撞到玩家
		if (VehicleModelGunCategoryIndex == 14) { // 直升机
			extraDistance = 6.0f; // 直升机需要更大的安全距离
		} else if (VehicleModelGunCategoryIndex == 15) { // 飞机
			extraDistance = 8.0f; // 飞机需要最大的安全距离
		}
		
		float spawnDistance = camToPlayerDist + maxDim.y + extraDistance;
		Vector3 spawnPos = get_coords_from_gameplay_cam(spawnDistance);
		
		// 创建载具（使用相机的旋转方向）
		Vehicle spawnedVeh = VEHICLE::CREATE_VEHICLE(readyVehicleHash, spawnPos.x, spawnPos.y, spawnPos.z, camRot.z, true, false);
		
		if (ENTITY::DOES_ENTITY_EXIST(spawnedVeh)) {
			// 设置载具旋转角度（船只需要特殊处理避免翻滚）
			if (VehicleModelGunCategoryIndex == 16) {
				// 船只特殊处理：船头跟随相机方向和俯仰角，但避免左右翻滚
				// pitch(x)跟随相机俯仰，roll(y)强制为0避免翻滚，yaw(z)跟随相机方向
				ENTITY::SET_ENTITY_ROTATION(spawnedVeh, camRot.x, 0.0f, camRot.z, 2, true);
			} else {
				// 其他载具：完全跟随相机旋转（车辆、飞机、直升机等）
				ENTITY::SET_ENTITY_ROTATION(spawnedVeh, camRot.x, camRot.y, camRot.z, 2, true);
			}
			
			ENTITY::SET_ENTITY_COLLISION(spawnedVeh, true, false);
			
			if (featureVehicleModelGunInvincible) {
				ENTITY::SET_ENTITY_INVINCIBLE(spawnedVeh, true);
			}
			
			// 移除最大速度限制，让物理引擎自然处理
			// ENTITY::SET_ENTITY_MAX_SPEED(spawnedVeh, 9999.0f);
			
			// 应用相对力：只施加向前的力，移除旋转力矩（偏移量改为0）
			float speed = MODEL_GUN_SPEED_VALUES[VehicleModelGunSpeedIndex];
			ENTITY::APPLY_FORCE_TO_ENTITY(spawnedVeh, 1, 0.0f, speed, 0.0f, 0.0f, 0.0f, 0.0f, 0, true, true, true, false, true);
			
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&spawnedVeh);
		}
	}
}

// 角色模型枪发射函数
void fire_ped_model_gun() {
	if (!featurePedModelGun) return;
	
	// 射速限制：防止高射速武器（如加特林）导致崩溃
	static DWORD lastFireTime = 0;
	const DWORD MIN_FIRE_INTERVAL = 150; // 最小发射间隔（毫秒），约每秒6-7发
	
	DWORD currentTime = GetTickCount();
	if (currentTime - lastFireTime < MIN_FIRE_INTERVAL) {
		return; // 未到达最小间隔，跳过本次发射
	}
	lastFireTime = currentTime;
	
	static Hash nextPedHash = 0; // 正在加载的模型
	static Hash readyPedHash = 0; // 已加载准备发射的模型
	static std::vector<Ped> pedGunSpawnedPeds; // 已发射的角色列表
	const int maxPedGunSpawned = 100; // 最大人物数量限制（循环保留100个）
	
	// 清理已不存在的实体（死亡或被删除的），避免计数一直增长
	for (auto it = pedGunSpawnedPeds.begin(); it != pedGunSpawnedPeds.end(); ) {
		if (!ENTITY::DOES_ENTITY_EXIST(*it)) {
			it = pedGunSpawnedPeds.erase(it);
		} else {
			++it;
		}
	}
	
	// 当达到上限时，删除最旧的人物（循环清理，始终保留100个）
	if ((int)pedGunSpawnedPeds.size() >= maxPedGunSpawned) {
		Ped oldestPed = pedGunSpawnedPeds.front();
		if (ENTITY::DOES_ENTITY_EXIST(oldestPed)) {
			//ENTITY::DELETE_ENTITY(&oldestPed);
			// 更推荐用 DELETE_PED，确保 ped 相关资源彻底清理
			PED::DELETE_PED(&oldestPed);
		}
		pedGunSpawnedPeds.erase(pedGunSpawnedPeds.begin());
	}

	// 1. 预加载逻辑（确保始终有一个模型在后台加载）
	if (nextPedHash == 0) {
		nextPedHash = get_random_ped_hash_by_category(PedModelGunCategoryIndex);
		
		if (!STREAMING::IS_MODEL_IN_CDIMAGE(nextPedHash)) {
			nextPedHash = GAMEPLAY::GET_HASH_KEY((char*)"a_f_y_beach_02");
		}
		STREAMING::REQUEST_MODEL(nextPedHash);
	}
	else if (STREAMING::HAS_MODEL_LOADED(nextPedHash)) {
		// 新模型加载完成，更新到ready状态
		if (readyPedHash != 0 && readyPedHash != nextPedHash) {
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(readyPedHash);
		}
		readyPedHash = nextPedHash;
		nextPedHash = 0; // 重置next
	}

	// 2. 发射逻辑（使用已加载好的ready模型）
	if (readyPedHash != 0 && STREAMING::HAS_MODEL_LOADED(readyPedHash)) {
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
		Vector3 camPos = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		
		Vector3 minDim, maxDim;
		GAMEPLAY::GET_MODEL_DIMENSIONS(readyPedHash, &minDim, &maxDim);
		
		// 计算生成位置
		float camToPlayerDist = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(camPos.x, camPos.y, camPos.z, playerPos.x, playerPos.y, playerPos.z, true);
		float spawnDistance = camToPlayerDist + maxDim.y + 0.5f;
		
		Vector3 spawnPos = get_coords_from_gameplay_cam(spawnDistance);
		
		// 创建角色
		Ped spawnedPed = PED::CREATE_PED(4, readyPedHash, spawnPos.x, spawnPos.y, spawnPos.z, camRot.z, true, true);
		
		if (ENTITY::DOES_ENTITY_EXIST(spawnedPed)) {
			pedGunSpawnedPeds.push_back(spawnedPed);
			
			// 只设置朝向（背对射击方向），保持站立姿势，不设置俯仰和翻滚
			ENTITY::SET_ENTITY_HEADING(spawnedPed, camRot.z + 360.0f);
			ENTITY::SET_ENTITY_COLLISION(spawnedPed, true, true);
			ENTITY::SET_ENTITY_HAS_GRAVITY(spawnedPed, true);
			ENTITY::SET_ENTITY_DYNAMIC(spawnedPed, true);
			
			if (featurePedModelGunInvincible) {
				ENTITY::SET_ENTITY_INVINCIBLE(spawnedPed, true);
				ENTITY::SET_ENTITY_PROOFS(spawnedPed, true, true, true, true, true, true, true, true);
			} else {
				ENTITY::SET_ENTITY_INVINCIBLE(spawnedPed, false);
				ENTITY::SET_ENTITY_HAS_GRAVITY(spawnedPed, true); // 确保开启重力
				ENTITY::SET_ENTITY_CAN_BE_DAMAGED(spawnedPed, true);
			}
			
			// 启用布娃娃物理
			PED::SET_PED_CAN_RAGDOLL(spawnedPed, true);
			PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(spawnedPed, true);
			PED::SET_PED_RAGDOLL_ON_COLLISION(spawnedPed, true);
			
			// 清除任务，禁止AI抢控制权
			AI::CLEAR_PED_TASKS_IMMEDIATELY(spawnedPed);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(spawnedPed, true);
			
			// 立刻进入布娃娃状态（类型0=自然掉落，更真实）
			// 无敌时快速恢复，不开无敌时正常受伤死亡
			if (featurePedModelGunInvincible) {
				PED::SET_PED_TO_RAGDOLL(spawnedPed, 3000, 3000, 0, true, true, false); // 无敌模式：3秒快速站起来
			} else {
				PED::SET_PED_TO_RAGDOLL(spawnedPed, 10000, 10000, 0, true, true, false); // 普通模式：10秒布娃娃，可能死亡
			}
			PED::SET_PED_RAGDOLL_FORCE_FALL(spawnedPed); // 强制进入下落/摔倒姿态，避免空中站立

			// 应用相机朝向速度：根据相机方向计算三维速度向量（与射击方向一致）
			Vector3 camRotRad = DegreeToRadian(camRot);
			float cosX = cos(camRotRad.x);
			Vector3 dir;
			dir.x = -sin(camRotRad.z) * cosX;
			dir.y =  cos(camRotRad.z) * cosX;
			dir.z =  sin(camRotRad.x);
			
			// 计算发射速度（1.25倍力度）
			float speed = MODEL_GUN_SPEED_VALUES[PedModelGunSpeedIndex];
			float finalSpeed = speed * 1.25f;
			
			// 设置最大速度限制，避免引擎过早钳制速度
			ENTITY::SET_ENTITY_MAX_SPEED(spawnedPed, finalSpeed * 1.5f);
			
			// 设置初始速度（准星方向）
			ENTITY::SET_ENTITY_VELOCITY(spawnedPed, dir.x * finalSpeed, dir.y * finalSpeed, dir.z * finalSpeed);
			
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&spawnedPed);
		}
	}
}
