/*
这段代码的部分最初来源于 GTA V SCRIPT HOOK SDK。
http://dev-c.com
(C) Alexander Blade 2015

它现在已成为 Enhanced Native Trainer 项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/

#include "misc.h"
#include "script.h"
#include "hotkeys.h"
#include "world.h"
#include "vehicles.h"
#include "airbrake.h"
#include "propplacement.h"
#include "screenshot.h"
#include <Psapi.h>
#include "../utils.h"
#include <iterator>
#include "..\ui_support\menu_functions.h"
#include <comutil.h>
#include <msxml6.h>
#include <fstream>

//==================
// 其他菜单选项
//==================

bool g_MenuLayoutNeedsRefresh = false;//添加刷新标志定义

bool iterated_radio_stations = false; //为了防止不必要的循环而反复执行。

int activeLineIndexTrainerConfig = 0;
int activeLineIndexPhoneBill = 0;
int activeLineIndexRadioSettings = 0;
int activeLineIndexHudSettings = 0;
int activeLineIndexDefMenuTab = 0;
int activeLineIndexBillSettings = 0;
int activeLineIndexPhoneOnBike = 0;
int activeLineIndexAirbrake = 0;
int activeLineHotkeyConfig = 0;
int activeLineIndexMenuKeySettings = 0; // 菜单按键设置

// 自由移动模式变量
bool airbrake_enable = true; // 启用自由移动
bool mouse_view_control = false; // 禁用鼠标视角控制
bool help_showing = true; // 显示帮助信息
bool frozen_time = false; // 时间冻结功能关闭
bool show_transparency = true; // 显示透明度

// 电话账单变量
bool featurePhoneBillEnabled = false;
bool featureGamePause = false;
bool featureZeroBalance = false;
bool featurePhone3DOnBike = false;
bool featureNoPhoneOnHUD = false;
int secs_passed, secs_curr = -1;
float temp_seconds, bill_seconds = 0;
float bill_to_pay = -1;

bool featureDisableRecording = false;
bool featureNoNotifications = false;

// 动态健康条变量
bool featureDynamicHealthBar = false;
int temp_h, temp_h_d = -1;
bool dynamic_loading = true;
bool been_damaged = false;
float curr_damaged_health, curr_damaged_armor = -1;
int healthbar_secs_curr, healthbar_seconds = -1; 
float health_bar_x = 0.015;
float health_bar_y = 0.966;

// 显示帧率
static int frames = 0;
static double FPStime, FPStime_passed, FPStime_curr, starttime = 0;
int fps = 0; 
char fps_to_show_char_modifiable[15];

// 骑车时使用手机
Object temp_obj = -1;
char* anim_dict = "anim@cellphone@in_car@ps";
char* animation_of_d = "cellphone_text_read_base";
bool accel = false;
bool p_exist = false;

bool radio_pressed = false;

int r_secs_passed, r_secs_curr, r_seconds = -1;

bool radio_v_checked = false;
bool no_blur_initialized = false;

bool no_phone, bill_no_phone = false;

Vehicle playerVeh = -1;

//跳过曲目
static uintptr_t* g_radioStationList = nullptr;
static int* g_radioStationCount = nullptr;
static void(*CRadioStation__Advance)(uintptr_t This, uint32_t a2);
static uintptr_t* g_unkRadioStationData = nullptr;
bool skip_track_pressed = false;

// 过场动画查看器，第一人称过场动画摄像机
bool cutscene_is_playing, cutscene_being_watched, found_ped_in_cutscene = false;
bool con_disabled = false;
bool manual_cutscene = false;
Ped curr_cut_ped_me, my_first_coords, curr_cut_ped, switched_c = -1;
bool featureFirstPersonCutscene = false;

bool featurePlayerRadio = false;
bool featureDisablePhone = false;
bool featureDisablePhoneMenu = false;
bool featurePlayerRadioUpdated = false;
bool featureRadioFreeze = false, featureRadioFreezeUpdated = false;
bool featureBoostRadio = true;
bool featureRealisticRadioVolume = false;
bool featureWantedMusic = false;
bool featureFlyingMusic = false;
bool featurePoliceScanner = false;
bool featureNoComleteMessage = false;
bool featurePoliceRadio = false;
//bool police_radio_check = false;
bool featureMiscLockRadio = false;
bool featureMiscHideHud = false;
bool featureMiscHideHudUpdated = false;
bool featurePhoneShowHud = false;
bool featureInVehicleNoHud = false;
bool featureMarkerHud = false;
bool phone_toggle = false;
bool phone_toggle_vehicle = false;
bool phone_toggle_defaultphone = false;
bool featureFirstPersonDeathCamera = false;
bool featureFirstPersonStuntJumpCamera = false;
bool featureNoStuntJumps = false;
bool featureHidePlayerInfo = false;
bool featureShowFPS = false;
bool featureShowModelName = false; // 模型名称显示变量，默认值定义，默认关闭
bool featurenowheelblurslow = false;
bool featureShowVehiclePreviews = true;
bool featureShowPedPreviews = true;
bool featureShowStatusMessage = true;
bool featureNoAutoRespawn = false;
bool featureMiscJellmanScenery = false;
bool featureEnableMissingRadioStation = false;

std::string screenfltr;
bool sfilter_enabled = false;

//bool featureBlockInputInMenu = false;
//bool featureControllerIgnoreInTrainer = false;

const int TRAINERCONFIG_HOTKEY_MENU = 99;
const int TRAINERCONFIG_MENU_KEY_SETTINGS = 67; // 菜单按键设置
const int TRAINERCONFIG_HOTKEY_FUNCTION_SETTINGS = 68; // 快捷键功能设置
const int TRAINERCONFIG_HOTKEY_KEY_SETTINGS = 69; // 快捷键按键设置
int radioStationIndex = -1;

Camera StuntCam = NULL;

// 第一人称过场动画摄像机变量
Cam CutCam = NULL;
Object xaxis, zaxis = -1;

// 主要角色
const Hash PLAYER_ZERO = 0xD7114C9;
const Hash PLAYER_ONE = 0x9B22DBAF;
const Hash PLAYER_TWO = 0x9B810FA2;

// 主要角色现金
const Hash SP0_TOTAL_CASH = 0x324C31D;
const Hash SP1_TOTAL_CASH = 0x44BD6982;
const Hash SP2_TOTAL_CASH = 0x8D75047D;

// 电话账单金额
int PhoneBillIndex = 2;
bool PhoneBillChanged = true;

// 电话账单免费时长
int PhoneFreeSecondsIndex = 0;
bool PhoneFreeSecondsChanged = true;
int PhoneBikeAnimationIndex = 0;
bool PhoneBikeAnimationChanged = true;

int missing_station = 0;

// 默认菜单标签
int DefMenuTabIndex = 0;
bool DefMenuTabChanged = true;

// 默认手机
const std::vector<std::string> MISC_PHONE_DEFAULT_CAPTIONS{ "关", "迈克尔", "崔佛", "富兰克", "军用", "序章" };
const int MISC_PHONE_DEFAULT_VALUES[] = { -1, 0, 1, 2, 3, 4 };
int PhoneDefaultIndex = 0;
bool PhoneDefaultChanged = true;

// 收音机关闭
const std::vector<std::string> MISC_RADIO_OFF_CAPTIONS{ "默认", "始终", "仅限摩托车" };
int RadioOffIndex = 0;

// 收音机关台切换
const std::vector<std::string> MISC_RADIO_SWITCHING_CAPTIONS{ "关", "下一首电台歌曲 (快捷键)", "每 3 分钟", "每 5 分钟", "每 7 分钟", "每 10 分钟", "每 15 分钟", "每 30 分钟" };
const int MISC_RADIO_SWITCHING_VALUES[] = { 0, 1, 180, 300, 420, 600, 900, 1800 };
int RadioSwitchingIndex = 0;

// 在“电台设置选项”页面持有“电台随机切换”下拉项的指针，用于在子菜单中即时刷新显示
static SelectFromListMenuItem* g_RadioSwitchingSelectItem = NULL;

// 警车收音机功能（驾驶位 + 引擎检测 + 自动清理）
static bool policeRadioActive = false;
static Vehicle lastPoliceVeh = 0;

// 骑车手机动作动画类型（替换原本误用“前几秒免费”的选项）
const std::vector<std::string> MISC_PHONE_BIKE_ANIM_CAPTIONS{
	"车内坐姿看手机",
	"普通站姿看手机",
	"街头随意看手机",
	"女性看手机邮件",
	"第一人称看手机"
};

// 训练器控制
int TrainerControlIndex = 0;
bool TrainerControlChanged = true;

// 菜单 (活动条) 滚动方式
int TrainerControlScrollingIndex = 0;
bool TrainerControlScrollingChanged = true;

// 字体设置变量
int FontHeaderIndex = 0;
bool FontHeaderChanged = true;

int FontItemIndex = 0;
bool FontItemChanged = true;

int FontWantedIndex = 0;
bool FontWantedChanged = true;

int FontStatusIndex = 0;
bool FontStatusChanged = true;

int MenuItemsCountIndex = 0;
bool MenuItemsCountChanged = true;

int activeLineIndexFontSettings = 0;

// 快捷键设置索引变量
int HotkeyIndex[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
bool HotkeyChanged[9] = {false, false, false, false, false, false, false, false, false};
bool HotkeyCtrl[9] = {false, false, false, false, false, false, false, false, false};
bool HotkeyAlt[9] = {false, false, false, false, false, false, false, false, false};
bool HotkeyShift[9] = {false, false, false, false, false, false, false, false, false};
int activeLineIndexHotkeyKeySettings = 0;

// 常用按键设置变量
int CommonKeyToggleMenuIndex = 4;      // 开关/菜单，默认 F4
int CommonKeyMoveUpIndex = 57;         // 向上/移动，默认小键盘 8
int CommonKeyMoveDownIndex = 51;       // 向下/移动，默认小键盘 2
int CommonKeyMoveLeftIndex = 53;       // 向左/移动，默认小键盘 4
int CommonKeyMoveRightIndex = 55;      // 向右/移动，默认小键盘 6
int CommonKeyConfirmSelectIndex = 54;        // 确认/选择，默认小键盘 5
int CommonKeyBackCancelIndex = 49;           // 返回/取消，默认小键盘 0
bool CommonKeyChanged[7] = {false, false, false, false, false, false, false};
int activeLineIndexCommonKeys = 0;

// 其他按键设置变量
int OtherKeyToggleFreeMoveIndex = 6;   // 开/关自由移动，默认 F6
int OtherKeyFreeCamToggleIndex = 7;    // 自由相机模式，默认 F7
int OtherKeyVehicleBoostIndex = 58;    // 车辆加速，默认小键盘 9
int OtherKeyVehicleStopIndex = 52;     // 停止车辆，默认小键盘 3
int OtherKeyVehicleRocketsIndex = 59;  // 车辆发射火箭，默认小键盘 +
int OtherKeyLeftBlinkIndex = 72;       // 左转向灯，默认左箭头
int OtherKeyRightBlinkIndex = 73;      // 右转向灯，默认右箭头
int OtherKeyEmergencyBlinkIndex = 63;  // 打开双闪，默认小键盘 .
bool OtherKeyChanged[8] = {false, false, false, false, false, false, false, false};
int activeLineIndexOtherKeys = 0;

void onchange_hotkey_function(int value, SelectFromListMenuItem* source){
	change_hotkey_function(source->extras.at(0), value);
}

// 生成显示当前绑定快捷键的标题（用于按键设置菜单，显示临时设置）
std::string get_hotkey_display_caption(int hotkeyIndex) {
	std::ostringstream caption;
	caption << "快捷键 " << (hotkeyIndex + 1);
	
	// 如果有绑定按键，显示按键信息
	if (HotkeyIndex[hotkeyIndex] > 0 && HotkeyIndex[hotkeyIndex] < MISC_HOTKEY_CAPTIONS.size()) {
		caption << "  [";
		
		// 添加修饰键
		if (HotkeyCtrl[hotkeyIndex]) caption << "Ctrl+";
		if (HotkeyAlt[hotkeyIndex]) caption << "Alt+";
		if (HotkeyShift[hotkeyIndex]) caption << "Shift+";
		
		// 添加主按键
		caption << MISC_HOTKEY_CAPTIONS[HotkeyIndex[hotkeyIndex]];
		caption << "]";
	}
	else {
		caption << " [未绑定]";
	}
	
	return caption.str();
}

// 生成显示实际保存的快捷键标题（用于功能设置菜单，显示已保存的键位）
std::string get_saved_hotkey_display_caption(int hotkeyIndex) {
	std::ostringstream caption;
	caption << "快捷键 " << (hotkeyIndex + 1);
	
	// 获取实际保存的键位配置
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	std::string keyName;
	switch(hotkeyIndex){
		case 0: keyName = KeyConfig::KEY_HOT_1; break;
		case 1: keyName = KeyConfig::KEY_HOT_2; break;
		case 2: keyName = KeyConfig::KEY_HOT_3; break;
		case 3: keyName = KeyConfig::KEY_HOT_4; break;
		case 4: keyName = KeyConfig::KEY_HOT_5; break;
		case 5: keyName = KeyConfig::KEY_HOT_6; break;
		case 6: keyName = KeyConfig::KEY_HOT_7; break;
		case 7: keyName = KeyConfig::KEY_HOT_8; break;
		case 8: keyName = KeyConfig::KEY_HOT_9; break;
		default: return caption.str() + " [未绑定]";
	}
	
	KeyConfig* key = keyConfig->get_key(keyName);
	if(key != NULL && key->keyCode != 0){
		// 查找对应的按键名称
		int keyIndex = 0;
		for(int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++){
			if(MISC_HOTKEY_VALUES[j] == key->keyCode){
				keyIndex = j;
				break;
			}
		}
		
		if(keyIndex > 0 && keyIndex < MISC_HOTKEY_CAPTIONS.size()){
			caption << "  [";
			
			// 添加修饰键
			if (key->modCtrl) caption << "Ctrl+";
			if (key->modAlt) caption << "Alt+";
			if (key->modShift) caption << "Shift+";
			
			// 添加主按键
			caption << MISC_HOTKEY_CAPTIONS[keyIndex];
			caption << "]";
		}
		else {
			caption << " [未绑定]";
		}
	}
	else {
		caption << " [未绑定]";
	}
	
	return caption.str();
}

// 检查快捷键是否重复（统一从 KeyInputConfig 读取实际应用的按键）
bool is_hotkey_duplicate(int hotkeyNum, int keyIndex, bool ctrl, bool alt, bool shift) {
	// 如果是未绑定，不检查重复
	if (keyIndex == 0 || keyIndex >= sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
		return false;
	}
	
	int newKeyCode = MISC_HOTKEY_VALUES[keyIndex];
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) return false;
	
	// 检查其他8个快捷键
	std::string hotkeyNames[] = {
		KeyConfig::KEY_HOT_1, KeyConfig::KEY_HOT_2, KeyConfig::KEY_HOT_3,
		KeyConfig::KEY_HOT_4, KeyConfig::KEY_HOT_5, KeyConfig::KEY_HOT_6,
		KeyConfig::KEY_HOT_7, KeyConfig::KEY_HOT_8, KeyConfig::KEY_HOT_9
	};
	
	for (int i = 0; i < 9; i++) {
		if (i == hotkeyNum) continue; // 跳过自己
		
		KeyConfig* key = keyConfig->get_key(hotkeyNames[i]);
		if (key != NULL && key->keyCode == newKeyCode && 
			key->modCtrl == ctrl && key->modAlt == alt && key->modShift == shift) {
			return true; // 与其他快捷键重复
		}
	}
	
	// 检查常用按键（仅当快捷键没有修饰键时才检测冲突）
	// 原因：带修饰键的快捷键（如Ctrl+F4）不应与单独的F4冲突
	if (!ctrl && !alt && !shift) {
		std::string commonKeyNames[] = {
			KeyConfig::KEY_TOGGLE_MAIN_MENU, KeyConfig::KEY_MENU_UP, KeyConfig::KEY_MENU_DOWN,
			KeyConfig::KEY_MENU_LEFT, KeyConfig::KEY_MENU_RIGHT, KeyConfig::KEY_MENU_SELECT,
			KeyConfig::KEY_MENU_BACK
		};
		
		for (int i = 0; i < 7; i++) {
			KeyConfig* key = keyConfig->get_key(commonKeyNames[i]);
			if (key != NULL && key->keyCode == newKeyCode) {
				return true; // 与常用按键重复
			}
		}
		
		// 检查其他按键
		std::string otherKeyNames[] = {
			KeyConfig::KEY_TOGGLE_AIRBRAKE, KeyConfig::KEY_FREECAM_TOGGLE, KeyConfig::KEY_VEH_BOOST,
			KeyConfig::KEY_VEH_STOP, KeyConfig::KEY_VEH_ROCKETS, KeyConfig::KEY_VEH_LEFTBLINK,
			KeyConfig::KEY_VEH_RIGHTBLINK, KeyConfig::KEY_VEH_EMERGENCYBLINK
		};
		
		for (int i = 0; i < 8; i++) {
			KeyConfig* key = keyConfig->get_key(otherKeyNames[i]);
			if (key != NULL && key->keyCode == newKeyCode) {
				return true; // 与其他按键重复
			}
		}
	}
	
	return false;
}

// 检查常用按键是否重复（统一从 KeyInputConfig 读取实际应用的按键）
bool is_common_key_duplicate(int commonKeyNum, int keyIndex) {
	// 如果是未绑定，不检查重复
	if (keyIndex == 0 || keyIndex >= sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
		return false;
	}
	
	int newKeyCode = MISC_HOTKEY_VALUES[keyIndex];
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) return false;
	
	// 检查其他6个常用按键
	std::string commonKeyNames[] = {
		KeyConfig::KEY_TOGGLE_MAIN_MENU, KeyConfig::KEY_MENU_UP, KeyConfig::KEY_MENU_DOWN,
		KeyConfig::KEY_MENU_LEFT, KeyConfig::KEY_MENU_RIGHT, KeyConfig::KEY_MENU_SELECT,
		KeyConfig::KEY_MENU_BACK
	};
	
	for (int i = 0; i < 7; i++) {
		if (i == commonKeyNum) continue; // 跳过自己
		
		KeyConfig* key = keyConfig->get_key(commonKeyNames[i]);
		if (key != NULL && key->keyCode == newKeyCode) {
			return true; // 与其他常用按键重复
		}
	}
	
	// 检查其他按键
	std::string otherKeyNames[] = {
		KeyConfig::KEY_TOGGLE_AIRBRAKE, KeyConfig::KEY_FREECAM_TOGGLE, KeyConfig::KEY_VEH_BOOST,
		KeyConfig::KEY_VEH_STOP, KeyConfig::KEY_VEH_ROCKETS, KeyConfig::KEY_VEH_LEFTBLINK,
		KeyConfig::KEY_VEH_RIGHTBLINK, KeyConfig::KEY_VEH_EMERGENCYBLINK
	};
	
	for (int i = 0; i < 8; i++) {
		KeyConfig* key = keyConfig->get_key(otherKeyNames[i]);
		if (key != NULL && key->keyCode == newKeyCode) {
			return true; // 与其他按键重复
		}
	}
	
	// 检查快捷键（仅当快捷键没有修饰键时才冲突，因为常用按键不支持修饰键）
	// 例如：F4（常用按键）与 F4（无修饰键的快捷键）冲突，但不与 Ctrl+F4 冲突
	std::string hotkeyNames[] = {
		KeyConfig::KEY_HOT_1, KeyConfig::KEY_HOT_2, KeyConfig::KEY_HOT_3,
		KeyConfig::KEY_HOT_4, KeyConfig::KEY_HOT_5, KeyConfig::KEY_HOT_6,
		KeyConfig::KEY_HOT_7, KeyConfig::KEY_HOT_8, KeyConfig::KEY_HOT_9
	};
	
	for (int i = 0; i < 9; i++) {
		KeyConfig* key = keyConfig->get_key(hotkeyNames[i]);
		// 仅当快捷键没有任何修饰键时才视为冲突
		if (key != NULL && key->keyCode == newKeyCode && 
			!key->modCtrl && !key->modAlt && !key->modShift) {
			return true; // 与无修饰键的快捷键重复
		}
	}
	
	return false;
}

// 检查其他按键是否重复（统一从 KeyInputConfig 读取实际应用的按键）
bool is_other_key_duplicate(int otherKeyNum, int keyIndex) {
	// 如果是未绑定，不检查重复
	if (keyIndex == 0 || keyIndex >= sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
		return false;
	}
	
	int newKeyCode = MISC_HOTKEY_VALUES[keyIndex];
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) return false;
	
	// 检查其他7个其他按键
	std::string otherKeyNames[] = {
		KeyConfig::KEY_TOGGLE_AIRBRAKE, KeyConfig::KEY_FREECAM_TOGGLE, KeyConfig::KEY_VEH_BOOST,
		KeyConfig::KEY_VEH_STOP, KeyConfig::KEY_VEH_ROCKETS, KeyConfig::KEY_VEH_LEFTBLINK,
		KeyConfig::KEY_VEH_RIGHTBLINK, KeyConfig::KEY_VEH_EMERGENCYBLINK
	};
	
	for (int i = 0; i < 8; i++) {
		if (i == otherKeyNum) continue; // 跳过自己
		
		KeyConfig* key = keyConfig->get_key(otherKeyNames[i]);
		if (key != NULL && key->keyCode == newKeyCode) {
			return true; // 与其他按键重复
		}
	}
	
	// 检查常用按键
	std::string commonKeyNames[] = {
		KeyConfig::KEY_TOGGLE_MAIN_MENU, KeyConfig::KEY_MENU_UP, KeyConfig::KEY_MENU_DOWN,
		KeyConfig::KEY_MENU_LEFT, KeyConfig::KEY_MENU_RIGHT, KeyConfig::KEY_MENU_SELECT,
		KeyConfig::KEY_MENU_BACK
	};
	
	for (int i = 0; i < 7; i++) {
		KeyConfig* key = keyConfig->get_key(commonKeyNames[i]);
		if (key != NULL && key->keyCode == newKeyCode) {
			return true; // 与常用按键重复
		}
	}
	
	// 检查快捷键（仅当快捷键没有修饰键时才冲突，因为其他按键不支持修饰键）
	// 例如：F6（其他按键）与 F6（无修饰键的快捷键）冲突，但不与 Ctrl+F6 冲突
	std::string hotkeyNames[] = {
		KeyConfig::KEY_HOT_1, KeyConfig::KEY_HOT_2, KeyConfig::KEY_HOT_3,
		KeyConfig::KEY_HOT_4, KeyConfig::KEY_HOT_5, KeyConfig::KEY_HOT_6,
		KeyConfig::KEY_HOT_7, KeyConfig::KEY_HOT_8, KeyConfig::KEY_HOT_9
	};
	
	for (int i = 0; i < 9; i++) {
		KeyConfig* key = keyConfig->get_key(hotkeyNames[i]);
		// 仅当快捷键没有任何修饰键时才视为冲突
		if (key != NULL && key->keyCode == newKeyCode && 
			!key->modCtrl && !key->modAlt && !key->modShift) {
			return true; // 与无修饰键的快捷键重复
		}
	}
	
	return false;
}

// 快捷键按键设置回调函数
void onchange_hotkey_key(int value, SelectFromListMenuItem* source){
	int hotkeyNum = source->extras.at(0); // 快捷键编号 (0-8)
	
	// 检查快捷键重复
	if (is_hotkey_duplicate(hotkeyNum, value, HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum])) {
		set_status_text("快捷键重复！\n已恢复为未绑定状态。");
		set_status_text_centre_screen("快捷键 ~r~重复！~s~已恢复为未绑定状态。");
		value = 0; // 恢复为未绑定状态
	}
	
	HotkeyIndex[hotkeyNum] = value;
	HotkeyChanged[hotkeyNum] = true;
	
	// 当主按键为未绑定时，自动清除所有修饰键
	if (value == 0) {
		HotkeyCtrl[hotkeyNum] = false;
		HotkeyAlt[hotkeyNum] = false;
		HotkeyShift[hotkeyNum] = false;
	}
	
	// 立即更新KeyInputConfig以使更改生效
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		std::string keyName;
		switch(hotkeyNum){
			case 0: keyName = KeyConfig::KEY_HOT_1; break;
			case 1: keyName = KeyConfig::KEY_HOT_2; break;
			case 2: keyName = KeyConfig::KEY_HOT_3; break;
			case 3: keyName = KeyConfig::KEY_HOT_4; break;
			case 4: keyName = KeyConfig::KEY_HOT_5; break;
			case 5: keyName = KeyConfig::KEY_HOT_6; break;
			case 6: keyName = KeyConfig::KEY_HOT_7; break;
			case 7: keyName = KeyConfig::KEY_HOT_8; break;
			case 8: keyName = KeyConfig::KEY_HOT_9; break;
		}
		
		// 获取按键值和名称
		int keyValue = MISC_HOTKEY_VALUES[value];
		char* keyValueName = keyValToName(keyValue);
		
		// 立即设置按键配置
		keyConfig->set_key((char*)keyName.c_str(), keyValueName, 
						   HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum]);
	}
}

void onchange_hotkey_ctrl(int value, SelectFromListMenuItem* source){
	int hotkeyNum = source->extras.at(0); // 快捷键编号 (0-8)
	bool newCtrl = (value == 1);
	
	// 检查快捷键重复
	if (is_hotkey_duplicate(hotkeyNum, HotkeyIndex[hotkeyNum], newCtrl, HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum])) {
		set_status_text("快捷键重复！\n已恢复为未绑定状态。");
		set_status_text_centre_screen("快捷键 ~r~重复！~s~已恢复为未绑定状态。");
		// 恢复为未绑定状态
		HotkeyIndex[hotkeyNum] = 0;
		HotkeyCtrl[hotkeyNum] = false;
		HotkeyAlt[hotkeyNum] = false;
		HotkeyShift[hotkeyNum] = false;
		HotkeyChanged[hotkeyNum] = true;
		return;
	}
	
	HotkeyCtrl[hotkeyNum] = newCtrl;
	HotkeyChanged[hotkeyNum] = true;
	
	// 立即更新KeyInputConfig以使更改生效
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		std::string keyName;
		switch(hotkeyNum){
			case 0: keyName = KeyConfig::KEY_HOT_1; break;
			case 1: keyName = KeyConfig::KEY_HOT_2; break;
			case 2: keyName = KeyConfig::KEY_HOT_3; break;
			case 3: keyName = KeyConfig::KEY_HOT_4; break;
			case 4: keyName = KeyConfig::KEY_HOT_5; break;
			case 5: keyName = KeyConfig::KEY_HOT_6; break;
			case 6: keyName = KeyConfig::KEY_HOT_7; break;
			case 7: keyName = KeyConfig::KEY_HOT_8; break;
			case 8: keyName = KeyConfig::KEY_HOT_9; break;
		}
		
		// 获取按键值和名称
		int keyValue = MISC_HOTKEY_VALUES[HotkeyIndex[hotkeyNum]];
		char* keyValueName = keyValToName(keyValue);
		
		// 立即设置按键配置
		keyConfig->set_key((char*)keyName.c_str(), keyValueName, 
						   HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum]);
	}
}

void onchange_hotkey_alt(int value, SelectFromListMenuItem* source){
	int hotkeyNum = source->extras.at(0); // 快捷键编号 (0-8)
	bool newAlt = (value == 1);
	
	// 检查快捷键重复
	if (is_hotkey_duplicate(hotkeyNum, HotkeyIndex[hotkeyNum], HotkeyCtrl[hotkeyNum], newAlt, HotkeyShift[hotkeyNum])) {
		set_status_text("快捷键重复！\n已恢复为未绑定状态。");
		set_status_text_centre_screen("快捷键 ~r~重复！~s~已恢复为未绑定状态。");
		// 恢复为未绑定状态
		HotkeyIndex[hotkeyNum] = 0;
		HotkeyCtrl[hotkeyNum] = false;
		HotkeyAlt[hotkeyNum] = false;
		HotkeyShift[hotkeyNum] = false;
		HotkeyChanged[hotkeyNum] = true;
		return;
	}
	
	HotkeyAlt[hotkeyNum] = newAlt;
	HotkeyChanged[hotkeyNum] = true;
	
	// 立即更新KeyInputConfig以使更改生效
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		std::string keyName;
		switch(hotkeyNum){
			case 0: keyName = KeyConfig::KEY_HOT_1; break;
			case 1: keyName = KeyConfig::KEY_HOT_2; break;
			case 2: keyName = KeyConfig::KEY_HOT_3; break;
			case 3: keyName = KeyConfig::KEY_HOT_4; break;
			case 4: keyName = KeyConfig::KEY_HOT_5; break;
			case 5: keyName = KeyConfig::KEY_HOT_6; break;
			case 6: keyName = KeyConfig::KEY_HOT_7; break;
			case 7: keyName = KeyConfig::KEY_HOT_8; break;
			case 8: keyName = KeyConfig::KEY_HOT_9; break;
		}
		
		// 获取按键值和名称
		int keyValue = MISC_HOTKEY_VALUES[HotkeyIndex[hotkeyNum]];
		char* keyValueName = keyValToName(keyValue);
		
		// 立即设置按键配置
		keyConfig->set_key((char*)keyName.c_str(), keyValueName, 
						   HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum]);
	}
}

void onchange_hotkey_shift(int value, SelectFromListMenuItem* source){
	int hotkeyNum = source->extras.at(0); // 快捷键编号 (0-8)
	bool newShift = (value == 1);
	
	// 检查快捷键重复
	if (is_hotkey_duplicate(hotkeyNum, HotkeyIndex[hotkeyNum], HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], newShift)) {
		set_status_text("快捷键重复！\n已恢复为未绑定状态。");
		set_status_text_centre_screen("快捷键 ~r~重复！~s~已恢复为未绑定状态。");
		// 恢复为未绑定状态
		HotkeyIndex[hotkeyNum] = 0;
		HotkeyCtrl[hotkeyNum] = false;
		HotkeyAlt[hotkeyNum] = false;
		HotkeyShift[hotkeyNum] = false;
		HotkeyChanged[hotkeyNum] = true;
		return;
	}
	
	HotkeyShift[hotkeyNum] = newShift;
	HotkeyChanged[hotkeyNum] = true;
	
	// 立即更新KeyInputConfig以使更改生效
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		std::string keyName;
		switch(hotkeyNum){
			case 0: keyName = KeyConfig::KEY_HOT_1; break;
			case 1: keyName = KeyConfig::KEY_HOT_2; break;
			case 2: keyName = KeyConfig::KEY_HOT_3; break;
			case 3: keyName = KeyConfig::KEY_HOT_4; break;
			case 4: keyName = KeyConfig::KEY_HOT_5; break;
			case 5: keyName = KeyConfig::KEY_HOT_6; break;
			case 6: keyName = KeyConfig::KEY_HOT_7; break;
			case 7: keyName = KeyConfig::KEY_HOT_8; break;
			case 8: keyName = KeyConfig::KEY_HOT_9; break;
		}
		
		// 获取按键值和名称
		int keyValue = MISC_HOTKEY_VALUES[HotkeyIndex[hotkeyNum]];
		char* keyValueName = keyValToName(keyValue);
		
		// 立即设置按键配置
		keyConfig->set_key((char*)keyName.c_str(), keyValueName, 
						   HotkeyCtrl[hotkeyNum], HotkeyAlt[hotkeyNum], HotkeyShift[hotkeyNum]);
	}
}

bool process_misc_hotkey_menu(){
	// 如果快捷键设置被重置，清除刷新标志
	if (g_HotkeyFunctionMenuNeedsRefresh) {
		g_HotkeyFunctionMenuNeedsRefresh = false;
	}

	std::vector<MenuItem<int>*> menuItems;

	for(int i = 1; i < 10; i++){
		std::ostringstream itemCaption;
		std::vector<std::string> captions;
		void(*callback)(int, SelectFromListMenuItem*);

		// 使用实际保存的键位信息显示标题
		itemCaption << get_saved_hotkey_display_caption(i - 1); // i-1因为数组索引从0开始

		bool keyAssigned = get_config()->get_key_config()->is_hotkey_assigned(i);
		if(!keyAssigned){
			captions.push_back("未绑定按键");
			callback = NULL;

			SelectFromListMenuItem* item = new SelectFromListMenuItem(captions, callback);
			item->caption = itemCaption.str();
			item->value = NULL;
			item->locked = true; // 未绑定时锁定不能操作
			menuItems.push_back(item);
		}
		else{
			for (const auto& var : HOTKEY_AVAILABLE_FUNCS){
				captions.push_back(var.caption);
			}
			callback = onchange_hotkey_function;

			SelectFromListMenuItem* item = new SelectFromListMenuItem(captions, callback);
			item->caption = itemCaption.str();
			item->wrap = keyAssigned;
			item->extras.push_back(i);
			item->value = get_hotkey_function_index(i);
			menuItems.push_back(item);
		}
	}

	draw_generic_menu<int>(menuItems, &activeLineHotkeyConfig, "快捷键功能设置", NULL, NULL, NULL);

	return false;
}

void process_misc_trainermenucoloring_menu(int part){
	std::vector<MenuItem<int> *> menuItems;
	int index = 0;
	ColorItem<int> *colorItem;

	for(auto a : ENTColor::colsCompCaptions){
		colorItem = new ColorItem<int>();
		colorItem->caption = a;
		colorItem->isLeaf = true;
		colorItem->colorval = ENTColor::colsMenu[part].rgba[index];
		colorItem->part = part;
		colorItem->component = index++;
		menuItems.push_back(colorItem);
	}

	draw_generic_menu<int>(menuItems, nullptr, ENTColor::colsCaptions[part], nullptr, nullptr, nullptr, nullptr);
}

bool onconfirm_trainermenucolors_menu(MenuItem<int> choice){
	if(choice.value >= 0 && choice.value < ENTColor::colsVarsNum){
		process_misc_trainermenucoloring_menu(choice.value);
	}
	else if(choice.value == ENTColor::colsVarsNum){
		write_config_ini_file();
		set_status_text_centre_screen("菜单颜色 ~g~已保存至 ~s~配置文件！"); // 屏幕中间提示，带闪烁
		set_status_text("菜单颜色，已保存到 INI 文件！");
		write_text_to_log_file("INI 配置文件，已写入或更新！");
		return true; // 返回 true 退出当前菜单，自动返回上一级菜单
	}
	else if(choice.value == ENTColor::colsVarsNum + 1){
		ENTColor::reset_colors();
		set_status_text("ENT 菜单颜色已重置为默认！");
		set_status_text_centre_screen("菜单颜色 ~g~已重置 ~s~为默认！"); // 屏幕中间提示，带闪烁
		return true; // 返回 true 退出当前菜单，自动返回上一级菜单
	}
	return false;
}

void process_misc_trainermenucolors_menu(){
	std::vector<MenuItem<int> *> menuItems;
	int index = 0;
	MenuItem<int> *item;

	for(auto a : ENTColor::colsCaptions){
		item = new MenuItem<int>();
		item->caption = a;
		item->value = index++;
		item->isLeaf = false;
		menuItems.push_back(item);
	}

	item = new MenuItem<int>();
	item->caption = "保存菜单颜色";
	item->value = index++;
	item->isLeaf = true;
	menuItems.insert(menuItems.begin(), item);

	item = new MenuItem<int>();
	item->caption = "重置菜单颜色";
	item->value = index++;
	item->isLeaf = true;
	menuItems.insert(menuItems.begin(), item);

	draw_generic_menu<int>(menuItems, nullptr, "菜单颜色设置", onconfirm_trainermenucolors_menu, nullptr, nullptr, nullptr);
}

bool onconfirm_trainerconfig_menu(MenuItem<int> choice){
	if(choice.value == TRAINERCONFIG_MENU_KEY_SETTINGS){
		process_misc_menu_key_settings_menu();
	}
	else if(choice.value == TRAINERCONFIG_HOTKEY_MENU){
		//write_text_to_log_file("onconfirm_trainerconfig");
		process_misc_hotkey_menu();
	}
	else if(choice.value == 63){
		process_misc_trainermenucolors_menu();
	}
	else if(choice.value == 64){
		process_misc_font_settings_menu();
	}
    else if(choice.value == 65){
		process_misc_menu_layout_settings_menu();
	}
    else if(choice.value == 66){
        process_misc_preview_settings_menu();
    }
	return false;
}

// 车辆预览图设置菜单实现
int activeLineIndexVehiclePreview = 0;

void process_misc_vehicle_preview_settings_menu() {
	const std::string caption = "车辆预览图设置";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;

	// 添加预览图左右判断依据设置
	listItem = new SelectFromListMenuItem(MISC_PREVIEW_POSITION_THRESHOLD_CAPTIONS, onchange_misc_preview_position_threshold_index);
	listItem->wrap = false;
	listItem->caption = "预览图左侧显示切换";
	listItem->value = PreviewPositionThresholdIndex;
	menuItems.push_back(listItem);

	// 添加预览图分辨率适配值设置
	listItem = new SelectFromListMenuItem(MISC_PREVIEW_RESOLUTION_SCALE_CAPTIONS, onchange_misc_preview_resolution_scale_index);
	listItem->wrap = false;
	listItem->caption = "预览图间距分辨率适配";
	listItem->value = PreviewResolutionScaleIndex;
	menuItems.push_back(listItem);

	// 添加预览图间距设置
	listItem = new SelectFromListMenuItem(MISC_PREVIEW_SPACING_CAPTIONS, onchange_misc_preview_spacing_index);
	listItem->wrap = false;
	listItem->caption = "车辆预览图间距";
	listItem->value = PreviewSpacingIndex;
	menuItems.push_back(listItem);

	// 添加显示车辆预览图选项
	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "显示车辆预览图";
	toggleItem->toggleValue = &featureShowVehiclePreviews;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexVehiclePreview, caption, NULL, NULL, NULL);
}

// 合并后的预览图设置（车辆 + 人物）
int activeLineIndexPreviewSettings = 0;

void process_misc_preview_settings_menu() {
    const std::string caption = "预览图设置（车辆+人物）";

    std::vector<MenuItem<int>*> menuItems;
    SelectFromListMenuItem *listItem;

    // 车辆预览图设置（三项 + 开关）
    listItem = new SelectFromListMenuItem(MISC_PREVIEW_POSITION_THRESHOLD_CAPTIONS, onchange_misc_preview_position_threshold_index);
    listItem->wrap = false;
    listItem->caption = "车辆：预览图左侧显示切换";
    listItem->value = PreviewPositionThresholdIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_PREVIEW_RESOLUTION_SCALE_CAPTIONS, onchange_misc_preview_resolution_scale_index);
    listItem->wrap = false;
    listItem->caption = "车辆：预览图间距分辨率适配";
    listItem->value = PreviewResolutionScaleIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_PREVIEW_SPACING_CAPTIONS, onchange_misc_preview_spacing_index);
    listItem->wrap = false;
    listItem->caption = "车辆：预览图间距";
    listItem->value = PreviewSpacingIndex;
    menuItems.push_back(listItem);

    ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
    toggleItem->caption = "车辆：显示预览图";
    toggleItem->toggleValue = &featureShowVehiclePreviews;
    menuItems.push_back(toggleItem);

    // 人物预览图设置（三项 + 开关）
    listItem = new SelectFromListMenuItem(MISC_PED_PREVIEW_POSITION_THRESHOLD_CAPTIONS, onchange_misc_ped_preview_position_threshold_index);
    listItem->wrap = false;
    listItem->caption = "人物：预览图左侧显示切换";
    listItem->value = PedPreviewPositionThresholdIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_PED_PREVIEW_RESOLUTION_SCALE_CAPTIONS, onchange_misc_ped_preview_resolution_scale_index);
    listItem->wrap = false;
    listItem->caption = "人物：预览图间距分辨率适配";
    listItem->value = PedPreviewResolutionScaleIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_PED_PREVIEW_SPACING_CAPTIONS, onchange_misc_ped_preview_spacing_index);
    listItem->wrap = false;
    listItem->caption = "人物：预览图间距";
    listItem->value = PedPreviewSpacingIndex;
    menuItems.push_back(listItem);

    toggleItem = new ToggleMenuItem<int>();
    toggleItem->caption = "人物：显示预览图";
    toggleItem->toggleValue = &featureShowPedPreviews;
    menuItems.push_back(toggleItem);

    draw_generic_menu<int>(menuItems, &activeLineIndexPreviewSettings, caption, NULL, NULL, NULL);
}

// 菜单按键设置确认处理函数
bool onconfirm_menu_key_settings_menu(MenuItem<int> choice){
	if(choice.value == TRAINERCONFIG_HOTKEY_FUNCTION_SETTINGS){
		process_misc_hotkey_menu();
	}
	else if(choice.value == TRAINERCONFIG_HOTKEY_KEY_SETTINGS){
		process_misc_hotkey_key_settings_menu();
	}
	else if(choice.value == TRAINERCONFIG_COMMON_KEYS){
		process_misc_common_keys_menu();
	}
	else if(choice.value == TRAINERCONFIG_OTHER_KEYS){
		process_misc_other_keys_menu();
	}
	else if(choice.value == -1){ // 保存设置
		save_hotkey_settings_to_xml();
		save_common_other_keys_to_xml();
		set_status_text("按键设置已保存至 XML");
		activeLineIndexMenuKeySettings = 0; // 重置页面状态
		return true; // 返回上一页
	}
	else if(choice.value == -2){ // 恢复默认设置
		reset_hotkey_settings_to_defaults();
		reset_common_other_keys_to_defaults();
		set_status_text("按键设置已恢复为默认");
		// 设置刷新标志，通知快捷键功能设置菜单需要更新locked状态
		g_HotkeyFunctionMenuNeedsRefresh = true;
		activeLineIndexMenuKeySettings = 0; // 重置页面状态
		return true; // 返回上一页
	}
	return false;
}

// 快捷键按键设置菜单实现
// 每个快捷键的独立菜单活动行索引
int activeLineIndexHotkeySettings[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// 单个快捷键设置菜单
bool process_single_hotkey_settings_menu(int hotkeyIndex){
	std::string caption = get_hotkey_display_caption(hotkeyIndex);
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;

	// 主按键选择
	listItem = new SelectFromListMenuItem(MISC_HOTKEY_CAPTIONS, onchange_hotkey_key);
	listItem->wrap = false;
	listItem->caption = "主按键";
	listItem->value = HotkeyIndex[hotkeyIndex];
	listItem->extras.push_back(hotkeyIndex);
	menuItems.push_back(listItem);

	// Ctrl修饰键
	std::vector<std::string> modifierCaptions = {"否", "是"};
	listItem = new SelectFromListMenuItem(modifierCaptions, onchange_hotkey_ctrl);
	listItem->wrap = false;
	listItem->caption = "Ctrl";
	listItem->value = HotkeyCtrl[hotkeyIndex] ? 1 : 0;
	listItem->extras.push_back(hotkeyIndex);
	menuItems.push_back(listItem);

	// Alt修饰键
	listItem = new SelectFromListMenuItem(modifierCaptions, onchange_hotkey_alt);
	listItem->wrap = false;
	listItem->caption = "Alt";
	listItem->value = HotkeyAlt[hotkeyIndex] ? 1 : 0;
	listItem->extras.push_back(hotkeyIndex);
	menuItems.push_back(listItem);

	// Shift修饰键
	listItem = new SelectFromListMenuItem(modifierCaptions, onchange_hotkey_shift);
	listItem->wrap = false;
	listItem->caption = "Shift";
	listItem->value = HotkeyShift[hotkeyIndex] ? 1 : 0;
	listItem->extras.push_back(hotkeyIndex);
	menuItems.push_back(listItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexHotkeySettings[hotkeyIndex], caption, NULL, NULL, NULL);
	return false;
}

// 快捷键按键设置主菜单
bool process_misc_hotkey_key_settings_menu(){
	const std::string caption = "快捷键按键设置";
	std::vector<MenuItem<int>*> menuItems;

	// 为每个快捷键创建子菜单项
	for(int i = 0; i < 9; i++){
		MenuItem<int>* item = new MenuItem<int>();
		item->caption = get_hotkey_display_caption(i);
		item->value = i;
		item->isLeaf = false;
		menuItems.push_back(item);
	}

	draw_generic_menu<int>(menuItems, &activeLineIndexHotkeyKeySettings, caption, onconfirm_hotkey_key_settings_menu, NULL, NULL);
	return false;
}

// 快捷键功能菜单刷新标志
bool g_HotkeyFunctionMenuNeedsRefresh = false;

// 快捷键按键设置确认处理函数
bool onconfirm_hotkey_key_settings_menu(MenuItem<int> choice){
	if(choice.value >= 0 && choice.value < 9){
		// 进入单个快捷键设置菜单
		process_single_hotkey_settings_menu(choice.value);
	}
	return false;
}

// 菜单按键设置菜单实现
void process_misc_menu_key_settings_menu() {
	const std::string caption = "菜单按键设置";

	std::vector<MenuItem<int>*> menuItems;

	// 添加快捷键功能设置菜单项（原快捷键设置，重命名）
	MenuItem<int>* hotkeyFunctionItem = new MenuItem<int>();
	hotkeyFunctionItem->caption = "快捷键功能设置";
	hotkeyFunctionItem->value = TRAINERCONFIG_HOTKEY_FUNCTION_SETTINGS;
	hotkeyFunctionItem->isLeaf = false;
	menuItems.push_back(hotkeyFunctionItem);

	// 添加快捷键按键设置菜单项（留空备用）
	MenuItem<int>* hotkeyKeyItem = new MenuItem<int>();
	hotkeyKeyItem->caption = "快捷键按键设置";
	hotkeyKeyItem->value = TRAINERCONFIG_HOTKEY_KEY_SETTINGS;
	hotkeyKeyItem->isLeaf = false;
	menuItems.push_back(hotkeyKeyItem);

	// 添加常用按键设置菜单项
	MenuItem<int>* commonKeysItem = new MenuItem<int>();
	commonKeysItem->caption = "常用按键设置";
	commonKeysItem->value = TRAINERCONFIG_COMMON_KEYS;
	commonKeysItem->isLeaf = false;
	menuItems.push_back(commonKeysItem);

	// 添加其他按键设置菜单项
	MenuItem<int>* otherKeysItem = new MenuItem<int>();
	otherKeysItem->caption = "其他按键设置";
	otherKeysItem->value = TRAINERCONFIG_OTHER_KEYS;
	otherKeysItem->isLeaf = false;
	menuItems.push_back(otherKeysItem);

	// 添加保存设置按钮
	MenuItem<int>* saveItem = new MenuItem<int>();
	saveItem->caption = "保存并写入配置文件";
	saveItem->value = -1;
	saveItem->isLeaf = true;
	menuItems.push_back(saveItem);

	// 添加恢复默认设置按钮
	MenuItem<int>* resetItem = new MenuItem<int>();
	resetItem->caption = "重置并恢复配置文件";
	resetItem->value = -2;
	resetItem->isLeaf = true;
	menuItems.push_back(resetItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexMenuKeySettings, caption, onconfirm_menu_key_settings_menu, NULL, NULL);
}

void process_misc_trainerconfig_menu(){
	const std::string caption = "修改器设置选项";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;

	// 添加菜单按键设置菜单项（首项位置）
	MenuItem<int>* menuKeySettingsItem = new MenuItem<int>();
	menuKeySettingsItem->caption = "菜单按键设置";
	menuKeySettingsItem->value = TRAINERCONFIG_MENU_KEY_SETTINGS;
	menuKeySettingsItem->isLeaf = false;
	menuItems.push_back(menuKeySettingsItem);

	// 添加修改器菜单颜色设置
	MenuItem<int>* colorSettingsItem = new MenuItem<int>();
	colorSettingsItem->caption = "菜单颜色设置";
	colorSettingsItem->value = 63;
	colorSettingsItem->isLeaf = false;
	menuItems.push_back(colorSettingsItem);

	// 添加字体设置菜单项
	MenuItem<int>* fontSettingsItem = new MenuItem<int>();
	fontSettingsItem->caption = "字体类型设置";
	fontSettingsItem->value = 64;
	fontSettingsItem->isLeaf = false;
	menuItems.push_back(fontSettingsItem);

	// 添加菜单布局设置菜单项
	MenuItem<int>* menuLayoutSettingsItem = new MenuItem<int>();
	menuLayoutSettingsItem->caption = "菜单布局设置";
	menuLayoutSettingsItem->value = 65;
	menuLayoutSettingsItem->isLeaf = false;
	menuItems.push_back(menuLayoutSettingsItem);

    // 合并后的预览图设置菜单项（车辆 + 人物）
    MenuItem<int>* previewSettingsItem = new MenuItem<int>();
    previewSettingsItem->caption = "预览图设置（车辆+人物）";
    previewSettingsItem->value = 66; // 复用原66，避免遍历判断新增分支
    previewSettingsItem->isLeaf = false;
    menuItems.push_back(previewSettingsItem);

	listItem = new SelectFromListMenuItem(MISC_TRAINERCONTROL_CAPTIONS, onchange_misc_trainercontrol_index);
	listItem->wrap = false;
	listItem->caption = "菜单控制方式";
	listItem->value = TrainerControlIndex;
	menuItems.push_back(listItem);

	listItem = new SelectFromListMenuItem(MISC_TRAINERCONTROLSCROLLING_CAPTIONS, onchange_misc_trainercontrolscrolling_index);
	listItem->wrap = false;
	listItem->caption = "菜单滚动方式";
	listItem->value = TrainerControlScrollingIndex;
	menuItems.push_back(listItem);

	// 添加菜单显示项目数设置
	SelectFromListMenuItem *menuItemsCountItem = new SelectFromListMenuItem(MISC_MENU_ITEMS_COUNT_CAPTIONS, onchange_misc_menu_items_count_index);
	menuItemsCountItem->wrap = false;
	menuItemsCountItem->caption = "菜单显示项目数";
	menuItemsCountItem->value = MenuItemsCountIndex;
	menuItems.push_back(menuItemsCountItem);

	//ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	//toggleItem->caption = "Lock Controls While In Menu";
	//toggleItem->toggleValue = &featureBlockInputInMenu;
	//menuItems.push_back(toggleItem);

	//toggleItem = new ToggleMenuItem<int>();
	//toggleItem->caption = "Turn Off All Controller Input In Trainer";
	//toggleItem->toggleValue = &featureControllerIgnoreInTrainer;
	//menuItems.push_back(toggleItem);

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启动时显示状态消息";
	toggleItem->toggleValue = &featureShowStatusMessage;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "包含 Nkjellman 的额外场景";
	toggleItem->toggleValue = &featureMiscJellmanScenery;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexTrainerConfig, caption, onconfirm_trainerconfig_menu, NULL, NULL);
}

// 获取电台中文本地化名称（带回退）
static std::string get_radio_station_label_with_fallback_by_index(int idx) {
	const char* key = AUDIO::GET_RADIO_STATION_NAME(idx);
	if (!key) return "";
	if (UI::DOES_TEXT_LABEL_EXIST(const_cast<char*>(key))) {
		std::string localized = UI::_GET_LABEL_TEXT(const_cast<char*>(key));
		if (!localized.empty() && localized != "NULL") return localized;
	}
	return key;
}

// 单选复选框：getter —— 判断本项是否被选中
static bool is_radio_freeze_toggle_selected(std::vector<int> extras) {
	int idx = (extras.size() > 0) ? extras[0] : -1;
	if (idx < 0) {
		// “无” 选项在未冻结时呈选中状态
		return !featureRadioFreeze;
	}
	return featureRadioFreeze && (radioStationIndex == idx);
}

// 单选复选框：setter —— 设置当前点击项为选中（互斥）
static void set_radio_freeze_toggle_selected(bool turnOn, std::vector<int> extras) {
	int idx = (extras.size() > 0) ? extras[0] : -1;
	// 点击“无”：仅在需要开启时处理（防止把“无”从选中状态切换为未选中后造成不明确状态）
	if (idx < 0) {
		if (turnOn) {
			featureRadioFreeze = false;
			featureRadioFreezeUpdated = true;
			set_status_text("电台固定已成功解除！\n您可以自由切换电台了！");
		}
		return;
	}

	if (turnOn) {
		// 选择具体电台：开启冻结并固定到该电台
		featureRadioFreeze = true;
		featureRadioFreezeUpdated = true;
		radioStationIndex = idx;
		// 若当前为“定时随机切换”，与冻结互斥：自动关闭随机切换并提示
		if (MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex] > 1) {
			RadioSwitchingIndex = 0;
			// 同步主菜单中的“电台随机切换”项显示为“关”
			if (g_RadioSwitchingSelectItem != NULL) {
				g_RadioSwitchingSelectItem->value = RadioSwitchingIndex;
			}
			set_status_text("电台已冻结, 并关闭随机切换!\n仅允许, 快捷键切换电台歌曲!");
		}
		std::string label = get_radio_station_label_with_fallback_by_index(idx);
		set_status_text(std::string("已冻结电台, 并固定为:~y~ ") + label);
		AUDIO::SET_RADIO_TO_STATION_INDEX(idx);
	} else {
		// 取消当前电台的选中：回到“无”（即不冻结）
		featureRadioFreeze = false;
		featureRadioFreezeUpdated = true;
		set_status_text("电台固定已成功解除！\n您可以自由切换电台了！");
	}
}

void play_cutscene(std::string curr_c) {
	char* cstr = new char[curr_c.length() + 1];
	strcpy(cstr, curr_c.c_str());

	CUTSCENE::REQUEST_CUTSCENE(cstr, 8);
	while (!CUTSCENE::HAS_CUTSCENE_LOADED() && !CONTROLS::IS_CONTROL_PRESSED(2, 22)) {
		make_periodic_feature_call();
		WAIT(0);
	}
	if (CUTSCENE::HAS_CUTSCENE_LOADED()) {
		cutscene_is_playing = true;
		manual_cutscene = true;
		CUTSCENE::SET_CUTSCENE_FADE_VALUES(0, 0, 0, 0);
		CUTSCENE::START_CUTSCENE(0);
		CAM::SET_WIDESCREEN_BORDERS(0, 0);
		delete[] cstr;
	}
}

void stop_cutscene() {
	OBJECT::DELETE_OBJECT(&xaxis);
	OBJECT::DELETE_OBJECT(&zaxis);
	if (CAM::DOES_CAM_EXIST(CutCam)) {
		CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
		CAM::DESTROY_CAM(CutCam, true);
	}
	CAM::DO_SCREEN_FADE_IN(0);
	CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
	CUTSCENE::REMOVE_CUTSCENE();
	CAM::DO_SCREEN_FADE_IN(0);
	curr_cut_ped_me = -1;
	my_first_coords = -1;
	curr_cut_ped = -1;
	cutscene_is_playing = false;
	cutscene_being_watched = false;
	switched_c = -1;
	found_ped_in_cutscene = false;
	con_disabled = false;
	manual_cutscene = false;
}

bool onconfirm_misc_cutscene_menu(MenuItem<int> choice) {
	if (choice.value == -1) {
		stop_cutscene();
	} 
	else if(choice.value == -2) {
		found_ped_in_cutscene = false;
	}
	else if (choice.value == -3) {
		keyboard_on_screen_already = true;
		curr_message = "输入过场动画名称 ( 例如: mph_nar_fin_ext 或 bmad_intro )";
		std::string result = show_keyboard("手动输入名称", NULL);
		if (!result.empty()) {
			result = trim(result);
			play_cutscene(result);
		}
	}
	else {
		std::string value_m = MISC_CUTSCENE_VALUES[choice.value];
		play_cutscene(value_m);
	}
	return false;
}

void process_misc_cutplayer_menu() {
	std::vector<MenuItem<int>*> menuItems;
	std::vector<std::string> captions;
	
	captions = MISC_CUTSCENE_CAPTIONS;// 过场动画 使用中文显示名
	ToggleMenuItem<int>* toggleItem;
	
	int i = -1;

	MenuItem<int> *item = new MenuItem<int>();
	item->caption = "按 [空格键] 停止播放";
	item->value = -1;
	item->isLeaf = true;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "第一人称视角过场动画";
	toggleItem->value = i++;
	toggleItem->toggleValue = &featureFirstPersonCutscene;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "切换观看视角";
	item->value = -2;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "手动输入名称";
	item->value = -3;
	item->isLeaf = true;
	menuItems.push_back(item);

	for each (std::string scenario in captions)
	{
		item = new MenuItem<int>();
		item->caption = scenario;
		item->value = i++;
		menuItems.push_back(item);
	}

	draw_generic_menu<int>(menuItems, nullptr, "播放过场动画", onconfirm_misc_cutscene_menu, nullptr, nullptr, nullptr);
}

bool onconfirm_misc_filters_menu(MenuItem<int> choice) {
	std::string value_m = MISC_FILTERS_VALUES[choice.value];
	char *cstr = new char[value_m.length() + 1];
	strcpy(cstr, value_m.c_str());

	GRAPHICS::SET_TIMECYCLE_MODIFIER(cstr);
	GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);
	screenfltr = cstr;
	return false;
}

void process_misc_filters_menu() {
	std::vector<MenuItem<int>*> menuItems;
	std::vector<std::string> captions;
	
	captions = MISC_FILTERS_CAPTIONS;// 屏幕滤镜 使用中文显示名
	MenuItem<int> *item = new MenuItem<int>();
	
	int i = 0;
	for each (std::string scenario in captions)
	{
		item = new MenuItem<int>();
		item->caption = scenario;
		item->value = i++;
		menuItems.push_back(item);
	}

	draw_generic_menu<int>(menuItems, nullptr, "屏幕滤镜", onconfirm_misc_filters_menu, nullptr, nullptr, nullptr);
}

bool onconfirm_misc_musicevent_menu(MenuItem<int> choice) {
	if (choice.value == -1) {
		AUDIO::TRIGGER_MUSIC_EVENT("AC_STOP");
	}
	else {
		std::string value_m = MISC_MUSICEVENT_VALUES[choice.value];
		char *cstr = new char[value_m.length() + 1];
		strcpy(cstr, value_m.c_str());
		AUDIO::TRIGGER_MUSIC_EVENT(cstr);
		delete[] cstr;
	}
	return false;
}

void process_misc_musicevent_menu() {
	std::vector<MenuItem<int>*> menuItems;
	std::vector<std::string> captions;
	
	captions = MISC_MUSICEVENT_CAPTIONS;// 场景音乐 使用中文显示名
	
	MenuItem<int> *item = new MenuItem<int>();
	item->caption = "~h~停止播放";
	item->value = -1;
	item->isLeaf = true;
	menuItems.push_back(item);

	int i = 0;
	for each (std::string scenario in captions)
	{
		item = new MenuItem<int>();
		item->caption = scenario;
		item->value = i++;
		menuItems.push_back(item);
	}

	draw_generic_menu<int>(menuItems, nullptr, "播放场景音乐", onconfirm_misc_musicevent_menu, nullptr, nullptr, nullptr);
}

void process_misc_freezeradio_menu(){ 
	std::vector<MenuItem<int> *> menuItems;
	int const stations = AUDIO::_MAX_RADIO_STATION_INDEX();

	// “无” —— 单选复选框（未冻结时选中）
	{
		FunctionDrivenToggleMenuItem<int>* tItem = new FunctionDrivenToggleMenuItem<int>();
		tItem->caption = "~h~取消冻结";
		tItem->value = -1;
		tItem->getter_call = is_radio_freeze_toggle_selected;
		tItem->setter_call = set_radio_freeze_toggle_selected;
		tItem->extra_arguments.push_back(-1);
		menuItems.push_back(tItem);
	}

	// 所有电台 —— 单选复选框（选择即冻结该电台）
	for(int a = 0; a < stations; a++){
		// 先用原始键过滤隐藏电台，避免因本地化导致判断不准确
		const char* key = AUDIO::GET_RADIO_STATION_NAME(a);
		if (key && std::string(key) == "HIDDEN_RADIO_MPSUM2_NEWS") {
			continue;
		}
		FunctionDrivenToggleMenuItem<int>* tItem = new FunctionDrivenToggleMenuItem<int>();
		// 显示游戏本地化的电台名称（中文等），缺失时回退到原始键
		std::string caption = get_radio_station_label_with_fallback_by_index(a);
		tItem->caption = caption;
		tItem->value = a;
		tItem->getter_call = is_radio_freeze_toggle_selected;
		tItem->setter_call = set_radio_freeze_toggle_selected;
		tItem->extra_arguments.push_back(a);
		menuItems.push_back(tItem);
	}

	// 切换项不需要确认回调；保留菜单常规行为（分页、高亮等）
	draw_generic_menu<int>(menuItems, nullptr, "冻结并固定电台", NULL, nullptr, nullptr, nullptr);
}

bool onconfirm_airbrake_menu(MenuItem<int> choice) {

	if (choice.value == -1) {
		if (airbrake_enable) process_airbrake_menu();
	}
	return false;
}
 
void process_airbrake_global_menu() {
	const std::string caption = "自由移动模式";

	std::vector<MenuItem<int>*> menuItems;
	MenuItem<int> *item;

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用";
	toggleItem->toggleValue = &airbrake_enable;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "自由移动模式 [默认 F6 开/关 ]";
	item->value = -1;
	item->isLeaf = true;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "鼠标模式";
	toggleItem->toggleValue = &mouse_view_control;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "显示帮助控制说明";
	toggleItem->toggleValue = &help_showing;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "冻结时间";
	toggleItem->toggleValue = &frozen_time;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "透 明 度";
	toggleItem->toggleValue = &show_transparency;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexAirbrake, caption, onconfirm_airbrake_menu, NULL, NULL);
}

bool onconfirm_defmenutab_menu(MenuItem<int> choice) {

	return false;
}

void process_def_menutab_menu() {
	const std::string caption = "暂停菜单设置";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	
	listItem = new SelectFromListMenuItem(MISC_DEF_MENUTAB_CAPTIONS, onchange_misc_def_menutab_index);
	listItem->wrap = false;
	listItem->caption = "默认的暂停菜单页";
	listItem->value = DefMenuTabIndex;
	menuItems.push_back(listItem);

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "暂停菜单打开时, 暂停游戏";
	toggleItem->toggleValue = &featureGamePause;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "暂停菜单页, 隐藏角色信息";
	toggleItem->toggleValue = &featureHidePlayerInfo;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexDefMenuTab, caption, onconfirm_defmenutab_menu, NULL, NULL);
}

bool onconfirm_billsettings_menu(MenuItem<int> choice) {

	return false;
}

void process_billsettings_menu() {
	const std::string caption = "手机电话费";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用电话费";
	toggleItem->toggleValue = &featurePhoneBillEnabled;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(MISC_PHONE_BILL_CAPTIONS, onchange_misc_phone_bill_index);
	listItem->wrap = false;
	listItem->caption = "每分钟话费金额";
	listItem->value = PhoneBillIndex;
	menuItems.push_back(listItem);

	listItem = new SelectFromListMenuItem(MISC_PHONE_FREESECONDS_CAPTIONS, onchange_misc_phone_freeseconds_index);
	listItem->wrap = false;
	listItem->caption = "前几秒免费";
	listItem->value = PhoneFreeSecondsIndex;
	menuItems.push_back(listItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "余额为零时禁止使用手机";
	toggleItem->toggleValue = &featureZeroBalance;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexBillSettings, caption, onconfirm_billsettings_menu, NULL, NULL);
}

bool onconfirm_phoneonbike_menu(MenuItem<int> choice) {

	return false;
}

void process_phoneonbike_menu() {
	const std::string caption = "骑车时使用手机";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用";
	toggleItem->toggleValue = &featurePhone3DOnBike;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "第一人称视角时, 屏幕上不显示手机";
	toggleItem->toggleValue = &featureNoPhoneOnHUD;
	menuItems.push_back(toggleItem);

	listItem = new SelectFromListMenuItem(MISC_PHONE_BIKE_ANIM_CAPTIONS, onchange_misc_phone_bike_index);
	listItem->wrap = false;
	listItem->caption = "手机动画类型";
	listItem->value = PhoneBikeAnimationIndex;
	menuItems.push_back(listItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexPhoneOnBike, caption, onconfirm_phoneonbike_menu, NULL, NULL);
}

bool onconfirm_phonebill_menu(MenuItem<int> choice){
	switch (activeLineIndexPhoneBill) {
		case 1:
			process_billsettings_menu();
			break;
		case 2:
			process_phoneonbike_menu();
			break;
		default:
			break;
		}
	return false;
}

void process_phone_bill_menu(){
	const std::string caption = "手机设置选项";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	MenuItem<int> *item;

	int i = 0;

	listItem = new SelectFromListMenuItem(MISC_PHONE_DEFAULT_CAPTIONS, onchange_misc_phone_default_index);
	listItem->wrap = false;
	listItem->caption = "默认的手机型号";
	listItem->value = PhoneDefaultIndex;
	menuItems.push_back(listItem);

	item = new MenuItem<int>();
	item->caption = "手机电话费";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "骑车时使用手机";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "禁用手机";
	toggleItem->toggleValue = &featureDisablePhone;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "菜单打开时禁用手机";
	toggleItem->toggleValue = &featureDisablePhoneMenu;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexPhoneBill, caption, onconfirm_phonebill_menu, NULL, NULL);
}

bool onconfirm_radiosettings_menu(MenuItem<int> choice) {
	switch (activeLineIndexRadioSettings) {
	case 2:
		// 下一曲收音机曲目
		if (getGameVersion() > 41) SKIP_RADIO_FORWARD_CUSTOM();
		else AUDIO::SKIP_RADIO_FORWARD();
		skip_track_pressed = true;
		break;
	case 4:
		process_misc_freezeradio_menu();
		break;
	default:
		break;
	}
	return false;
}

void process_radio_settings_menu() {
	const std::string caption = "电台设置选项";

	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	MenuItem<int> *item;

	int i = 0;

	listItem = new SelectFromListMenuItem(MISC_RADIO_OFF_CAPTIONS, onchange_misc_radio_off_index);
	listItem->wrap = false;
	listItem->caption = "关闭电台";
	listItem->value = RadioOffIndex;
	menuItems.push_back(listItem);

	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "随身电台";
	toggleItem->toggleValue = &featurePlayerRadio;
	menuItems.push_back(toggleItem);

	item = new MenuItem<int>();
	item->caption = "下一首电台曲目";
	item->value = i++;
	item->isLeaf = true;
	menuItems.push_back(item);

	listItem = new SelectFromListMenuItem(MISC_RADIO_SWITCHING_CAPTIONS, onchange_misc_radio_switching_index);
	listItem->wrap = false;
	listItem->caption = "电台随机切换模式";
	listItem->value = RadioSwitchingIndex;
	// 记录指针以便在“冻结并固定电台”子菜单中能即时刷新当前选中值
	g_RadioSwitchingSelectItem = listItem;
	menuItems.push_back(listItem);

	item = new MenuItem<int>();
	item->caption = "冻结并固定电台";
	item->value = i++;
	item->isLeaf = false;
	menuItems.push_back(item);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "提高电台的音量";
	toggleItem->toggleValue = &featureBoostRadio;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "使用第一人称电台音量";
	toggleItem->toggleValue = &featureRealisticRadioVolume;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "解锁 缺失/隐藏的 电台";
	toggleItem->toggleValue = &featureEnableMissingRadioStation;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用警车的电台";
	toggleItem->toggleValue = &featurePoliceRadio;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexRadioSettings, caption, onconfirm_radiosettings_menu, NULL, NULL);

	// 菜单退出后清理指针，避免悬挂引用
	g_RadioSwitchingSelectItem = NULL;
}

bool onconfirm_hudsettings_menu(MenuItem<int> choice) {
	return false;
}

void process_hud_settings_menu() {
	const std::string caption = "小地图设置";

	std::vector<MenuItem<int>*> menuItems;

	int i = 0;
	
	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "隐藏/显示 小地图";
	toggleItem->toggleValue = &featureMiscHideHud;
	toggleItem->toggleValueUpdated = &featureMiscHideHudUpdated;
	menuItems.push_back(toggleItem);
	
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "仅在打开电话时显示 小地图";
	toggleItem->toggleValue = &featurePhoneShowHud;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "仅在车辆中显示 小地图";
	toggleItem->toggleValue = &featureInVehicleNoHud;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "仅在设置导航点后显示 小地图";
	toggleItem->toggleValue = &featureMarkerHud;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "显示实时动态生命值条";
	toggleItem->toggleValue = &featureDynamicHealthBar;
	menuItems.push_back(toggleItem);
	
	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "禁用动态模糊和减速效果";
	toggleItem->toggleValue = &featurenowheelblurslow;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "禁用屏幕录像";
	toggleItem->toggleValue = &featureDisableRecording;
	menuItems.push_back(toggleItem);

	toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "禁用任何游戏通知";
	toggleItem->toggleValue = &featureNoNotifications;
	menuItems.push_back(toggleItem);

	draw_generic_menu<int>(menuItems, &activeLineIndexHudSettings, caption, onconfirm_hudsettings_menu, NULL, NULL);
}

int activeLineIndexMisc = 0;

bool onconfirm_misc_menu(MenuItem<int> choice){
	switch(activeLineIndexMisc){
		case 0:
			process_misc_trainerconfig_menu();
			break;
		case 1:
			process_radio_settings_menu();
			break;
		case 2:
			process_hud_settings_menu();
			break;
		case 3:
			process_phone_bill_menu();
			break;
		case 4:
			process_def_menutab_menu();
			break;
		case 5:
			process_misc_musicevent_menu();
			break;
		case 6:
			process_misc_cutplayer_menu();
			break;
		case 7:
			process_misc_filters_menu();
			break;
		case 16:
			process_airbrake_global_menu();
			break;
		case 17:
			process_misc_freecam_menu();
			break;
		case 18:
			process_misc_screenshot_menu();
			break;
		default:
			// 可切换功能
			break;
	}
	return false;
}

void process_misc_menu(){
	const int lineCount = 19; 

	const std::string caption = "其他选项";

	StandardOrToggleMenuDef lines[lineCount] = {
		{"修改器设置", NULL, NULL, false},
		{"电台设置", NULL, NULL, false},
		{"小地图设置", NULL, NULL, false},
		{"手机设置", NULL, NULL, false},
		{"暂停菜单设置", NULL, NULL, false},
		{"播放场景音乐", nullptr, nullptr, false},
		{"播放过场动画", nullptr, nullptr, false},
		{"屏幕滤镜", nullptr, nullptr, false},
		{"无通缉音乐", &featureWantedMusic, NULL, true}, 
		{"无飞行音乐", &featureFlyingMusic, NULL, true}, 
		{"无警察电台广播", &featurePoliceScanner, NULL, true }, 
		{"无任务完成消息", &featureNoComleteMessage, NULL, true },
		{"第一人称特技跳跃视角", &featureFirstPersonStuntJumpCamera, NULL },
		{"无特技跳跃", &featureNoStuntJumps, NULL },
		{"FPS 帧率显示 ", &featureShowFPS, NULL }, 
		{"模型名称显示", &featureShowModelName, NULL }, 
		{"自由移动模式", NULL, NULL, false},
		{"自由相机模式", NULL, NULL, false},
		{"游戏全屏截图", NULL, NULL, false},
	};
	
	draw_menu_from_struct_def(lines, lineCount, &activeLineIndexMisc, caption, onconfirm_misc_menu);
}

// THE ORIGINAL CODE IS BY CAMXXCORE
#define XOR_32_64 0x31 // 逻辑异或
#define RET 0xC3 // 返回

bool setupPatches() {
	auto result = FindPatternJACCO("\x38\x51\x64\x74\x19", "xxxxx");
	if (!result) {
		return false;
	}

	auto address = result + 26;
	address = address + *(int32_t*)address + 4u;
	auto vigNetCallPtr = result + 8;
	auto timescalePtr = result + 34;
	
	unsigned char vigNetPatch[] = { RET, 0x90, 0x90, 0x90, 0x90 }; // 移除晕影
	unsigned char vigNetCall[] = { 0x90, 0x90, 0x90, 0x90, 0x90 }; // 晕影调用补丁（NOP）
	unsigned char timeScaleOverride[] = { XOR_32_64, 0xD2 }; // 时间缩放覆盖补丁

	memcpy((void*)address, vigNetPatch, sizeof(vigNetPatch) / sizeof(vigNetPatch[0]));
	memcpy((void*)vigNetCallPtr, vigNetCall, sizeof(vigNetCall) / sizeof(vigNetCall[0]));
	memcpy((void*)timescalePtr, timeScaleOverride, sizeof(timeScaleOverride) / sizeof(timeScaleOverride[0]));
	return true;
}

void initialize() {
	if (!setupPatches()) {
		return;
	}
	setupPatches();
	// 初始化快捷键设置
	load_hotkey_settings_from_xml();
	// 初始化常用按键和其他按键设置（包含自由相机热键）
	load_common_other_keys_from_xml();
	// 初始化GDI截图系统
	init_gdi_screenshot_system();
}

void onchange_misc_phone_bill_index(int value, SelectFromListMenuItem* source){
	PhoneBillIndex = value;
	PhoneBillChanged = true;
}

void onchange_misc_phone_default_index(int value, SelectFromListMenuItem* source) {
	PhoneDefaultIndex = value;
	PhoneDefaultChanged = true;
}

void onchange_misc_radio_off_index(int value, SelectFromListMenuItem* source) {
	RadioOffIndex = value;
}

void onchange_misc_radio_switching_index(int value, SelectFromListMenuItem* source) {
	// 当电台已被冻结且用户试图开启“定时随机切换”（值>1）时，提示并阻止后续随机切换
	if (featureRadioFreeze && MISC_RADIO_SWITCHING_VALUES[value] > 1) {
		set_status_text("电台已冻结, 并禁止随机切换!\n仅允许, 快捷键切换电台歌曲!");
		RadioSwitchingIndex = 0; // 恢复为“关”
		if (source) source->value = RadioSwitchingIndex; // 同步界面显示
		return;
	}

	// 其它情况正常更新（允许 0=关 和 1=下一首电台歌曲）
	RadioSwitchingIndex = value;
}

void onchange_misc_trainercontrol_index(int value, SelectFromListMenuItem* source) {
	TrainerControlIndex = value;
	TrainerControlChanged = true;
}

void onchange_misc_trainercontrolscrolling_index(int value, SelectFromListMenuItem* source) {
	TrainerControlScrollingIndex = value;
	TrainerControlScrollingChanged = true;
}

void onchange_misc_def_menutab_index(int value, SelectFromListMenuItem* source) {
	DefMenuTabIndex = value;
	DefMenuTabChanged = true;
}

void onchange_misc_phone_freeseconds_index(int value, SelectFromListMenuItem* source){
	PhoneFreeSecondsIndex = value;
	PhoneFreeSecondsChanged = true;
}

void onchange_misc_phone_bike_index(int value, SelectFromListMenuItem* source) {
	PhoneBikeAnimationIndex = value;
	PhoneBikeAnimationChanged = true;
}

void onchange_misc_font_header_index(int value, SelectFromListMenuItem* source) {
    FontHeaderIndex = value;
    fontHeader = MISC_FONT_HEADER_VALUES[value]; // 更新全局字体变量
    FontHeaderChanged = true;
}

void onchange_misc_font_item_index(int value, SelectFromListMenuItem* source) {
    FontItemIndex = value;
    fontItem = MISC_FONT_ITEM_VALUES[value]; // 更新全局字体变量
    FontItemChanged = true;
}

void onchange_misc_font_wanted_index(int value, SelectFromListMenuItem* source) {
    FontWantedIndex = value;
    fontWanted = MISC_FONT_WANTED_VALUES[value]; // 更新全局字体变量
    FontWantedChanged = true;
}

void onchange_misc_font_status_index(int value, SelectFromListMenuItem* source) {
    FontStatusIndex = value;
    fontStatus = MISC_FONT_STATUS_VALUES[value]; // 更新全局字体变量
    FontStatusChanged = true;
}

void onchange_misc_menu_items_count_index(int value, SelectFromListMenuItem* source) {
    MenuItemsCountIndex = value;
    itemsPerLine = MISC_MENU_ITEMS_COUNT_VALUES[value]; // 更新全局菜单显示项目数变量
    MenuItemsCountChanged = true;
}

void process_misc_font_settings_menu() {
    const std::string caption = "字体类型设置";

    std::vector<MenuItem<int>*> menuItems;
    SelectFromListMenuItem *listItem;

    listItem = new SelectFromListMenuItem(MISC_FONT_HEADER_CAPTIONS, onchange_misc_font_header_index);
    listItem->wrap = false;
    listItem->caption = "标题选项字体类型";
    listItem->value = FontHeaderIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_FONT_ITEM_CAPTIONS, onchange_misc_font_item_index);
    listItem->wrap = false;
    listItem->caption = "项目选项字体类型";
    listItem->value = FontItemIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_FONT_WANTED_CAPTIONS, onchange_misc_font_wanted_index);
    listItem->wrap = false;
    listItem->caption = "通缉等级星星类型";
    listItem->value = FontWantedIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_FONT_STATUS_CAPTIONS, onchange_misc_font_status_index);
    listItem->wrap = false;
    listItem->caption = "状态显示字体类型";
    listItem->value = FontStatusIndex;
    menuItems.push_back(listItem);

    draw_generic_menu<int>(menuItems, &activeLineIndexFontSettings, caption, NULL, NULL, NULL);
}

void HUD_switching() {
	featureMiscHideHud = !featureMiscHideHud;
	//if (featureMiscHideHud) set_status_text("HUD OFF");
	//else set_status_text("HUD ON");
	WAIT(100);
}

void Traffic_switching() {
	featureWorldNoTraffic = !featureWorldNoTraffic;
	featureWorldNoTrafficUpdated = !featureWorldNoTrafficUpdated;
	WAIT(100);
}

void reset_misc_globals(){
	featureMiscHideHud =
		featurePhoneShowHud = 
		featureInVehicleNoHud =
		featureMarkerHud =
		featureDynamicHealthBar =
		featureDisableRecording =
		featureNoNotifications =
		featurePlayerRadio =
		featureDisablePhone =
		featureDisablePhoneMenu =
		featureMiscLockRadio =
		featureMiscJellmanScenery =
		featureRadioFreeze =
		featureWantedMusic = 
		featureFlyingMusic = 
		featurePoliceScanner = 
		featureNoComleteMessage =
		featurePoliceRadio =
		featureEnableMissingRadioStation = false;

	PhoneBillIndex = 2;
	PhoneDefaultIndex = 0;
	RadioOffIndex = 0;
	RadioSwitchingIndex = 0;
	TrainerControlIndex = 0;
	TrainerControlScrollingIndex = 0;
	PhoneFreeSecondsIndex = 0;
	PhoneBikeAnimationIndex = 0;
	DefMenuTabIndex = 0;
	FontHeaderIndex = 0;
	fontHeader = MISC_FONT_HEADER_VALUES[0]; // 重置为默认标题字体
	FontItemIndex = 0;
	fontItem = MISC_FONT_ITEM_VALUES[0]; // 重置为默认项目字体
	FontWantedIndex = 0;
	fontWanted = MISC_FONT_WANTED_VALUES[0]; // 重置为默认通缉字体
	MenuItemsCountIndex = 0;
	itemsPerLine = MISC_MENU_ITEMS_COUNT_VALUES[0]; // 重置为默认菜单显示项目数
	FontStatusIndex = 0;
	fontStatus = MISC_FONT_STATUS_VALUES[0]; // 重置为默认状态字体

	// 重置菜单布局设置
	reset_menu_layout_to_defaults();

	// 重置自由相机设置
	reset_freecam_settings_to_defaults();

	//featureControllerIgnoreInTrainer = false;
	//featureBlockInputInMenu = false;
	featureShowVehiclePreviews = true;
	featureShowPedPreviews = true;
	featureShowStatusMessage = true;
	airbrake_enable = true;
	show_transparency = true;
	featureFirstPersonCutscene = false;
	mouse_view_control = false;
	help_showing = true;
	frozen_time = false;
	featurePhoneBillEnabled = false;
	featureGamePause = false;
	featureZeroBalance = false;
	featurePhone3DOnBike = false;
	featureNoPhoneOnHUD = false;
	featureFirstPersonDeathCamera = false;
	featureFirstPersonStuntJumpCamera = false;
	featureNoStuntJumps = false;
	featureHidePlayerInfo = false;
	featureShowFPS = false;
	featurenowheelblurslow = false;
	featureNoAutoRespawn = false;
	featureRealisticRadioVolume = false;
	featureShowModelName = false;// 模型名称显示变量，重置/覆盖当前值，默认关闭

	featureRadioFreezeUpdated =
	featureMiscHideHudUpdated =
	featureBoostRadio = true;

	// 重置截图功能（调用screenshot.cpp中的重置函数）
	reset_screenshot_settings();

	ENTColor::reset_colors();
}

void update_misc_features(BOOL playerExists, Ped playerPed){
	// 收音机关闭
	if (NPC_RAGDOLL_VALUES[RadioOffIndex] > 0 && !PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) radio_pressed = false;
	if (NPC_RAGDOLL_VALUES[RadioOffIndex] > 0 && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0) && CONTROLS::IS_CONTROL_PRESSED(2, 85)) {
		radio_pressed = true;
		AUDIO::SET_VEHICLE_RADIO_ENABLED(PED::GET_VEHICLE_PED_IS_USING(playerPed), true);
		AUDIO::SET_USER_RADIO_CONTROL_ENABLED(true);
	}
	if (NPC_RAGDOLL_VALUES[RadioOffIndex] > 0 && radio_pressed == false) {
		if (NPC_RAGDOLL_VALUES[RadioOffIndex] == 1) {
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
				Vehicle playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				AUDIO::SET_VEHICLE_RADIO_ENABLED(playerVeh, false);
			}
			AUDIO::SET_USER_RADIO_CONTROL_ENABLED(false);
		}
		if (NPC_RAGDOLL_VALUES[RadioOffIndex] == 2) {
			Vehicle cur_veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
				if (VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(cur_veh)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(cur_veh))) {
					AUDIO::SET_VEHICLE_RADIO_ENABLED(cur_veh, false);
					AUDIO::SET_USER_RADIO_CONTROL_ENABLED(false);
				}
			}
			if ((PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0) && !VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(cur_veh)) && !VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(cur_veh))) || !PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
				AUDIO::SET_VEHICLE_RADIO_ENABLED(cur_veh, true);
				AUDIO::SET_USER_RADIO_CONTROL_ENABLED(true);
			}
		}
	}
	if (NPC_RAGDOLL_VALUES[RadioOffIndex] == 0) {
		AUDIO::SET_VEHICLE_RADIO_ENABLED(PED::GET_VEHICLE_PED_IS_USING(playerPed), true);
		AUDIO::SET_USER_RADIO_CONTROL_ENABLED(true);
		radio_pressed = false;
	}

	// 便携式收音机
	if (featurePlayerRadio || featurePlayerRadioUpdated) {
		if (featurePlayerRadio) AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(true);
		else AUDIO::SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(false);
	}

	// 无通缉音乐
	if (featureWantedMusic) AUDIO::SET_AUDIO_FLAG("WantedMusicDisabled", true);
	else AUDIO::SET_AUDIO_FLAG("WantedMusicDisabled", false);
	
	// 无飞行音乐
	if (featureFlyingMusic) AUDIO::SET_AUDIO_FLAG("DisableFlightMusic", true);
	else AUDIO::SET_AUDIO_FLAG("DisableFlightMusic", false);
	
	// 无警察扫描器
	if (featurePoliceScanner) AUDIO::SET_AUDIO_FLAG("PoliceScannerDisabled", true);
	else AUDIO::SET_AUDIO_FLAG("PoliceScannerDisabled", false);
	
	// 无“任务完成”消息
	if (featureNoComleteMessage) {
		if (!SCRIPT::HAS_SCRIPT_LOADED("family3") && !SCRIPT::HAS_SCRIPT_LOADED("jewelry_heist") && !SCRIPT::HAS_SCRIPT_LOADED("family5") && !SCRIPT::HAS_SCRIPT_LOADED("wardrobe_sp") && !SCRIPT::HAS_SCRIPT_LOADED("family6"))
			GAMEPLAY::TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("mission_stat_watcher");
	}

	// 收音机增强
	if (featureBoostRadio) {
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		AUDIO::SET_VEHICLE_RADIO_LOUD(playerVeh, 1);
	}
	if (!featureBoostRadio) AUDIO::SET_VEHICLE_RADIO_LOUD(PED::GET_VEHICLE_PED_IS_USING(playerPed), 0);
	
	// 一致的收音机音量
	if (featureRealisticRadioVolume && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0) && radio_v_checked == false) {
		Vehicle cur_v = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		if (VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(1) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		if (VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(1) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		//if ((VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(cur_v)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(cur_v))) && CAM::_0xEE778F8C7E1142E2(2) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		//if ((VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(cur_v)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(cur_v))) && CAM::_0xEE778F8C7E1142E2(2) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		if (VEHICLE::IS_THIS_MODEL_A_BOAT(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(3) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		if (VEHICLE::IS_THIS_MODEL_A_BOAT(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(3) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		if (VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(4) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		if (VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(4) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		if ((ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE") || ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE2")) && CAM::_0xEE778F8C7E1142E2(5) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		if ((ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE") || ENTITY::GET_ENTITY_MODEL(cur_v) == GAMEPLAY::GET_HASH_KEY("SUBMERSIBLE2")) && CAM::_0xEE778F8C7E1142E2(5) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		if (VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(6) != 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(false);
		if (VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(cur_v)) && CAM::_0xEE778F8C7E1142E2(6) == 4) AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		radio_v_checked = true;
	}
	if (featureRealisticRadioVolume && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0) && CONTROLS::IS_CONTROL_JUST_RELEASED(2, 0)) {
		WAIT(100);
		radio_v_checked = false;
	}
	if (((!featureRealisticRadioVolume && PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) || (featureRealisticRadioVolume && !PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))) && radio_v_checked == true) {
		AUDIO::SET_FRONTEND_RADIO_ACTIVE(true);
		radio_v_checked = false;
	}
	
	// 收音机关台切换（若已冻结电台则不进行随机切换，避免与固定频道冲突）
	if (MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex] > 0 && !featureRadioFreeze && (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0) || featurePlayerRadio || featurePlayerRadioUpdated)) {
		if (MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex] > 1) {
			r_secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / CLOCKS_PER_SEC) - r_secs_curr) != 0) {
				r_seconds = r_seconds + 1;
				r_secs_curr = r_secs_passed;
			}
		}
		if (MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex] == 1 && (is_hotkey_held_veh_radio_skip() || skip_track_pressed == true)) {
			// 文案为“下一首电台歌曲”，应当跳过当前电台曲目而非随机切台
			if (getGameVersion() > 41) SKIP_RADIO_FORWARD_CUSTOM();
			else AUDIO::SKIP_RADIO_FORWARD();
			skip_track_pressed = false;
		} else if (MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex] > 1 && r_seconds > MISC_RADIO_SWITCHING_VALUES[RadioSwitchingIndex]) {
			// 定时随机切换：随机选择一个电台
			int const stations = AUDIO::_MAX_RADIO_STATION_INDEX();
			int random_station = (rand() % stations);
			AUDIO::SET_RADIO_TO_STATION_INDEX(random_station);
			r_seconds = 0;
		}
	}

		// 警车中的收音机
		if (featurePoliceRadio)
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();
	
		// 玩家是否在警车内
		if (PED::IS_PED_IN_ANY_POLICE_VEHICLE(playerPed))
		{
			Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	
			// 检查是否驾驶员
			if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh, -1) == playerPed)
			{
				// 检查引擎是否启动
				if (VEHICLE::GET_IS_VEHICLE_ENGINE_RUNNING(veh))
				{
					if (!policeRadioActive)
					{
						AUDIO::SET_VEHICLE_RADIO_ENABLED(veh, true);
						AUDIO::SET_RADIO_AUTO_UNFREEZE(true);
						AUDIO::SET_USER_RADIO_CONTROL_ENABLED(true);
	
						policeRadioActive = true;
						lastPoliceVeh = veh;
					}
				}
				else
				{
					// 引擎没开则关闭
					if (policeRadioActive && lastPoliceVeh == veh)
					{
						AUDIO::SET_VEHICLE_RADIO_ENABLED(veh, false);
						policeRadioActive = false;
						lastPoliceVeh = 0;
					}
				}
			}
			else
			{
				// 副驾或后排关闭
				if (policeRadioActive && lastPoliceVeh == veh)
				{
					AUDIO::SET_VEHICLE_RADIO_ENABLED(veh, false);
					policeRadioActive = false;
					lastPoliceVeh = 0;
				}
			}
		}
		else
		{
			// 离开警车清理
			if (policeRadioActive && lastPoliceVeh != 0)
			{
				AUDIO::SET_VEHICLE_RADIO_ENABLED(lastPoliceVeh, false);
				policeRadioActive = false;
				lastPoliceVeh = 0;
			}
		}
	}
	else
	{
		// 功能关闭时强制清理状态
		if (policeRadioActive && lastPoliceVeh != 0)
		{
			AUDIO::SET_VEHICLE_RADIO_ENABLED(lastPoliceVeh, false);
			policeRadioActive = false;
			lastPoliceVeh = 0;
		}
	}
	
	// 冻结收音机频道
	if (featureRadioFreeze) {
		if (AUDIO::GET_PLAYER_RADIO_STATION_INDEX() != radioStationIndex && AUDIO::GET_PLAYER_RADIO_STATION_INDEX() != 255) {
			AUDIO::SET_RADIO_TO_STATION_INDEX(radioStationIndex);
		}
	}
	else if (featureRadioFreezeUpdated) {
		// 暂时留空。
	}

	// 隐藏 HUD
	if (featureMiscHideHud/* || (featureMiscHideENTHud && menu_showing == true)*/) {
		for (int i = 0; i < 21; i++) {
			//至少在理论上...
			switch (i) {
			case 5:  //多人游戏消息
			case 10: //帮助文本
			case 11: //浮动帮助 1
			case 12: //浮动帮助 2
			case 14: //准星
			case 16: //无线电轮
			case 19: //武器轮
				continue;
			}
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(i);
		}

		UI::DISPLAY_RADAR(false);
		featureMiscHideHudUpdated = false;
	}
	else if (/*(*/featureMiscHideHudUpdated/* && !featureMiscHideENTHud) || (featureMiscHideENTHud && menu_showing == false)*/){
		UI::DISPLAY_RADAR(true);
		featureMiscHideHudUpdated = false;
	}
	
	// 如果手持电话，则显示 HUD
	if (featurePhoneShowHud) {
		if (!phone_toggle) {
			UI::DISPLAY_RADAR(false);
			featureMiscHideHudUpdated = false;
		}
		
		if (PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed)) {
			UI::DISPLAY_RADAR(true);
			phone_toggle = true;
		}
	
		if (!PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed)) {
			UI::DISPLAY_RADAR(false);
			phone_toggle = false;
		}
	}
	else if (!featureMiscHideHud && !featureInVehicleNoHud && !featureMarkerHud/* && !featureMiscHideENTHud*/) {
		UI::DISPLAY_RADAR(true);
		phone_toggle = false;
	}
	
	// 仅在车辆内显示 HUD
	if (featureInVehicleNoHud) {
		if (!phone_toggle_vehicle && !featurePhoneShowHud) {
			UI::DISPLAY_RADAR(false);
			featureMiscHideHudUpdated = false;
		}

		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1)) {
			UI::DISPLAY_RADAR(true);
			phone_toggle_vehicle = true;
		}

		if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 1) && !featurePhoneShowHud) {
			UI::DISPLAY_RADAR(false);
			phone_toggle_vehicle = false;
		}
	}
	else if (!featureMiscHideHud && !featurePhoneShowHud && !featureMarkerHud/* && !featureMiscHideENTHud*/) {
		UI::DISPLAY_RADAR(true);
		phone_toggle_vehicle = false;
	}
	
	// 仅在设置了标记时显示 HUD
	if (featureMarkerHud) {
		if (!phone_toggle_vehicle && !featurePhoneShowHud && !featureInVehicleNoHud) {
			UI::DISPLAY_RADAR(false);
			featureMiscHideHudUpdated = false;
		}
		bool blipFound = false;
		int blipIterator = UI::_GET_BLIP_INFO_ID_ITERATOR(); // 查找标记图标
		for (Blip i = UI::GET_FIRST_BLIP_INFO_ID(blipIterator); UI::DOES_BLIP_EXIST(i) != 0; i = UI::GET_NEXT_BLIP_INFO_ID(blipIterator)) {
			if (UI::GET_BLIP_INFO_ID_TYPE(i) == 4) {
				blipFound = true;
				break;
			}
		}
		if (blipFound) {
			UI::DISPLAY_RADAR(true);
			phone_toggle_vehicle = true;
		}
		if (!blipFound && !featurePhoneShowHud && !featureInVehicleNoHud) {
			UI::DISPLAY_RADAR(false);
			phone_toggle_vehicle = false;
		}
	}
	else if (!featureMiscHideHud && !featurePhoneShowHud && !featureInVehicleNoHud/* && !featureMiscHideENTHud*/) {
		UI::DISPLAY_RADAR(true);
		phone_toggle_vehicle = false;
	}

	// 禁用录制
	if (featureDisableRecording) {
		CONTROLS::DISABLE_CONTROL_ACTION(2, 170, 1); // 保存回放片段
		CONTROLS::DISABLE_CONTROL_ACTION(2, 288, 1); // 开始/停止回放录制
		CONTROLS::DISABLE_CONTROL_ACTION(2, 289, 1); // 开始/停止回放录制（次要）
		CONTROLS::DISABLE_CONTROL_ACTION(2, 302, 1); // 回放录制
	}

	// 无通知
	if (featureNoNotifications) UI::THEFEED_HIDE_THIS_FRAME();

	// 默认手机
	if (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] > -1) {
		if (PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed) && phone_toggle_defaultphone == false) {
			MOBILE::CREATE_MOBILE_PHONE(MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex]);
			phone_toggle_defaultphone = true;
		}
		if (!PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed) && phone_toggle_defaultphone == true) {
			MOBILE::DESTROY_MOBILE_PHONE();
			phone_toggle_defaultphone = false;
		}
	}
	
	// 骑车时使用手机
	if (featurePhone3DOnBike) {
		Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

	// 根据索引设置骑车时手机动画（不再依赖“前几秒免费”的值）
	Vector3 veh_s = ENTITY::GET_ENTITY_VELOCITY(PED::GET_VEHICLE_PED_IS_USING(playerPed));
	switch (PhoneBikeAnimationIndex) {
		case 0:
			anim_dict = "anim@cellphone@in_car@ps";
			animation_of_d = "cellphone_text_read_base";
			break;
		case 1:
			anim_dict = "cellphone@";
			animation_of_d = "cellphone_text_read_base_cover_low";
			break;
		case 2:
			anim_dict = "cellphone@str";
			animation_of_d = "cellphone_text_read_a";
			break;
		case 3:
			anim_dict = "cellphone@female";
			animation_of_d = "cellphone_email_read_base";
			break;
		case 4:
			anim_dict = "cellphone@first_person";
			animation_of_d = "cellphone_text_read_base";
			break;
		default:
			break;
	}

	// 确保选定的动画字典已加载
	if (!STREAMING::HAS_ANIM_DICT_LOADED(anim_dict)) {
		STREAMING::REQUEST_ANIM_DICT(anim_dict);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(anim_dict)) WAIT(0);
	}

		if ((PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1) && (VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(veh)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(veh)))) && PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed)) { // PED::IS_PED_ON_ANY_BIKE(playerPed)
			
			if (featureNoPhoneOnHUD && CAM::_0xEE778F8C7E1142E2(2) == 4/* && PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)*/) {
				if (PED::GET_VEHICLE_PED_IS_IN(playerPed, 1) != GAMEPLAY::GET_HASH_KEY("VERUS") && PED::GET_VEHICLE_PED_IS_IN(playerPed, 1) != GAMEPLAY::GET_HASH_KEY("SEASHARK") &&
					PED::GET_VEHICLE_PED_IS_IN(playerPed, 1) != GAMEPLAY::GET_HASH_KEY("SEASHARK2") && PED::GET_VEHICLE_PED_IS_IN(playerPed, 1) != GAMEPLAY::GET_HASH_KEY("SEASHARK3")) MOBILE::SET_MOBILE_PHONE_POSITION(10000, 10000, 10000);
			}
			
			Hash temp_Hash = -1;
			Vector3 temp_pos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
			
			if (STREAMING::HAS_ANIM_DICT_LOADED(anim_dict) && p_exist == false && CONTROLS::GET_CONTROL_VALUE(0, 9) == 127) { // 127 表示轮子没有转动
				WAIT(0);
				AI::TASK_PLAY_ANIM(playerPed, anim_dict, animation_of_d, 8.0, 0.0, -1, 9, 0, 0, 0, 0);
				if (!ENTITY::DOES_ENTITY_EXIST(temp_obj)) {
					if (PED::GET_PED_TYPE(playerPed) == 0 && (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == -1 || MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 3)) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing"); // 迈克尔
						if (PED::GET_PED_TYPE(playerPed) == 1 && (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == -1 || MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 3)) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing_03"); // 富兰克林
						if ((PED::GET_PED_TYPE(playerPed) == 2 || PED::GET_PED_TYPE(playerPed) == 3) && (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == -1 || MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 3))
							temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing_02"); // 崔佛
						if (PED::GET_PED_TYPE(playerPed) != 0 && PED::GET_PED_TYPE(playerPed) != 1 && PED::GET_PED_TYPE(playerPed) != 2 && PED::GET_PED_TYPE(playerPed) != 3 &&
							(MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == -1 || MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 3)) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_prologue_phone");
						if (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 0) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing");
						if (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 1) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing_02");
						if (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 2) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_phone_ing_03");
						if (MISC_PHONE_DEFAULT_VALUES[PhoneDefaultIndex] == 4) temp_Hash = GAMEPLAY::GET_HASH_KEY("prop_prologue_phone");
						temp_obj = OBJECT::CREATE_OBJECT(temp_Hash, temp_pos.x, temp_pos.y, temp_pos.z, 1, true, 1);
						int PlayerIndex1 = PED::GET_PED_BONE_INDEX(playerPed, 0x6f06);
						ENTITY::ATTACH_ENTITY_TO_ENTITY(temp_obj, playerPed, PlayerIndex1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
				}
				p_exist = true;
			}

			if (CONTROLS::IS_CONTROL_RELEASED(2, 71) && CONTROLS::IS_CONTROL_RELEASED(2, 72) && accel == true) { // 加速/刹车
				AI::STOP_ANIM_TASK(playerPed, anim_dict, animation_of_d, 1.0);
				CONTROLS::DISABLE_CONTROL_ACTION(2, 71, 1);
				CONTROLS::DISABLE_CONTROL_ACTION(2, 72, 1);
				accel = false;
				p_exist = false;
			}
			// 修正控制映射：72=刹车(S)，76=手刹(空格)，62=小键盘5(菜单确认)
			if (CONTROLS::IS_CONTROL_RELEASED(2, 63) && CONTROLS::IS_CONTROL_RELEASED(2, 64)) VEHICLE::_SET_BIKE_LEAN_ANGLE(PED::GET_VEHICLE_PED_IS_USING(playerPed), 0, 0); //  && CONTROLS::IS_CONTROL_PRESSED(2, 71)
			if (CONTROLS::IS_CONTROL_JUST_PRESSED(2, 75) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 72) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 63) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 64) ||
				(CONTROLS::IS_CONTROL_JUST_PRESSED(2, 71) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 76) && veh_s.x < 2 && veh_s.y < 2)) { // 离开/刹车/左/右/加速/下
				AI::STOP_ANIM_TASK(playerPed, anim_dict, animation_of_d, 1.0);
				accel = true;
				p_exist = false;
			}
		}
		
		if ((PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1) && ((VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(veh)) || VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(veh))) && !PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed) && STREAMING::HAS_ANIM_DICT_LOADED(anim_dict))) ||
			(!PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1) && STREAMING::HAS_ANIM_DICT_LOADED(anim_dict))) {
			OBJECT::DELETE_OBJECT(&temp_obj);
			AI::STOP_ANIM_TASK(playerPed, anim_dict, animation_of_d, 1.0);
			//STREAMING::REMOVE_ANIM_DICT(anim_dict);
			//STREAMING::REMOVE_ANIM_DICT(animation_of_d);
			accel = false;
			p_exist = false;
		} 
	}
	
	// 锁定玩家车辆车门
	if (featureLockVehicleDoors) {
		if (featureLockVehicleDoorsUpdated == false) {
			if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) veh_l = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (!PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 1)) {
				find_nearest_vehicle();
				veh_l = temp_vehicle;
			}
			VEHICLE::SET_VEHICLE_DOORS_LOCKED(veh_l, 4);
		}
		featureLockVehicleDoorsUpdated = true;
		PED::SET_PED_CAN_BE_DRAGGED_OUT(VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh_l, -1), false);
	}
	if (!featureLockVehicleDoors && featureLockVehicleDoorsUpdated == true) {
		VEHICLE::SET_VEHICLE_DOORS_LOCKED(veh_l, 0);
		PED::SET_PED_CAN_BE_DRAGGED_OUT(VEHICLE::GET_PED_IN_VEHICLE_SEAT(veh_l, -1), true);

		featureLockVehicleDoorsUpdated = false;
	}

	// 动态健康条
	if (featureDynamicHealthBar && ENTITY::DOES_ENTITY_EXIST(playerPed) && !ENTITY::IS_ENTITY_DEAD(PLAYER::PLAYER_PED_ID()) && !DLC2::GET_IS_LOADING_SCREEN_ACTIVE() && !STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS() && dynamic_loading == true && apply_pressed == false) {
		temp_h = ENTITY::GET_ENTITY_HEALTH(PLAYER::PLAYER_PED_ID()) - 100;
		temp_h_d = floor(ENTITY::GET_ENTITY_HEALTH(PLAYER::PLAYER_PED_ID()) / 100);
		oldplayerPed = playerPed;
		dynamic_loading = false;
	}
	if (featureDynamicHealthBar && !CUTSCENE::IS_CUTSCENE_PLAYING() && ENTITY::DOES_ENTITY_EXIST(playerPed) && !DLC2::GET_IS_LOADING_SCREEN_ACTIVE() && !STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS()) {
		if (!featureMiscHideHud && !featurePhoneShowHud && !featureInVehicleNoHud && !featureMarkerHud/* && !featureMiscHideENTHud*/) UI::DISPLAY_RADAR(false); // 如果 HUD 已经隐藏，就没有必要再次隐藏
		//auto addr = getScriptHandleBaseAddress(playerPed);
		//float health = (*(float *)(addr + 0x280)) - 100;
		float health = ENTITY::GET_ENTITY_HEALTH(playerPed) - 100;
		float playerArmour = PED::GET_PED_ARMOUR(playerPed);

		if (!ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ANY_OBJECT(playerPed) && !ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ANY_PED(playerPed) && !ENTITY::HAS_ENTITY_BEEN_DAMAGED_BY_ANY_VEHICLE(playerPed)) {
			//curr_damaged_health = (*(float *)(addr + 0x280)) - 100;
			float health = ENTITY::GET_ENTITY_HEALTH(playerPed) - 100;
			curr_damaged_armor = PED::GET_PED_ARMOUR(playerPed);
		}
		if (curr_damaged_health != health || curr_damaged_armor != playerArmour) {
			healthbar_seconds = -1;
			been_damaged = true;
			curr_damaged_health = health;
			curr_damaged_armor = playerArmour;
		}
		if (been_damaged == true) {
			secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / CLOCKS_PER_SEC) - healthbar_secs_curr) != 0) {
				healthbar_seconds = healthbar_seconds + 1;
				healthbar_secs_curr = secs_passed;
			}
			if (healthbar_seconds == 15) {
				been_damaged = false;
				healthbar_seconds = -1;
			}
			// 生命值
			if (health < (temp_h / 5)) {
				GRAPHICS::DRAW_RECT(health_bar_x + 0.035, health_bar_y + 0.01, 0.070, 0.017, 41, 86, 40, 110);
				GRAPHICS::DRAW_RECT(health_bar_x + 0.035, health_bar_y + 0.01, 0.070, 0.009, 41, 56, 40, 245); // 220, 20, 20, 245 // 55
				if ((health_bar_x + ((health / temp_h_d) / (temp_h / temp_h_d / 0.070))) > 0.015)
					GRAPHICS::DRAW_RECT(health_bar_x + 0.00 + ((health / temp_h_d) / (temp_h / temp_h_d / 0.035)), health_bar_y + 0.01, ((health / temp_h_d) / (temp_h / temp_h_d / 0.070)), 0.009, 220, 20, 20, 255);
			}
			else {
				GRAPHICS::DRAW_RECT(health_bar_x + 0.035, health_bar_y + 0.01, 0.070, 0.017, 41, 86, 40, 110);
				GRAPHICS::DRAW_RECT(health_bar_x + 0.035, health_bar_y + 0.01, 0.070, 0.009, 41, 56, 40, 245); // 75
				if (((health / temp_h_d) / (temp_h / temp_h_d / 0.070)) < 0.070)
					GRAPHICS::DRAW_RECT(health_bar_x + 0.00 + ((health / temp_h_d) / (temp_h / temp_h_d / 0.035)), health_bar_y + 0.01, ((health / temp_h_d) / (temp_h / temp_h_d / 0.070)), 0.009, 78, 150, 77, 255);
				else GRAPHICS::DRAW_RECT(health_bar_x + 0.035, health_bar_y + 0.01, 0.070, 0.009, 78, 150, 77, 255);
			}

			GRAPHICS::DRAW_RECT(health_bar_x + 0.071, health_bar_y + 0.01, 0.001, 0.009, 255, 170, 110, 255); // 垂直条 // 0.017

			// 装甲
			GRAPHICS::DRAW_RECT(health_bar_x + 0.0885, health_bar_y + 0.01, 0.034, 0.017, 38, 85, 87, 110); // health_bar_x + 0.0880 // 0.036
			GRAPHICS::DRAW_RECT(health_bar_x + 0.0885, health_bar_y + 0.01, 0.034, 0.009, 39, 55, 56, 245); // 90
			if ((playerArmour / 2935) < 0.035) GRAPHICS::DRAW_RECT(health_bar_x + 0.0715 + (playerArmour / 5871), health_bar_y + 0.01, (playerArmour / 2935), 0.009, 62, 129, 164, 255);
			else GRAPHICS::DRAW_RECT(health_bar_x + 0.0885, health_bar_y + 0.01, 0.034, 0.009, 62, 129, 164, 255);
		}
	}

	// 默认菜单标签
	if (MISC_DEF_MANUTAB_VALUES[DefMenuTabIndex] > -2 && PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID()) == 1 && !CUTSCENE::IS_CUTSCENE_PLAYING() && keyboard_on_screen_already == false) {
		int GetHash = GAMEPLAY::GET_HASH_KEY("FE_MENU_VERSION_SP_PAUSE");
		if (IsKeyDown(VK_ESCAPE)/*CONTROLS::IS_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_PAUSE_ALTERNATE)*/ || CONTROLS::IS_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_PAUSE)/* || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 199) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 200)*/) {
			UI::ACTIVATE_FRONTEND_MENU(GetHash, featureGamePause, MISC_DEF_MANUTAB_VALUES[DefMenuTabIndex]);
			AUDIO::SET_AUDIO_FLAG("PlayMenuMusic", true);
		} else AUDIO::SET_AUDIO_FLAG("PlayMenuMusic", false);
	} 
	
	// 没有脚本化模糊和慢动作
	if (!featurenowheelblurslow) no_blur_initialized = false;
	if (featurenowheelblurslow && (CONTROLS::IS_CONTROL_PRESSED(2, 37) || CONTROLS::IS_CONTROL_PRESSED(2, 85) || CONTROLS::IS_CONTROL_PRESSED(2, 19))) { // 武器/收音机/角色轮盘
		if (no_blur_initialized == false) {
			initialize();
			no_blur_initialized = true;
		}
		GAMEPLAY::SET_TIME_SCALE(1.0f);
		GRAPHICS::_STOP_ALL_SCREEN_EFFECTS();
	}

	// 无电话 && 如果菜单打开则禁用电话
	if (featureDisablePhone || (featureDisablePhoneMenu && menu_showing == true)) {
		if (PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed)) CONTROLS::_SET_CONTROL_NORMAL(0, 177, 1);
		GAMEPLAY::TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("cellphone_controller");
		no_phone = true;
	}
	if ((!featureDisablePhone && no_phone == true && !featureDisablePhoneMenu) || (featureDisablePhoneMenu && menu_showing == false && !featureDisablePhone && no_phone == true) || (!featureDisablePhoneMenu && no_phone == true && !featureDisablePhone)) {
		SCRIPT::REQUEST_SCRIPT("cellphone_controller");
		SYSTEM::START_NEW_SCRIPT("cellphone_controller", 1424);
		no_phone = false;
	}

	// 电话账单
	if (featurePhoneBillEnabled) {
		float mins = -1;
		if (PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed) && AUDIO::IS_MOBILE_PHONE_CALL_ONGOING()) {
			secs_passed = clock() / CLOCKS_PER_SEC;
			if (((clock() / CLOCKS_PER_SEC) - secs_curr) != 0) {
				temp_seconds = temp_seconds + 1;
				if (temp_seconds > MISC_PHONE_FREESECONDS_VALUES[PhoneFreeSecondsIndex]) bill_seconds = bill_seconds + 1;
				secs_curr = secs_passed;
			}
		}
		if (!AUDIO::IS_MOBILE_PHONE_CALL_ONGOING() && temp_seconds != 0) temp_seconds = 0;
		if (!AUDIO::IS_MOBILE_PHONE_CALL_ONGOING() && bill_seconds > 0) { 
			int outValue_your_phone_bill = -1;
			int statHash_all_your_money = -1;
			mins = bill_seconds / 60.0;
			bill_to_pay = MISC_PHONE_BILL_VALUES[PhoneBillIndex] * mins;
			
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_ZERO) {
				STATS::STAT_GET_INT(SP0_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP0_TOTAL_CASH;
				STATS::STAT_SET_INT(statHash_all_your_money, outValue_your_phone_bill - bill_to_pay, true);
			}
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_ONE) {
				STATS::STAT_GET_INT(SP1_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP1_TOTAL_CASH;
				STATS::STAT_SET_INT(statHash_all_your_money, outValue_your_phone_bill - bill_to_pay, true);
			}
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_TWO) {
				STATS::STAT_GET_INT(SP2_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP2_TOTAL_CASH;
				STATS::STAT_SET_INT(statHash_all_your_money, outValue_your_phone_bill - bill_to_pay, true);
			}
			temp_seconds = 0;
			bill_seconds = 0;
		}
		if (featureZeroBalance && !SCRIPT::HAS_SCRIPT_LOADED("prologue1")) {
			int outValue_your_phone_bill = -1;
			int statHash_all_your_money = -1;
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_ZERO) {
				STATS::STAT_GET_INT(SP0_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP0_TOTAL_CASH;
				if (outValue_your_phone_bill < 1) {
					MOBILE::DESTROY_MOBILE_PHONE();
					CONTROLS::DISABLE_CONTROL_ACTION(2, 27, 1);
					GAMEPLAY::TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("cellphone_controller");
					bill_no_phone = true;
				}
				else if (bill_no_phone == true) {
					SCRIPT::REQUEST_SCRIPT("cellphone_controller");
					SYSTEM::START_NEW_SCRIPT("cellphone_controller", 1424);
					bill_no_phone = false;
				}
			}
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_ONE) {
				STATS::STAT_GET_INT(SP1_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP1_TOTAL_CASH;
				if (outValue_your_phone_bill < 1) {
					MOBILE::DESTROY_MOBILE_PHONE();
					CONTROLS::DISABLE_CONTROL_ACTION(2, 27, 1);
					GAMEPLAY::TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("cellphone_controller");
					bill_no_phone = true;
				}
				else if (bill_no_phone == true) {
					SCRIPT::REQUEST_SCRIPT("cellphone_controller");
					SYSTEM::START_NEW_SCRIPT("cellphone_controller", 1424);
					bill_no_phone = false;
				}
			}
			if (ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()) == PLAYER_TWO) {
				STATS::STAT_GET_INT(SP2_TOTAL_CASH, &outValue_your_phone_bill, -1);
				statHash_all_your_money = SP2_TOTAL_CASH;
				if (outValue_your_phone_bill < 1) {
					MOBILE::DESTROY_MOBILE_PHONE();
					CONTROLS::DISABLE_CONTROL_ACTION(2, 27, 1);
					GAMEPLAY::TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("cellphone_controller");
					bill_no_phone = true;
				}
				else if (bill_no_phone == true) {
					SCRIPT::REQUEST_SCRIPT("cellphone_controller");
					SYSTEM::START_NEW_SCRIPT("cellphone_controller", 1424);
					bill_no_phone = false;
				}
			}
		}
		if ((!featureZeroBalance && bill_no_phone == true) || (featureZeroBalance && SCRIPT::HAS_SCRIPT_LOADED("prologue1") && bill_no_phone == true)) {
			SCRIPT::REQUEST_SCRIPT("cellphone_controller");
			SYSTEM::START_NEW_SCRIPT("cellphone_controller", 1424);
			bill_no_phone = false;
		}
	}
	
	// 第一人称特技跳跃镜头
	if (featureFirstPersonStuntJumpCamera) {
		if (GAMEPLAY::IS_STUNT_JUMP_IN_PROGRESS()) {
			Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPed, true);
			Vector3 curRotation = ENTITY::GET_ENTITY_ROTATION(PED::GET_VEHICLE_PED_IS_USING(playerPed), 2);
			if (!CAM::DOES_CAM_EXIST(StuntCam)) {
				StuntCam = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_FLY_CAMERA", playerPosition.x, playerPosition.y, playerPosition.z, curRotation.x, curRotation.y, curRotation.z, 50.0, true, 2);

				if (!PED::IS_PED_ON_ANY_BIKE(playerPed)) CAM::ATTACH_CAM_TO_PED_BONE(StuntCam, playerPed, 31086, 0, -0.15, 0.05, 1); 
				if (PED::IS_PED_ON_ANY_BIKE(playerPed)) CAM::ATTACH_CAM_TO_PED_BONE(StuntCam, playerPed, 31086, 0, -0.15, -0.10, 1); 
				CAM::_SET_CAM_DOF_MAX_NEAR_IN_FOCUS_DISTANCE_BLEND_LEVEL(StuntCam, 1.0);
				CAM::_SET_CAM_DOF_MAX_NEAR_IN_FOCUS_DISTANCE(StuntCam, 1.0);
				CAM::_SET_CAM_DOF_FOCUS_DISTANCE_BIAS(StuntCam, 1.0);
				CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, true);
				CAM::SET_CAM_ACTIVE(StuntCam, true);
				CAM::SET_CAM_NEAR_CLIP(StuntCam, .329);
			}
			CAM::SET_CAM_ROT(StuntCam, curRotation.x, curRotation.y, curRotation.z, 2);
		}

		if (!GAMEPLAY::IS_STUNT_JUMP_IN_PROGRESS() && CAM::DOES_CAM_EXIST(StuntCam)) {
			ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), 1, 1);
			CAM::RENDER_SCRIPT_CAMS(false, false, 0, false, false);
			CAM::DETACH_CAM(StuntCam);
			CAM::SET_CAM_ACTIVE(StuntCam, false);
			CAM::DESTROY_CAM(StuntCam, true);
		}
	}
		
	// 当前是否正在播放过场动画？
	if (cutscene_is_playing == true) {
		con_disabled = true;
	}
	else { 
		if (con_disabled == true && manual_cutscene == true && GAMEPLAY::GET_MISSION_FLAG() == 0) {
			con_disabled = false;
			manual_cutscene = false;
		}
		OBJECT::DELETE_OBJECT(&xaxis);
		OBJECT::DELETE_OBJECT(&zaxis);
		if (CAM::DOES_CAM_EXIST(CutCam)) {
			CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
			CAM::DESTROY_CAM(CutCam, true);
		}
		curr_cut_ped_me = -1;
		my_first_coords = -1;
		curr_cut_ped = -1;
		cutscene_being_watched = false;
		found_ped_in_cutscene = false;
		switched_c = -1;
	}
	if (cutscene_is_playing == true && CUTSCENE::IS_CUTSCENE_PLAYING()) cutscene_being_watched = true;
	if (cutscene_being_watched == true && (!CUTSCENE::IS_CUTSCENE_PLAYING() || ((CUTSCENE::GET_CUTSCENE_TOTAL_DURATION() - CUTSCENE::GET_CUTSCENE_TIME() < 3000) && CAM::IS_SCREEN_FADING_OUT() && manual_cutscene == true))) { // && CUTSCENE::HAS_CUTSCENE_FINISHED()
		if (manual_cutscene == true) {
			CAM::DO_SCREEN_FADE_IN(0);
			CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
			CUTSCENE::REMOVE_CUTSCENE();
			CAM::DO_SCREEN_FADE_IN(0);
		}
		cutscene_is_playing = false;
		cutscene_being_watched = false;
	}
	if (CUTSCENE::IS_CUTSCENE_PLAYING()) cutscene_is_playing = true;
	if (CUTSCENE::IS_CUTSCENE_PLAYING() && manual_cutscene == true && CONTROLS::IS_CONTROL_JUST_PRESSED(2, 22)) stop_cutscene();

	// 第一人称过场动画镜头
	if (featureFirstPersonCutscene) {
		if (CUTSCENE::IS_CUTSCENE_PLAYING()) {
			Vector3 Pedrotation = ENTITY::GET_ENTITY_ROTATION(curr_cut_ped, 2);
			int PlayerIndex = PED::GET_PED_BONE_INDEX(curr_cut_ped, 8433);
			int PedHash = GAMEPLAY::GET_HASH_KEY("bot_01b_bit_03"); // prop_wardrobe_door_01
			Vector3 Ped1Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped, 0.0f, 1.0f, 0.0f);
			Vector3 Ped2Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped, 0.0f, 2.0f, 0.0f);
			
			if (!CAM::DOES_CAM_EXIST(CutCam)) { 
				const int US_ARR_PED_SIZE = 1024;
				Ped us_ped[US_ARR_PED_SIZE];
				int found_ped = worldGetAllPeds(us_ped, US_ARR_PED_SIZE);
				for (int i = 0; i < found_ped; i++) {
					if (ENTITY::IS_ENTITY_ON_SCREEN(us_ped[i]) && (ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_zero") ||
						ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_one") || ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_two") ||
						ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"mp_f_freemode_01") || ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"mp_m_freemode_01")) && found_ped_in_cutscene == false &&
						ENTITY::IS_ENTITY_VISIBLE(us_ped[i]) && switched_c != us_ped[i] && PED::GET_PED_TYPE(us_ped[i]) != 28) {
						curr_cut_ped_me = us_ped[i];
						my_first_coords = us_ped[i];
						PlayerIndex = PED::GET_PED_BONE_INDEX(curr_cut_ped, 8433);
						Ped1Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped_me, 0.0f, 1.0f, 0.0f);
						Ped2Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped_me, 0.0f, 2.0f, 0.0f);
						xaxis = OBJECT::CREATE_OBJECT(PedHash, Ped1Coords.x, Ped1Coords.y, Ped1Coords.z, 1, true, 1);
						zaxis = OBJECT::CREATE_OBJECT(PedHash, Ped2Coords.x, Ped2Coords.y, Ped2Coords.z, 1, true, 1);
						ENTITY::SET_ENTITY_VISIBLE(xaxis, false);
						ENTITY::SET_ENTITY_VISIBLE(zaxis, false);
						ENTITY::SET_ENTITY_COLLISION(xaxis, false, true);
						ENTITY::SET_ENTITY_COLLISION(zaxis, false, true);
						ENTITY::ATTACH_ENTITY_TO_ENTITY(xaxis, curr_cut_ped_me, PlayerIndex, 0.0f, 0.0f, -0.1f, 105.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
						ENTITY::ATTACH_ENTITY_TO_ENTITY(zaxis, curr_cut_ped_me, PlayerIndex, 0.0f, 0.08f, -0.1f, 50.0f, 0.0f, 0.0f, false, false, false, true, 0, true);

						Vector3 coordsPed = ENTITY::GET_ENTITY_COORDS(curr_cut_ped_me, true);
						CutCam = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_FLY_CAMERA", coordsPed.x, coordsPed.y, coordsPed.z, Pedrotation.x, Pedrotation.y, Pedrotation.z, 50.0, true, 2);
						CAM::ATTACH_CAM_TO_ENTITY(CutCam, zaxis, 0, 0, 0, true);
						CAM::SET_CAM_NEAR_CLIP(CutCam, .229); // 329
					}
				}
			}
			if (CAM::DOES_CAM_EXIST(CutCam)) {
				if (cutscene_being_watched == true && found_ped_in_cutscene == false) {
					const int US_ARR_PED_SIZE = 1024;
					Ped us_ped[US_ARR_PED_SIZE];
					int found_ped = worldGetAllPeds(us_ped, US_ARR_PED_SIZE);
					for (int i = 0; i < found_ped; i++) {
						Vector3 coordsme = ENTITY::GET_ENTITY_COORDS(my_first_coords, true);
						Vector3 coordsPed_temp = ENTITY::GET_ENTITY_COORDS(us_ped[i], true);
						float dist_t = SYSTEM::VDIST(coordsme.x, coordsme.y, coordsme.z, coordsPed_temp.x, coordsPed_temp.y, coordsPed_temp.z);
						if (dist_t < 10) { // 20 
							if (ENTITY::IS_ENTITY_ON_SCREEN(us_ped[i]) && found_ped_in_cutscene == false &&
								ENTITY::IS_ENTITY_VISIBLE(us_ped[i]) && switched_c != us_ped[i] && PED::GET_PED_TYPE(us_ped[i]) != 28) { 
								if (curr_cut_ped_me != -1) curr_cut_ped = curr_cut_ped_me;
								else curr_cut_ped = us_ped[i];
								OBJECT::DELETE_OBJECT(&xaxis);
								OBJECT::DELETE_OBJECT(&zaxis);

								CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
								CAM::DESTROY_CAM(CutCam, true);
								
								if (ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_zero") || ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_one") ||
									ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_two")) PlayerIndex = PED::GET_PED_BONE_INDEX(curr_cut_ped, 8433);
								else PlayerIndex = PED::GET_PED_BONE_INDEX(curr_cut_ped, 31086); // 8433
								Ped1Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped, 0.0f, 1.0f, 0.0f);
								Ped2Coords = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(curr_cut_ped, 0.0f, 2.0f, 0.0f);
								xaxis = OBJECT::CREATE_OBJECT(PedHash, Ped1Coords.x, Ped1Coords.y, Ped1Coords.z, 1, true, 1);
								zaxis = OBJECT::CREATE_OBJECT(PedHash, Ped2Coords.x, Ped2Coords.y, Ped2Coords.z, 1, true, 1);
								ENTITY::SET_ENTITY_VISIBLE(xaxis, false);
								ENTITY::SET_ENTITY_VISIBLE(zaxis, false);
								ENTITY::SET_ENTITY_COLLISION(xaxis, false, true);
								ENTITY::SET_ENTITY_COLLISION(zaxis, false, true);
								if (ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_zero") || ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_one") ||
									ENTITY::GET_ENTITY_MODEL(us_ped[i]) == GAMEPLAY::GET_HASH_KEY((char *)"player_two")) {
									ENTITY::ATTACH_ENTITY_TO_ENTITY(xaxis, curr_cut_ped, PlayerIndex, 0.0f, 0.0f, -0.1f, 105.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
									ENTITY::ATTACH_ENTITY_TO_ENTITY(zaxis, curr_cut_ped, PlayerIndex, 0.0f, 0.08f, -0.1f, 50.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
								}
								else {
									ENTITY::ATTACH_ENTITY_TO_ENTITY(xaxis, curr_cut_ped, PlayerIndex, 0.0f, 0.0f, -0.1f, 105.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
									ENTITY::ATTACH_ENTITY_TO_ENTITY(zaxis, curr_cut_ped, PlayerIndex, 0.0f, 0.08f, -0.1f, 0.0f, 0.0f, 0.0f, false, false, false, true, 0, true);
								}

								Vector3 coordsPed = ENTITY::GET_ENTITY_COORDS(curr_cut_ped, true);
								CutCam = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_FLY_CAMERA", coordsPed.x, coordsPed.y, coordsPed.z, Pedrotation.x, Pedrotation.y, Pedrotation.z, 50.0, true, 2);
								CAM::ATTACH_CAM_TO_ENTITY(CutCam, zaxis, 0, 0, 0, true);
								CAM::SET_CAM_NEAR_CLIP(CutCam, .229); // 329
								curr_cut_ped_me = -1;
								switched_c = curr_cut_ped;
								found_ped_in_cutscene = true;
							}
						}
					}
				} 
				if (!ENTITY::DOES_ENTITY_EXIST(curr_cut_ped)) {
					OBJECT::DELETE_OBJECT(&xaxis);
					OBJECT::DELETE_OBJECT(&zaxis);
					if (CAM::DOES_CAM_EXIST(CutCam)) {
						CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
						CAM::DESTROY_CAM(CutCam, true);
					}
					found_ped_in_cutscene = false;
					switched_c = -1;
				}
				if (CAM::DOES_CAM_EXIST(CutCam)) {
					CAM::_SET_CAM_DOF_MAX_NEAR_IN_FOCUS_DISTANCE_BLEND_LEVEL(CutCam, 1.0);
					CAM::_SET_CAM_DOF_MAX_NEAR_IN_FOCUS_DISTANCE(CutCam, 1.0);
					CAM::_SET_CAM_DOF_FOCUS_DISTANCE_BIAS(CutCam, 1.0);
					CAM::RENDER_SCRIPT_CAMS(true, false, 1, false, false);

					CAM::STOP_CUTSCENE_CAM_SHAKING();
					CUTSCENE::CAN_SET_EXIT_STATE_FOR_CAMERA(1);
					Vector3 Ped1rotation = ENTITY::GET_ENTITY_ROTATION(xaxis, 2);
					Vector3 Ped2rotation = ENTITY::GET_ENTITY_ROTATION(zaxis, 2);
					CAM::SET_CAM_ROT(CutCam, Ped1rotation.x, Pedrotation.y, Ped2rotation.z, 2);
				}
			}
		} // if 语句的结束 (CUTSCENE::IS_CUTSCENE_PLAYING())
		else if (cutscene_is_playing == false) {
			if (con_disabled == true && manual_cutscene == true) {
				con_disabled = false;
				manual_cutscene = false;
			}
			OBJECT::DELETE_OBJECT(&xaxis);
			OBJECT::DELETE_OBJECT(&zaxis);
			if (CAM::DOES_CAM_EXIST(CutCam)) {
				CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
				CAM::DESTROY_CAM(CutCam, true);
			}
			found_ped_in_cutscene = false;
			switched_c = -1;
		}
	} // if 语句的结束 (featureFirstPersonCutscene)
	else {
		OBJECT::DELETE_OBJECT(&xaxis);
		OBJECT::DELETE_OBJECT(&zaxis);
		if (CAM::DOES_CAM_EXIST(CutCam)) {
			CAM::RENDER_SCRIPT_CAMS(false, false, 1, false, false);
			CAM::DESTROY_CAM(CutCam, true);
		}
		found_ped_in_cutscene = false;
		switched_c = -1;
	}

	// 无特技跳跃
	if (featureNoStuntJumps && GAMEPLAY::IS_STUNT_JUMP_IN_PROGRESS()) GAMEPLAY::CANCEL_STUNT_JUMP();

	// FPS 计数器
	// 修改FPS显示逻辑：当菜单左侧偏移量>=100时不隐藏FPS，但自由移动和物体摆放模式时仍隐藏
	bool shouldShowFPS = featureShowFPS && 
		(menu_showing == false || 
		 (menu_showing == true && menuLeftOffset >= 100.0f)) &&
		!(is_in_airbrake_mode() || is_in_prop_placement_mode());
	
	if (shouldShowFPS) {
		FPStime_passed = clock() / CLOCKS_PER_SEC;
		if (((clock() / CLOCKS_PER_SEC) - FPStime_curr) != 0) {
			FPStime = FPStime + 1;
			FPStime_curr = FPStime_passed;
		}

		frames++;
		
		if (FPStime - starttime > 0.05 && frames > 10) { // 0.25
			fps = (double)frames / (FPStime - starttime);
			if ((FPStime - starttime) >= 0) starttime = FPStime;
			frames = 0;
		}
			
		sprintf(fps_to_show_char_modifiable, "帧率 : %d", fps); // 将 fps 值格式化为字符串并存储到 fps_to_show_char_modifiable 中  
		UI::SET_TEXT_FONT(fontStatus); // 设置 FPS 字体类型 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45  
		UI::SET_TEXT_PROPORTIONAL(1); // 启用文本的比例缩放，保持文本比例不变  
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB: 255, 242, 0），透明度为255（不透明）  
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255); // 设置文本边缘效果，类型为 3，颜色为黑色（RGB: 0, 0, 0），透明度为255  
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255); // 设置文本阴影效果，偏移量为 10，颜色为黑色（RGB: 10, 10, 10），透明度为255  
		UI::SET_TEXT_OUTLINE(); // 启用文本描边效果  
		UI::_SET_TEXT_ENTRY("STRING"); // 设置文本条目类型为字符串  
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(fps_to_show_char_modifiable); // 将 fps_to_show_char_modifiable 中的文本添加到文本组件中  
		UI::_DRAW_TEXT(0.005, 0.135); // 在屏幕坐标 ( x=0.005, y=0.135 ) 处绘制文本  
		GRAPHICS::DRAW_RECT(0.035, 0.15, 0.08, 0.03, 10, 10, 10, 180); 
		// 在屏幕坐标 ( x=0.0, y=0.15 ) 处绘制一个矩形，宽度为 0.05，高度为 0.03，颜色为深灰色（RGB: 10, 10, 10），透明度为 150  
	}
	
	// 在暂停菜单中隐藏玩家信息
	if (featureHidePlayerInfo) UI::_SET_DIRECTOR_MODE(true);
	else UI::_SET_DIRECTOR_MODE(false);
		
	// 启用1.44版本的新电台。感谢Sjaak发现了这一点！
	if (featureEnableMissingRadioStation)
	{
		if (!iterated_radio_stations) missing_station = missing_station + 1;
		if (missing_station > 300) { // 1000
			int version = getGameVersion();
			if ((version > 41 || version == -1))
			{
				//if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, true) && !iterated_radio_stations)
				if (!iterated_radio_stations)
				{
					for (int i = 0; i < 100; i++)
					{
						char* radio_station = AUDIO::GET_RADIO_STATION_NAME(i);
						// 跳过空白占位电台 Weazel News，不将其解锁显示
						if (radio_station && std::string(radio_station) == "HIDDEN_RADIO_MPSUM2_NEWS") {
							continue;
						}
						UNK3::_LOCK_RADIO_STATION(radio_station, 0);
					}
					WAIT(1000);
					iterated_radio_stations = true;
				}
			}
			else
			{
				set_status_text("您的游戏版本过旧, 此功能\n需1.44或更高版本才能运行!");
				featureEnableMissingRadioStation = false;
			}
		}
	}
	if (!featureEnableMissingRadioStation)
	{
		iterated_radio_stations = false;
		missing_station = 0;
	}

	// 仅在首次运行时锁定空白占位电台，避免每帧重复调用
	static bool weazelLockedOnce = false;
	if (!weazelLockedOnce) {
		UNK3::_LOCK_RADIO_STATION(const_cast<char*>("HIDDEN_RADIO_MPSUM2_NEWS"), 1);
		weazelLockedOnce = true;
	}

	if (sfilter_enabled == false && screenfltr != "DEFAULT" && screenfltr != "") {
		GRAPHICS::SET_TIMECYCLE_MODIFIER((char*)screenfltr.c_str());
		GRAPHICS::SET_TIMECYCLE_MODIFIER_STRENGTH(1.0f);
		sfilter_enabled = true;
	}
	if (DLC2::GET_IS_LOADING_SCREEN_ACTIVE()) sfilter_enabled = false;

	// 更新自由相机功能
	update_freecam_features(playerExists, playerPed);

	// 游戏截图消息更新（需要在主循环中每帧调用）
	update_screenshot_message();
	
	// 游戏截图功能
	if (featureScreenshotEnabled) {
		// 检测截图按键
		int screenshotKey = MISC_SCREENSHOT_KEY_VALUES[ScreenshotKeyIndex];
		bool shouldTakeScreenshot = false;
		
		// 检测所有支持的截图按键
		if (screenshotKey != VK_NOTHING && IsKeyJustUp(KeyConfig::KEY_SCREENSHOT)) {
			shouldTakeScreenshot = true;
		}
		
		if (shouldTakeScreenshot) {
			take_screenshot();
		}
	}
}

