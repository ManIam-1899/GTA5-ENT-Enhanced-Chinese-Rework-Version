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
#include <string>
#include <vector>
#include "..\storage\database.h"

extern bool featureFreezeTime;
extern bool featureFreezeTimeUpdated;
extern bool featureTimeSyncedUpdated;

// 模拟时钟设置
extern bool featureAnalogClockEnabled;
extern int analogClockStyleIndex; // 默认时钟
extern int analogClockTimeSourceIndex; // 0: 游戏时间, 1: 现实时间
extern float analogClockPosX; // 屏幕坐标 0..1, 中心点
extern float analogClockPosY; // 屏幕坐标 0..1, 中心点
extern bool analogClockShowLabel; // 显示"现实时间/游戏时间"标签
extern bool analogClockShowDigital; // 显示数字时间
extern bool analogClockShowDate; // 显示日期
extern int analogClockPixelSize; // 时钟贴图尺寸（像素），按屏幕分辨率1:1绘制

void process_time_menu();

void reset_time_globals();

void update_time_features(Player player);

void add_time_feature_enablements(std::vector<FeatureEnabledLocalDefinition>* results);

void add_time_generic_settings(std::vector<StringPairSettingDBRow>* results);

void handle_generic_settings_time(std::vector<StringPairSettingDBRow>* settings);

void all_time_flow_rate();

// 模拟时钟菜单
void process_analog_clock_menu();

void movetime_day_forward();

void movetime_day_backward();

void set_date();

void set_time();

void toggle_game_speed();

void movetime_hour_forward();

void movetime_hour_backward();

void movetime_fivemin_forward();

void movetime_fivemin_backward();

void movetime_set(int hour, int minute);

std::string get_day_of_game_week();

bool flowtime_menu_interrupt();