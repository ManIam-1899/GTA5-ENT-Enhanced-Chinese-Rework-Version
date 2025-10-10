/*
部分代码最初来源于 GTA V SCRIPT HOOK SDK。
http://dev-c.com
(C) Alexander Blade 2015

现为增强版原生训练器项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/

#include "vehicles.h"
#include "speed_altitude.h"
#include "..\features\vehmodmenu.h"
#include "hotkeys.h"
#include "script.h"
#include "..\ui_support\menu_functions.h"
#include "..\io\config_io.h"
#include "..\debug\debuglog.h"
#include "area_effect.h"
#include <fstream>
#include "vehicle_weapons.h"
#include <string>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include <vector>
#include <cstdlib>
#include <cmath>

bool featureKMH = false;
bool featureAltitude = false; // 旧海拔高度默认关闭
bool featureSpeedOnFoot = false;
bool featureSpeedOnGround = false;
bool featureSpeedInAir = false;
// 速度/高度显示 总开关及更新标志
bool featureSpeedAltitudeMaster = false;
bool featureSpeedAltitudeMasterUpdated = false;
// 子项更新标志（用于互斥逻辑）
bool featureSpeedOnFootUpdated = false;
bool featureSpeedOnGroundUpdated = false;
bool featureSpeedInAirUpdated = false;

int SpeedSizeIndex = 0;
bool SizeChanged = true;
int SpeedPositionIndexN = 0;
bool PositionChanged = true;

float textX, textY = -1;
float rectXScaled, rectYScaled = -1;

//////////////////////////////////////////// 显示速度 / 高度 ///////////////////////////////////////////
void update_speed_text(int speed, Vector3 player_coords)
{
	std::string speedometerStatusLines[1];
	std::stringstream ss;
	int col2_R = ENTColor::colsMenu[5].rgba[0];
	int col2_G = ENTColor::colsMenu[5].rgba[1];
	int col2_B = ENTColor::colsMenu[5].rgba[2];

	// 速度高度字体颜色：将字体颜色改为白色（原为深橙色）
	col2_R = 255;
	col2_G = 242;
	col2_B = 0;

	int numLines = sizeof(speedometerStatusLines) / sizeof(speedometerStatusLines[0]);

	if (featureKMH) {
		ss << "千米/时:    " << round((speed * 1.609344) * 2.3);

		if (featureAltitude) {
			ss << "\n海拔高度:  " << floor(player_coords.z * 1) / 1;
		}
	}
	else {
		ss << "英里/时:    " << round(speed * 2.3);

		if (featureAltitude) {
			ss << "\n海拔高度:  " << floor(player_coords.z * 1) / 1;
		}
	}

	int index = 0;
	speedometerStatusLines[index++] = ss.str();
	float size = SPEED_SIZE_VALUES[SpeedSizeIndex];
	int screen_w, screen_h;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);

	if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 0) { // 右下角
		textX = (97.4 - (size * 2.5)) / 100;
		textY = (85 - (size * 1.2)) / 100;
	}
	if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 1) { // 底部居中
		textX = (50 - (size * 1.1)) / 100;
		textY = (95 - (size * 1.2)) / 100;
	}
	if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 2) { // 右上角
		textX = (97.4 - (size * 2.5)) / 100;
		textY = (10.5 + (size * 0.0001)) / 100;
	}

	int numActualLines = 0;
	for (int i = 0; i < numLines; i++) {
		numActualLines++;
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM((char *)speedometerStatusLines[i].c_str());
		text_parameters(size / 10, size / 10, col2_R, col2_G, col2_B, 255);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(textX, textY);
		textY += 0.025f;
	}

	//if (size < 4) { // 绘制背景
	if (SpeedSizeIndex < 1) { // 绘制背景
		if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 0) { // 右下角
			rectXScaled = 1 - ((300 / (float)screen_w) / 4);
			rectYScaled = 0.95 - (((0 + (1 * 18)) / (float)screen_h) * 5);
		}
		if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 1) { // 底部居中
			rectXScaled = 0.55 - ((230 / (float)screen_w) / 4);
			rectYScaled = 1 - (((0 + (1 * 11)) / (float)screen_h) * 5);
		}
		if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 2) { // 右上角
			rectXScaled = 1 - ((300 / (float)screen_w) / 4);
			rectYScaled = 0.24 - (((0 + (1 * 18)) / (float)screen_h) * 5);
		}
		float rectWidthScaled = (230 / (float)screen_w) / 2;
		float rectHeightScaled = (0 + (1 * 18)) / (float)screen_h;
		int rect_col[4] = { 0, 0, 0, 180 }; // 128, 128, 128, 75   速度显示 背景透明度
		GRAPHICS::DRAW_RECT(rectXScaled, rectYScaled, rectWidthScaled, rectHeightScaled, rect_col[0], rect_col[1], rect_col[2], rect_col[3]);

		if (featureAltitude) {
			if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 0) { // 右下角
				rectXScaled = 1 - ((300 / (float)screen_w) / 4);
				rectYScaled = 0.95 - (((0 + (1 * 18)) / (float)screen_h) * 5) + ((0 + (1 * 18)) / (float)screen_h);
			}
			if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 1) { // 底部居中
				rectXScaled = 0.55 - ((230 / (float)screen_w) / 4);
				rectYScaled = 1 - (((0 + (1 * 11)) / (float)screen_h) * 5) + ((0 + (1 * 18)) / (float)screen_h);
			}
			if (NPC_RAGDOLL_VALUES[SpeedPositionIndexN] == 2) { // 右上角
				rectXScaled = 1 - ((300 / (float)screen_w) / 4);
				rectYScaled = 0.24 - (((0 + (1 * 18)) / (float)screen_h) * 5) + ((0 + (1 * 18)) / (float)screen_h);
			}
			float rectWidthScaled = (230 / (float)screen_w) / 2;
			float rectHeightScaled = (0 + (1 * 18)) / (float)screen_h;
			int rect_col[4] = { 0, 0, 0, 180 }; // 128, 128, 128, 75   高度显示 背景透明度
			GRAPHICS::DRAW_RECT(rectXScaled, rectYScaled, rectWidthScaled, rectHeightScaled, rect_col[0], rect_col[1], rect_col[2], rect_col[3]);
		}
	}
}

void update_speedaltitude(Ped playerPed) {
	// 总开关：关闭则不显示旧的速度/高度文本
	if(!featureSpeedAltitudeMaster) return;

	// 步行
	if (featureSpeedOnFoot) {
		if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
			update_speed_text(ENTITY::GET_ENTITY_SPEED(playerPed), ENTITY::GET_ENTITY_COORDS(playerPed, true));
		}
	}

	// 在地面上
	if (featureSpeedOnGround) {
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
			Entity veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (!is_this_a_heli_or_plane(veh)) {
				update_speed_text(ENTITY::GET_ENTITY_SPEED(veh), ENTITY::GET_ENTITY_COORDS(playerPed, true));
			}
		}
	}

	// 在空中
	if (featureSpeedInAir) {
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) {
			Entity veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			if (is_this_a_heli_or_plane(veh)) {
				update_speed_text(ENTITY::GET_ENTITY_SPEED(veh), ENTITY::GET_ENTITY_COORDS(playerPed, true));
			}
		}
	}
}


//////////////////////////////////////////// 模拟速度表：菜单与渲染 //////////////////////////////////////////

bool featureAnalogSpeedometerEnabled = false;
bool featureAnalogSpeedometerEnabledUpdated = false;
int analogSpeedoStyleIndex = 0; // 0 默认, 1 风格1, 2 风格2, 3 风格3
float analogSpeedoPosX = 0.90f; // 右上默认（与时钟保持一致初始，后由预设控制）
float analogSpeedoPosY = 0.85f; // 右下为常见预设，菜单“预设位置”会覆盖此值
int analogSpeedoPixelSize = 256; // 默认 256 像素
int analogSpeedoPresetIndex = 0; // 0 右下, 1 左下, 2 左上, 3 右上, 4 自定义
int analogSpeedoUnitIndex = 0; // 0 千米, 1 英里
bool analogShowDigitalSpeed = false; // 默认关闭数字速度显示（需求）
bool analogShowVehicleHealth = false;
bool analogShowAltitude = false;

static SelectFromListMenuItem* gASPosXItem = nullptr;
static SelectFromListMenuItem* gASPosYItem = nullptr;
static SelectFromListMenuItem* gASPresetItem = nullptr;

static const std::vector<std::string> ANALOG_SPEEDO_STYLE_CAPTIONS{ "默认", "风格 1", "风格 2", "风格 3", "风格 4", "风格 5", "风格 6", "风格 7", "风格 8", "风格 9", "风格 10", "风格 11", "风格 12", "风格 13" };
static const std::vector<std::string> ANALOG_SPEEDO_UNIT_CAPTIONS{ "千米", "英里" };
static const std::vector<int> ANALOG_PIXEL_SIZES{ 128,160,192,224,256,288,320,352,384,416,448,480,512 };

// 每种样式在各单位下的最大刻度（用于将速度线性映射到 0..270°）
// 根据你的规范：KM/H 与 MPH 的最大刻度统一为 270。
static const int ANALOG_STYLE_MAX_KMH[14] = { 270,270,270,270,270,270,270,270,270,270,270,270,270,270 };
static const int ANALOG_STYLE_MAX_MPH[14] = { 270,270,270,270,270,270,270,270,270,270,270,270,270,270 };

static inline void release_speedo_textures(){
	const char* dictName = "ENT_textures";
	if(GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictName)){
		GRAPHICS::SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED((char*)dictName);
	}
}

static inline void ensure_speedo_textures_loaded(){
	static bool s_prev_enabled = false;
	if(s_prev_enabled != featureAnalogSpeedometerEnabled){
		s_prev_enabled = featureAnalogSpeedometerEnabled;
		if(!featureAnalogSpeedometerEnabled){
			release_speedo_textures();
			return;
		}else{
			const char* dictWarm = "ENT_textures";
			GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictWarm, false);
			for(int i=0;i<3;++i){
				if(GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictWarm)) break;
				WAIT(0);
				GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictWarm, false);
			}
		}
	}
	if(!featureAnalogSpeedometerEnabled) return;
	const char* dictName = "ENT_textures";
	if(GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictName)) return;
	GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictName, false);
}

static inline void draw_analog_speedometer(Ped playerPed){
	if(!featureAnalogSpeedometerEnabled) return;
	if(!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0)) return;
	ensure_speedo_textures_loaded();
	const char* dict = "ENT_textures";
	// 贴图存在性检查：即使纹理未加载也允许文本渲染
	auto has_texture = [](const char* d, const char* n) -> bool {
		Vector3 res = GRAPHICS::GET_TEXTURE_RESOLUTION((char*)d, (char*)n);
		return (res.x > 0.0f && res.y > 0.0f);
	};

	Entity veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	// 参考 Menyoo：使用车辆前向速度分量并转换为 KM/H；根据单位可转为 MPH
	float speedf = fabsf(3.6f * ENTITY::GET_ENTITY_SPEED_VECTOR(veh, true).y);
    if(analogSpeedoUnitIndex==1){
        // MPH（提高转换精度）
        speedf *= 0.621371f;
    }

	int screenW=0, screenH=0; GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&screenW, &screenH);
	float onePixelW = screenW>0 ? (1.0f/(float)screenW) : 0.0005f;
	float onePixelH = screenH>0 ? (1.0f/(float)screenH) : 0.0005f;
	int pxSize = analogSpeedoPixelSize; if(pxSize<128) pxSize=128; else if(pxSize>512) pxSize=512;
	float sizeX = pxSize * onePixelW;
	float sizeY = pxSize * onePixelH;

	// 根据样式选择贴图名
	int style = analogSpeedoStyleIndex; if(style<0) style=0; if(style>13) style=13;
	char baseName[32]; char needleName[32];
	sprintf_s(baseName, "speedo_base%d", style);
	sprintf_s(needleName, "speedo_needle%d", style);

	float cx = analogSpeedoPosX; if(cx<0.0f) cx=0.0f; if(cx>1.0f) cx=1.0f;
	float cy = analogSpeedoPosY; if(cy<0.0f) cy=0.0f; if(cy>1.0f) cy=1.0f;
	// 将中心点对齐到最近的像素，减少采样模糊
	cx = ((float)((int)(cx * screenW + 0.5f))) * onePixelW;
	cy = ((float)((int)(cy * screenH + 0.5f))) * onePixelH;

	// 指针角度：按最大刻度 270 线性映射到 0..270°（顺时针，从“左下45°”起点）
	int vmax = (analogSpeedoUnitIndex==0) ? ANALOG_STYLE_MAX_KMH[style] : ANALOG_STYLE_MAX_MPH[style];
	if(vmax <= 0) vmax = 270;
	float angle = (speedf >= (float)vmax) ? 270.0f : ((speedf / (float)vmax) * 270.0f);

	// 当纹理字典尚未加载时，跳过表盘与指针绘制，避免出现白色占位
	bool dictLoaded = GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dict);
	if(dictLoaded){
		// 绘制表盘（存在性检查）
		if(has_texture(dict, baseName)){
			GRAPHICS::DRAW_SPRITE((char*)dict, baseName, cx, cy, sizeX, sizeY, 0.0f, 255, 255, 255, 255);
		}
		// 绘制指针（存在性检查，绕中心旋转）
		if(has_texture(dict, needleName)){
			GRAPHICS::DRAW_SPRITE((char*)dict, needleName, cx, cy, sizeX, sizeY, angle, 255, 255, 255, 255);
		}
	}

	// 文本显示（参考模拟时钟的字体与像素对齐）
	if(analogShowDigitalSpeed || analogShowVehicleHealth || analogShowAltitude){
		float bottomY = cy + (sizeY * 0.5f);
		float topY    = cy - (sizeY * 0.5f);
		const int textOffsetPx = 6;   // 与表盘的距离
		const int lineSpacingPx = 35;  // 行间距
		float lineX = cx; // 居中对齐，锚点为表盘中心（像素对齐）
		lineX = ((float)((int)(lineX * screenW + 0.5f))) * onePixelW;

		// 需要显示的行数
		int totalLines = (analogShowDigitalSpeed?1:0) + (analogShowVehicleHealth?1:0) + (analogShowAltitude?1:0);
		bool drawBelow = true;
		if ((bottomY + ((textOffsetPx + totalLines*lineSpacingPx) * onePixelH)) > (1.0f - 6*onePixelH)){
			drawBelow = false; // 底部空间不足，移至上方
		}

		// 第一行高度（用于上方绘制避免重叠）
		float firstLineHeight = UI::_GET_TEXT_SCALE_HEIGHT(0.38f, fontStatus);
		float lineY = drawBelow
			? (bottomY + (textOffsetPx * onePixelH))
			: (topY - (textOffsetPx * onePixelH) - firstLineHeight);
		// 像素对齐
		lineY = ((float)((int)(lineY * screenH + 0.5f))) * onePixelH;

		auto draw_line = [&](const char* text){
			UI::SET_TEXT_FONT(fontStatus);
			UI::SET_TEXT_SCALE(0.0, 0.38);
			UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 220);
			UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
			UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
			UI::SET_TEXT_OUTLINE();
			UI::SET_TEXT_CENTRE(1);
			UI::_SET_TEXT_ENTRY("STRING");
			UI::_ADD_TEXT_COMPONENT_SCALEFORM((char*)text);
			UI::_DRAW_TEXT(lineX, lineY);
			// 下一行位置（像素对齐）
			float nextY = drawBelow ? (lineY + (lineSpacingPx * onePixelH)) : (lineY - (lineSpacingPx * onePixelH));
			lineY = ((float)((int)(nextY * screenH + 0.5f))) * onePixelH;
		};

		if(analogShowDigitalSpeed){
			char line[64];
			int spdInt = (int)(speedf + 0.5f);
			if(analogSpeedoUnitIndex==0){ sprintf_s(line, "当前速度: %d Km/h", spdInt); }
			else{ sprintf_s(line, "当前速度: %d mph", spdInt); }
			draw_line(line);
		}
		if(analogShowVehicleHealth){
			int health = (int)ENTITY::GET_ENTITY_HEALTH(veh);
			char line[64];
			sprintf_s(line, "车辆健康: %d", health);
			draw_line(line);
		}
		if(analogShowAltitude){
			Vector3 coords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
			int alt = (int)(coords.z);
			char line[64];
			sprintf_s(line, "海拔高度: %d", alt);
			draw_line(line);
		}
	}
}

static void onchange_analog_speedo_style(int value, SelectFromListMenuItem* source){
	analogSpeedoStyleIndex = value;
}

static void onchange_analog_speedo_unit(int value, SelectFromListMenuItem* source){
	analogSpeedoUnitIndex = value;
}

static void onchange_analog_speedo_pos_x(int value, SelectFromListMenuItem* source){
	analogSpeedoPosX = (value * 5) / 1000.0f;
	analogSpeedoPresetIndex = 4;
	if(gASPresetItem) gASPresetItem->value = 4;
}

static void onchange_analog_speedo_pos_y(int value, SelectFromListMenuItem* source){
	analogSpeedoPosY = (value * 5) / 1000.0f;
	analogSpeedoPresetIndex = 4;
	if(gASPresetItem) gASPresetItem->value = 4;
}

void process_analog_speedometer_menu(){
	std::vector<MenuItem<int>*> menuItems;
	ToggleMenuItem<int>* togItem;
	SelectFromListMenuItem* listItem;

	// 启用模拟速度表
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "启用模拟速度表";
	togItem->value = 0;
	togItem->toggleValue = &featureAnalogSpeedometerEnabled;
	togItem->toggleValueUpdated = &featureAnalogSpeedometerEnabledUpdated;
	menuItems.push_back(togItem);

	// 样式
	listItem = new SelectFromListMenuItem(ANALOG_SPEEDO_STYLE_CAPTIONS, onchange_analog_speedo_style);
	listItem->wrap = false;
	listItem->caption = "模拟速度表样式";
	listItem->value = analogSpeedoStyleIndex;
	menuItems.push_back(listItem);

	// X/Y 位置（0..1000，步进5）
	{
		std::vector<std::string> posx; for(int i=0;i<=1000;i+=5){ posx.push_back(std::to_string(i)); }
		int currentX = (int)(analogSpeedoPosX * 1000.0f + 0.5f);
		SelectFromListMenuItem* posxItem = new SelectFromListMenuItem(posx, onchange_analog_speedo_pos_x);
		posxItem->wrap = false; posxItem->caption = "速度表X位置"; posxItem->value = currentX/5;
		menuItems.push_back(posxItem);
		gASPosXItem = posxItem;
	}
	{
		std::vector<std::string> posy; for(int i=0;i<=1000;i+=5){ posy.push_back(std::to_string(i)); }
		int currentY = (int)(analogSpeedoPosY * 1000.0f + 0.5f);
		SelectFromListMenuItem* posyItem = new SelectFromListMenuItem(posy, onchange_analog_speedo_pos_y);
		posyItem->wrap = false; posyItem->caption = "速度表Y位置"; posyItem->value = currentY/5;
		menuItems.push_back(posyItem);
		gASPosYItem = posyItem;
	}

	// 尺寸（像素）128..512
	{
		std::vector<std::string> captions; captions.reserve(ANALOG_PIXEL_SIZES.size());
		int currentIndex = 0;
		for(size_t i=0;i<ANALOG_PIXEL_SIZES.size();++i){
			captions.push_back(std::to_string(ANALOG_PIXEL_SIZES[i]));
			if(analogSpeedoPixelSize == ANALOG_PIXEL_SIZES[i]) currentIndex = (int)i;
		}
		auto onchange_size_pixels = [](int value, SelectFromListMenuItem*){
			int idx = value < 0 ? 0 : (value >= (int)ANALOG_PIXEL_SIZES.size()? (int)ANALOG_PIXEL_SIZES.size()-1 : value);
			analogSpeedoPixelSize = ANALOG_PIXEL_SIZES[idx];
		};
		listItem = new SelectFromListMenuItem(captions, onchange_size_pixels);
		listItem->wrap = false; listItem->caption = "速度表尺寸 (像素)"; listItem->value = currentIndex;
		menuItems.push_back(listItem);
	}

	// 单位（千米/英里）
	listItem = new SelectFromListMenuItem(ANALOG_SPEEDO_UNIT_CAPTIONS, onchange_analog_speedo_unit);
	listItem->wrap = false; listItem->caption = "速度表单位"; listItem->value = analogSpeedoUnitIndex;
	menuItems.push_back(listItem);

	// 预设位置：右下 左下 左上 右上 自定义
	{
		static const std::vector<std::string> PRESET{ "右下", "左下", "左上", "右上", "自定义" };
		auto onpos = [](int v, SelectFromListMenuItem*){
			analogSpeedoPresetIndex = v;
			switch(v){
				case 0: analogSpeedoPosX = 0.90f; analogSpeedoPosY = 0.85f; break; // 右下
				case 1: analogSpeedoPosX = 0.10f; analogSpeedoPosY = 0.85f; break; // 左下
				case 2: analogSpeedoPosX = 0.10f; analogSpeedoPosY = 0.15f; break; // 左上
				case 3: analogSpeedoPosX = 0.90f; analogSpeedoPosY = 0.15f; break; // 右上
				case 4: /* 自定义：不改动坐标 */ break;
			}
			int newX = (int)(analogSpeedoPosX * 1000.0f + 0.5f);
			if(gASPosXItem) gASPosXItem->value = newX/5;
			int newY = (int)(analogSpeedoPosY * 1000.0f + 0.5f);
			if(gASPosYItem) gASPosYItem->value = newY/5;
		};
		// 推断当前预设
		int curX = (int)(analogSpeedoPosX * 1000.0f + 0.5f);
		int curY = (int)(analogSpeedoPosY * 1000.0f + 0.5f);
		int inferred = 4;
		if(curX==900 && curY==850) inferred = 0; else if(curX==100 && curY==850) inferred = 1;
		else if(curX==100 && curY==150) inferred = 2; else if(curX==900 && curY==150) inferred = 3;
		analogSpeedoPresetIndex = inferred;
		SelectFromListMenuItem* preset = new SelectFromListMenuItem(PRESET, onpos);
		preset->wrap = false; preset->caption = "预设位置"; preset->value = analogSpeedoPresetIndex;
		menuItems.push_back(preset);
		gASPresetItem = preset;
	}

	// 显示数字速度
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "显示数字速度";
	togItem->value = 0; togItem->toggleValue = &analogShowDigitalSpeed; togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);

	// 车辆健康值显示
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "车辆健康值显示";
	togItem->value = 0; togItem->toggleValue = &analogShowVehicleHealth; togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);

	// 海拔高度显示
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "海拔高度显示";
	togItem->value = 0; togItem->toggleValue = &analogShowAltitude; togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);

	draw_generic_menu<int>(menuItems, NULL, "模拟速度表选项", NULL, NULL, NULL, NULL);
}