void add_misc_feature_enablements(std::vector<FeatureEnabledLocalDefinition>* results){
	results->push_back(FeatureEnabledLocalDefinition{"featurePlayerRadio", &featurePlayerRadio, &featurePlayerRadioUpdated });
	results->push_back(FeatureEnabledLocalDefinition{"featureRadioFreeze", &featureRadioFreeze, &featureRadioFreezeUpdated });
	results->push_back(FeatureEnabledLocalDefinition{"featureBoostRadio", &featureBoostRadio }); 
	results->push_back(FeatureEnabledLocalDefinition{"featureRealisticRadioVolume", &featureRealisticRadioVolume});
	results->push_back(FeatureEnabledLocalDefinition{"featureWantedMUsic", &featureWantedMusic}); 
	results->push_back(FeatureEnabledLocalDefinition{"featureDisablePhone", &featureDisablePhone});
	results->push_back(FeatureEnabledLocalDefinition{"featureDisablePhoneMenu", &featureDisablePhoneMenu});
	results->push_back(FeatureEnabledLocalDefinition{"featureFlyingMusic", &featureFlyingMusic}); 
	results->push_back(FeatureEnabledLocalDefinition{"featurePoliceScanner", &featurePoliceScanner}); 
	results->push_back(FeatureEnabledLocalDefinition{"featureNoComleteMessage", &featureNoComleteMessage}); 
	results->push_back(FeatureEnabledLocalDefinition{"featurePoliceRadio", &featurePoliceRadio}); 
	results->push_back(FeatureEnabledLocalDefinition{"featureMiscLockRadio", &featureMiscLockRadio});
	results->push_back(FeatureEnabledLocalDefinition{"featureMiscHideHud", &featureMiscHideHud, &featureMiscHideHudUpdated});
	results->push_back(FeatureEnabledLocalDefinition{"featurePhoneShowHud", &featurePhoneShowHud}); 
	results->push_back(FeatureEnabledLocalDefinition{"featureInVehicleNoHud", &featureInVehicleNoHud});
	results->push_back(FeatureEnabledLocalDefinition{"featureMarkerHud", &featureMarkerHud});
	results->push_back(FeatureEnabledLocalDefinition{"featureDynamicHealthBar", &featureDynamicHealthBar});
	results->push_back(FeatureEnabledLocalDefinition{"featureDisableRecording", &featureDisableRecording});
	results->push_back(FeatureEnabledLocalDefinition{"featureNoNotifications", &featureNoNotifications});
	results->push_back(FeatureEnabledLocalDefinition{"mouse_view_control", &mouse_view_control});
	results->push_back(FeatureEnabledLocalDefinition{"airbrake_enable", &airbrake_enable});
	results->push_back(FeatureEnabledLocalDefinition{"featureFirstPersonCutscene", &featureFirstPersonCutscene});
	results->push_back(FeatureEnabledLocalDefinition{"help_showing", &help_showing});
	results->push_back(FeatureEnabledLocalDefinition{"frozen_time", &frozen_time});
	results->push_back(FeatureEnabledLocalDefinition{"show_transparency", &show_transparency});
	results->push_back(FeatureEnabledLocalDefinition{"featurePhoneBillEnabled", &featurePhoneBillEnabled});
	results->push_back(FeatureEnabledLocalDefinition{"featureNoGamePause", &featureGamePause});
	results->push_back(FeatureEnabledLocalDefinition{"featureZeroBalance", &featureZeroBalance});
	results->push_back(FeatureEnabledLocalDefinition{"featurePhone3DOnBike", &featurePhone3DOnBike});
	results->push_back(FeatureEnabledLocalDefinition{"featureNoPhoneOnHUD", &featureNoPhoneOnHUD});
	results->push_back(FeatureEnabledLocalDefinition{"featureShowVehiclePreviews", &featureShowVehiclePreviews});
	results->push_back(FeatureEnabledLocalDefinition{"featureShowStatusMessage", &featureShowStatusMessage});
	results->push_back(FeatureEnabledLocalDefinition{"featureNoAutoRespawn", &featureNoAutoRespawn});
	results->push_back(FeatureEnabledLocalDefinition{"featureShowFPS", &featureShowFPS});
	results->push_back(FeatureEnabledLocalDefinition{"featureShowModelName", &featureShowModelName});// 模型名称显示，保存和加载功能状态的机制
	results->push_back(FeatureEnabledLocalDefinition{"featurenowheelblurslow", &featurenowheelblurslow});
	results->push_back(FeatureEnabledLocalDefinition{"featureHiddenRadioStation", &featureEnableMissingRadioStation});
	results->push_back(FeatureEnabledLocalDefinition{"featureFirstPersonDeathCamera", &featureFirstPersonDeathCamera});
	results->push_back(FeatureEnabledLocalDefinition{"featureFirstPersonStuntJumpCamera", &featureFirstPersonStuntJumpCamera});
	results->push_back(FeatureEnabledLocalDefinition{"featureNoStuntJumps", &featureNoStuntJumps});
	results->push_back(FeatureEnabledLocalDefinition{"featureHidePlayerInfo", &featureHidePlayerInfo});
	results->push_back(FeatureEnabledLocalDefinition{"featureMiscJellmanScenery", &featureMiscJellmanScenery});
	results->push_back(FeatureEnabledLocalDefinition{"featureFreeCamEnabled", &featureFreeCamEnabled});
	// 截图选项加载和保存
	results->push_back(FeatureEnabledLocalDefinition{"featureScreenshotEnabled", &featureScreenshotEnabled});  // 从screenshot.cpp引用
	//results->push_back(FeatureEnabledLocalDefinition{"featureControllerIgnoreInTrainer", &featureControllerIgnoreInTrainer});
	//results->push_back(FeatureEnabledLocalDefinition{"featureBlockInputInMenu", &featureBlockInputInMenu});
}

