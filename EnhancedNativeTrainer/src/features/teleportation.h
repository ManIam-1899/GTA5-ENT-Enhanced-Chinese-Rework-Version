/*
这段代码的部分最初来源于 GTA V SCRIPT HOOK SDK。
http://dev-c.com
(C) Alexander Blade 2015

它现在已成为 Enhanced Native Trainer 项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/

#pragma once

#include "..\..\inc\natives.h"
#include "..\..\inc\types.h"
#include "..\..\inc\enums.h"
#include "..\..\inc\main.h"
#include <string>
#include "..\ui_support\menu_functions.h"
#include "..\storage\database.h"
#include "..\ent-enums.h"
#include "..\utils.h"
#include "..\common\ENTUtil.h"
#include <random>

bool process_teleport_menu(int categoryIndex);

void reset_teleporter_globals();

void teleport_to_marker();

void teleport_to_mission_marker();

void teleport_to_vehicle_as_passenger();

void teleport_to_last_vehicle();

void teleport_forward();

void handle_generic_settings_teleportation(std::vector<StringPairSettingDBRow>* settings);

void add_coords_generic_settings(std::vector<StringPairSettingDBRow>* results);

void onchange_tel_chauffeur_index(int value, SelectFromListMenuItem *source);

void onchange_tel_chauffeur_speed_index(int value, SelectFromListMenuItem *source);

void onchange_tel_chauffeur_altitude_index(int value, SelectFromListMenuItem *source);

void onchange_tel_chauffeur_drivingstyles_index(int value, SelectFromListMenuItem *source);

void onchange_tel_3dmarker_index(int value, SelectFromListMenuItem *source);

void onchange_tel_3dmarker_martype_index(int value, SelectFromListMenuItem *source);

Vector3 get_blip_marker();

void update_teleport_features();

// 加载指定坐标的地面高度（优化版）
bool load_ground_at_3dcoord(Vector3& location);

// 优先级1优化：使用 SET_PED_COORDS_KEEP_VEHICLE 的传送函数
void teleport_to_coords(Vector3 coords);
