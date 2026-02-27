#pragma once
#include <vector>
#include <string>
#include "script.h" // 提供 Vector3 类型定义
#include "..\storage\database.h" // 引入 StringPairSettingDBRow 类型

extern bool featureKMH;
extern bool featureAltitude;
extern bool featureSpeedOnFoot;
extern bool featureSpeedOnGround;
extern bool featureSpeedInAir;
// 速度/高度显示 总开关及更新标志
extern bool featureSpeedAltitudeMaster;
extern bool featureSpeedAltitudeMasterUpdated;
// 子项更新标志用于互斥逻辑
extern bool featureSpeedOnFootUpdated;
extern bool featureSpeedOnGroundUpdated;
extern bool featureSpeedInAirUpdated;

//速度表大小
const int SPEED_SIZE_VALUES[] = { 3, 3, 5, 7, 10, 12, 15, 17, 20, 23 };
extern int SpeedSizeIndex;
extern bool SizeChanged;

//速度表位置
const std::vector<std::string> SPEED_POSITION_CAPTIONS{ "右下角", "底部中心", "右上角" };
extern int SpeedPositionIndexN;
extern bool PositionChanged;

void update_speed_text(int speed, Vector3 player_coords);

// ===== 模拟速度表：菜单与绘制状态 =====
extern bool featureAnalogSpeedometerEnabled;
extern bool featureAnalogSpeedometerEnabledUpdated;
extern int analogSpeedoStyleIndex; // 0 默认样式
extern float analogSpeedoPosX; // 归一化屏幕坐标 (0..1)
extern float analogSpeedoPosY; // 归一化屏幕坐标 (0..1)
extern int analogSpeedoPixelSize; // 像素尺寸 128..512
extern int analogSpeedoPresetIndex; // 0 右下, 1 左下, 2 左上, 3 右上, 4 自定义
extern int analogSpeedoUnitIndex; // 0 千米, 1 英里
extern bool analogShowDigitalSpeed; // 显示数字速度
extern bool analogShowVehicleHealth; // 显示车辆健康值
extern bool analogShowAltitude; // 显示海拔高度

// 子菜单入口
void process_analog_speedometer_menu();

// 在速度/高度更新循环末尾追加模拟速度表绘制（前置声明）
void update_speedaltitude_append_analog(Ped playerPed);

// 重置速度/高度与模拟速度表的所有相关全局变量
void reset_speed_altitude_globals();

// 通用设置：保存/加载（用于自动保存/自动加载）
void add_speed_altitude_generic_settings(std::vector<StringPairSettingDBRow>* results);
void handle_generic_settings_speed_altitude(std::vector<StringPairSettingDBRow>* settings);