void add_misc_generic_settings(std::vector<StringPairSettingDBRow>* results){
	results->push_back(StringPairSettingDBRow{"radioStationIndex", std::to_string(radioStationIndex)});
	results->push_back(StringPairSettingDBRow{"PhoneBillIndex", std::to_string(PhoneBillIndex)});
	results->push_back(StringPairSettingDBRow{"PhoneDefaultIndex", std::to_string(PhoneDefaultIndex)});
	results->push_back(StringPairSettingDBRow{"RadioOffIndex", std::to_string(RadioOffIndex)});
	results->push_back(StringPairSettingDBRow{"RadioSwitchingIndex", std::to_string(RadioSwitchingIndex)});
	results->push_back(StringPairSettingDBRow{"TrainerControlIndex", std::to_string(TrainerControlIndex)});
	results->push_back(StringPairSettingDBRow{"TrainerControlScrollingIndex", std::to_string(TrainerControlScrollingIndex)});
	results->push_back(StringPairSettingDBRow{"PhoneFreeSecondsIndex", std::to_string(PhoneFreeSecondsIndex)});
	results->push_back(StringPairSettingDBRow{"PhoneBikeAnimationIndex", std::to_string(PhoneBikeAnimationIndex)});
	results->push_back(StringPairSettingDBRow{"DefMenuTabIndex", std::to_string(DefMenuTabIndex)});
	results->push_back(StringPairSettingDBRow{"FontHeaderIndex", std::to_string(FontHeaderIndex)});
	results->push_back(StringPairSettingDBRow{"FontItemIndex", std::to_string(FontItemIndex)});
	results->push_back(StringPairSettingDBRow{"FontWantedIndex", std::to_string(FontWantedIndex)});
	results->push_back(StringPairSettingDBRow{"FontStatusIndex", std::to_string(FontStatusIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemsCountIndex", std::to_string(MenuItemsCountIndex)});
	// 添加菜单布局设置
	results->push_back(StringPairSettingDBRow{"MenuWidthIndex", std::to_string(MenuWidthIndex)});
	results->push_back(StringPairSettingDBRow{"MenuHeightIndex", std::to_string(MenuHeightIndex)});
	results->push_back(StringPairSettingDBRow{"MenuTopOffsetIndex", std::to_string(MenuTopOffsetIndex)});
	results->push_back(StringPairSettingDBRow{"MenuLeftOffsetIndex", std::to_string(MenuLeftOffsetIndex)});
	results->push_back(StringPairSettingDBRow{"MenuTextLeftOffsetIndex", std::to_string(MenuTextLeftOffsetIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemHeightIndex", std::to_string(MenuItemHeightIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemSpacingIndex", std::to_string(MenuItemSpacingIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemTextOffsetIndex", std::to_string(MenuItemTextOffsetIndex)});
	results->push_back(StringPairSettingDBRow{"MenuToggleIconRightMarginIndex", std::to_string(MenuToggleIconRightMarginIndex)});
	results->push_back(StringPairSettingDBRow{"MenuWantedStarRightMarginIndex", std::to_string(MenuWantedStarRightMarginIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemTopOffsetIndex", std::to_string(MenuItemTopOffsetIndex)});
	results->push_back(StringPairSettingDBRow{"MenuMainHeaderFontScaleIndex", std::to_string(MenuMainHeaderFontScaleIndex)});
	results->push_back(StringPairSettingDBRow{"MenuHeaderFontScaleIndex", std::to_string(MenuHeaderFontScaleIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemFontScaleIndex", std::to_string(MenuItemFontScaleIndex)});
	results->push_back(StringPairSettingDBRow{"MenuWantedFontScaleIndex", std::to_string(MenuWantedFontScaleIndex)});
	results->push_back(StringPairSettingDBRow{"MenuItemHighlightTextScaleIndex", std::to_string(MenuItemHighlightTextScaleIndex)});
	// 添加预览图设置
	results->push_back(StringPairSettingDBRow{"PreviewPositionThresholdIndex", std::to_string(PreviewPositionThresholdIndex)});
	results->push_back(StringPairSettingDBRow{"PreviewResolutionScaleIndex", std::to_string(PreviewResolutionScaleIndex)});
	results->push_back(StringPairSettingDBRow{"PreviewSpacingIndex", std::to_string(PreviewSpacingIndex)});
	// 添加人物预览图设置
	results->push_back(StringPairSettingDBRow{"PedPreviewPositionThresholdIndex", std::to_string(PedPreviewPositionThresholdIndex)});
	results->push_back(StringPairSettingDBRow{"PedPreviewResolutionScaleIndex", std::to_string(PedPreviewResolutionScaleIndex)});
	results->push_back(StringPairSettingDBRow{"PedPreviewSpacingIndex", std::to_string(PedPreviewSpacingIndex)});
	results->push_back(StringPairSettingDBRow{"featureShowPedPreviews", featureShowPedPreviews ? "1" : "0"});
	results->push_back(StringPairSettingDBRow{"screenfltr", screenfltr});
	// 添加自由相机设置（热键在其他按键设置中保存）
	results->push_back(StringPairSettingDBRow{"FreeCamFollowIndex", std::to_string(FreeCamFollowIndex)});
	results->push_back(StringPairSettingDBRow{"FreeCamFovIndex", std::to_string(FreeCamFovIndex)});
	results->push_back(StringPairSettingDBRow{"FreeCamSpeedSlowIndex", std::to_string(FreeCamSpeedSlowIndex)});
	results->push_back(StringPairSettingDBRow{"FreeCamSpeedMediumIndex", std::to_string(FreeCamSpeedMediumIndex)});
	results->push_back(StringPairSettingDBRow{"FreeCamSpeedFastIndex", std::to_string(FreeCamSpeedFastIndex)});
	results->push_back(StringPairSettingDBRow{"FreeCamInfoDisplayIndex", std::to_string(FreeCamInfoDisplayIndex)});
    results->push_back(StringPairSettingDBRow{"FreeCamCrosshairStyleIndex", std::to_string(FreeCamCrosshairStyleIndex)});
    results->push_back(StringPairSettingDBRow{"FreeCamCrosshairColorIndex", std::to_string(FreeCamCrosshairColorIndex)});
	// 添加截图设置
	results->push_back(StringPairSettingDBRow{"ScreenshotKeyIndex", std::to_string(ScreenshotKeyIndex)});  // 从screenshot.cpp引用
}

void handle_generic_settings_misc(std::vector<StringPairSettingDBRow>* settings){
	for(int a = 0; a < settings->size(); a++){
		StringPairSettingDBRow setting = settings->at(a);
		if(setting.name.compare("radioStationIndex") == 0){
			radioStationIndex = stoi(setting.value);
		}
		else if (setting.name.compare("PhoneBillIndex") == 0){
			PhoneBillIndex = stoi(setting.value);
		}
		else if (setting.name.compare("PhoneDefaultIndex") == 0) {
			PhoneDefaultIndex = stoi(setting.value);
		}
		else if (setting.name.compare("RadioOffIndex") == 0) {
			RadioOffIndex = stoi(setting.value);
		}
		else if (setting.name.compare("RadioSwitchingIndex") == 0) {
			RadioSwitchingIndex = stoi(setting.value);
		}
		else if (setting.name.compare("TrainerControlIndex") == 0) {
			TrainerControlIndex = stoi(setting.value);
		}
		else if (setting.name.compare("TrainerControlScrollingIndex") == 0) {
			TrainerControlScrollingIndex = stoi(setting.value);
		}
		else if (setting.name.compare("PhoneFreeSecondsIndex") == 0){
			PhoneFreeSecondsIndex = stoi(setting.value);
		}
		else if (setting.name.compare("PhoneBikeAnimationIndex") == 0) {
			PhoneBikeAnimationIndex = stoi(setting.value);
		}
		else if (setting.name.compare("DefMenuTabIndex") == 0) {
			DefMenuTabIndex = stoi(setting.value);
		}
		else if (setting.name.compare("FontHeaderIndex") == 0) {
            FontHeaderIndex = stoi(setting.value);
            fontHeader = MISC_FONT_HEADER_VALUES[FontHeaderIndex];
        }
		else if (setting.name.compare("FontItemIndex") == 0) {
            FontItemIndex = stoi(setting.value);
            fontItem = MISC_FONT_ITEM_VALUES[FontItemIndex];
        }
		else if (setting.name.compare("FontWantedIndex") == 0) {
            FontWantedIndex = stoi(setting.value);
            fontWanted = MISC_FONT_WANTED_VALUES[FontWantedIndex];
        }
		else if (setting.name.compare("FontStatusIndex") == 0) {
            FontStatusIndex = stoi(setting.value);
            fontStatus = MISC_FONT_STATUS_VALUES[FontStatusIndex];
        }
		else if (setting.name.compare("MenuItemsCountIndex") == 0) {
            MenuItemsCountIndex = stoi(setting.value);
            itemsPerLine = MISC_MENU_ITEMS_COUNT_VALUES[MenuItemsCountIndex];
            MenuItemsCountChanged = true;
        }
		// 添加菜单布局设置的加载
		else if (setting.name.compare("MenuWidthIndex") == 0) {
			MenuWidthIndex = stoi(setting.value);
			if (MenuWidthIndex < 0) MenuWidthIndex = 0;
			if (MenuWidthIndex >= (int)MISC_MENU_WIDTH_CAPTIONS.size()) MenuWidthIndex = (int)MISC_MENU_WIDTH_CAPTIONS.size() - 1;
			menuWidth = MISC_MENU_WIDTH_VALUES[MenuWidthIndex];
			MenuWidthChanged = true;
		}
		else if (setting.name.compare("MenuHeightIndex") == 0) {
			MenuHeightIndex = stoi(setting.value);
			if (MenuHeightIndex < 0) MenuHeightIndex = 0;
			if (MenuHeightIndex >= (int)MISC_MENU_HEIGHT_CAPTIONS.size()) MenuHeightIndex = (int)MISC_MENU_HEIGHT_CAPTIONS.size() - 1;
			menuHeight = MISC_MENU_HEIGHT_VALUES[MenuHeightIndex];
			MenuHeightChanged = true;
		}
		else if (setting.name.compare("MenuTopOffsetIndex") == 0) {
			MenuTopOffsetIndex = stoi(setting.value);
			if (MenuTopOffsetIndex < 0) MenuTopOffsetIndex = 0;
			if (MenuTopOffsetIndex >= (int)MISC_MENU_TOP_OFFSET_CAPTIONS.size()) MenuTopOffsetIndex = (int)MISC_MENU_TOP_OFFSET_CAPTIONS.size() - 1;
			menuTopOffset = MISC_MENU_TOP_OFFSET_VALUES[MenuTopOffsetIndex];
			MenuTopOffsetChanged = true;
		}
		else if (setting.name.compare("MenuLeftOffsetIndex") == 0) {
			MenuLeftOffsetIndex = stoi(setting.value);
			if (MenuLeftOffsetIndex < 0) MenuLeftOffsetIndex = 0;
			if (MenuLeftOffsetIndex >= (int)MISC_MENU_LEFT_OFFSET_CAPTIONS.size()) MenuLeftOffsetIndex = (int)MISC_MENU_LEFT_OFFSET_CAPTIONS.size() - 1;
			menuLeftOffset = MISC_MENU_LEFT_OFFSET_VALUES[MenuLeftOffsetIndex];
			MenuLeftOffsetChanged = true;
		}
		else if (setting.name.compare("MenuTextLeftOffsetIndex") == 0) {
			MenuTextLeftOffsetIndex = stoi(setting.value);
			if (MenuTextLeftOffsetIndex < 0) MenuTextLeftOffsetIndex = 0;
			if (MenuTextLeftOffsetIndex >= (int)MISC_MENU_TEXT_LEFT_OFFSET_CAPTIONS.size()) MenuTextLeftOffsetIndex = (int)MISC_MENU_TEXT_LEFT_OFFSET_CAPTIONS.size() - 1;
			menuTextLeftOffset = MISC_MENU_TEXT_LEFT_OFFSET_VALUES[MenuTextLeftOffsetIndex];
			MenuTextLeftOffsetChanged = true;
		}
		else if (setting.name.compare("MenuItemHeightIndex") == 0) {
			MenuItemHeightIndex = stoi(setting.value);
			if (MenuItemHeightIndex < 0) MenuItemHeightIndex = 0;
			if (MenuItemHeightIndex >= (int)MISC_MENU_ITEM_HEIGHT_CAPTIONS.size()) MenuItemHeightIndex = (int)MISC_MENU_ITEM_HEIGHT_CAPTIONS.size() - 1;
			menuItemHeight = MISC_MENU_ITEM_HEIGHT_VALUES[MenuItemHeightIndex];
			MenuItemHeightChanged = true;
		}
		else if (setting.name.compare("MenuItemSpacingIndex") == 0) {
			MenuItemSpacingIndex = stoi(setting.value);
			if (MenuItemSpacingIndex < 0) MenuItemSpacingIndex = 0;
			if (MenuItemSpacingIndex >= (int)MISC_MENU_ITEM_SPACING_CAPTIONS.size()) MenuItemSpacingIndex = (int)MISC_MENU_ITEM_SPACING_CAPTIONS.size() - 1;
			menuItemSpacing = MISC_MENU_ITEM_SPACING_VALUES[MenuItemSpacingIndex];
			MenuItemSpacingChanged = true;
		}
		else if (setting.name.compare("MenuItemTextOffsetIndex") == 0) {
			MenuItemTextOffsetIndex = stoi(setting.value);
			if (MenuItemTextOffsetIndex < 0) MenuItemTextOffsetIndex = 0;
			if (MenuItemTextOffsetIndex >= (int)MISC_MENU_ITEM_TEXT_OFFSET_CAPTIONS.size()) MenuItemTextOffsetIndex = (int)MISC_MENU_ITEM_TEXT_OFFSET_CAPTIONS.size() - 1;
			menuItemTextOffset = MISC_MENU_ITEM_TEXT_OFFSET_VALUES[MenuItemTextOffsetIndex];
			MenuItemTextOffsetChanged = true;
		}
		else if (setting.name.compare("MenuToggleIconRightMarginIndex") == 0) {
			MenuToggleIconRightMarginIndex = stoi(setting.value);
			if (MenuToggleIconRightMarginIndex < 0) MenuToggleIconRightMarginIndex = 0;
			if (MenuToggleIconRightMarginIndex >= (int)MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_CAPTIONS.size()) MenuToggleIconRightMarginIndex = (int)MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_CAPTIONS.size() - 1;
			menuItemToggleIconRightMargin = MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_VALUES[MenuToggleIconRightMarginIndex];
			MenuToggleIconRightMarginChanged = true;
		}
		else if (setting.name.compare("MenuWantedStarRightMarginIndex") == 0) {
			MenuWantedStarRightMarginIndex = stoi(setting.value);
			if (MenuWantedStarRightMarginIndex < 0) MenuWantedStarRightMarginIndex = 0;
			if (MenuWantedStarRightMarginIndex >= (int)MISC_MENU_WANTED_STAR_RIGHT_MARGIN_CAPTIONS.size()) MenuWantedStarRightMarginIndex = (int)MISC_MENU_WANTED_STAR_RIGHT_MARGIN_CAPTIONS.size() - 1;
			menuItemWantedStarRightMargin = MISC_MENU_WANTED_STAR_RIGHT_MARGIN_VALUES[MenuWantedStarRightMarginIndex];
			MenuWantedStarRightMarginChanged = true;
		}
		else if (setting.name.compare("MenuItemTopOffsetIndex") == 0) {
			MenuItemTopOffsetIndex = stoi(setting.value);
			if (MenuItemTopOffsetIndex < 0) MenuItemTopOffsetIndex = 0;
			if (MenuItemTopOffsetIndex >= (int)MISC_MENU_ITEM_TOP_OFFSET_CAPTIONS.size()) MenuItemTopOffsetIndex = (int)MISC_MENU_ITEM_TOP_OFFSET_CAPTIONS.size() - 1;
			menuItemTopOffset = MISC_MENU_ITEM_TOP_OFFSET_VALUES[MenuItemTopOffsetIndex];
			MenuItemTopOffsetChanged = true;
		}
		else if (setting.name.compare("MenuMainHeaderFontScaleIndex") == 0) {
			MenuMainHeaderFontScaleIndex = stoi(setting.value);
			if (MenuMainHeaderFontScaleIndex < 0) MenuMainHeaderFontScaleIndex = 0;
			if (MenuMainHeaderFontScaleIndex >= (int)MISC_MENU_MAIN_HEADER_FONT_SCALE_CAPTIONS.size()) MenuMainHeaderFontScaleIndex = (int)MISC_MENU_MAIN_HEADER_FONT_SCALE_CAPTIONS.size() - 1;
			menuMainHeaderFontScale = MISC_MENU_MAIN_HEADER_FONT_SCALE_VALUES[MenuMainHeaderFontScaleIndex];
			MenuMainHeaderFontScaleChanged = true;
		}
		else if (setting.name.compare("MenuHeaderFontScaleIndex") == 0) {
			MenuHeaderFontScaleIndex = stoi(setting.value);
			if (MenuHeaderFontScaleIndex < 0) MenuHeaderFontScaleIndex = 0;
			if (MenuHeaderFontScaleIndex >= (int)MISC_MENU_HEADER_FONT_SCALE_CAPTIONS.size()) MenuHeaderFontScaleIndex = (int)MISC_MENU_HEADER_FONT_SCALE_CAPTIONS.size() - 1;
			menuHeaderFontScale = MISC_MENU_HEADER_FONT_SCALE_VALUES[MenuHeaderFontScaleIndex];
			MenuHeaderFontScaleChanged = true;
		}
		else if (setting.name.compare("MenuItemFontScaleIndex") == 0) {
			MenuItemFontScaleIndex = stoi(setting.value);
			if (MenuItemFontScaleIndex < 0) MenuItemFontScaleIndex = 0;
			if (MenuItemFontScaleIndex >= (int)MISC_MENU_ITEM_FONT_SCALE_CAPTIONS.size()) MenuItemFontScaleIndex = (int)MISC_MENU_ITEM_FONT_SCALE_CAPTIONS.size() - 1;
			menuItemFontScale = MISC_MENU_ITEM_FONT_SCALE_VALUES[MenuItemFontScaleIndex];
			MenuItemFontScaleChanged = true;
		}
		else if (setting.name.compare("MenuWantedFontScaleIndex") == 0) {
			MenuWantedFontScaleIndex = stoi(setting.value);
			if (MenuWantedFontScaleIndex < 0) MenuWantedFontScaleIndex = 0;
			if (MenuWantedFontScaleIndex >= (int)MISC_MENU_WANTED_FONT_SCALE_CAPTIONS.size()) MenuWantedFontScaleIndex = (int)MISC_MENU_WANTED_FONT_SCALE_CAPTIONS.size() - 1;
			menuWantedFontScale = MISC_MENU_WANTED_FONT_SCALE_VALUES[MenuWantedFontScaleIndex];
			MenuWantedFontScaleChanged = true;
		}
		else if (setting.name.compare("MenuItemHighlightTextScaleIndex") == 0) {
			MenuItemHighlightTextScaleIndex = stoi(setting.value);
			if (MenuItemHighlightTextScaleIndex < 0) MenuItemHighlightTextScaleIndex = 0;
			if (MenuItemHighlightTextScaleIndex >= (int)MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_CAPTIONS.size()) MenuItemHighlightTextScaleIndex = (int)MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_CAPTIONS.size() - 1;
			menuItemHighlightTextScale = MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_VALUES[MenuItemHighlightTextScaleIndex];
			MenuItemHighlightTextScaleChanged = true;
		}
		// 添加预览图设置的加载
		else if (setting.name.compare("PreviewPositionThresholdIndex") == 0) {
			PreviewPositionThresholdIndex = stoi(setting.value);
			if (PreviewPositionThresholdIndex < 0) PreviewPositionThresholdIndex = 0;
			if (PreviewPositionThresholdIndex >= (int)MISC_PREVIEW_POSITION_THRESHOLD_CAPTIONS.size()) PreviewPositionThresholdIndex = (int)MISC_PREVIEW_POSITION_THRESHOLD_CAPTIONS.size() - 1;
			previewPositionThreshold = MISC_PREVIEW_POSITION_THRESHOLD_VALUES[PreviewPositionThresholdIndex];
			PreviewPositionThresholdChanged = true;
		}
		else if (setting.name.compare("PreviewResolutionScaleIndex") == 0) {
			PreviewResolutionScaleIndex = stoi(setting.value);
			if (PreviewResolutionScaleIndex < 0) PreviewResolutionScaleIndex = 0;
			if (PreviewResolutionScaleIndex >= (int)MISC_PREVIEW_RESOLUTION_SCALE_CAPTIONS.size()) PreviewResolutionScaleIndex = (int)MISC_PREVIEW_RESOLUTION_SCALE_CAPTIONS.size() - 1;
			previewResolutionScale = MISC_PREVIEW_RESOLUTION_SCALE_VALUES[PreviewResolutionScaleIndex];
			PreviewResolutionScaleChanged = true;
		}
		else if (setting.name.compare("PreviewSpacingIndex") == 0) {
			PreviewSpacingIndex = stoi(setting.value);
			if (PreviewSpacingIndex < 0) PreviewSpacingIndex = 0;
			if (PreviewSpacingIndex >= (int)MISC_PREVIEW_SPACING_CAPTIONS.size()) PreviewSpacingIndex = (int)MISC_PREVIEW_SPACING_CAPTIONS.size() - 1;
			previewSpacing = MISC_PREVIEW_SPACING_VALUES[PreviewSpacingIndex];
			PreviewSpacingChanged = true;
		}
		// 添加人物预览图设置的加载
		else if (setting.name.compare("PedPreviewPositionThresholdIndex") == 0) {
			PedPreviewPositionThresholdIndex = stoi(setting.value);
			if (PedPreviewPositionThresholdIndex < 0) PedPreviewPositionThresholdIndex = 0;
			if (PedPreviewPositionThresholdIndex >= (int)MISC_PED_PREVIEW_POSITION_THRESHOLD_CAPTIONS.size()) PedPreviewPositionThresholdIndex = (int)MISC_PED_PREVIEW_POSITION_THRESHOLD_CAPTIONS.size() - 1;
			pedPreviewPositionThreshold = MISC_PED_PREVIEW_POSITION_THRESHOLD_VALUES[PedPreviewPositionThresholdIndex];
			PedPreviewPositionThresholdChanged = true;
		}
		else if (setting.name.compare("PedPreviewResolutionScaleIndex") == 0) {
			PedPreviewResolutionScaleIndex = stoi(setting.value);
			if (PedPreviewResolutionScaleIndex < 0) PedPreviewResolutionScaleIndex = 0;
			if (PedPreviewResolutionScaleIndex >= (int)MISC_PED_PREVIEW_RESOLUTION_SCALE_CAPTIONS.size()) PedPreviewResolutionScaleIndex = (int)MISC_PED_PREVIEW_RESOLUTION_SCALE_CAPTIONS.size() - 1;
			pedPreviewResolutionScale = MISC_PED_PREVIEW_RESOLUTION_SCALE_VALUES[PedPreviewResolutionScaleIndex];
			PedPreviewResolutionScaleChanged = true;
		}
		else if (setting.name.compare("PedPreviewSpacingIndex") == 0) {
			PedPreviewSpacingIndex = stoi(setting.value);
			if (PedPreviewSpacingIndex < 0) PedPreviewSpacingIndex = 0;
			if (PedPreviewSpacingIndex >= (int)MISC_PED_PREVIEW_SPACING_CAPTIONS.size()) PedPreviewSpacingIndex = (int)MISC_PED_PREVIEW_SPACING_CAPTIONS.size() - 1;
			pedPreviewSpacing = MISC_PED_PREVIEW_SPACING_VALUES[PedPreviewSpacingIndex];
			PedPreviewSpacingChanged = true;
		}
		else if (setting.name.compare("featureShowPedPreviews") == 0) {
			featureShowPedPreviews = (setting.value == "1");
		}
		else if (setting.name.compare("screenfltr") == 0) {
			screenfltr = setting.value;
		}
		// 添加自由相机设置的加载（热键在其他按键设置中加载）
		else if (setting.name.compare("FreeCamFollowIndex") == 0) {
			FreeCamFollowIndex = stoi(setting.value);
			FreeCamFollowChanged = true;
		}
		else if (setting.name.compare("FreeCamFovIndex") == 0) {
			FreeCamFovIndex = stoi(setting.value);
			FreeCamFovChanged = true;
		}
		else if (setting.name.compare("FreeCamSpeedSlowIndex") == 0) {
			FreeCamSpeedSlowIndex = stoi(setting.value);
			FreeCamSpeedSlowChanged = true;
		}
		else if (setting.name.compare("FreeCamSpeedMediumIndex") == 0) {
			FreeCamSpeedMediumIndex = stoi(setting.value);
			FreeCamSpeedMediumChanged = true;
		}
		else if (setting.name.compare("FreeCamSpeedFastIndex") == 0) {
			FreeCamSpeedFastIndex = stoi(setting.value);
			FreeCamSpeedFastChanged = true;
		}
		else if (setting.name.compare("FreeCamInfoDisplayIndex") == 0) {
			FreeCamInfoDisplayIndex = stoi(setting.value);
			FreeCamInfoDisplayChanged = true;
		}
		else if (setting.name.compare("FreeCamCrosshairStyleIndex") == 0) {
			FreeCamCrosshairStyleIndex = stoi(setting.value);
			FreeCamCrosshairStyleChanged = true;
		}
		else if (setting.name.compare("FreeCamCrosshairColorIndex") == 0) {
			FreeCamCrosshairColorIndex = stoi(setting.value);
			FreeCamCrosshairColorChanged = true;
		}
		// 添加截图设置的加载
		else if (setting.name.compare("ScreenshotKeyIndex") == 0) {
			ScreenshotKeyIndex = stoi(setting.value);
			if (ScreenshotKeyIndex < 0) ScreenshotKeyIndex = 0;
			if (ScreenshotKeyIndex >= (int)MISC_SCREENSHOT_KEY_CAPTIONS.size()) ScreenshotKeyIndex = (int)MISC_SCREENSHOT_KEY_CAPTIONS.size() - 1;
			ScreenshotKeyChanged = true;
		}
	}
}

bool is_vehicle_preview_enabled(){
	return featureShowVehiclePreviews;
}

bool is_ped_preview_enabled(){
	return featureShowPedPreviews;
}

//bool 菜单中是否阻止输入(){
//	return 菜单中阻止输入功能;
//}

//bool 修改器中是否忽略控制器(){
//	return 修改器中忽略控制器功能;
//}

bool is_hud_hidden(){
	return featureMiscHideHud;
}

void set_hud_hidden(bool hidden){
	featureMiscHideHud = hidden;
	featureMiscHideHudUpdated = true;
}

void set_hud_shown(bool hidden){
	featurePhoneShowHud = hidden;
}

bool is_jellman_scenery_enabled(){
	return featureMiscJellmanScenery;
}

void SkipRadioFwd1(uint32_t a1)
{
	if (!g_radioStationList) {
		write_text_to_log_file("[错误] 未找到 RegisterFile 句柄！");
		return;
	}
	for (int i = 0; i < *g_radioStationCount; i++)
	{
		uintptr_t radioStation = g_radioStationList[i];

		if (radioStation)
		{
			CRadioStation__Advance(radioStation, a1);
		}
	}
}

static void SkipRadioFwd2Internal(uintptr_t a1, uint32_t a2)
{
	uint64_t v2; // rax 寄存器

	if (*(uint32_t*)(a1 + 0x2BC) == 2)
	{
		v2 = *(uint64_t*)(a1 + 0x2A0);
		if (v2)
		{
			*(uint32_t*)(a1 + 0x2C0) = 0;
			*(uint8_t*)(a1 + 0x2CC) = 1;
			*(uint32_t*)(a1 + 0x2B4) = a2;
			*(uint64_t*)(a1 + 0x2A8) = v2;
			*(uint32_t*)(a1 + 0x2BC) = 5;
		}
	}
}

void SkipRadioFwd2(uint32_t a1)
{
	if (!g_unkRadioStationData) {
		write_text_to_log_file("[错误] 未找到 RegisterFile 句柄！");
		return;
	}
	uint32_t v1; // esi 寄存器
	uintptr_t* v2; // rbx 寄存器
	size_t v3; // rdi 寄存器

	v1 = a1;
	v2 = g_unkRadioStationData;
	v3 = 3;
	do
	{
		SkipRadioFwd2Internal(*v2, v1);
		++v2;
		--v3;
	} while (v3);
}

void SKIP_RADIO_FORWARD_CUSTOM()
	{
		SkipRadioFwd1(300000);
		SkipRadioFwd2(300000);
	}

// 需要对此进行压缩，因为已经有一个用于可调雪地的扫描器
bool CompareMemoryJACCO(const uint8_t* pData, const uint8_t* bMask, const char* sMask)
{
	for (; *sMask; ++sMask, ++pData, ++bMask)
		if (*sMask == 'x' && *pData != *bMask)
			return false;

	return *sMask == NULL;
}

intptr_t FindPatternJACCO(const char* bMask, const char* sMask)
{
	// 游戏基址与大小
	static intptr_t pGameBase = (intptr_t)GetModuleHandle(nullptr);
	static uint32_t pGameSize = 0;
	if (!pGameSize)
	{
		MODULEINFO info;
		GetModuleInformation(GetCurrentProcess(), (HMODULE)pGameBase, &info, sizeof(MODULEINFO));
		pGameSize = info.SizeOfImage;
	}

	// 扫描
	for (uint32_t i = 0; i < pGameSize; i++)
		if (CompareMemoryJACCO((uint8_t*)(pGameBase + i), (uint8_t*)bMask, sMask))
			return pGameBase + i;

	return 0;
}

void SInit()
{
	uintptr_t address = FindPatternJACCO("\x3B\x0D\x00\x00\x00\x00\x73\x0E\x48\x8B\x05\x00\x00\x00\x00\x8B\xC9", "xx????xxxxx????xx");

	if (!address) {
		write_text_to_log_file("[错误] 加载电台列表失败！");
		return;
	}

	g_radioStationList = *(uintptr_t**)(address + *(int*)(address + 11) + 15);
	g_radioStationCount = (int*)(address + *(int*)(address + 2) + 6);

	address = FindPatternJACCO("\x80\xB9\x00\x00\x00\x00\x00\x8B\xF2\x48\x8B\xD9\x0F\x85", "xx?????xxxxxxx");
	if (!address) {
		write_text_to_log_file("[错误] 加载电台清单失败！");
		return;
	}
	CRadioStation__Advance = (decltype(CRadioStation__Advance))(address - 15);

	address = FindPatternJACCO("\x48\x8D\x1D\x00\x00\x00\x00\xBF\x00\x00\x00\x00\x48\x83\x3B\x00", "xxx????x????xxxx");
	if (!address) {
		write_text_to_log_file("[错误] 无法加载电台列表！");
		return;
	}
	g_unkRadioStationData = (uintptr_t*)(address + *(int*)(address + 3) + 7);
}

// 菜单布局设置变量定义
int MenuWidthIndex = MENU_WIDTH_DEFAULT_INDEX; // 使用默认索引常量
bool MenuWidthChanged = false;

int MenuHeightIndex = MENU_HEIGHT_DEFAULT_INDEX; // 使用默认索引常量
bool MenuHeightChanged = false;

int MenuTopOffsetIndex = MENU_TOP_OFFSET_DEFAULT_INDEX; // 使用默认索引常量
bool MenuTopOffsetChanged = false;

int MenuLeftOffsetIndex = MENU_LEFT_OFFSET_DEFAULT_INDEX; // 使用默认索引常量
bool MenuLeftOffsetChanged = false;

int MenuTextLeftOffsetIndex = MENU_TEXT_LEFT_OFFSET_DEFAULT_INDEX; // 使用默认索引常量
bool MenuTextLeftOffsetChanged = false;

// 菜单项设置变量定义
int MenuItemHeightIndex = MENU_ITEM_HEIGHT_DEFAULT_INDEX; // 使用默认索引常量
bool MenuItemHeightChanged = false;

int MenuItemSpacingIndex = MENU_ITEM_SPACING_DEFAULT_INDEX; // 使用默认索引常量
bool MenuItemSpacingChanged = false;

int MenuItemTextOffsetIndex = MENU_ITEM_TEXT_OFFSET_DEFAULT_INDEX; // 使用默认索引常量
bool MenuItemTextOffsetChanged = false;

int MenuToggleIconRightMarginIndex = MENU_TOGGLE_ICON_RIGHT_MARGIN_DEFAULT_INDEX; // 使用默认索引常量
bool MenuToggleIconRightMarginChanged = false;

int MenuWantedStarRightMarginIndex = MENU_WANTED_STAR_RIGHT_MARGIN_DEFAULT_INDEX; // 使用默认索引常量
bool MenuWantedStarRightMarginChanged = false;

int MenuItemTopOffsetIndex = MENU_ITEM_TOP_OFFSET_DEFAULT_INDEX; // 使用默认索引常量
bool MenuItemTopOffsetChanged = false;

int MenuMainHeaderFontScaleIndex = MENU_MAIN_HEADER_FONT_SCALE_DEFAULT_INDEX;
bool MenuMainHeaderFontScaleChanged = false;

int MenuHeaderFontScaleIndex = MENU_HEADER_FONT_SCALE_DEFAULT_INDEX;
bool MenuHeaderFontScaleChanged = false;

int MenuItemFontScaleIndex = MENU_ITEM_FONT_SCALE_DEFAULT_INDEX;
bool MenuItemFontScaleChanged = false;

int MenuWantedFontScaleIndex = MENU_WANTED_FONT_SCALE_DEFAULT_INDEX;
bool MenuWantedFontScaleChanged = false;

int MenuItemHighlightTextScaleIndex = MENU_ITEM_HIGHLIGHT_TEXT_SCALE_DEFAULT_INDEX;
bool MenuItemHighlightTextScaleChanged = false;

// 预览图设置变量定义
int PreviewPositionThresholdIndex = PREVIEW_POSITION_THRESHOLD_DEFAULT_INDEX; // 预览图左右判断依据
bool PreviewPositionThresholdChanged = false;

int PreviewResolutionScaleIndex = PREVIEW_RESOLUTION_SCALE_DEFAULT_INDEX; // 预览图分辨率适配值
bool PreviewResolutionScaleChanged = false;

int PreviewSpacingIndex = PREVIEW_SPACING_DEFAULT_INDEX; // 预览图间距
bool PreviewSpacingChanged = false;

// 人物预览图设置变量定义
int PedPreviewPositionThresholdIndex = PED_PREVIEW_POSITION_THRESHOLD_DEFAULT_INDEX; // 人物预览图左右判断依据
bool PedPreviewPositionThresholdChanged = false;
float pedPreviewPositionThreshold = MISC_PED_PREVIEW_POSITION_THRESHOLD_VALUES[PED_PREVIEW_POSITION_THRESHOLD_DEFAULT_INDEX];

int PedPreviewResolutionScaleIndex = PED_PREVIEW_RESOLUTION_SCALE_DEFAULT_INDEX; // 人物预览图分辨率适配值
bool PedPreviewResolutionScaleChanged = false;
float pedPreviewResolutionScale = MISC_PED_PREVIEW_RESOLUTION_SCALE_VALUES[PED_PREVIEW_RESOLUTION_SCALE_DEFAULT_INDEX];

int PedPreviewSpacingIndex = PED_PREVIEW_SPACING_DEFAULT_INDEX; // 人物预览图间距
bool PedPreviewSpacingChanged = false;
float pedPreviewSpacing = MISC_PED_PREVIEW_SPACING_VALUES[PED_PREVIEW_SPACING_DEFAULT_INDEX];

// 自由相机模式变量定义
bool featureFreeCamEnabled = true;
int FreeCamFollowIndex = 0; // 默认跟随（顺序：跟随0，不跟随1）
bool FreeCamFollowChanged = false;
int FreeCamFovIndex = 9; // 默认50
bool FreeCamFovChanged = false;
int FreeCamSpeedSlowIndex = 0; // 默认0.35
bool FreeCamSpeedSlowChanged = false;
int FreeCamSpeedMediumIndex = 10; // 默认1.0
bool FreeCamSpeedMediumChanged = false;
int FreeCamSpeedFastIndex = 14; // 默认3.0
bool FreeCamSpeedFastChanged = false;
int FreeCamInfoDisplayIndex = 0; // 默认底部显示（顺序：底部0，顶部1，不显示2）
bool FreeCamInfoDisplayChanged = false;

bool freeCamActive = false;
Camera freeCamHandle = NULL;
int currentSpeedMode = 0; // 0=慢速, 1=中速, 2=快速
bool freeCamPlayerWasVisible = true;
bool freeCamPlayerHadCollision = true;
bool lastShiftState = false;
int activeLineIndexFreeCam = 0;

// 自由相机中心十字变量定义
int FreeCamCrosshairStyleIndex = 0; // 0=不显示
bool FreeCamCrosshairStyleChanged = false;
int FreeCamCrosshairColorIndex = 0; // 0=白色
bool FreeCamCrosshairColorChanged = false;



// 菜单布局设置相关函数实现
void onchange_misc_menu_width_index(int value, SelectFromListMenuItem* source) {
    MenuWidthIndex = value;
    menuWidth = MISC_MENU_WIDTH_VALUES[value]; // 更新全局菜单宽度变量
    MenuWidthChanged = true;
}

void onchange_misc_menu_height_index(int value, SelectFromListMenuItem* source) {
    MenuHeightIndex = value;
    menuHeight = MISC_MENU_HEIGHT_VALUES[value]; // 更新全局菜单高度变量
    MenuHeightChanged = true;
}

void onchange_misc_menu_top_offset_index(int value, SelectFromListMenuItem* source) {
    MenuTopOffsetIndex = value;
    menuTopOffset = MISC_MENU_TOP_OFFSET_VALUES[value]; // 更新全局菜单顶部偏移量变量
    MenuTopOffsetChanged = true;
}

void onchange_misc_menu_left_offset_index(int value, SelectFromListMenuItem* source) {
    MenuLeftOffsetIndex = value;
    menuLeftOffset = MISC_MENU_LEFT_OFFSET_VALUES[value]; // 更新全局菜单左侧偏移量变量
    MenuLeftOffsetChanged = true;
}

void onchange_misc_menu_text_left_offset_index(int value, SelectFromListMenuItem* source) {
    MenuTextLeftOffsetIndex = value;
    menuTextLeftOffset = MISC_MENU_TEXT_LEFT_OFFSET_VALUES[value]; // 更新全局菜单文本左侧偏移量变量
    MenuTextLeftOffsetChanged = true;
}

// 菜单项设置相关函数实现
void onchange_misc_menu_item_height_index(int value, SelectFromListMenuItem* source) {
    MenuItemHeightIndex = value;
    menuItemHeight = MISC_MENU_ITEM_HEIGHT_VALUES[value]; // 更新全局菜单项高度变量
    MenuItemHeightChanged = true;
}

void onchange_misc_menu_item_spacing_index(int value, SelectFromListMenuItem* source) {
    MenuItemSpacingIndex = value;
    menuItemSpacing = MISC_MENU_ITEM_SPACING_VALUES[value]; // 更新全局菜单项间距变量
    MenuItemSpacingChanged = true;
}

void onchange_misc_menu_item_text_offset_index(int value, SelectFromListMenuItem* source) {
    MenuItemTextOffsetIndex = value;
    menuItemTextOffset = MISC_MENU_ITEM_TEXT_OFFSET_VALUES[value]; // 更新全局菜单项文本偏移量变量
    MenuItemTextOffsetChanged = true;
}

void onchange_misc_menu_toggle_icon_right_margin_index(int value, SelectFromListMenuItem* source) {
	MenuToggleIconRightMarginIndex = value;
	menuItemToggleIconRightMargin = MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_VALUES[value];
	MenuToggleIconRightMarginChanged = true;
}

void onchange_misc_menu_wanted_star_right_margin_index(int value, SelectFromListMenuItem* source) {
	MenuWantedStarRightMarginIndex = value;
	menuItemWantedStarRightMargin = MISC_MENU_WANTED_STAR_RIGHT_MARGIN_VALUES[value];
	MenuWantedStarRightMarginChanged = true;
}

void onchange_misc_menu_main_header_font_scale_index(int value, SelectFromListMenuItem* source) {
	MenuMainHeaderFontScaleIndex = value;
	menuMainHeaderFontScale = MISC_MENU_MAIN_HEADER_FONT_SCALE_VALUES[value];
	MenuMainHeaderFontScaleChanged = true;
}

void onchange_misc_menu_header_font_scale_index(int value, SelectFromListMenuItem* source) {
	MenuHeaderFontScaleIndex = value;
	menuHeaderFontScale = MISC_MENU_HEADER_FONT_SCALE_VALUES[value];
	MenuHeaderFontScaleChanged = true;
}

void onchange_misc_menu_item_font_scale_index(int value, SelectFromListMenuItem* source) {
	MenuItemFontScaleIndex = value;
	menuItemFontScale = MISC_MENU_ITEM_FONT_SCALE_VALUES[value];
	MenuItemFontScaleChanged = true;
}

void onchange_misc_menu_wanted_font_scale_index(int value, SelectFromListMenuItem* source) {
	MenuWantedFontScaleIndex = value;
	menuWantedFontScale = MISC_MENU_WANTED_FONT_SCALE_VALUES[value];
	MenuWantedFontScaleChanged = true;
}

void onchange_misc_menu_item_highlight_text_scale_index(int value, SelectFromListMenuItem* source) {
	MenuItemHighlightTextScaleIndex = value;
	menuItemHighlightTextScale = MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_VALUES[value];
	MenuItemHighlightTextScaleChanged = true;
}

// 预览图设置相关函数实现
void onchange_misc_preview_position_threshold_index(int value, SelectFromListMenuItem* source) {
	PreviewPositionThresholdIndex = value;
	previewPositionThreshold = MISC_PREVIEW_POSITION_THRESHOLD_VALUES[value];
	PreviewPositionThresholdChanged = true;
}

void onchange_misc_preview_resolution_scale_index(int value, SelectFromListMenuItem* source) {
	PreviewResolutionScaleIndex = value;
	previewResolutionScale = MISC_PREVIEW_RESOLUTION_SCALE_VALUES[value];
	PreviewResolutionScaleChanged = true;
}

void onchange_misc_preview_spacing_index(int value, SelectFromListMenuItem* source) {
	PreviewSpacingIndex = value;
	previewSpacing = MISC_PREVIEW_SPACING_VALUES[value];
	PreviewSpacingChanged = true;
}

// 人物预览图设置相关函数实现
void onchange_misc_ped_preview_position_threshold_index(int value, SelectFromListMenuItem* source) {
	PedPreviewPositionThresholdIndex = value;
	pedPreviewPositionThreshold = MISC_PED_PREVIEW_POSITION_THRESHOLD_VALUES[value];
	PedPreviewPositionThresholdChanged = true;
}

void onchange_misc_ped_preview_resolution_scale_index(int value, SelectFromListMenuItem* source) {
	PedPreviewResolutionScaleIndex = value;
	pedPreviewResolutionScale = MISC_PED_PREVIEW_RESOLUTION_SCALE_VALUES[value];
	PedPreviewResolutionScaleChanged = true;
}

void onchange_misc_ped_preview_spacing_index(int value, SelectFromListMenuItem* source) {
	PedPreviewSpacingIndex = value;
	pedPreviewSpacing = MISC_PED_PREVIEW_SPACING_VALUES[value];
	PedPreviewSpacingChanged = true;
}

// 菜单项宽度和左侧偏移已合并到标题设置中，不再需要单独的onchange函数

void onchange_misc_menu_item_top_offset_index(int value, SelectFromListMenuItem* source) {
    MenuItemTopOffsetIndex = value;
    menuItemTopOffset = MISC_MENU_ITEM_TOP_OFFSET_VALUES[value]; // 更新全局菜单项顶部偏移量变量
    MenuItemTopOffsetChanged = true;
}

// 菜单布局设置菜单实现
int activeLineIndexMenuLayout = 0;

void reset_menu_layout_to_defaults() {
    // 使用默认索引常量，避免与用户选择索引0混淆
    MenuWidthIndex = MENU_WIDTH_DEFAULT_INDEX;
    menuWidth = MISC_MENU_WIDTH_VALUES[MenuWidthIndex];
    MenuWidthChanged = true;

    MenuHeightIndex = MENU_HEIGHT_DEFAULT_INDEX;
    menuHeight = MISC_MENU_HEIGHT_VALUES[MenuHeightIndex];
    MenuHeightChanged = true;

    MenuTopOffsetIndex = MENU_TOP_OFFSET_DEFAULT_INDEX;
    menuTopOffset = MISC_MENU_TOP_OFFSET_VALUES[MenuTopOffsetIndex];
    MenuTopOffsetChanged = true;

    MenuLeftOffsetIndex = MENU_LEFT_OFFSET_DEFAULT_INDEX;
    menuLeftOffset = MISC_MENU_LEFT_OFFSET_VALUES[MenuLeftOffsetIndex];
    MenuLeftOffsetChanged = true;

    MenuTextLeftOffsetIndex = MENU_TEXT_LEFT_OFFSET_DEFAULT_INDEX;
    menuTextLeftOffset = MISC_MENU_TEXT_LEFT_OFFSET_VALUES[MenuTextLeftOffsetIndex];
    MenuTextLeftOffsetChanged = true;

    MenuItemHeightIndex = MENU_ITEM_HEIGHT_DEFAULT_INDEX;
    menuItemHeight = MISC_MENU_ITEM_HEIGHT_VALUES[MenuItemHeightIndex];
    MenuItemHeightChanged = true;

    MenuItemSpacingIndex = MENU_ITEM_SPACING_DEFAULT_INDEX;
    menuItemSpacing = MISC_MENU_ITEM_SPACING_VALUES[MenuItemSpacingIndex];
    MenuItemSpacingChanged = true;

    MenuItemTextOffsetIndex = MENU_ITEM_TEXT_OFFSET_DEFAULT_INDEX;
    menuItemTextOffset = MISC_MENU_ITEM_TEXT_OFFSET_VALUES[MenuItemTextOffsetIndex];
    MenuItemTextOffsetChanged = true;

    MenuToggleIconRightMarginIndex = MENU_TOGGLE_ICON_RIGHT_MARGIN_DEFAULT_INDEX;
    menuItemToggleIconRightMargin = MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_VALUES[MenuToggleIconRightMarginIndex];
    MenuToggleIconRightMarginChanged = true;

    MenuWantedStarRightMarginIndex = MENU_WANTED_STAR_RIGHT_MARGIN_DEFAULT_INDEX;
    menuItemWantedStarRightMargin = MISC_MENU_WANTED_STAR_RIGHT_MARGIN_VALUES[MenuWantedStarRightMarginIndex];
    MenuWantedStarRightMarginChanged = true;

    MenuItemTopOffsetIndex = MENU_ITEM_TOP_OFFSET_DEFAULT_INDEX;
    menuItemTopOffset = MISC_MENU_ITEM_TOP_OFFSET_VALUES[MenuItemTopOffsetIndex];
    MenuItemTopOffsetChanged = true;

    MenuMainHeaderFontScaleIndex = MENU_MAIN_HEADER_FONT_SCALE_DEFAULT_INDEX;
    menuMainHeaderFontScale = MISC_MENU_MAIN_HEADER_FONT_SCALE_VALUES[MenuMainHeaderFontScaleIndex];
    MenuMainHeaderFontScaleChanged = true;

    MenuHeaderFontScaleIndex = MENU_HEADER_FONT_SCALE_DEFAULT_INDEX;
    menuHeaderFontScale = MISC_MENU_HEADER_FONT_SCALE_VALUES[MenuHeaderFontScaleIndex];
    MenuHeaderFontScaleChanged = true;

    MenuItemFontScaleIndex = MENU_ITEM_FONT_SCALE_DEFAULT_INDEX;
    menuItemFontScale = MISC_MENU_ITEM_FONT_SCALE_VALUES[MenuItemFontScaleIndex];
    MenuItemFontScaleChanged = true;

    MenuWantedFontScaleIndex = MENU_WANTED_FONT_SCALE_DEFAULT_INDEX;
    menuWantedFontScale = MISC_MENU_WANTED_FONT_SCALE_VALUES[MenuWantedFontScaleIndex];
    MenuWantedFontScaleChanged = true;

    MenuItemHighlightTextScaleIndex = MENU_ITEM_HIGHLIGHT_TEXT_SCALE_DEFAULT_INDEX;
    menuItemHighlightTextScale = MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_VALUES[MenuItemHighlightTextScaleIndex];
    MenuItemHighlightTextScaleChanged = true;

    // 重置预览图设置
    PreviewPositionThresholdIndex = PREVIEW_POSITION_THRESHOLD_DEFAULT_INDEX;
    previewPositionThreshold = MISC_PREVIEW_POSITION_THRESHOLD_VALUES[PreviewPositionThresholdIndex];
    PreviewPositionThresholdChanged = true;

    PreviewResolutionScaleIndex = PREVIEW_RESOLUTION_SCALE_DEFAULT_INDEX;
    previewResolutionScale = MISC_PREVIEW_RESOLUTION_SCALE_VALUES[PreviewResolutionScaleIndex];
    PreviewResolutionScaleChanged = true;

    PreviewSpacingIndex = PREVIEW_SPACING_DEFAULT_INDEX;
    previewSpacing = MISC_PREVIEW_SPACING_VALUES[PreviewSpacingIndex];
    PreviewSpacingChanged = true;

    // 重置人物预览图设置
    PedPreviewPositionThresholdIndex = PED_PREVIEW_POSITION_THRESHOLD_DEFAULT_INDEX;
    pedPreviewPositionThreshold = MISC_PED_PREVIEW_POSITION_THRESHOLD_VALUES[PedPreviewPositionThresholdIndex];
    PedPreviewPositionThresholdChanged = true;

    PedPreviewResolutionScaleIndex = PED_PREVIEW_RESOLUTION_SCALE_DEFAULT_INDEX;
    pedPreviewResolutionScale = MISC_PED_PREVIEW_RESOLUTION_SCALE_VALUES[PedPreviewResolutionScaleIndex];
    PedPreviewResolutionScaleChanged = true;

    PedPreviewSpacingIndex = PED_PREVIEW_SPACING_DEFAULT_INDEX;
    pedPreviewSpacing = MISC_PED_PREVIEW_SPACING_VALUES[PedPreviewSpacingIndex];
    PedPreviewSpacingChanged = true;

    featureShowPedPreviews = true;
}

bool onconfirm_menu_layout_reset(MenuItem<int> choice) {
	if (choice.value == -0xA1B2C3) {// 检查特殊值-0xA1B2C3，避免与下拉菜单的索引冲突
		reset_menu_layout_to_defaults();
		set_status_text_centre_screen("菜单布局 ~g~已重置 ~s~为默认！"); // 屏幕中间提示，带闪烁
		set_status_text("菜单布局已重置为默认！");

		// 异步线程只设置刷新标志
		DWORD myThreadID;
		HANDLE myHandle = CreateThread(0, 0, [](LPVOID) -> DWORD {
			g_MenuLayoutNeedsRefresh = true; // 标志位交给主线程去刷新
			return 0;
			}, 0, 0, &myThreadID);
		CloseHandle(myHandle);

		return true; // 返回 true 退出当前菜单
	}
	return false;
}

// 将按键值转换为按键名称
char* keyValToName(int keyValue) {
	for (int i = 0; i < (sizeof ALL_KEYS / sizeof ALL_KEYS[0]); i++) {
		if (ALL_KEYS[i].keyCode == keyValue) {
			return ALL_KEYS[i].name;
		}
	}
	return "VK_NOTHING";
}

// 恢复快捷键默认设置
void reset_hotkey_settings_to_defaults(){
	// 清空游戏内快捷键数组
	for(int i = 0; i < 9; i++){
		HotkeyIndex[i] = 0; // 默认为"未绑定"
		HotkeyCtrl[i] = false;
		HotkeyAlt[i] = false;
		HotkeyShift[i] = false;
		HotkeyChanged[i] = true;
	}
	
	// 重置快捷键功能设置为未绑定状态
	for(int i = 0; i < 9; i++){
		change_hotkey_function(i + 1, 0); // 设置为"无功能"
	}
	
	// 清空XML配置文件中的快捷键设置
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if(keyConfig != NULL){
		std::string keyNames[] = {
			KeyConfig::KEY_HOT_1, KeyConfig::KEY_HOT_2, KeyConfig::KEY_HOT_3,
			KeyConfig::KEY_HOT_4, KeyConfig::KEY_HOT_5, KeyConfig::KEY_HOT_6,
			KeyConfig::KEY_HOT_7, KeyConfig::KEY_HOT_8, KeyConfig::KEY_HOT_9
		};
		
		// 将所有快捷键重置为未绑定状态
		for(int i = 0; i < 9; i++){
			keyConfig->set_key((char*)keyNames[i].c_str(), "VK_NOTHING", false, false, false);
		}
		
		// 保存到XML文件
		write_xml_config_file();
	}
}

// 写入XML配置文件
void write_xml_config_file(){
	HRESULT hr = CoInitialize(NULL);
	
	// 创建XML文档
	MSXML2::IXMLDOMDocumentPtr spXMLDoc;
	spXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	
	// 设置preserveWhiteSpace为true以保留空行和格式
	spXMLDoc->put_preserveWhiteSpace(VARIANT_TRUE);
	
	// 加载现有的XML文件
	if(!spXMLDoc->load("Enhanced Native Trainer/ent-config.xml")){
		write_text_to_log_file("无法加载XML 配置文件进行更新");
		if(SUCCEEDED(hr)) CoUninitialize();
		return;
	}
	
	// 获取所有按键节点
	IXMLDOMNodeListPtr nodes = spXMLDoc->selectNodes(L"//ent-config/keys/key");
	long length;
	nodes->get_length(&length);
	
	// 更新快捷键配置
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	
	for(int i = 0; i < length; i++){
		IXMLDOMNode *node = nullptr;
		nodes->get_item(i, &node);
		if(!node) continue;

		IXMLDOMNamedNodeMap *attribs = nullptr;
		node->get_attributes(&attribs);
		if(!attribs){
			node->Release();
			continue;
		}
		
		// 获取function属性
		IXMLDOMNode *funcNode = nullptr;
		attribs->getNamedItem(L"function", &funcNode);
		if(funcNode != NULL){
			VARIANT var;
			VariantInit(&var);
			funcNode->get_nodeValue(&var);
			std::string functionName = _com_util::ConvertBSTRToString(V_BSTR(&var));
			VariantClear(&var);

			// 获取当前按键配置
			KeyConfig* key = keyConfig->get_key(functionName);
			if(key != NULL){
				// 更新value属性
				IXMLDOMNode *valueNode = nullptr;
				attribs->getNamedItem(L"value", &valueNode);
				if(valueNode != NULL){
					std::string keyValueName = keyValToName(key->keyCode);
					BSTR valueBstr = _com_util::ConvertStringToBSTR(keyValueName.c_str());
					VARIANT valueVar;
					VariantInit(&valueVar);
					V_VT(&valueVar) = VT_BSTR;
					V_BSTR(&valueVar) = valueBstr;
					valueNode->put_nodeValue(valueVar);
					VariantClear(&valueVar);
					valueNode->Release();
				}
				
				// 更新modCtrl属性
				IXMLDOMNode *ctrlNode = nullptr;
				attribs->getNamedItem(L"modCtrl", &ctrlNode);
				if(ctrlNode != NULL){
					std::string ctrlValue = key->modCtrl ? "true" : "false";
					BSTR ctrlBstr = _com_util::ConvertStringToBSTR(ctrlValue.c_str());
					VARIANT ctrlVar;
					VariantInit(&ctrlVar);
					V_VT(&ctrlVar) = VT_BSTR;
					V_BSTR(&ctrlVar) = ctrlBstr;
					ctrlNode->put_nodeValue(ctrlVar);
					VariantClear(&ctrlVar);
					ctrlNode->Release();
				}
				
				// 更新modAlt属性
				IXMLDOMNode *altNode = nullptr;
				attribs->getNamedItem(L"modAlt", &altNode);
				if(altNode != NULL){
					std::string altValue = key->modAlt ? "true" : "false";
					BSTR altBstr = _com_util::ConvertStringToBSTR(altValue.c_str());
					VARIANT altVar;
					VariantInit(&altVar);
					V_VT(&altVar) = VT_BSTR;
					V_BSTR(&altVar) = altBstr;
					altNode->put_nodeValue(altVar);
					VariantClear(&altVar);
					altNode->Release();
				}
				
				// 更新modShift属性
				IXMLDOMNode *shiftNode = nullptr;
				attribs->getNamedItem(L"modShift", &shiftNode);
				if(shiftNode != NULL){
					std::string shiftValue = key->modShift ? "true" : "false";
					BSTR shiftBstr = _com_util::ConvertStringToBSTR(shiftValue.c_str());
					VARIANT shiftVar;
					VariantInit(&shiftVar);
					V_VT(&shiftVar) = VT_BSTR;
					V_BSTR(&shiftVar) = shiftBstr;
					shiftNode->put_nodeValue(shiftVar);
					VariantClear(&shiftVar);
					shiftNode->Release();
				}
			}
			
			funcNode->Release();
		}
		
		attribs->Release();
		node->Release();
	}
	
	// 保存XML文件到临时文件
	std::string tempFileName = "Enhanced Native Trainer/ent-config.xml.tmp";
	spXMLDoc->save(tempFileName.c_str());
	
	// 读取临时文件内容
	std::ifstream inFile(tempFileName, std::ios::binary);
	std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
	inFile.close();
	
	// 删除临时文件
	DeleteFileA(tempFileName.c_str());
	
	// 手动添加换行符以保持格式（统一使用Windows CRLF格式）
	// 确保<?xml version="1.0" encoding="utf-8"?>后面有换行符
	size_t xmlDeclPos = content.find("?>");
	if(xmlDeclPos != std::string::npos){
		xmlDeclPos += 2; // 移动到?>后面
		// 检查后面是否有换行符
		if(xmlDeclPos < content.length() && content[xmlDeclPos] != '\n' && content[xmlDeclPos] != '\r'){
			// 添加换行符（使用Windows CRLF格式）
			content.insert(xmlDeclPos, "\r\n");
		}
	}
	
	// 确保<ent-config>前面有换行符
	size_t entConfigPos = content.find("<ent-config>");
	if(entConfigPos != std::string::npos && entConfigPos > 0){
		// 检查前面是否有换行符
		if(content[entConfigPos-1] != '\n' && content[entConfigPos-1] != '\r'){
			// 添加换行符（使用Windows CRLF格式）
			content.insert(entConfigPos, "\r\n");
		}
	}
	
	// 将所有LF转换为CRLF（方案一：强制二进制模式写入并手动转换换行符）
	std::string result;
	result.reserve(content.length() * 2); // 预分配足够空间
	for(size_t i = 0; i < content.length(); i++){
		if(content[i] == '\n' && (i == 0 || content[i-1] != '\r')){
			// 发现单独的LF，转换为CRLF
			result += "\r\n";
		} else {
			// 保持原字符
			result += content[i];
		}
	}
	
	// 以二进制模式写入最终文件，确保CRLF格式
	std::ofstream outFile("Enhanced Native Trainer/ent-config.xml", std::ios::binary);
	outFile.write(result.c_str(), result.length());
	outFile.close();
	
	write_text_to_log_file("XML 配置文件已更新");

	if(SUCCEEDED(hr)) CoUninitialize();
}

// 保存快捷键设置到XML文件
void save_hotkey_settings_to_xml(){
	// 获取配置对象
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	
	// 为每个快捷键保存设置
	for(int i = 0; i < 9; i++){
		if(HotkeyChanged[i]){
			std::string keyName;
			switch(i){
				case 0: keyName = KeyConfig::KEY_HOT_1; break;
				case 1: keyName = KeyConfig::KEY_HOT_2; break;
				case 2: keyName = KeyConfig::KEY_HOT_3; break;
				case 3: keyName = KeyConfig::KEY_HOT_4; break;
				case 4: keyName = KeyConfig::KEY_HOT_5; break;
				case 5: keyName = KeyConfig::KEY_HOT_6; break;
				case 6: keyName = KeyConfig::KEY_HOT_7; break;
				case 7: keyName = KeyConfig::KEY_HOT_8; break;
				case 8: keyName = KeyConfig::KEY_HOT_9; break;
			}
			
			// 获取按键值
			int keyValue = MISC_HOTKEY_VALUES[HotkeyIndex[i]];
			
			// 获取按键名称
			char* keyValueName = keyValToName(keyValue);
			
			// 设置按键配置
			keyConfig->set_key((char*)keyName.c_str(), keyValueName, 
							   HotkeyCtrl[i], HotkeyAlt[i], HotkeyShift[i]);
			
			HotkeyChanged[i] = false;
		}
	}
	
	// 写入XML配置文件
	write_xml_config_file();
}

// 从XML文件加载快捷键设置
void load_hotkey_settings_from_xml(){
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	
	for(int i = 0; i < 9; i++){
		std::string keyName;
		switch(i){
			case 0: keyName = KeyConfig::KEY_HOT_1; break;
			case 1: keyName = KeyConfig::KEY_HOT_2; break;
			case 2: keyName = KeyConfig::KEY_HOT_3; break;
			case 3: keyName = KeyConfig::KEY_HOT_4; break;
			case 4: keyName = KeyConfig::KEY_HOT_5; break;
			case 5: keyName = KeyConfig::KEY_HOT_6; break;
			case 6: keyName = KeyConfig::KEY_HOT_7; break;
			case 7: keyName = KeyConfig::KEY_HOT_8; break;
			case 8: keyName = KeyConfig::KEY_HOT_9; break;
		}
		
		KeyConfig* key = keyConfig->get_key(keyName);
		if(key != NULL){
			// 查找对应的索引
			for(int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++){
				if(MISC_HOTKEY_VALUES[j] == key->keyCode){
					HotkeyIndex[i] = j;
					break;
				}
			}
			HotkeyCtrl[i] = key->modCtrl;
			HotkeyAlt[i] = key->modAlt;
			HotkeyShift[i] = key->modShift;
		}
	}
}

// 常用按键显示标题函数
std::string get_common_key_display_caption(int keyIndex) {
	std::ostringstream caption;
	const char* keyNames[] = {
		"开关/菜单",
		"向上/移动", 
		"向下/移动",
		"向左/移动",
		"向右/移动",
		"确认/选择",
		"返回/取消"
	};
	
	if (keyIndex >= 0 && keyIndex < 7) {
		caption << keyNames[keyIndex];
		
		// 按“快捷键按键设置”的显示逻辑：直接读取 KeyInputConfig 显示实际应用的按键
		// 这样可确保与内存/XML最新生效的按键一致显示
		KeyInputConfig* keyConfig = get_config()->get_key_config();
		std::string keyName;
		switch(keyIndex) {
			case 0: keyName = KeyConfig::KEY_TOGGLE_MAIN_MENU; break;
			case 1: keyName = KeyConfig::KEY_MENU_UP; break;
			case 2: keyName = KeyConfig::KEY_MENU_DOWN; break;
			case 3: keyName = KeyConfig::KEY_MENU_LEFT; break;
			case 4: keyName = KeyConfig::KEY_MENU_RIGHT; break;
			case 5: keyName = KeyConfig::KEY_MENU_SELECT; break;
			case 6: keyName = KeyConfig::KEY_MENU_BACK; break;
		}
		
		int displayIndex = 0; // 0 表示“未绑定”
		if (keyConfig != NULL) {
			KeyConfig* key = keyConfig->get_key(keyName);
			if (key != NULL && key->keyCode != VK_NOTHING) {
				for (int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++) {
					if (MISC_HOTKEY_VALUES[j] == key->keyCode) {
						displayIndex = j;
						break;
					}
				}
			}
		}
		
		if (displayIndex > 0 && displayIndex < (int)MISC_HOTKEY_CAPTIONS.size()) {
			caption << "  [" << MISC_HOTKEY_CAPTIONS[displayIndex] << "]";
		} else {
			caption << " [未绑定]";
		}
	}
	
	return caption.str();
}

// 根据 Key 名称从 KeyInputConfig 计算展示索引（与“快捷键按键设置”一致）
// 工具函数：从 Key 名读取按键码并映射到下拉索引（找不到时回退 defaultIndex）
static int get_key_index_from_config_by_name(const std::string &keyName, int defaultIndex) {
	// defaultIndex 用于在找不到映射但存在有效按键码时作为后备值
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) return defaultIndex;
	KeyConfig* key = keyConfig->get_key(keyName);
	if (key == NULL) return defaultIndex;
	if (key->keyCode == VK_NOTHING) return 0; // 未绑定
	for (int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++) {
		if (MISC_HOTKEY_VALUES[j] == key->keyCode) {
			return j;
		}
	}
	// 找不到对应映射时，退回默认索引
	return defaultIndex;
}

// 其他按键显示标题函数
std::string get_other_key_display_caption(int keyIndex) {
	std::ostringstream caption;
	const char* keyNames[] = {
		"自由移动",
		"自由相机",
		"车辆加速", 
		"车辆停止",
		"载具武器",
		"左转向灯",
		"右转向灯",
		"打开双闪"
	};
	
	if (keyIndex >= 0 && keyIndex < 8) {
		caption << keyNames[keyIndex];
		
		// 按“快捷键按键设置”的显示逻辑：直接读取 KeyInputConfig 显示实际应用的按键
		KeyInputConfig* keyConfig = get_config()->get_key_config();
		std::string keyName;
		switch(keyIndex) {
			case 0: keyName = KeyConfig::KEY_TOGGLE_AIRBRAKE; break;
			case 1: keyName = KeyConfig::KEY_FREECAM_TOGGLE; break;
			case 2: keyName = KeyConfig::KEY_VEH_BOOST; break;
			case 3: keyName = KeyConfig::KEY_VEH_STOP; break;
			case 4: keyName = KeyConfig::KEY_VEH_ROCKETS; break;
			case 5: keyName = KeyConfig::KEY_VEH_LEFTBLINK; break;
			case 6: keyName = KeyConfig::KEY_VEH_RIGHTBLINK; break;
			case 7: keyName = KeyConfig::KEY_VEH_EMERGENCYBLINK; break;
		}
		
		int displayIndex = 0; // 0 表示“未绑定”
		if (keyConfig != NULL) {
			KeyConfig* key = keyConfig->get_key(keyName);
			if (key != NULL && key->keyCode != VK_NOTHING) {
				for (int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++) {
					if (MISC_HOTKEY_VALUES[j] == key->keyCode) {
						displayIndex = j;
						break;
					}
				}
			}
		}
		
		if (displayIndex > 0 && displayIndex < (int)MISC_HOTKEY_CAPTIONS.size()) {
			caption << "  [" << MISC_HOTKEY_CAPTIONS[displayIndex] << "]";
		} else {
			caption << " [未绑定]";
		}
	}
	
	return caption.str();
}

// 常用按键设置回调函数
void onchange_common_key(int value, SelectFromListMenuItem* source) {
	int keyIndex = source->extras.at(0);
	
	if (keyIndex >= 0 && keyIndex < 7) {
		// 检查按键重复
		if (is_common_key_duplicate(keyIndex, value)) {
			// 获取默认值
			int defaultValues[] = {4, 57, 51, 53, 55, 54, 49}; // F4, 小键盘8/2/4/6/5/0
			int defaultValue = defaultValues[keyIndex];
			
			// 检查默认值是否也被占用
			if (is_common_key_duplicate(keyIndex, defaultValue)) {
				// 默认值也被占用，恢复为未绑定
				set_status_text("按键重复, 默认值也被占用！\n已恢复为：未绑定状态！");
				set_status_text_centre_screen("按键 ~r~重复！~s~已恢复为：未绑定状态！");
				value = 0; // 未绑定
			} else {
				// 默认值未被占用，恢复为默认值
				set_status_text("按键重复！\n已恢复默认绑定键位。");
				set_status_text_centre_screen("按键 ~r~重复！~s~已恢复默认绑定键位。");
				value = defaultValue;
			}
		}
		
		// 统一更新索引变量（无论是否重复，value都是最终要设置的值）
		switch(keyIndex) {
			case 0: CommonKeyToggleMenuIndex = value; break;
			case 1: CommonKeyMoveUpIndex = value; break;
			case 2: CommonKeyMoveDownIndex = value; break;
			case 3: CommonKeyMoveLeftIndex = value; break;
			case 4: CommonKeyMoveRightIndex = value; break;
			case 5: CommonKeyConfirmSelectIndex = value; break;
			case 6: CommonKeyBackCancelIndex = value; break;
		}
		CommonKeyChanged[keyIndex] = true;
		
		// 立即更新KeyInputConfig以使更改生效
		KeyInputConfig* keyConfig = get_config()->get_key_config();
		if (keyConfig != NULL) {
			std::string keyName;
			int keyValue;
			
			switch(keyIndex) {
				case 0: keyName = KeyConfig::KEY_TOGGLE_MAIN_MENU; keyValue = CommonKeyToggleMenuIndex; break;
				case 1: keyName = KeyConfig::KEY_MENU_UP; keyValue = CommonKeyMoveUpIndex; break;
				case 2: keyName = KeyConfig::KEY_MENU_DOWN; keyValue = CommonKeyMoveDownIndex; break;
				case 3: keyName = KeyConfig::KEY_MENU_LEFT; keyValue = CommonKeyMoveLeftIndex; break;
				case 4: keyName = KeyConfig::KEY_MENU_RIGHT; keyValue = CommonKeyMoveRightIndex; break;
				case 5: keyName = KeyConfig::KEY_MENU_SELECT; keyValue = CommonKeyConfirmSelectIndex; break;
				case 6: keyName = KeyConfig::KEY_MENU_BACK; keyValue = CommonKeyBackCancelIndex; break;
			}
			
			if (keyValue >= 0 && keyValue < sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
				int actualKeyValue = MISC_HOTKEY_VALUES[keyValue];
				char* keyValueName = keyValToName(actualKeyValue);
				keyConfig->set_key((char*)keyName.c_str(), keyValueName, false, false, false);
			}
		}
	}
}

// 其他按键设置回调函数
void onchange_other_key(int value, SelectFromListMenuItem* source) {
	int keyIndex = source->extras.at(0);
	
	if (keyIndex >= 0 && keyIndex < 8) {
		// 检查按键重复
		if (is_other_key_duplicate(keyIndex, value)) {
			// 获取默认值
			int defaultValues[] = {6, 7, 58, 52, 59, 72, 73, 63}; // F6/F7, 小键盘9/3/+, 左右箭头, 小键盘.
			int defaultValue = defaultValues[keyIndex];
			
			// 检查默认值是否也被占用
			if (is_other_key_duplicate(keyIndex, defaultValue)) {
				// 默认值也被占用，恢复为未绑定
				set_status_text("按键重复, 默认值也被占用！\n已恢复为：未绑定状态！");
				set_status_text_centre_screen("按键 ~r~重复！~s~已恢复为：未绑定状态！");
				value = 0; // 未绑定
			} else {
				// 默认值未被占用，恢复为默认值
				set_status_text("按键重复！\n已恢复默认绑定键位。");
				set_status_text_centre_screen("按键 ~r~重复！~s~已恢复默认绑定键位。");
				value = defaultValue;
			}
		}
		
		// 统一更新索引变量（无论是否重复，value都是最终要设置的值）
		switch(keyIndex) {
			case 0: OtherKeyToggleFreeMoveIndex = value; break;
			case 1: OtherKeyFreeCamToggleIndex = value; break;
			case 2: OtherKeyVehicleBoostIndex = value; break;
			case 3: OtherKeyVehicleStopIndex = value; break;
			case 4: OtherKeyVehicleRocketsIndex = value; break;
			case 5: OtherKeyLeftBlinkIndex = value; break;
			case 6: OtherKeyRightBlinkIndex = value; break;
			case 7: OtherKeyEmergencyBlinkIndex = value; break;
		}
		OtherKeyChanged[keyIndex] = true;
		
		// 立即更新KeyInputConfig以使更改生效
		KeyInputConfig* keyConfig = get_config()->get_key_config();
		if (keyConfig != NULL) {
			std::string keyName;
			int keyValue;
			
			switch(keyIndex) {
				case 0: keyName = KeyConfig::KEY_TOGGLE_AIRBRAKE; keyValue = OtherKeyToggleFreeMoveIndex; break;
				case 1: keyName = KeyConfig::KEY_FREECAM_TOGGLE; keyValue = OtherKeyFreeCamToggleIndex; break;
				case 2: keyName = KeyConfig::KEY_VEH_BOOST; keyValue = OtherKeyVehicleBoostIndex; break;
				case 3: keyName = KeyConfig::KEY_VEH_STOP; keyValue = OtherKeyVehicleStopIndex; break;
				case 4: keyName = KeyConfig::KEY_VEH_ROCKETS; keyValue = OtherKeyVehicleRocketsIndex; break;
				case 5: keyName = KeyConfig::KEY_VEH_LEFTBLINK; keyValue = OtherKeyLeftBlinkIndex; break;
				case 6: keyName = KeyConfig::KEY_VEH_RIGHTBLINK; keyValue = OtherKeyRightBlinkIndex; break;
				case 7: keyName = KeyConfig::KEY_VEH_EMERGENCYBLINK; keyValue = OtherKeyEmergencyBlinkIndex; break;
			}
			
			if (keyValue >= 0 && keyValue < sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
				int actualKeyValue = MISC_HOTKEY_VALUES[keyValue];
				char* keyValueName = keyValToName(actualKeyValue);
				keyConfig->set_key((char*)keyName.c_str(), keyValueName, false, false, false);
			}
		}
	}
}

// 常用按键设置菜单
void process_misc_common_keys_menu() {
	const std::string caption = "常用按键设置";
	std::vector<MenuItem<int>*> menuItems;
	
	// 添加7个常用按键设置项
	for (int i = 0; i < 7; i++) {
		SelectFromListMenuItem* listItem = new SelectFromListMenuItem(MISC_HOTKEY_CAPTIONS, onchange_common_key);
		listItem->wrap = false;
		listItem->caption = get_common_key_display_caption(i);
		listItem->extras.push_back(i);
		
		// 初始值改为从 KeyInputConfig 动态读取，保持与“快捷键按键设置”一致
		// 避免仅显示默认索引，保证与实际生效按键匹配
		int defaultIndex = 0;
		switch(i) {
			case 0: defaultIndex = CommonKeyToggleMenuIndex; break;
			case 1: defaultIndex = CommonKeyMoveUpIndex; break;
			case 2: defaultIndex = CommonKeyMoveDownIndex; break;
			case 3: defaultIndex = CommonKeyMoveLeftIndex; break;
			case 4: defaultIndex = CommonKeyMoveRightIndex; break;
			case 5: defaultIndex = CommonKeyConfirmSelectIndex; break;
			case 6: defaultIndex = CommonKeyBackCancelIndex; break;
		}
		std::string keyName;
		switch(i) {
			case 0: keyName = KeyConfig::KEY_TOGGLE_MAIN_MENU; break;
			case 1: keyName = KeyConfig::KEY_MENU_UP; break;
			case 2: keyName = KeyConfig::KEY_MENU_DOWN; break;
			case 3: keyName = KeyConfig::KEY_MENU_LEFT; break;
			case 4: keyName = KeyConfig::KEY_MENU_RIGHT; break;
			case 5: keyName = KeyConfig::KEY_MENU_SELECT; break;
			case 6: keyName = KeyConfig::KEY_MENU_BACK; break;
		}
		listItem->value = get_key_index_from_config_by_name(keyName, defaultIndex); // 以配置为准
		
		menuItems.push_back(listItem);
	}
	
	draw_generic_menu<int>(menuItems, &activeLineIndexCommonKeys, caption, NULL, NULL, NULL, NULL);
}

// 其他按键设置菜单
void process_misc_other_keys_menu() {
	const std::string caption = "其他按键设置";
	std::vector<MenuItem<int>*> menuItems;
	
	// 添加8个其他按键设置项
	for (int i = 0; i < 8; i++) {
		SelectFromListMenuItem* listItem = new SelectFromListMenuItem(MISC_HOTKEY_CAPTIONS, onchange_other_key);
		listItem->wrap = false;
		listItem->caption = get_other_key_display_caption(i);
		listItem->extras.push_back(i);
		
		// 初始值改为从 KeyInputConfig 动态读取，保持与“快捷键按键设置”一致
		// 避免仅显示默认索引，保证与实际生效按键匹配
		int defaultIndex = 0;
		switch(i) {
			case 0: defaultIndex = OtherKeyToggleFreeMoveIndex; break;
			case 1: defaultIndex = OtherKeyFreeCamToggleIndex; break;
			case 2: defaultIndex = OtherKeyVehicleBoostIndex; break;
			case 3: defaultIndex = OtherKeyVehicleStopIndex; break;
			case 4: defaultIndex = OtherKeyVehicleRocketsIndex; break;
			case 5: defaultIndex = OtherKeyLeftBlinkIndex; break;
			case 6: defaultIndex = OtherKeyRightBlinkIndex; break;
			case 7: defaultIndex = OtherKeyEmergencyBlinkIndex; break;
		}
		std::string keyName;
		switch(i) {
			case 0: keyName = KeyConfig::KEY_TOGGLE_AIRBRAKE; break;
			case 1: keyName = KeyConfig::KEY_FREECAM_TOGGLE; break;
			case 2: keyName = KeyConfig::KEY_VEH_BOOST; break;
			case 3: keyName = KeyConfig::KEY_VEH_STOP; break;
			case 4: keyName = KeyConfig::KEY_VEH_ROCKETS; break;
			case 5: keyName = KeyConfig::KEY_VEH_LEFTBLINK; break;
			case 6: keyName = KeyConfig::KEY_VEH_RIGHTBLINK; break;
			case 7: keyName = KeyConfig::KEY_VEH_EMERGENCYBLINK; break;
		}
		listItem->value = get_key_index_from_config_by_name(keyName, defaultIndex); // 以配置为准
		
		menuItems.push_back(listItem);
	}
	
	draw_generic_menu<int>(menuItems, &activeLineIndexOtherKeys, caption, NULL, NULL, NULL, NULL);
}

// 保存常用按键和其他按键设置到XML文件
void save_common_other_keys_to_xml(){
	// 获取配置对象
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	
	// 保存常用按键设置
	for(int i = 0; i < 7; i++){
		if(CommonKeyChanged[i]){
			std::string keyName;
			int keyValue = 0;
			
			switch(i){
				case 0: 
					keyName = KeyConfig::KEY_TOGGLE_MAIN_MENU;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyToggleMenuIndex];
					break;
				case 1: 
					keyName = KeyConfig::KEY_MENU_UP;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyMoveUpIndex];
					break;
				case 2: 
					keyName = KeyConfig::KEY_MENU_DOWN;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyMoveDownIndex];
					break;
				case 3: 
					keyName = KeyConfig::KEY_MENU_LEFT;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyMoveLeftIndex];
					break;
				case 4: 
					keyName = KeyConfig::KEY_MENU_RIGHT;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyMoveRightIndex];
					break;
				case 5: 
					keyName = KeyConfig::KEY_MENU_SELECT;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyConfirmSelectIndex];
					break;
				case 6: 
					keyName = KeyConfig::KEY_MENU_BACK;
					keyValue = MISC_HOTKEY_VALUES[CommonKeyBackCancelIndex];
					break;
			}
			
			// 获取按键名称
			char* keyValueName = keyValToName(keyValue);
			
			// 设置按键配置
			keyConfig->set_key((char*)keyName.c_str(), keyValueName, false, false, false);
			
			CommonKeyChanged[i] = false;
		}
	}
	
	// 保存其他按键设置
	for(int i = 0; i < 8; i++){
		if(OtherKeyChanged[i]){
			std::string keyName;
			int keyValue = 0;
			
			switch(i){
				case 0: 
					keyName = KeyConfig::KEY_TOGGLE_AIRBRAKE; // 自由移动功能
					keyValue = MISC_HOTKEY_VALUES[OtherKeyToggleFreeMoveIndex];
					break;
				case 1: 
					keyName = KeyConfig::KEY_FREECAM_TOGGLE; // 自由相机功能
					keyValue = MISC_HOTKEY_VALUES[OtherKeyFreeCamToggleIndex];
					break;
				case 2: 
					keyName = KeyConfig::KEY_VEH_BOOST;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyVehicleBoostIndex];
					break;
				case 3: 
					keyName = KeyConfig::KEY_VEH_STOP;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyVehicleStopIndex];
					break;
				case 4: 
					keyName = KeyConfig::KEY_VEH_ROCKETS;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyVehicleRocketsIndex];
					break;
				case 5: 
					keyName = KeyConfig::KEY_VEH_LEFTBLINK;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyLeftBlinkIndex];
					break;
				case 6: 
					keyName = KeyConfig::KEY_VEH_RIGHTBLINK;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyRightBlinkIndex];
					break;
				case 7: 
					keyName = KeyConfig::KEY_VEH_EMERGENCYBLINK;
					keyValue = MISC_HOTKEY_VALUES[OtherKeyEmergencyBlinkIndex];
					break;
			}
			
			// 获取按键名称
			char* keyValueName = keyValToName(keyValue);
			
			// 设置按键配置
			keyConfig->set_key((char*)keyName.c_str(), keyValueName, false, false, false);
			
			OtherKeyChanged[i] = false;
		}
	}
	
	// 写入XML配置文件
	write_xml_config_file();
}

// 重置常用按键和其他按键设置为默认值
void reset_common_other_keys_to_defaults(){
	// 重置常用按键为默认值
	CommonKeyToggleMenuIndex = 4;      // 默认 F4
	CommonKeyMoveUpIndex = 57;         // 默认小键盘 8
	CommonKeyMoveDownIndex = 51;       // 默认小键盘 2
	CommonKeyMoveLeftIndex = 53;       // 默认小键盘 4
	CommonKeyMoveRightIndex = 55;      // 默认小键盘 6
	CommonKeyConfirmSelectIndex = 54;        // 默认小键盘 5
	CommonKeyBackCancelIndex = 49;           // 默认小键盘 0
	
	for(int i = 0; i < 7; i++){
		CommonKeyChanged[i] = true;
	}
	
	// 重置其他按键为默认值
	OtherKeyToggleFreeMoveIndex = 6;   // 默认 F6
	OtherKeyFreeCamToggleIndex = 7;    // 默认 F7
	OtherKeyVehicleBoostIndex = 58;    // 默认小键盘 9
	OtherKeyVehicleStopIndex = 52;     // 默认小键盘 3
	OtherKeyVehicleRocketsIndex = 59;  // 默认小键盘 +
	OtherKeyLeftBlinkIndex = 72;       // 默认左箭头
	OtherKeyRightBlinkIndex = 73;      // 默认右箭头
	OtherKeyEmergencyBlinkIndex = 63;  // 默认小键盘 .
	
	for(int i = 0; i < 8; i++){
		OtherKeyChanged[i] = true;
	}
	
	// 设置XML配置文件中的默认按键设置
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if(keyConfig != NULL){
		// 重置常用按键为默认值
		keyConfig->set_key((char*)KeyConfig::KEY_TOGGLE_MAIN_MENU.c_str(), "VK_F4", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_UP.c_str(), "VK_NUMPAD8", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_DOWN.c_str(), "VK_NUMPAD2", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_LEFT.c_str(), "VK_NUMPAD4", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_RIGHT.c_str(), "VK_NUMPAD6", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_SELECT.c_str(), "VK_NUMPAD5", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_MENU_BACK.c_str(), "VK_NUMPAD0", false, false, false);
		
		// 重置其他按键为默认值
		keyConfig->set_key((char*)KeyConfig::KEY_TOGGLE_AIRBRAKE.c_str(), "VK_F6", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_FREECAM_TOGGLE.c_str(), "VK_F7", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_BOOST.c_str(), "VK_NUMPAD9", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_STOP.c_str(), "VK_NUMPAD3", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_ROCKETS.c_str(), "VK_ADD", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_LEFTBLINK.c_str(), "VK_LEFT", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_RIGHTBLINK.c_str(), "VK_RIGHT", false, false, false);
		keyConfig->set_key((char*)KeyConfig::KEY_VEH_EMERGENCYBLINK.c_str(), "VK_DECIMAL", false, false, false);
		
		// 保存到XML文件
		write_xml_config_file();
	}
}

// 从XML文件加载常用按键和其他按键设置
void load_common_other_keys_from_xml(){
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	
	// 默认值数组
	int commonKeyDefaults[] = {4, 57, 51, 53, 55, 54, 49}; // F4, 小键盘8, 小键盘2, 小键盘4, 小键盘6, 小键盘5, 小键盘0
	int otherKeyDefaults[] = {6, 7, 58, 52, 59, 72, 73, 63}; // F6, F7, 小键盘9, 小键盘3, 小键盘+, 左箭头, 右箭头, 小键盘.
	
	// 加载常用按键设置
	std::string commonKeyNames[] = {
		KeyConfig::KEY_TOGGLE_MAIN_MENU, KeyConfig::KEY_MENU_UP, KeyConfig::KEY_MENU_DOWN,
		KeyConfig::KEY_MENU_LEFT, KeyConfig::KEY_MENU_RIGHT, KeyConfig::KEY_MENU_SELECT,
		KeyConfig::KEY_MENU_BACK
	};
	
	int* commonKeyIndices[] = {
		&CommonKeyToggleMenuIndex, &CommonKeyMoveUpIndex, &CommonKeyMoveDownIndex,
		&CommonKeyMoveLeftIndex, &CommonKeyMoveRightIndex, &CommonKeyConfirmSelectIndex,
		&CommonKeyBackCancelIndex
	};
	
	for(int i = 0; i < 7; i++){
		KeyConfig* key = keyConfig->get_key(commonKeyNames[i]);
		if(key != NULL && key->keyCode != VK_NOTHING){
			// 查找对应的索引
			bool found = false;
			for(int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++){
				if(MISC_HOTKEY_VALUES[j] == key->keyCode){
					*commonKeyIndices[i] = j;
					found = true;
					break;
				}
			}
			// 如果没找到对应的按键值，使用默认值
			if(!found){
				*commonKeyIndices[i] = commonKeyDefaults[i];
			}
		} else {
			// 如果没有配置或配置为VK_NOTHING，使用默认值
			*commonKeyIndices[i] = commonKeyDefaults[i];
		}
	}
	
	// 加载其他按键设置
	std::string otherKeyNames[] = {
		KeyConfig::KEY_TOGGLE_AIRBRAKE, KeyConfig::KEY_FREECAM_TOGGLE, KeyConfig::KEY_VEH_BOOST, KeyConfig::KEY_VEH_STOP,
		KeyConfig::KEY_VEH_ROCKETS, KeyConfig::KEY_VEH_LEFTBLINK, KeyConfig::KEY_VEH_RIGHTBLINK,
		KeyConfig::KEY_VEH_EMERGENCYBLINK
	};
	
	int* otherKeyIndices[] = {
		&OtherKeyToggleFreeMoveIndex, &OtherKeyFreeCamToggleIndex, &OtherKeyVehicleBoostIndex, &OtherKeyVehicleStopIndex,
		&OtherKeyVehicleRocketsIndex, &OtherKeyLeftBlinkIndex, &OtherKeyRightBlinkIndex,
		&OtherKeyEmergencyBlinkIndex
	};
	
	for(int i = 0; i < 8; i++){
		KeyConfig* key = keyConfig->get_key(otherKeyNames[i]);
		if(key != NULL && key->keyCode != VK_NOTHING){
			// 查找对应的索引
			bool found = false;
			for(int j = 0; j < sizeof(MISC_HOTKEY_VALUES)/sizeof(int); j++){
				if(MISC_HOTKEY_VALUES[j] == key->keyCode){
					*otherKeyIndices[i] = j;
					found = true;
					break;
				}
			}
			// 如果没找到对应的按键值，使用默认值
			if(!found){
				*otherKeyIndices[i] = otherKeyDefaults[i];
			}
		} else {
			// 如果没有配置或配置为VK_NOTHING，使用默认值
			*otherKeyIndices[i] = otherKeyDefaults[i];
		}
	}
}

void process_misc_menu_layout_settings_menu() {
    const std::string caption = "菜单布局设置";

    // 如果上次操作触发了重置，强制刷新当前菜单项显示的 value
    if (g_MenuLayoutNeedsRefresh) {
        g_MenuLayoutNeedsRefresh = false;
        // 直接继续构建菜单即可，下面使用的 Index 都是最新的 0
    }

    std::vector<MenuItem<int>*> menuItems;
    SelectFromListMenuItem *listItem;

    // 添加重置按钮
    MenuItem<int>* resetItem = new MenuItem<int>();
    resetItem->caption = "重置为默认布局";
    resetItem->value = -0xA1B2C3; // 使用一个特殊值，避免与下拉菜单的索引冲突
    resetItem->isLeaf = true;
    menuItems.push_back(resetItem);

    // 添加菜单宽度设置
    listItem = new SelectFromListMenuItem(MISC_MENU_WIDTH_CAPTIONS, onchange_misc_menu_width_index);
    listItem->wrap = false;
    listItem->caption = "菜单 宽度";
    listItem->value = MenuWidthIndex;
    menuItems.push_back(listItem);

    // 添加菜单左侧偏移量设置
    listItem = new SelectFromListMenuItem(MISC_MENU_LEFT_OFFSET_CAPTIONS, onchange_misc_menu_left_offset_index);
    listItem->wrap = false;
    listItem->caption = "菜单 位置";
    listItem->value = MenuLeftOffsetIndex;
    menuItems.push_back(listItem);

    // 添加菜单顶部偏移量设置
    listItem = new SelectFromListMenuItem(MISC_MENU_TOP_OFFSET_CAPTIONS, onchange_misc_menu_top_offset_index);
    listItem->wrap = false;
    listItem->caption = "标题栏 上下位置";
    listItem->value = MenuTopOffsetIndex;
    menuItems.push_back(listItem);

    // 添加菜单项顶部偏移量设置
    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_TOP_OFFSET_CAPTIONS, onchange_misc_menu_item_top_offset_index);
    listItem->wrap = false;
    listItem->caption = "项目栏 上下位置";
    listItem->value = MenuItemTopOffsetIndex;
    menuItems.push_back(listItem);

    // 添加菜单高度设置
    listItem = new SelectFromListMenuItem(MISC_MENU_HEIGHT_CAPTIONS, onchange_misc_menu_height_index);
    listItem->wrap = false;
    listItem->caption = "标题栏 高度";
    listItem->value = MenuHeightIndex;
    menuItems.push_back(listItem);

    // 添加菜单项高度设置
    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_HEIGHT_CAPTIONS, onchange_misc_menu_item_height_index);
    listItem->wrap = false;
    listItem->caption = "项目栏 高度";
    listItem->value = MenuItemHeightIndex;
    menuItems.push_back(listItem);

    // 添加菜单项间距设置
    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_SPACING_CAPTIONS, onchange_misc_menu_item_spacing_index);
    listItem->wrap = false;
    listItem->caption = "项目栏 间距";
    listItem->value = MenuItemSpacingIndex;
    menuItems.push_back(listItem);

    // 添加菜单文本左侧偏移量设置
    listItem = new SelectFromListMenuItem(MISC_MENU_TEXT_LEFT_OFFSET_CAPTIONS, onchange_misc_menu_text_left_offset_index);
    listItem->wrap = false;
    listItem->caption = "标题文本 左右边距";
    listItem->value = MenuTextLeftOffsetIndex;
    menuItems.push_back(listItem);

    // 添加菜单项文本偏移量设置
    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_TEXT_OFFSET_CAPTIONS, onchange_misc_menu_item_text_offset_index);
    listItem->wrap = false;
    listItem->caption = "项目文本 左右边距";
    listItem->value = MenuItemTextOffsetIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_TOGGLE_ICON_RIGHT_MARGIN_CAPTIONS, onchange_misc_menu_toggle_icon_right_margin_index);
    listItem->wrap = false;
    listItem->caption = "复选框 右侧边距";
    listItem->value = MenuToggleIconRightMarginIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_WANTED_STAR_RIGHT_MARGIN_CAPTIONS, onchange_misc_menu_wanted_star_right_margin_index);
    listItem->wrap = false;
    listItem->caption = "通缉星 右侧边距";
    listItem->value = MenuWantedStarRightMarginIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_MAIN_HEADER_FONT_SCALE_CAPTIONS,     onchange_misc_menu_main_header_font_scale_index);
    listItem->wrap = false;
    listItem->caption = "首页标题 文本大小";
    listItem->value = MenuMainHeaderFontScaleIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_HEADER_FONT_SCALE_CAPTIONS,     onchange_misc_menu_header_font_scale_index);
    listItem->wrap = false;
    listItem->caption = "普通标题 文本大小";
    listItem->value = MenuHeaderFontScaleIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_FONT_SCALE_CAPTIONS,     onchange_misc_menu_item_font_scale_index);
    listItem->wrap = false;
    listItem->caption = "项目栏 文本大小";
    listItem->value = MenuItemFontScaleIndex;
    menuItems.push_back(listItem);

    listItem = new SelectFromListMenuItem(MISC_MENU_WANTED_FONT_SCALE_CAPTIONS,     onchange_misc_menu_wanted_font_scale_index);
    listItem->wrap = false;
    listItem->caption = "通缉星 图标大小";
    listItem->value = MenuWantedFontScaleIndex;
    menuItems.push_back(listItem);

    // 菜单高亮文本放大：默认关闭，用户可按需选择放大量
    listItem = new SelectFromListMenuItem(MISC_MENU_ITEM_HIGHLIGHT_TEXT_SCALE_CAPTIONS, onchange_misc_menu_item_highlight_text_scale_index);
    listItem->wrap = false;
    listItem->caption = "菜单高亮 文本放大";
    listItem->value = MenuItemHighlightTextScaleIndex;
    menuItems.push_back(listItem);

    draw_generic_menu<int>(menuItems, &activeLineIndexMenuLayout, caption, onconfirm_menu_layout_reset, NULL, NULL);
}