// 在原速度/高度更新循环末尾，追加模拟速度表绘制（仅玩家驾驶载具时显示）
// 注意：与数字速度保持互不干扰，独立显示
// 此函数在每帧调用
void update_speedaltitude_append_analog(Ped playerPed){
	// 互斥控制：根据复选框更新标志进行一次性状态切换
	if(featureAnalogSpeedometerEnabledUpdated){
		// 开启模拟速度表时，关闭传统速度/高度（总开关）与三个子项
		if(featureAnalogSpeedometerEnabled){
			featureSpeedAltitudeMaster = false;
			featureSpeedOnFoot = false; featureSpeedOnFootUpdated = false;
			featureSpeedOnGround = false; featureSpeedOnGroundUpdated = false;
			featureSpeedInAir = false; featureSpeedInAirUpdated = false;
			set_status_text("模拟速度表：已启用\n并（关闭传统速度/高度显示）");
		}
		else{
			set_status_text("模拟速度表：已关闭");
		}
		featureAnalogSpeedometerEnabledUpdated = false;
	}
	if(featureSpeedAltitudeMasterUpdated){
		// 打开总开关时，关闭模拟速度表
		if(featureSpeedAltitudeMaster){
			featureAnalogSpeedometerEnabled = false;
			set_status_text("传统速度/高度：已启用\n并（关闭模拟速度表）");
		}
		featureSpeedAltitudeMasterUpdated = false;
	}
	// 若任一子项被打开，则确保总开关为开并关闭模拟速度表
	if(featureSpeedOnFootUpdated || featureSpeedOnGroundUpdated || featureSpeedInAirUpdated){
		if(featureSpeedOnFoot || featureSpeedOnGround || featureSpeedInAir){
			featureSpeedAltitudeMaster = true;
			featureAnalogSpeedometerEnabled = false;
			set_status_text("传统速度/高度：已启用\n并（关闭模拟速度表）");
		}
		featureSpeedOnFootUpdated = false;
		featureSpeedOnGroundUpdated = false;
		featureSpeedInAirUpdated = false;
	}
	if(featureAnalogSpeedometerEnabled){
		draw_analog_speedometer(playerPed);
	}
}