// ========================================
// 自由相机模式功能实现
// ========================================

// 自由相机模式回调函数
void onchange_freecam_follow_index(int value, SelectFromListMenuItem* source) {
	FreeCamFollowIndex = value;
	FreeCamFollowChanged = true;
}

void onchange_freecam_fov_index(int value, SelectFromListMenuItem* source) {
	FreeCamFovIndex = value;
	FreeCamFovChanged = true;
	
	// 实时更新相机视野距离
	if (freeCamActive && CAM::DOES_CAM_EXIST(freeCamHandle)) {
		CAM::SET_CAM_FOV(freeCamHandle, MISC_FREECAM_FOV_VALUES[value]);
	}
}

void onchange_freecam_speed_slow_index(int value, SelectFromListMenuItem* source) {
	FreeCamSpeedSlowIndex = value;
	FreeCamSpeedSlowChanged = true;
}

void onchange_freecam_speed_medium_index(int value, SelectFromListMenuItem* source) {
	FreeCamSpeedMediumIndex = value;
	FreeCamSpeedMediumChanged = true;
}

void onchange_freecam_speed_fast_index(int value, SelectFromListMenuItem* source) {
	FreeCamSpeedFastIndex = value;
	FreeCamSpeedFastChanged = true;
}

void onchange_freecam_info_display_index(int value, SelectFromListMenuItem* source) {
	FreeCamInfoDisplayIndex = value;
	FreeCamInfoDisplayChanged = true;
}

void onchange_freecam_crosshair_style_index(int value, SelectFromListMenuItem* source) {
	FreeCamCrosshairStyleIndex = value;
	FreeCamCrosshairStyleChanged = true;
}

void onchange_freecam_crosshair_color_index(int value, SelectFromListMenuItem* source) {
	FreeCamCrosshairColorIndex = value;
	FreeCamCrosshairColorChanged = true;
}

// 绘制自由相机十字准星
void draw_freecam_crosshair() {
	if (MISC_FREECAM_CROSSHAIR_STYLE_VALUES[FreeCamCrosshairStyleIndex] == 0) {
		return; // 不显示十字准星
	}

	// 中心点
	float centerX = 0.5f;
	float centerY = 0.5f;

	// 尺寸（像素）
	float lengthPx = 40.0f;//十字准星长度
	float thickPx = 2.0f;//十字准星厚度

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
	switch (FreeCamCrosshairColorIndex) {
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

	bool dashed = (MISC_FREECAM_CROSSHAIR_STYLE_VALUES[FreeCamCrosshairStyleIndex] == 2);

	if (!dashed) {
		// 实线十字准星
		GRAPHICS::DRAW_RECT(centerX, centerY, lengthX, thickY, r, g, b, a); // 水平线
		GRAPHICS::DRAW_RECT(centerX, centerY, thickX, lengthY, r, g, b, a); // 垂直线
	} else {
		// 虚线十字准星
		int segments = 6;//虚线十字准星线段数量
		float gapPx = 4.0f;//虚线十字准星线段间距
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
	}
}

// 重置自由相机设置为默认值
void reset_freecam_settings_to_defaults() {
	featureFreeCamEnabled = true;
	FreeCamFollowIndex = 0; // 跟随（顺序：跟随0，不跟随1）
	FreeCamFovIndex = 9; // 50
	FreeCamSpeedSlowIndex = 0; // 0.35
	FreeCamSpeedMediumIndex = 10; // 1.0
	FreeCamSpeedFastIndex = 14; // 3.0
	FreeCamInfoDisplayIndex = 0; // 底部显示（顺序：底部0，顶部1，不显示2）
	FreeCamCrosshairStyleIndex = 0; // 不显示（0=不显示,1=实线,2=虚线）
	FreeCamCrosshairColorIndex = 0; // 白色（0=白色,1=红色,2=粉红色,3=绿色,4=蓝色,5=黄色,6=橙色,7=紫色,8=黑色,9=灰色）
	
	FreeCamFollowChanged = true;
	FreeCamFovChanged = true;
	FreeCamSpeedSlowChanged = true;
	FreeCamSpeedMediumChanged = true;
	FreeCamSpeedFastChanged = true;
	FreeCamInfoDisplayChanged = true;
	FreeCamCrosshairStyleChanged = true;
	FreeCamCrosshairColorChanged = true;
}

// 自由相机设置菜单确认处理
bool onconfirm_freecam_menu(MenuItem<int> choice) {
	if (choice.value == -1) { // 触发开关自由相机模式
		// 只有当总开关启用时，才能触发自由相机
		if (!featureFreeCamEnabled) {
			return false; // 总开关未启用，不执行任何操作，静默返回
		}
		
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (freeCamActive) {
			deactivate_freecam(playerPed);
		} else {
			activate_freecam(playerPed);
		}
		return false; // 不退出菜单
	}
	return false;
}

// 自由相机设置菜单
void process_misc_freecam_menu() {
	const std::string caption = "自由相机模式";
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	MenuItem<int>* item;
	
	// 启用自由相机
	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用";
	toggleItem->toggleValue = &featureFreeCamEnabled;
	menuItems.push_back(toggleItem);
	
	// 触发开关自由相机模式
	item = new MenuItem<int>();
	item->caption = "自由相机模式 [默认 F7 开/关 ]";
	item->value = -1;
	item->isLeaf = true;
	menuItems.push_back(item);
	
	// 玩家跟随相机（顺序改为：跟随，不跟随）
	listItem = new SelectFromListMenuItem(MISC_FREECAM_FOLLOW_CAPTIONS, onchange_freecam_follow_index);
	listItem->wrap = false;
	listItem->caption = "玩家跟随相机";
	listItem->value = FreeCamFollowIndex;
	menuItems.push_back(listItem);
	
	// 相机视野距离（FOV - 控制远近镜头）
	listItem = new SelectFromListMenuItem(MISC_FREECAM_FOV_CAPTIONS, onchange_freecam_fov_index);
	listItem->wrap = false;
	listItem->caption = "相机视野范围";
	listItem->value = FreeCamFovIndex;
	menuItems.push_back(listItem);
	
	// 相机移动速度-慢速
	listItem = new SelectFromListMenuItem(MISC_FREECAM_SPEED_CAPTIONS, onchange_freecam_speed_slow_index);
	listItem->wrap = false;
	listItem->caption = "相机移动速度-慢速";
	listItem->value = FreeCamSpeedSlowIndex;
	menuItems.push_back(listItem);
	
	// 相机移动速度-中速
	listItem = new SelectFromListMenuItem(MISC_FREECAM_SPEED_CAPTIONS, onchange_freecam_speed_medium_index);
	listItem->wrap = false;
	listItem->caption = "相机移动速度-中速";
	listItem->value = FreeCamSpeedMediumIndex;
	menuItems.push_back(listItem);
	
	// 相机移动速度-快速
	listItem = new SelectFromListMenuItem(MISC_FREECAM_SPEED_CAPTIONS, onchange_freecam_speed_fast_index);
	listItem->wrap = false;
	listItem->caption = "相机移动速度-快速";
	listItem->value = FreeCamSpeedFastIndex;
	menuItems.push_back(listItem);
	
	// 自由相机信息显示（顺序改为：底部显示，顶部显示，不显示）
	listItem = new SelectFromListMenuItem(MISC_FREECAM_INFO_DISPLAY_CAPTIONS, onchange_freecam_info_display_index);
	listItem->wrap = false;
	listItem->caption = "自由相机信息显示";
	listItem->value = FreeCamInfoDisplayIndex;
	menuItems.push_back(listItem);

	// 中心十字显示（顺序改为：不显示,实线,虚线）
	listItem = new SelectFromListMenuItem(MISC_FREECAM_CROSSHAIR_STYLE_CAPTIONS, onchange_freecam_crosshair_style_index);
	listItem->wrap = false;
	listItem->caption = "中心十字显示";
	listItem->value = FreeCamCrosshairStyleIndex;
	menuItems.push_back(listItem);

	// 中心十字颜色
	listItem = new SelectFromListMenuItem(MISC_FREECAM_CROSSHAIR_COLOR_CAPTIONS, onchange_freecam_crosshair_color_index);
	listItem->wrap = false;
	listItem->caption = "中心十字颜色";
	listItem->value = FreeCamCrosshairColorIndex;
	menuItems.push_back(listItem);
	
	draw_generic_menu<int>(menuItems, &activeLineIndexFreeCam, caption, onconfirm_freecam_menu, NULL, NULL);
}

// 停用自由相机
void deactivate_freecam(Ped playerPed) {
	if (!freeCamActive) return;
	
	// 销毁相机
	if (CAM::DOES_CAM_EXIST(freeCamHandle)) {
		CAM::RENDER_SCRIPT_CAMS(false, false, 0, false, false);
		CAM::SET_CAM_ACTIVE(freeCamHandle, false);
		CAM::DESTROY_CAM(freeCamHandle, true);
		freeCamHandle = NULL;
	}
	
	// 恢复玩家状态
	Entity ent = PED::IS_PED_IN_ANY_VEHICLE(playerPed, false) ? PED::GET_VEHICLE_PED_IS_IN(playerPed, false) : playerPed;
	
	ENTITY::SET_ENTITY_VISIBLE(ent, freeCamPlayerWasVisible);
	ENTITY::SET_ENTITY_VISIBLE(playerPed, freeCamPlayerWasVisible);
	ENTITY::SET_ENTITY_COLLISION(ent, freeCamPlayerHadCollision, true);
	ENTITY::FREEZE_ENTITY_POSITION(ent, false);
	
	// 重新设置当前坐标以触发物理引擎重新计算，解决车辆冻结问题
	Vector3 currentPos = ENTITY::GET_ENTITY_COORDS(ent, false);
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ent, currentPos.x, currentPos.y, currentPos.z, 1, 1, 1);
	
	// 恢复玩家透明度（完全不透明）
	ENTITY::RESET_ENTITY_ALPHA(playerPed);
	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
		ENTITY::RESET_ENTITY_ALPHA(ent);
	}
	
	// 重新启用所有被禁用的控制
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_VEH_HORN, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_LOOK_BEHIND, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_VEH_LOOK_BEHIND, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_SELECT_WEAPON, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_VEH_ACCELERATE, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_VEH_BRAKE, TRUE);
	CONTROLS::ENABLE_CONTROL_ACTION(2, INPUT_VEH_RADIO_WHEEL, TRUE);
	
	// 清除游戏焦点设置，恢复默认的环境加载行为
	STREAMING::CLEAR_FOCUS();
	
	// 标记自由相机已关闭（保持速度模式不重置，下次打开继续使用上次的速度）
	freeCamActive = false;
	//currentSpeedMode = 0;//保持当前值，不重置为0
	
	// 恢复菜单显示
	set_menu_showing(true);
}