// 重置速度/高度与模拟速度表的所有相关全局变量
void reset_speed_altitude_globals(){
    // 传统速度/高度默认值
    featureKMH = false;
    featureAltitude = false; // 重置时旧海拔高度保持关闭
    featureSpeedOnFoot = false;
    featureSpeedOnGround = false;
    featureSpeedInAir = false;
    featureSpeedAltitudeMaster = false; // 重置时关闭传统显示总开关（忽略互斥逻辑）
    featureSpeedAltitudeMasterUpdated = false;
    featureSpeedOnFootUpdated = false;
    featureSpeedOnGroundUpdated = false;
    featureSpeedInAirUpdated = false;

    SpeedSizeIndex = 0;
    SizeChanged = true;
    SpeedPositionIndexN = 0;
    PositionChanged = true;
    textX = -1.0f; textY = -1.0f;
    rectXScaled = -1.0f; rectYScaled = -1.0f;

    // 模拟速度表默认值（确保重置后关闭）
    featureAnalogSpeedometerEnabled = false; // 重置时关闭模拟速度表（忽略互斥逻辑）
    featureAnalogSpeedometerEnabledUpdated = false;
    analogSpeedoStyleIndex = 0;
    analogSpeedoPosX = 0.90f;
    analogSpeedoPosY = 0.85f;
    analogSpeedoPixelSize = 256;
    analogSpeedoPresetIndex = 0; // 右下
    analogSpeedoUnitIndex = 0;   // 千米
    analogShowDigitalSpeed = false; // 默认关闭数字速度显示（需求）
    analogShowVehicleHealth = false;
    analogShowAltitude = false;
    gASPosXItem = nullptr;
    gASPosYItem = nullptr;
    gASPresetItem = nullptr;
}