// 激活自由相机
void activate_freecam(Ped playerPed) {
	if (freeCamActive) return;
	
	// 如果物体摆放模式已激活，则无效化自由相机激活（防止功能冲突）
	if (is_in_prop_placement_mode()) {
		return; // 静默返回，不显示提示
	}
	
	// 如果自由移动模式已激活，先关闭它
	if (is_in_airbrake_mode()) {
		exit_airbrake_menu_if_showing();
		WAIT(50); // 等待自由移动模式完全关闭
	}
	
	Entity ent = PED::IS_PED_IN_ANY_VEHICLE(playerPed, false) ? PED::GET_VEHICLE_PED_IS_IN(playerPed, false) : playerPed;
	
	// 记录玩家当前状态
	freeCamPlayerWasVisible = ENTITY::IS_ENTITY_VISIBLE(ent);
	freeCamPlayerHadCollision = true; // 默认有碰撞
	
	// 创建相机
	Vector3 camPos = CAM::GET_GAMEPLAY_CAM_COORD();
	Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	float fov = MISC_FREECAM_FOV_VALUES[FreeCamFovIndex];
	
	freeCamHandle = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_CAMERA", camPos.x, camPos.y, camPos.z, 
												  camRot.x, camRot.y, camRot.z, fov, true, 2);
	
	if (CAM::DOES_CAM_EXIST(freeCamHandle)) {
		CAM::SET_CAM_ACTIVE(freeCamHandle, true);
		CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false);
		
		// 设置玩家状态 - 隐藏并禁用碰撞
		ENTITY::SET_ENTITY_VISIBLE(ent, false);
		ENTITY::SET_ENTITY_VISIBLE(playerPed, false);
		ENTITY::SET_ENTITY_COLLISION(ent, false, false);
		ENTITY::FREEZE_ENTITY_POSITION(ent, true);
		
		// 使用透明度增强隐藏效果（防止在某些情况下玩家可见）
		ENTITY::SET_ENTITY_ALPHA(playerPed, 0, false);
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
			ENTITY::SET_ENTITY_ALPHA(ent, 0, false);
		}
		
		// 标记自由相机已关闭（保持速度模式不重置，下次打开继续使用上次的速度）
		freeCamActive = true;
		//currentSpeedMode = 0;//保持当前值，不重置为0
		
		// 隐藏菜单
		set_menu_showing(false);
		
		set_status_text("自由相机模式已激活!\n按 ~y~Shift ~s~切换移动速度!");
	}
}

// 更新自由相机
void update_freecam_features(BOOL playerExists, Ped playerPed) {
	if (!featureFreeCamEnabled) {
		if (freeCamActive) {
			deactivate_freecam(playerPed);
		}
		return;
	}
	
	// 检查热键（从其他按键设置中获取）
	if (OtherKeyFreeCamToggleIndex > 0 && OtherKeyFreeCamToggleIndex < sizeof(MISC_HOTKEY_VALUES)/sizeof(int)) {
		int keyValue = MISC_HOTKEY_VALUES[OtherKeyFreeCamToggleIndex];
		if (IsKeyJustUp(keyValue)) {
			if (freeCamActive) {
				deactivate_freecam(playerPed);
			} else {
				activate_freecam(playerPed);
			}
		}
	}
	
	if (!freeCamActive || !CAM::DOES_CAM_EXIST(freeCamHandle)) return;
	
	// 禁用某些控制
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_VEH_HORN, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_LOOK_BEHIND, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_VEH_LOOK_BEHIND, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_SELECT_WEAPON, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_VEH_ACCELERATE, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_VEH_BRAKE, TRUE);
	CONTROLS::DISABLE_CONTROL_ACTION(2, INPUT_VEH_RADIO_WHEEL, TRUE);
	
	// 检查Shift键切换速度模式
	bool currentShiftState = IsKeyDown(VK_SHIFT);
	if (currentShiftState && !lastShiftState) {
		currentSpeedMode = (currentSpeedMode + 1) % 3;
		const char* speedNames[] = { "~y~慢速", "~y~中速", "~y~快速" };
		set_status_text(std::string("速度模式: ") + speedNames[currentSpeedMode]);
	}
	lastShiftState = currentShiftState;
	
	// 获取当前速度
	float speed;
	switch(currentSpeedMode) {
		case 1: speed = MISC_FREECAM_SPEED_VALUES[FreeCamSpeedMediumIndex]; break;
		case 2: speed = MISC_FREECAM_SPEED_VALUES[FreeCamSpeedFastIndex]; break;
		default: speed = MISC_FREECAM_SPEED_VALUES[FreeCamSpeedSlowIndex]; break;
	}
	
	// 【鼠标控制】更新相机旋转 - 鼠标移动改变视角
	Vector3 currentRot = CAM::GET_CAM_ROT(freeCamHandle, 2);
	float mouseDeltaX = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, INPUT_LOOK_LR);  // 鼠标左右
	float mouseDeltaY = CONTROLS::GET_DISABLED_CONTROL_NORMAL(0, INPUT_LOOK_UD);  // 鼠标上下
	
	// 根据鼠标移动更新相机旋转角度
	Vector3 nextRot;
	nextRot.x = currentRot.x - mouseDeltaY * 11.0f;
	
	// 限制俯仰角范围，防止万向锁导致视角晃动（限制在 -89° 到 +89° 之间）
	if (nextRot.x > 89.0f) nextRot.x = 89.0f;
	if (nextRot.x < -89.0f) nextRot.x = -89.0f;
	
	nextRot.y = 0.0f; // 不允许翻滚
	nextRot.z = currentRot.z - mouseDeltaX * 11.0f;
	CAM::SET_CAM_ROT(freeCamHandle, nextRot.x, nextRot.y, nextRot.z, 2);
	
	// 【键盘控制】相对于相机视角方向移动
	// WASD控制前后左右（相对于相机朝向）
	// Q/空格 上升，E/Ctrl 下降
	float offsetX = CONTROLS::GET_CONTROL_NORMAL(0, INPUT_MOVE_LR) * speed;      // A/D 左右
	float offsetY = -CONTROLS::GET_CONTROL_NORMAL(0, INPUT_MOVE_UD) * speed;     // W/S 前后
	
	// Q/E 或 空格/Ctrl 控制上下
	float offsetZ = 0.0f;
	if (IsKeyDown(VK_KEY_Q) || IsKeyDown(VK_SPACE)) offsetZ = speed;
	if (IsKeyDown(VK_KEY_E) || IsKeyDown(VK_CONTROL)) offsetZ = -speed;
	
	if (offsetX != 0.0f || offsetY != 0.0f || offsetZ != 0.0f) {
		// 获取相机当前位置
		Vector3 currentPos = CAM::GET_CAM_COORD(freeCamHandle);
		
		// 计算相对于相机朝向的世界坐标偏移
		// 参考MenyooSP: cam.GetOffsetInWorldCoords(offset)
		float rotZ = nextRot.z * 0.0174532925f; // 转为弧度
		float rotX = nextRot.x * 0.0174532925f;
		
		// 前进方向（相机朝向）
		float forwardX = -sin(rotZ) * cos(rotX);
		float forwardY = cos(rotZ) * cos(rotX);
		float forwardZ = sin(rotX);
		
		// 右方向（相机右侧）
		float rightX = cos(rotZ);
		float rightY = sin(rotZ);
		
		// 计算新位置：当前位置 + 前进方向*前后偏移 + 右方向*左右偏移 + 垂直偏移
		float newPosX = currentPos.x + forwardX * offsetY + rightX * offsetX;
		float newPosY = currentPos.y + forwardY * offsetY + rightY * offsetX;
		float newPosZ = currentPos.z + forwardZ * offsetY + offsetZ;
		
		// 更新相机位置
		CAM::SET_CAM_COORD(freeCamHandle, newPosX, newPosY, newPosZ);
		
		// 如果启用"玩家跟随相机"，同步更新玩家位置
		if (MISC_FREECAM_FOLLOW_VALUES[FreeCamFollowIndex] == 0) {
			Entity ent = PED::IS_PED_IN_ANY_VEHICLE(playerPed, false) ? PED::GET_VEHICLE_PED_IS_IN(playerPed, false) : playerPed;
			ENTITY::SET_ENTITY_COORDS(ent, newPosX, newPosY, newPosZ, false, false, false, true);
		}
	}
	
	// 【环境加载】在不跟随模式下，强制加载相机位置的周围环境
	if (MISC_FREECAM_FOLLOW_VALUES[FreeCamFollowIndex] == 1) { // 1=不跟随
		Vector3 camPos = CAM::GET_CAM_COORD(freeCamHandle);
		// 设置游戏焦点到相机位置，强制加载该位置周围的环境
		// 注意：玩家位置的环境会自动保持加载状态，不会被卸载
		STREAMING::_SET_FOCUS_AREA(camPos.x, camPos.y, camPos.z, 0.0f, 0.0f, 0.0f);
	} else { // 0=跟随
		// 清除焦点设置，让游戏自动根据玩家位置加载环境
		STREAMING::CLEAR_FOCUS();
	}

	// 绘制自由相机十字准星
	draw_freecam_crosshair();

	// 显示屏幕信息（居中+黑色半透明背景）
	int displayValue = MISC_FREECAM_INFO_DISPLAY_VALUES[FreeCamInfoDisplayIndex];
	if (displayValue != 2) { // 0=底部, 1=顶部, 2=不显示, 
		const char* followText = MISC_FREECAM_FOLLOW_VALUES[FreeCamFollowIndex] == 0 ? "跟随" : "不跟随";
		const char* speedNames[] = { "慢速", "中速", "快速" };
		const char* hotkeyName = OtherKeyFreeCamToggleIndex < MISC_HOTKEY_CAPTIONS.size() ? MISC_HOTKEY_CAPTIONS[OtherKeyFreeCamToggleIndex].c_str() : "未绑定";
		
		char infoText[256];
		sprintf_s(infoText, "自由相机模式  |  热键: %s  |  速度: %s  |  视野: %.0f  |  %s", 
				  hotkeyName, speedNames[currentSpeedMode], MISC_FREECAM_FOV_VALUES[FreeCamFovIndex], followText);
		
		// displayValue: 0=底部, 1=顶部
		float yPos = (displayValue == 0) ? 0.95f : 0.025f;// 屏幕底部 - 顶部位置
		
		// 绘制黑色半透明背景（居中）
		GRAPHICS::DRAW_RECT(0.5f, yPos + 0.012f, 0.40f, 0.046f, 0, 0, 0, 180);
		
		// 绘制文本（居中显示）
		UI::SET_TEXT_FONT(fontStatus);
		UI::SET_TEXT_SCALE(0.0, 0.38);
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_OUTLINE();
		UI::SET_TEXT_CENTRE(1); // 居中对齐
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_STRING(infoText);
		UI::_DRAW_TEXT(0.5f, yPos);
	}
}