// ===== 通用设置保存：速度/高度与模拟速度表 =====
void add_speed_altitude_generic_settings(std::vector<StringPairSettingDBRow>* results){
    // 传统速度/高度文本显示
    results->push_back(StringPairSettingDBRow{ "featureSpeedAltitudeMaster", std::to_string(featureSpeedAltitudeMaster ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "featureKMH", std::to_string(featureKMH ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "featureAltitude", std::to_string(featureAltitude ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "featureSpeedOnFoot", std::to_string(featureSpeedOnFoot ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "featureSpeedOnGround", std::to_string(featureSpeedOnGround ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "featureSpeedInAir", std::to_string(featureSpeedInAir ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "SpeedSizeIndex", std::to_string(SpeedSizeIndex) });
    results->push_back(StringPairSettingDBRow{ "SpeedPositionIndexN", std::to_string(SpeedPositionIndexN) });

    // 模拟速度表
    results->push_back(StringPairSettingDBRow{ "featureAnalogSpeedometerEnabled", std::to_string(featureAnalogSpeedometerEnabled ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoStyleIndex", std::to_string(analogSpeedoStyleIndex) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoPosX", std::to_string(analogSpeedoPosX) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoPosY", std::to_string(analogSpeedoPosY) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoPixelSize", std::to_string(analogSpeedoPixelSize) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoPresetIndex", std::to_string(analogSpeedoPresetIndex) });
    results->push_back(StringPairSettingDBRow{ "analogSpeedoUnitIndex", std::to_string(analogSpeedoUnitIndex) });
    results->push_back(StringPairSettingDBRow{ "analogShowDigitalSpeed", std::to_string(analogShowDigitalSpeed ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "analogShowVehicleHealth", std::to_string(analogShowVehicleHealth ? 1 : 0) });
    results->push_back(StringPairSettingDBRow{ "analogShowAltitude", std::to_string(analogShowAltitude ? 1 : 0) });
}

static inline bool parse_bool(const std::string& v){ return (v == "1" || v == "true" || v == "True" || v == "TRUE"); }

void handle_generic_settings_speed_altitude(std::vector<StringPairSettingDBRow>* settings){
    for(int i=0;i<settings->size();++i){
        const StringPairSettingDBRow& setting = settings->at(i);
        // 传统速度/高度
        if(setting.name.compare("featureSpeedAltitudeMaster")==0){ featureSpeedAltitudeMaster = parse_bool(setting.value); }
        else if(setting.name.compare("featureKMH")==0){ featureKMH = parse_bool(setting.value); }
        else if(setting.name.compare("featureAltitude")==0){ featureAltitude = parse_bool(setting.value); }
        else if(setting.name.compare("featureSpeedOnFoot")==0){ featureSpeedOnFoot = parse_bool(setting.value); }
        else if(setting.name.compare("featureSpeedOnGround")==0){ featureSpeedOnGround = parse_bool(setting.value); }
        else if(setting.name.compare("featureSpeedInAir")==0){ featureSpeedInAir = parse_bool(setting.value); }
        else if(setting.name.compare("SpeedSizeIndex")==0){ SpeedSizeIndex = stoi(setting.value); }
        else if(setting.name.compare("SpeedPositionIndexN")==0){ SpeedPositionIndexN = stoi(setting.value); }

        // 模拟速度表
        else if(setting.name.compare("featureAnalogSpeedometerEnabled")==0){ featureAnalogSpeedometerEnabled = parse_bool(setting.value); }
        else if(setting.name.compare("analogSpeedoStyleIndex")==0){ analogSpeedoStyleIndex = stoi(setting.value); }
        else if(setting.name.compare("analogSpeedoPosX")==0){ analogSpeedoPosX = (float)atof(setting.value.c_str()); }
        else if(setting.name.compare("analogSpeedoPosY")==0){ analogSpeedoPosY = (float)atof(setting.value.c_str()); }
        else if(setting.name.compare("analogSpeedoPixelSize")==0){ analogSpeedoPixelSize = stoi(setting.value); }
        else if(setting.name.compare("analogSpeedoPresetIndex")==0){ analogSpeedoPresetIndex = stoi(setting.value); }
        else if(setting.name.compare("analogSpeedoUnitIndex")==0){ analogSpeedoUnitIndex = stoi(setting.value); }
        else if(setting.name.compare("analogShowDigitalSpeed")==0){ analogShowDigitalSpeed = parse_bool(setting.value); }
        else if(setting.name.compare("analogShowVehicleHealth")==0){ analogShowVehicleHealth = parse_bool(setting.value); }
        else if(setting.name.compare("analogShowAltitude")==0){ analogShowAltitude = parse_bool(setting.value); }
    }
    // 读取阶段防御性夹取：与绘制阶段约束保持一致
    if(analogSpeedoStyleIndex < 0) analogSpeedoStyleIndex = 0; else if(analogSpeedoStyleIndex > 13) analogSpeedoStyleIndex = 13;
    if(analogSpeedoPosX < 0.0f) analogSpeedoPosX = 0.0f; else if(analogSpeedoPosX > 1.0f) analogSpeedoPosX = 1.0f;
    if(analogSpeedoPosY < 0.0f) analogSpeedoPosY = 0.0f; else if(analogSpeedoPosY > 1.0f) analogSpeedoPosY = 1.0f;
    if(analogSpeedoPixelSize < 128) analogSpeedoPixelSize = 128; else if(analogSpeedoPixelSize > 512) analogSpeedoPixelSize = 512;
    if(analogSpeedoUnitIndex < 0) analogSpeedoUnitIndex = 0; else if(analogSpeedoUnitIndex > 1) analogSpeedoUnitIndex = 1;
    // 加载后的互斥校正：若传统显示已开启，则确保模拟速度表关闭
    if(featureSpeedAltitudeMaster && (featureSpeedOnFoot || featureSpeedOnGround || featureSpeedInAir)){
        featureAnalogSpeedometerEnabled = false;
    }
}
