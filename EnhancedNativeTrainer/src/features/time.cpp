/*
这段代码最初是作为 GTA V SCRIPT HOOK SDK 的一部分而创建的。
http://dev-c.com
(C) Alexander Blade 2015

现在它已成为 Enhanced Native Trainer 项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/
#include "time.h"
#include "vehicles.h"
#include "hotkeys.h"
#include "propplacement.h"
#include "airbrake.h"
#include <iomanip>
#include "..\ui_support\menu_functions.h"
#include "script.h"
#include <stdio.h>
#include "..\..\inc\main.h"

const std::vector<std::string> TIME_SPEED_CAPTIONS{ "最低", "0.1x", "0.2x", "0.3x", "0.4x", "0.5x", "0.6x", "0.7x", "0.8x", "0.9x", "1x (正常)" };
const std::vector<float> TIME_SPEED_VALUES{ 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };
const int DEFAULT_TIME_SPEED = 10;

const std::vector<std::string> TIME_FLOW_RATE_CAPTIONS{ "冻结时间 (0秒/秒)", "每秒半秒 (0.5秒/秒)", "现实时间 (1秒/秒)", "每秒 2 秒", "每秒 3 秒", "每秒 4 秒", "每秒 5 秒", "每秒 6 秒", "每秒 7 秒", "每秒 8 秒", "每秒 9 秒", "每秒 10 秒", "每秒 12 秒", "每秒 15 秒", "正常时间流速 (30秒/秒)", "每秒 1 分钟", "每秒 2 分钟", "每秒 3 分钟", "每秒 4 分钟", "每秒 5 分钟", "每秒 6 分钟", "每秒 7 分钟", "每秒 8 分钟", "每秒 9 分钟", "每秒 10 分钟", "每秒 12 分钟", "每秒 15 分钟", "每秒 30 分钟", "每秒 1 小时", "每秒 2 小时", "每秒 3 小时", "每秒 4 小时", "每秒 5 小时", "每秒 6 小时", "每秒 12 小时", "每秒 1 天" };
const std::vector<float> TIME_FLOW_RATE_VALUES{ 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 12.0f, 15.0f, 30.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f, 360.0f, 420.0f, 480.0f, 540.0f, 600.0f, 720.0f, 900.0f, 1800.0f, 3600.0f, 7200.0f, 10800.0f, 14400.0f, 18000.0f, 21600.0f, 43200.0f, 86400.0f };
const int DEFAULT_TIME_FLOW_RATE = 10;

const int DEFAULT_HOTKEY_FLOW_RATE = 10;

const int TIME_TO_SLOW_AIM = 2000;

std::vector<float> VEH_S;
std::vector<Vehicle> VEH_CURR;

int timeSpeedIndexWhileAiming = DEFAULT_TIME_SPEED;
int timeSpeedIndex = DEFAULT_TIME_SPEED;

int timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE;
int HotkeyFlowRateIndex = DEFAULT_HOTKEY_FLOW_RATE;

bool featureTimeSynced = false;
bool featureTimeSyncedUpdated = false;
bool featureShowtime = false;
bool featurehotkeytime = false;
bool featureSpeedAimInVeh = false;
bool featureFreezeTime = false;
bool featureFreezeTimeUpdated = false;
bool timeFlowRateChanged = true, timeFlowRateLocked = true;
bool HotkeyFlowRateChanged = true, HotkeyFlowRateLocked = true;

bool slow_aim = false;

float frozentimestate = -1;

bool requireRefreshOfTime = false;

int activeLineIndexTime = 0;
int activeLineIndexTimeFlow = 0;
int timeSettingsAnalogRowIndex = -1;

float timeFactor = 1000.0f / TIME_FLOW_RATE_VALUES.at(timeFlowRateIndex);

int timeSinceAimingBegan = 0;

bool weHaveChangedTimeScale;

// ================= 模拟时钟：设置及资源 =================
bool featureAnalogClockEnabled = false;
int analogClockStyleIndex = 0; // 默认时钟
int analogClockTimeSourceIndex = 0; // 0 游戏时间, 1 现实时间
float analogClockPosX = 0.90f; // 默认右上角
float analogClockPosY = 0.15f;
bool analogClockShowLabel = true;
bool analogClockShowDigital = true;
bool analogClockShowDate = true; // 默认显示日期
bool featureAnalogClockEnabledUpdated = false; // 模拟时钟开关更新状态
int analogClockPixelSize = 256; // 默认按256像素绘制（与常见YTD贴图匹配）
int analogClockPresetIndex = 0; // 0 右上(默认), 1 右下, 2 左上, 3 左下, 4 自定义
// 用于在“预设位置”选择时同步更新 X/Y 列表当前值（非捕获lambda）
static SelectFromListMenuItem* gAnalogPosXItem = nullptr;
static SelectFromListMenuItem* gAnalogPosYItem = nullptr;
static SelectFromListMenuItem* gAnalogPresetItem = nullptr;

static inline void ensure_clock_textures_loaded();
static inline void draw_analog_clock();

float quadratic_time_transition(float start, float end, float progress) {
	//二次方程相关内容
	float t = 1 - progress;
	t = 1 - (t * t);

	float difference = end - start;

	return (start + (difference * t));
}

bool onconfirm_time_set_menu(MenuItem<int> choice) {
	switch (choice.value) {//预设的时间点
	case 0:
		// 半夜 00:00
		movetime_set(0, 0);
		break;
	case 1:
		// 早晨 06:00
		movetime_set(6, 0);
		break;
	case 2:
		// 上午 08:00
		movetime_set(8, 0);
		break;
	case 3:
		// 上午 10:00
		movetime_set(10, 0);
		break;
	case 4:
		// 中午 12:00
		movetime_set(12, 0);
		break;
	case 5:
		// 下午 14:00
		movetime_set(14, 0);
		break;
	case 6:
		// 下午 16:00
		movetime_set(16, 0);
		break;
	case 7:
		// 傍晚 18:00
		movetime_set(18, 0);
		break;
	case 8:
		// 晚上 20:00
		movetime_set(20, 0);
		break;
	case 9:
		// 晚上 22:00
		movetime_set(22, 0);
		break;
	}

	return false;
}

void onconfirm_time_flow_rate(MenuItem<int> choice) {
	if (timeFlowRateLocked = !timeFlowRateLocked) {
		std::ostringstream ss;
		ss << "时间流速: " << TIME_FLOW_RATE_CAPTIONS.at(choice.value);
		set_status_text(ss.str());
	}
}

void onchange_game_speed_callback(int value, SelectFromListMenuItem* source) {
	timeSpeedIndex = value;
	std::ostringstream ss;
	ss << "游戏速度: " << TIME_SPEED_CAPTIONS.at(value);
	set_status_text(ss.str());
}

void onchange_aiming_speed_callback(int value, SelectFromListMenuItem* source) {
	timeSpeedIndexWhileAiming = value;
	std::ostringstream ss;
	ss << "瞄准速度: " << TIME_SPEED_CAPTIONS.at(value);
	set_status_text(ss.str());
}

void onchange_time_flow_rate_callback(int value, SelectFromListMenuItem* source) {
	timeFlowRateIndex = value, timeFlowRateChanged = true, timeFlowRateLocked = false;
	featureFreezeTime = (value == 0);
	if (value == 0) {
		// 选择“冻结时间”项时，关闭系统时间同步，保持互斥
		featureTimeSynced = false;
	}
}

void onchange_hotkey_flow_rate_callback(int value, SelectFromListMenuItem* source) {
	HotkeyFlowRateIndex = value, HotkeyFlowRateChanged = true, HotkeyFlowRateLocked = false;
}

void onchange_hotkey_freeze_unfreeze_time() {
	if (timeFlowRateIndex != 0) {
		frozentimestate = timeFlowRateIndex;
		timeFlowRateIndex = 0;
		timeFlowRateChanged = true;
		write_text_to_log_file("时间已冻结！");//提示重复了，这里改为日志记录
		requireRefreshOfTime = true;
		// 通过热键开启冻结时，关闭系统时间同步，保持互斥
		featureTimeSynced = false;
	}
	else
	{
		if (frozentimestate != -1) {
			timeFlowRateIndex = frozentimestate;
			timeFlowRateChanged = true;
		}
		else {
			timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE;
			timeFlowRateChanged = true;
		}
		write_text_to_log_file("时间已解冻！");//提示重复了，这里改为日志记录
		requireRefreshOfTime = true;
	}
	// 同步复选框状态
	featureFreezeTime = (timeFlowRateIndex == 0);
	featureFreezeTimeUpdated = true;
}

bool onconfirm_time_flowrate_menu(MenuItem<int> choice) {
	if (choice.value == 0) {
		if (featureTimeSynced) {
			set_status_text("时间已与电脑系统同步！");
		}
	}
	// 基于选中行索引打开“模拟时钟”子菜单
	if (activeLineIndexTimeFlow == timeSettingsAnalogRowIndex) {
		process_analog_clock_menu();
	}
	return false;
}

bool flowtime_menu_interrupt() {
	if (requireRefreshOfTime) {
		return true;
	}
	return false;
}

void all_time_flow_rate() {
	// 保留菜单选择的行索引，避免刷新后光标跳到第一项
	do {
		requireRefreshOfTime = false;
		std::vector<MenuItem<int>*> menuItems;
		MenuItem<int>* item;
		int index = 0;

		ToggleMenuItem<int>* togItem = new ToggleMenuItem<int>();
		togItem->caption = "时间与电脑系统同步";
		togItem->value = 0;
		togItem->toggleValue = &featureTimeSynced;
		// 添加更新标记，以便在切换时处理互斥逻辑
		togItem->toggleValueUpdated = &featureTimeSyncedUpdated;
		menuItems.push_back(togItem);

		SelectFromListMenuItem* listItem = new SelectFromListMenuItem(TIME_SPEED_CAPTIONS, onchange_hotkey_flow_rate_callback);
		listItem->wrap = false;
		listItem->caption = "全局游戏速度";
		listItem->value = HotkeyFlowRateIndex;
		menuItems.push_back(listItem);

		listItem = new SelectFromListMenuItem(TIME_SPEED_CAPTIONS, onchange_aiming_speed_callback);
		listItem->wrap = false;
		listItem->caption = "瞄准时的游戏速度";
		listItem->value = timeSpeedIndexWhileAiming;
		menuItems.push_back(listItem);

		togItem = new ToggleMenuItem<int>();
		togItem->caption = "仅在车辆内瞄准时的游戏速度";
		togItem->value = 0;
		togItem->toggleValue = &featureSpeedAimInVeh;
		togItem->toggleValueUpdated = NULL;
		menuItems.push_back(togItem);

		listItem = new SelectFromListMenuItem(TIME_FLOW_RATE_CAPTIONS, onchange_time_flow_rate_callback);
		listItem->caption = "时间流速";
		listItem->value = timeFlowRateIndex;
		listItem->wrap = false;
		listItem->onConfirmFunction = onconfirm_time_flow_rate;
		menuItems.push_back(listItem);

		ToggleMenuItem<int>* freezeTog = new ToggleMenuItem<int>();
		freezeTog->caption = "冻结时间";
		freezeTog->value = 0;
		freezeTog->toggleValue = &featureFreezeTime;
		freezeTog->toggleValueUpdated = &featureFreezeTimeUpdated;
		menuItems.push_back(freezeTog);

		togItem = new ToggleMenuItem<int>();
		togItem->caption = "显示当前游戏内时间";
		togItem->value = 0;
		togItem->toggleValue = &featureShowtime;
		togItem->toggleValueUpdated = NULL;
		menuItems.push_back(togItem);

		togItem = new ToggleMenuItem<int>();
		togItem->caption = "快速时间切换 [右Alt + 1-8, 右Alt + 小键盘 -/+]";
		togItem->value = 0;
		togItem->toggleValue = &featurehotkeytime;
		togItem->toggleValueUpdated = NULL;
		menuItems.push_back(togItem);

		// 模拟时钟子菜单入口（移入时间设置内）
		MenuItem<int>* analogItem = new MenuItem<int>();
		analogItem->caption = "模拟时钟 (显示圆形钟表)";
		analogItem->value = -1;
		analogItem->isLeaf = false;
		menuItems.push_back(analogItem);
		// 记录该项的行索引，供确认回调按索引路由
		timeSettingsAnalogRowIndex = static_cast<int>(menuItems.size()) - 1;

		// 使用持久化的行索引以在刷新后保持光标位置
		draw_generic_menu<int>(menuItems, &activeLineIndexTimeFlow, "时间设置", onconfirm_time_flowrate_menu, nullptr, nullptr, flowtime_menu_interrupt);
	} while (requireRefreshOfTime);
}

void process_time_set_menu() {
	std::vector<MenuItem<int>*> menuItems;
	int index = 0;

	MenuItem<int>* item = new MenuItem<int>();
	item->caption = "半夜 12:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "早晨 6:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "上午 8:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "上午 10:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "中午 12:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "下午 2:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "下午 4:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "傍晚 6:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "晚上 8:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "晚上 10:00 点";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	draw_generic_menu<int>(menuItems, nullptr, "预设的时间点", onconfirm_time_set_menu, nullptr, nullptr, nullptr);
}

bool onconfirm_time_menu(MenuItem<int> choice) {
	switch (activeLineIndexTime) {
	case 0:
		process_time_set_menu();
		break;
	case 1:
		movetime_hour_forward();
		break;
	case 2:
		movetime_hour_backward();
		break;
	case 3:
		movetime_fivemin_forward();
		break;
	case 4:
		movetime_fivemin_backward();
		break;
	case 5:
		movetime_day_forward();
		break;
	case 6:
		movetime_day_backward();
		break;
	case 7:
		set_date();
		break;
	case 8:
		set_time();
		break;
	case 9:
		all_time_flow_rate();
		break;
	}
	return false;
}

void process_time_menu() {
	const std::string caption = "时间选项";

	std::vector<MenuItem<int>*> menuItems;

	int index = 0;

	MenuItem<int>* item = new MenuItem<int>();
	item->caption = "预设的时间点";
	item->value = -1;
	item->isLeaf = false;
	menuItems.insert(menuItems.begin(), item);

	item = new MenuItem<int>();
	item->caption = "前进 1 小时";// 前进
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "后退 1 小时";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "前进 5 分钟";// 前进
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "后退 5 分钟";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "前进 1 天";// 前进
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "后退 1 天";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "设置日期 (年/月/日)";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "设置时间 (时:分)";
	item->value = index++;
	item->isLeaf = true;
	menuItems.push_back(item);

	item = new MenuItem<int>();
	item->caption = "时间设置";
	item->value = index++;
	item->isLeaf = false;
	menuItems.insert(menuItems.end(), item);

	draw_generic_menu<int>(menuItems, &activeLineIndexTime, caption, onconfirm_time_menu, nullptr, nullptr, nullptr);
}

void reset_time_globals() {
	featureTimeSynced = false;
	featureTimeSyncedUpdated = false;
	timeFlowRateChanged = true;
	HotkeyFlowRateChanged = true;
	featureShowtime = false;
	featurehotkeytime = false;
	featureSpeedAimInVeh = false;
	featureFreezeTime = false;
	featureFreezeTimeUpdated = false;

	timeSpeedIndexWhileAiming = DEFAULT_TIME_SPEED;
	timeSpeedIndex = DEFAULT_TIME_SPEED;
	timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE;
	HotkeyFlowRateIndex = DEFAULT_HOTKEY_FLOW_RATE;
	frozentimestate = -1;

	// 模拟时钟默认值
	featureAnalogClockEnabled = false;
	analogClockStyleIndex = 0;// 默认时钟
	analogClockTimeSourceIndex = 0;
	analogClockPosX = 0.90f;
	analogClockPosY = 0.15f;
	analogClockShowLabel = true;
	analogClockShowDigital = true;
	analogClockShowDate = true; // 默认显示日期
	analogClockPixelSize = 256;
	analogClockPresetIndex = 0; // 默认右上
}

void add_time_feature_enablements(std::vector<FeatureEnabledLocalDefinition>* results) {
	results->push_back(FeatureEnabledLocalDefinition{ "featureTimeSynced", &featureTimeSynced, &featureTimeSyncedUpdated });
	results->push_back(FeatureEnabledLocalDefinition{ "featureShowtime", &featureShowtime });
	results->push_back(FeatureEnabledLocalDefinition{ "featurehotkeytime", &featurehotkeytime });
	results->push_back(FeatureEnabledLocalDefinition{ "featureSpeedAimInVeh", &featureSpeedAimInVeh });
	results->push_back(FeatureEnabledLocalDefinition{ "featureFreezeTime", &featureFreezeTime, &featureFreezeTimeUpdated });
	results->push_back(FeatureEnabledLocalDefinition{ "featureAnalogClockEnabled", &featureAnalogClockEnabled });
}

void movetime_day_forward() {
	/*
	bool timeWasPaused = featureTimePaused;
	TIME::PAUSE_CLOCK(true);
	*/

	int calDay = TIME::GET_CLOCK_DAY_OF_MONTH();
	int calMon = TIME::GET_CLOCK_MONTH();
	int calYear = TIME::GET_CLOCK_YEAR();

	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();

	bool leapYear = false;
	if (calYear % 4 == 0) {
		leapYear = true;
	}

	/*
	std::ostringstream ss2;
	ss2 << "Date is: ";
	ss2 << std::setfill('0') << std::setw(2) << calDay;
	ss2 << ".";
	ss2 << std::setfill('0') << std::setw(2) << calMon;
	ss2 << ".";
	ss2 << calYear;
	set_status_text(ss2.str());
	*/

	if ((calDay == 27 && calMon == 2 && !leapYear) ||
		(calDay == 28 && calMon == 2 && leapYear) ||
		(calDay == 30 && (calMon == 4 || calMon == 6 || calMon == 9 || calMon == 11)) ||
		(calDay == 31)) {
		calDay = 1;
		if (calMon == 12) {
			calMon = 1;
			calYear++;
		}
		else {
			calMon++;
		}
	}
	else {
		calDay++;
	}

	TIME::SET_CLOCK_DATE(calDay, calMon, calYear);
	TIME::SET_CLOCK_TIME(gameHour, gameMins, 0);

	std::ostringstream ss;
	ss << "当前日期: "; // 左下角 日期提示
	ss << std::setfill('0') << std::setw(4) << TIME::GET_CLOCK_YEAR(); // 年，格式化为4位数（不足补零）
	ss << "."; // 分隔符
	ss << std::setfill('0') << std::setw(2) << TIME::GET_CLOCK_MONTH(); // 月，格式化为两位数（不足补零）
	ss << "."; // 分隔符
	ss << std::setfill('0') << std::setw(2) << TIME::GET_CLOCK_DAY_OF_MONTH(); // 日，格式化为两位数（不足补零）
	ss << "  "; // 分隔符
	ss << get_day_of_game_week(); // 星期
	set_status_text(ss.str()); // 将格式化后的字符串设置为状态文本

	//TIME::PAUSE_CLOCK(timeWasPaused);
}

void movetime_day_backward() {
	int calDay = TIME::GET_CLOCK_DAY_OF_MONTH();
	int calMon = TIME::GET_CLOCK_MONTH();
	int calYear = TIME::GET_CLOCK_YEAR();

	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();

	bool leapYear = false;
	if (calYear % 4 == 0) {
		leapYear = true;
	}

	if (calDay != 1) {
		calDay--;
	}
	else if (calMon == 1) {
		calDay = 31;
		calMon = 12;
		calYear--;
	}
	else {
		if (calMon == 5 || calMon == 7 || calMon == 10 || calMon == 12) {
			calDay = 30;
		}
		if (calMon == 3) {
			if (leapYear) {
				calDay = 29;
			}
			else {
				calDay = 28;
			}
		}
		else {
			calDay = 31;
		}
		calMon--;
	}

	TIME::SET_CLOCK_DATE(calDay, calMon, calYear);
	TIME::SET_CLOCK_TIME(gameHour, gameMins, 0);

	std::ostringstream ss;
	ss << "当前日期: "; // 左下角 日期提示
	ss << std::setfill('0') << std::setw(4) << calYear; // 年，格式化为4位数（不足补零）
	ss << "."; // 分隔符
	ss << std::setfill('0') << std::setw(2) << calMon; // 月，格式化为两位数（不足补零）
	ss << "."; // 分隔符
	ss << std::setfill('0') << std::setw(2) << calDay; // 日，格式化为两位数（不足补零）
	ss << "  "; // 分隔符
	ss << get_day_of_game_week(); // 星期
	set_status_text(ss.str()); // 将格式化后的字符串设置为状态文本
}

void set_date() {
	keyboard_on_screen_already = true;
	curr_message = "输入新日期 (示例: 2025/2/1 示例: 2025.2.1)"; // 提示用户输入格式
	std::string lastDateSpawn;
	std::string tmp_Year, tmp_Mon, tmp_Day;   // 年、月、日字段

	std::string result = show_keyboard("手动输入日期", (char*)lastDateSpawn.c_str());
	if (!result.empty()) {
		result = trim(result); // 去除首尾空白
		lastDateSpawn = result;

		std::string a = result;
		int found_separator = 0; // 记录分隔符数量
		bool found_symbol = false; // 标记是否遇到数字

		// 支持的分隔符：/、空格、.
		std::string separators = "/ .";

		// 解析输入字符串
		for (int i = 0; i < a.size(); i++) {
			bool is_separator = (separators.find(a[i]) != std::string::npos);
			bool is_digit = (a[i] >= '0' && a[i] <= '9');

			// 如果遇到非数字、非分隔符的字符，返回默认日期
			if (!is_separator && !is_digit) {
				TIME::SET_CLOCK_DATE(1, 1, 2025);
				set_status_text("~r~错误: ~s~日期格式设置不正确！\n已恢复默认日期: 2025.01.01");
				return;
			}

			if (!is_separator) {
				found_symbol = true; // 标记已找到数字
			}
			if (is_separator && found_symbol) {
				found_separator++; // 遇到分隔符且之前有数字，计数加1
				found_symbol = false;
			}

			if (is_digit) {
				if (found_separator == 0) tmp_Year += a[i]; // 年
				else if (found_separator == 1) tmp_Mon += a[i]; // 月
				else if (found_separator == 2) tmp_Day += a[i]; // 日
			}
		}

		// 验证输入完整性：必须有2个分隔符且字段非空
		if (found_separator != 2 || tmp_Year.empty() || tmp_Mon.empty() || tmp_Day.empty()) {
			TIME::SET_CLOCK_DATE(1, 1, 2025);
			set_status_text("~r~错误: ~s~日期格式设置不正确！\n已恢复默认日期: 2025.01.01");
			return;
		}

		// 保存原始输入值，用于检测是否有非法输入
		std::string original_year = tmp_Year;
		std::string original_month = tmp_Mon;
		std::string original_day = tmp_Day;

		// 限制长度和范围，并处理异常
		int year = 2025, month = 1, day = 1; // 默认值
		std::string::size_type sz;
		bool date_modified = false; // 标记日期是否被修改过

		try {
			// 年份处理 - 添加更严格的限制
			if (tmp_Year.length() > 4) {
				year = 2025; // 长度大于4位，使用默认年份
				date_modified = true;
			}
			else {
				year = std::stoi(tmp_Year, &sz);
				// 年份范围限制在合理区间，例如1945-2099
				if (year < 1945 || year > 2099) {
					date_modified = true;
					year = (year < 1945) ? 1945 : 2099;
				}
			}

			// 月份处理
			if (tmp_Mon.length() > 2) {
				month = 1; // 长度大于2位，使用默认月份
				date_modified = true;
			}
			else {
				month = std::stoi(tmp_Mon, &sz);
				if (month < 1 || month > 12) {
					date_modified = true;
					month = (month < 1) ? 1 : 12;
				}
			}

			// 日期处理 - 根据月份确定最大天数
			int max_days = 31; // 默认最大天数
			if (month == 4 || month == 6 || month == 9 || month == 11) {
				max_days = 30;
			}
			else if (month == 2) {
				// 简单闰年判断
				if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
					max_days = 29;
				}
				else {
					max_days = 28;
				}
			}

			if (tmp_Day.length() > 2) {
				day = 1; // 长度大于2位，使用默认天数
				date_modified = true;
			}
			else {
				day = std::stoi(tmp_Day, &sz);
				if (day < 1 || day > max_days) {
					date_modified = true;
					day = (day < 1) ? 1 : max_days;
				}
			}
		}
		catch (const std::exception& e) {
			// 解析失败，返回默认日期
			TIME::SET_CLOCK_DATE(1, 1, 2025);
			set_status_text("~r~错误: ~s~日期解析失败！\n已恢复默认日期: 2025.01.01");
			return;
		}

		// 设置日期
		TIME::SET_CLOCK_DATE(day, month, year);

		// 根据是否修改了日期来显示不同的状态信息
		std::ostringstream ss;
		if (date_modified ||
			original_year != std::to_string(year) ||
			original_month != std::to_string(month) ||
			original_day != std::to_string(day)) {
			// 有非法值，显示警告信息
			ss << "~r~警告: ~s~输入日期 " << original_year << "." << original_month << "." << original_day;
			ss << " 超出范围, 已调整为: ";
			ss << std::setfill('0') << std::setw(4) << year; // 年
			ss << ".";
			ss << std::setfill('0') << std::setw(2) << month; // 月
			ss << ".";
			ss << std::setfill('0') << std::setw(2) << day; // 日
		}
		else {
			// 合法值，显示正常信息
			ss << "当前日期: ";
			ss << std::setfill('0') << std::setw(4) << year; // 年
			ss << ".";
			ss << std::setfill('0') << std::setw(2) << month; // 月
			ss << ".";
			ss << std::setfill('0') << std::setw(2) << day; // 日
			ss << "  " << get_day_of_game_week(); // 星期
		}
		set_status_text(ss.str());
	}
	else {
		// 输入为空，返回默认日期
		TIME::SET_CLOCK_DATE(1, 1, 2025);
		set_status_text("用户已经取消输入！\n恢复默认日期: 2025.01.01");
	}
}

void set_time() {
	keyboard_on_screen_already = true;  // 标记键盘已在屏幕上
	curr_message = "输入新时间 (示例: 8:30 示例: 8.30)";  // 设置提示信息
	std::string lastTimeSpawn;  // 用于存储上一次输入的时间
	std::string result = show_keyboard("手动输入时间", (char*)lastTimeSpawn.c_str());  // 显示键盘并获取用户输入

	if (!result.empty()) {  // 用户输入非空
		result = trim(result);  // 去除首尾空白
		lastTimeSpawn = result;  // 保存用户输入

		std::string a = result;  // 临时存储输入字符串
		int found_separator = 0;  // 记录分隔符数量
		bool found_symbol = false;  // 标记是否遇到有效字符
		std::string separators = ": .";  // 支持的分隔符：冒号、空格、点

		std::string tmp_Hour, tmp_Min;  // 存储小时和分钟的字符串

		// 解析输入字符串
		for (int i = 0; i < a.size(); i++) {
			bool is_separator = (separators.find(a[i]) != std::string::npos);  // 检查是否为分隔符
			bool is_digit = (a[i] >= '0' && a[i] <= '9');  // 检查是否为数字

			if (!is_separator && !is_digit) {  // 遇到非法字符
				movetime_set(12, 0);  // 设置默认时间
				set_status_text("~r~错误: ~s~时间格式设置不正确！\n已恢复默认时间: 12:00");
				return;
			}

			if (!is_separator) {  // 非分隔符字符
				found_symbol = true;
			}
			if (is_separator && found_symbol) {  // 遇到分隔符且之前有数字
				found_separator++;
				found_symbol = false;
			}

			if (is_digit) {  // 处理数字字符
				if (found_separator == 0) tmp_Hour += a[i];  // 小时部分
				else if (found_separator == 1) tmp_Min += a[i];  // 分钟部分
			}
		}

		// 验证输入完整性
		if (found_separator != 1 || tmp_Hour.empty() || tmp_Min.empty()) {  // 格式错误
			movetime_set(12, 0);
			set_status_text("~r~错误: ~s~时间格式设置不正确！\n已恢复默认时间: 12:00");
			return;
		}

		// 保存原始输入值，用于后续比较
		std::string original_hour = tmp_Hour;
		std::string original_min = tmp_Min;

		// 处理时间值并捕获异常
		int hour = 12, min = 0;  // 默认时间
		bool time_modified = false;  // 标记时间是否被调整

		try {
			// 处理小时
			if (tmp_Hour.length() > 2) {  // 长度超限，截断
				tmp_Hour.resize(2);
				time_modified = true;
			}
			hour = std::stoi(tmp_Hour);  // 转换为整数
			if (hour > 23) {  // 超出范围，调整
				hour = 23;
				time_modified = true;
			}
			else if (hour < 0) {
				hour = 0;
				time_modified = true;
			}

			// 处理分钟
			if (tmp_Min.length() > 2) {  // 长度超限，截断
				tmp_Min.resize(2);
				time_modified = true;
			}
			min = std::stoi(tmp_Min);  // 转换为整数
			if (min > 59) {  // 超出范围，调整
				min = 59;
				time_modified = true;
			}
			else if (min < 0) {
				min = 0;
				time_modified = true;
			}
		}
		catch (const std::exception& e) {  // 解析异常
			movetime_set(12, 0);
			set_status_text("~r~错误: ~s~时间解析失败！\n已恢复默认时间: 12:00");
			return;
		}

		// 设置游戏时间
		movetime_set(hour, min);

		// 显示状态信息
		std::ostringstream ss;  // 用于构建提示信息
		if (time_modified || original_hour != std::to_string(hour) || original_min != std::to_string(min)) {
			// 时间被调整，显示警告
			ss << "~r~警告: ~s~输入时间 " << original_hour << ":" << original_min;
			ss << "\n超出范围, 已调整为: ";
			ss << std::setfill('0') << std::setw(2) << hour << ":";
			ss << std::setfill('0') << std::setw(2) << min;
		}
		else {
			// 正常设置，显示当前时间
			ss << "当前时间: ";
			ss << std::setfill('0') << std::setw(2) << hour << ":";
			ss << std::setfill('0') << std::setw(2) << min;
		}
		set_status_text(ss.str());  // 显示提示信息
	}
	else {  // 用户取消输入
		movetime_set(12, 0);
		set_status_text("用户已经取消输入！\n恢复默认时间: 12:00");
	}
}

void movetime_hour_forward() {
	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();
	gameHour++;
	if (gameHour == 24) {
		movetime_day_forward();
		gameHour = 00;
	}
	TIME::SET_CLOCK_TIME(gameHour, gameMins, 00);
	char text[32];
	sprintf_s(text, "当前时间:  %02d:%02d", gameHour, gameMins);
	set_status_text(text);
}

void movetime_hour_backward() {
	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();
	gameHour--;
	if (gameHour == -1) {
		movetime_day_backward();
		gameHour = 23;
	}
	TIME::SET_CLOCK_TIME(gameHour, gameMins, 00);
	char text[32];
	sprintf_s(text, "当前时间:  %02d:%02d", gameHour, gameMins);
	set_status_text(text);
}

void movetime_fivemin_forward() {
	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();

	if (gameHour == 23 && gameMins > 54) {
		movetime_day_forward();
		gameHour = 0;
		gameMins = (gameMins + (-55));
	}
	else if (gameMins > 54) {
		gameHour++;
		gameMins = gameMins + (-55);
	}
	else {
		gameMins = gameMins + 5;
	}

	TIME::SET_CLOCK_TIME(gameHour, gameMins, 00);
	char text[32];
	sprintf_s(text, "当前时间:  %02d:%02d", gameHour, gameMins);
	set_status_text(text);
}

void movetime_fivemin_backward() {
	int gameHour = TIME::GET_CLOCK_HOURS();
	int gameMins = TIME::GET_CLOCK_MINUTES();

	if (gameHour == 0 && gameMins < 5) {
		movetime_day_backward();
		gameHour = 23;
		gameMins = gameMins + 55;
	}
	else if (gameMins < 5) {
		gameHour--;
		gameMins = gameMins + 55;
	}
	else {
		gameMins = gameMins - 5;
	}

	TIME::SET_CLOCK_TIME(gameHour, gameMins, 00);
	char text[32];
	sprintf_s(text, "当前时间:  %02d:%02d", gameHour, gameMins);
	set_status_text(text);
}

void movetime_set(int hour, int minute) {
	TIME::SET_CLOCK_TIME(hour, minute, 0);
	char text[32];
	sprintf_s(text, "当前时间:  %02d:%02d", TIME::GET_CLOCK_HOURS(), TIME::GET_CLOCK_MINUTES());
	set_status_text(text);
}

void toggle_game_speed()
{
	if (HotkeyFlowRateIndex != DEFAULT_HOTKEY_FLOW_RATE && PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID()) && !PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID())) HotkeyFlowRateLocked = !HotkeyFlowRateLocked;
	WAIT(100);
}

std::string get_day_of_game_week() {
	int day = TIME::GET_CLOCK_DAY_OF_WEEK();
	switch (day) {
	case 0:
		return "周日";
	case 1:
		return "周一";
	case 2:
		return "周二";
	case 3:
		return "周三";
	case 4:
		return "周四";
	case 5:
		return "周五";
	case 6:
		return "周六";
	}
	return std::string();
}

void handle_generic_settings_time(std::vector<StringPairSettingDBRow>* settings) {
	for (int i = 0; i < settings->size(); i++) {
		StringPairSettingDBRow setting = settings->at(i);
		if (setting.name.compare("timeSpeedIndexWhileAiming") == 0) {
			timeSpeedIndexWhileAiming = stoi(setting.value);
		}
		else if (setting.name.compare("timeFlowRateIndex") == 0) {
			timeFlowRateIndex = stoi(setting.value);
		}
		else if (setting.name.compare("HotkeyFlowRateIndex") == 0) {
			HotkeyFlowRateIndex = stoi(setting.value);
		}
		else if (setting.name.compare("analogClockStyleIndex") == 0) {
			analogClockStyleIndex = stoi(setting.value);
		}
		else if (setting.name.compare("analogClockTimeSourceIndex") == 0) {
			analogClockTimeSourceIndex = stoi(setting.value);
		}
		else if (setting.name.compare("analogClockPosX") == 0) {
			analogClockPosX = (float)atof(setting.value.c_str());
		}
		else if (setting.name.compare("analogClockPosY") == 0) {
			analogClockPosY = (float)atof(setting.value.c_str());
		}
		else if (setting.name.compare("analogClockShowLabel") == 0) {
			analogClockShowLabel = (setting.value == "1" || setting.value == "true");
		}
		else if (setting.name.compare("analogClockShowDigital") == 0) {
			analogClockShowDigital = (setting.value == "1" || setting.value == "true");
		}
		else if (setting.name.compare("analogClockShowDate") == 0) {
			analogClockShowDate = (setting.value == "1" || setting.value == "true");
		}
		else if (setting.name.compare("analogClockPixelSize") == 0) {
			int v = stoi(setting.value);
			// 将任意配置值映射到菜单预设集合（128..512，步进32）的最近值
			static const int kPixSizes[] = {128,160,192,224,256,288,320,352,384,416,448,480,512};
			int nearest = kPixSizes[0];
			int bestDiff = (v >= nearest) ? (v - nearest) : (nearest - v);
			for (size_t i = 1; i < sizeof(kPixSizes)/sizeof(kPixSizes[0]); ++i) {
				int s = kPixSizes[i];
				int d = (v >= s) ? (v - s) : (s - v);
				if (d < bestDiff) { bestDiff = d; nearest = s; }
			}
			analogClockPixelSize = nearest;
		}
	}
}

void add_time_generic_settings(std::vector<StringPairSettingDBRow>* results) {
	results->push_back(StringPairSettingDBRow{ "timeSpeedIndexWhileAiming", std::to_string(timeSpeedIndexWhileAiming) });
	results->push_back(StringPairSettingDBRow{ "timeFlowRateIndex", std::to_string(timeFlowRateIndex) });
	results->push_back(StringPairSettingDBRow{ "HotkeyFlowRateIndex", std::to_string(HotkeyFlowRateIndex) });
	results->push_back(StringPairSettingDBRow{ "analogClockStyleIndex", std::to_string(analogClockStyleIndex) });
	results->push_back(StringPairSettingDBRow{ "analogClockTimeSourceIndex", std::to_string(analogClockTimeSourceIndex) });
	results->push_back(StringPairSettingDBRow{ "analogClockPosX", std::to_string(analogClockPosX) });
	results->push_back(StringPairSettingDBRow{ "analogClockPosY", std::to_string(analogClockPosY) });
	results->push_back(StringPairSettingDBRow{ "analogClockShowLabel", std::to_string(analogClockShowLabel ? 1 : 0) });
	results->push_back(StringPairSettingDBRow{ "analogClockShowDigital", std::to_string(analogClockShowDigital ? 1 : 0) });
	results->push_back(StringPairSettingDBRow{ "analogClockShowDate", std::to_string(analogClockShowDate ? 1 : 0) });
	results->push_back(StringPairSettingDBRow{ "analogClockPixelSize", std::to_string(analogClockPixelSize) });
}

static inline void apply_freeze_time_state(bool suppressStatus = false);

void update_time_features(Player player) {
    // 处理“冻结时间”复选框状态变化（优先），并确保与“系统同步”互斥
    // 首次加载配置会将 updateFlag 置为 true，这里用静态量抑制初次提示
    static bool freezeStateInitialized = false;

    if (featureFreezeTimeUpdated) {
        featureFreezeTimeUpdated = false;
        if (featureFreezeTime) {
            // 当开启冻结时间时，关闭系统时间同步
            featureTimeSynced = false;
        }
        // 仅在“冻结时间”为关闭且首次加载时抑制“时间已解冻”提示
        apply_freeze_time_state(!featureFreezeTime && !freezeStateInitialized);
        freezeStateInitialized = true;
    }

    // 处理“时间与电脑系统同步”复选框状态变化，并确保与“冻结时间”互斥
    if (featureTimeSyncedUpdated) {
        featureTimeSyncedUpdated = false;
        if (featureTimeSynced) {
            // 关闭冻结时间（如果已开启），并恢复正常流速
            if (featureFreezeTime) {
                featureFreezeTime = false;
                // 如果是首次加载并且此处关闭了冻结时间，则抑制“时间已解冻”提示
                apply_freeze_time_state(!featureFreezeTime && !freezeStateInitialized);
                freezeStateInitialized = true;
            }
            if (timeFlowRateIndex != DEFAULT_TIME_FLOW_RATE) {
                timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE;
                timeFlowRateChanged = true;
            }
            requireRefreshOfTime = true;
        }
    }

    // 处理模拟时钟开关状态变化
    if (featureAnalogClockEnabledUpdated) {
        featureAnalogClockEnabledUpdated = false;
        if (featureAnalogClockEnabled) {
            set_status_text("模拟时钟 已开启");
        } else {
            set_status_text("模拟时钟 已关闭");
        }
    }

    // 时间同步
    if (featureTimeSynced) {
        if (timeFlowRateIndex != DEFAULT_TIME_FLOW_RATE) {
            timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE, timeFlowRateChanged = true;
        }

        time_t now = time(0);
        tm t;
        localtime_s(&t, &now);
        TIME::SET_CLOCK_TIME(t.tm_hour, t.tm_min, t.tm_sec);
    }

	if ((PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0) && featureSpeedAimInVeh) || !featureSpeedAimInVeh) slow_aim = true;
	if (!PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0) && featureSpeedAimInVeh) slow_aim = false;

	// 时间流逝速率
	if (timeFlowRateChanged) {
		timeFlowRateChanged = false;

		if (timeFlowRateIndex == DEFAULT_TIME_FLOW_RATE) {
			TIME::PAUSE_CLOCK(false);
		}
		else {
			TIME::PAUSE_CLOCK(true);
		}
		timeFactor = timeFlowRateIndex == 0 ? -1.0f : 1000.0f / TIME_FLOW_RATE_VALUES.at(timeFlowRateIndex);
		SYSTEM::SETTIMERA(0);
	}
	if (timeFlowRateIndex != DEFAULT_TIME_FLOW_RATE) {
		TIME::PAUSE_CLOCK(true);
		if (timeFlowRateIndex > 0) {
			int hours, minutes, seconds = static_cast<int>(static_cast<float>(SYSTEM::TIMERA()) / timeFactor);
			hours = seconds / 3600, seconds %= 3600;
			minutes = seconds / 60, seconds %= 60;
			SYSTEM::SETTIMERA(SYSTEM::TIMERA() - static_cast<int>(static_cast<float>(hours * 3600 + minutes * 60 + seconds) * timeFactor));
			TIME::ADD_TO_CLOCK_TIME(hours, minutes, seconds);
		}
	}

	if ((is_in_airbrake_mode() && is_airbrake_frozen_time()) || (is_in_prop_placement_mode() && is_prop_placement_frozen_time())) {
		GAMEPLAY::SET_TIME_SCALE(0.0f);
		weHaveChangedTimeScale = true;
	}
	else if (CONTROLS::IS_CONTROL_PRESSED(0, 19) || PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID())) {
		// 什么也不做，让游戏为我们选择速度
	}
	else if (is_hotkey_held_normal_speed()) {
		GAMEPLAY::SET_TIME_SCALE(1.0f);
		weHaveChangedTimeScale = true;
	}
	else if (is_hotkey_held_slow_mo()) {
		GAMEPLAY::SET_TIME_SCALE(0.0f);
		weHaveChangedTimeScale = true;
	}
	else if (is_hotkey_held_half_normal_speed()) {
		GAMEPLAY::SET_TIME_SCALE(0.4f);
		weHaveChangedTimeScale = true;
	}
	else if (!HotkeyFlowRateLocked && HotkeyFlowRateIndex != DEFAULT_HOTKEY_FLOW_RATE && PLAYER::IS_PLAYER_CONTROL_ON(player) && !PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID())) { // 通过快捷键切换游戏速度
		GAMEPLAY::SET_TIME_SCALE(TIME_SPEED_VALUES.at(HotkeyFlowRateIndex));
		weHaveChangedTimeScale = true;
	}
	else if (PLAYER::IS_PLAYER_FREE_AIMING(player) && PLAYER::IS_PLAYER_CONTROL_ON(player) && slow_aim == true) {
		if (timeSinceAimingBegan == 0) {
			timeSinceAimingBegan = GetTickCount();
		}
		else { // 这必须修复一个bug：即使未瞄准时游戏仍然卡顿
			GAMEPLAY::SET_TIME_SCALE(1.0f);
			weHaveChangedTimeScale = true;
		}

		if ((GetTickCount() - timeSinceAimingBegan) < TIME_TO_SLOW_AIM) {
			float fullSpeedTime = weHaveChangedTimeScale ? TIME_SPEED_VALUES.at(timeSpeedIndex) : 1.0f;
			float targetTime = TIME_SPEED_VALUES.at(timeSpeedIndexWhileAiming);

			float progress = ((float)(GetTickCount() - timeSinceAimingBegan) / TIME_TO_SLOW_AIM);

			float rate = quadratic_time_transition(fullSpeedTime, targetTime, progress);

			GAMEPLAY::SET_TIME_SCALE(rate);
		}
		else {
			GAMEPLAY::SET_TIME_SCALE(TIME_SPEED_VALUES.at(timeSpeedIndexWhileAiming));
			weHaveChangedTimeScale = true;
		}
	}
	else if (weHaveChangedTimeScale) {
		GAMEPLAY::SET_TIME_SCALE(1.0f);
		weHaveChangedTimeScale = false;
	}

	if (timeSinceAimingBegan > 0 && !(PLAYER::IS_PLAYER_FREE_AIMING(player) && PLAYER::IS_PLAYER_CONTROL_ON(player))) {
		timeSinceAimingBegan = 0;
	}

	// 显示当前时间
	// 修改时间显示逻辑：当菜单左侧偏移量>=150时不隐藏时间，但自由移动和物体摆放模式时仍隐藏
	bool shouldShowTime = featureShowtime && 
		(menu_showing == false || 
		 (menu_showing == true && menuLeftOffset >= 120.0f)) &&
		!(is_in_airbrake_mode() || is_in_prop_placement_mode());
	
	if (shouldShowTime) {
		int currHours = TIME::GET_CLOCK_HOURS(); // 获取当前小时（0-23）
		int currMins = TIME::GET_CLOCK_MINUTES(); // 获取当前分钟（0-59）
		int currSecs = TIME::GET_CLOCK_SECONDS(); // 获取当前秒数（0-59）
		int calDay = TIME::GET_CLOCK_DAY_OF_MONTH(); // 获取当前日期（1-31）
		int calMon = TIME::GET_CLOCK_MONTH(); // 获取当前月份（1-12）
		int calYear = TIME::GET_CLOCK_YEAR(); // 获取当前年份（例如 2025）
		int day = TIME::GET_CLOCK_DAY_OF_WEEK(); // 获取当前星期（0-6，0表示星期日）

		// 时间格式化（小时、分钟、秒）
		char hours_to_show_char_modifiable[4]; // 小时，增加到4字节
		char mins_to_show_char_modifiable[4]; // 分钟，增加到4字节
		char secs_to_show_char_modifiable[4]; // 秒，增加到4字节
		snprintf(hours_to_show_char_modifiable, sizeof(hours_to_show_char_modifiable), "%02d", currHours); // 格式化小时（0-23 → "00" 到 "23"）
		snprintf(mins_to_show_char_modifiable, sizeof(mins_to_show_char_modifiable), "%02d", currMins); // 格式化分钟（0-59 → "00" 到 "59"）
		snprintf(secs_to_show_char_modifiable, sizeof(secs_to_show_char_modifiable), "%02d", currSecs); // 格式化秒（0-59 → "00" 到 "59"）

		// 绘制背景矩形（用于衬托时间文本，增强可读性）
		GRAPHICS::DRAW_RECT(
			0.0,     // X 坐标：矩形中心水平位置（0.0 = 屏幕左边缘，1.0 = 右边缘）
			0.20,    // Y 坐标：矩形中心垂直位置（0.20 = 距离屏幕顶部20%的位置）
			0.17,    // 宽度：占屏幕总宽度的12%
			0.03,    // 高度：占屏幕总高度的3%
			10,      // 红色分量（R=10，深灰色）
			10,      // 绿色分量（G=10，深灰色）
			10,      // 蓝色分量（B=10，深灰色）
			180      // 透明度（Alpha=150，0为完全透明，255为完全不透明，此处约59%不透明）
		);

		// 小时
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，时 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 255
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(hours_to_show_char_modifiable); // 直接使用格式化后的小时
		UI::_DRAW_TEXT(0.005, 0.185); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 分隔符 ":"
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，分隔符 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 255
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(":");
		UI::_DRAW_TEXT(0.025, 0.185); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 分钟
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，分钟 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 255
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(mins_to_show_char_modifiable); // 直接使用格式化后的分钟
		UI::_DRAW_TEXT(0.035, 0.185); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 分隔符 ":"
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，分隔符 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 255
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(":");
		UI::_DRAW_TEXT(0.055, 0.185); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 秒
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，秒 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 255); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 255
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(secs_to_show_char_modifiable); // 直接使用格式化后的秒
		UI::_DRAW_TEXT(0.065, 0.185); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 日期格式化
		char year_to_show_char_modifiable[12]; // 年，增加到12字节
		char month_to_show_char_modifiable[12]; // 月，增加到12字节
		char day_to_show_char_modifiable[12]; // 日，增加到12字节
		snprintf(year_to_show_char_modifiable, sizeof(year_to_show_char_modifiable), "%04d 年", calYear); // 将年份格式化为四位数 + " 年"
		snprintf(month_to_show_char_modifiable, sizeof(month_to_show_char_modifiable), "%02d 月", calMon); // 将月份格式化为两位数 + " 月"
		snprintf(day_to_show_char_modifiable, sizeof(day_to_show_char_modifiable), "%02d 日", calDay); // 将日期格式化为两位数 + " 日"

		// 星期处理
		char* week_to_show_char = "未知星期"; // 默认值
		if (day == 0) week_to_show_char = "星期日";
		else if (day == 1) week_to_show_char = "星期一";
		else if (day == 2) week_to_show_char = "星期二";
		else if (day == 3) week_to_show_char = "星期三";
		else if (day == 4) week_to_show_char = "星期四";
		else if (day == 5) week_to_show_char = "星期五";
		else if (day == 6) week_to_show_char = "星期六";

		// 年份
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，年 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 200); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 200
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 100);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(year_to_show_char_modifiable); // 当前世纪中的年份
		UI::_DRAW_TEXT(0.005, 0.220); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 月份
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，月 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 200); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 200
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 100);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(month_to_show_char_modifiable); // 一年中的月份
		UI::_DRAW_TEXT(0.005, 0.260); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 日期
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，日 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 200); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 200
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 100);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(day_to_show_char_modifiable); // 一个月中的日期
		UI::_DRAW_TEXT(0.005, 0.300); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）

		// 星期
		UI::SET_TEXT_FONT(fontStatus); // 设置文本字体类型，星期 4
		UI::SET_TEXT_SCALE(0.0, 0.45); // 设置文本的缩放比例，宽度为 0.0，高度为 0.45
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 200); // 设置文本颜色为黄色（RGB值为 255, 242, 0），透明度为 200
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 100);
		UI::SET_TEXT_OUTLINE();
		UI::_SET_TEXT_ENTRY("STRING");
		UI::_ADD_TEXT_COMPONENT_SCALEFORM(week_to_show_char); // 一周中的星期
		UI::_DRAW_TEXT(0.005, 0.340); // 绘制文本，指定文本的屏幕位置（x = 水平，y = 垂直）
	} // 显示当前时间结束

	if (featurehotkeytime) {
		if (GetKeyState(VK_RMENU) & 0x8000) {
			PED::SET_PED_CAN_SWITCH_WEAPON(PLAYER::PLAYER_PED_ID(), false);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(19);
			UI::HIDE_HUD_COMPONENT_THIS_FRAME(20);
		}
		else if (veh_to_spawn == "") PED::SET_PED_CAN_SWITCH_WEAPON(PLAYER::PLAYER_PED_ID(), true);

		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('1') & 0x8000) {
			movetime_set(0, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('2') & 0x8000) {
			movetime_set(5, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('3') & 0x8000) {
			movetime_set(6, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('4') & 0x8000) {
			movetime_set(8, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('5') & 0x8000) {
			movetime_set(12, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('6') & 0x8000) {
			movetime_set(16, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('7') & 0x8000) {
			movetime_set(18, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && GetKeyState('8') & 0x8000) {
			movetime_set(21, 0);
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && (/*(GetKeyState(VK_OEM_PLUS) & 0x8000) || */(GetKeyState('0') & 0x8000) || (GetKeyState(VK_ADD) & 0x8000))) {
			movetime_fivemin_forward();
		}
		if (GetKeyState(VK_RMENU) & 0x8000 && (/*(GetKeyState(VK_OEM_MINUS) & 0x8000) || */(GetKeyState('9') & 0x8000) || (GetKeyState(VK_SUBTRACT) & 0x8000))) {
			movetime_fivemin_backward();
		}
	}

	// 绘制模拟时钟（最后绘制，避免被菜单遮挡）
	if (featureAnalogClockEnabled) {
		ensure_clock_textures_loaded();
		draw_analog_clock();
	}

} // 更新时间功能结束
static inline void apply_freeze_time_state(bool suppressStatus) {
	if (featureFreezeTime) {
		if (timeFlowRateIndex != 0) {
			frozentimestate = timeFlowRateIndex;
		}
		timeFlowRateIndex = 0;
		timeFlowRateChanged = true;
		if (!suppressStatus) set_status_text("~y~时间已冻结！");
		requireRefreshOfTime = true;
	}
	else {
		if (frozentimestate != -1) {
			timeFlowRateIndex = frozentimestate;
		}
		else {
			timeFlowRateIndex = DEFAULT_TIME_FLOW_RATE;
		}
		timeFlowRateChanged = true;
		if (!suppressStatus) set_status_text("~g~时间已解冻！");
		requireRefreshOfTime = true;
	}
}

// ================= 模拟时钟：菜单与渲染 =================

static const std::vector<std::string> ANALOG_STYLE_CAPTIONS{ "默认", "风格 1", "风格 2", "风格 3", "风格 4", "风格 5", "风格 6", "风格 7", "风格 8", "风格 9", "风格 10" };
static const std::vector<std::string> ANALOG_TIME_SOURCE_CAPTIONS{ "游戏时间", "现实时间" };

// 统一的贴图名称表（供菜单回调与绘制共同使用）
static const char* kFACE_NAMES[11]  = {"Clock0_face",  "Clock1_face",  "Clock2_face",  "Clock3_face",  "Clock4_face",  "Clock5_face",  "Clock6_face",  "Clock7_face",  "Clock8_face",  "Clock9_face",  "Clock10_face"};
static const char* kHANDH_NAMES[11] = {"Clock0_handh", "Clock1_handh", "Clock2_handh", "Clock3_handh", "Clock4_handh", "Clock5_handh", "Clock6_handh", "Clock7_handh", "Clock8_handh", "Clock9_handh", "Clock10_handh"};
static const char* kHANDM_NAMES[11] = {"Clock0_handm", "Clock1_handm", "Clock2_handm", "Clock3_handm", "Clock4_handm", "Clock5_handm", "Clock6_handm", "Clock7_handm", "Clock8_handm", "Clock9_handm", "Clock10_handm"};
static const char* kHANDS_NAMES[11] = {"Clock0_hands", "Clock1_hands", "Clock2_hands", "Clock3_hands", "Clock4_hands", "Clock5_hands", "Clock6_hands", "Clock7_hands", "Clock8_hands", "Clock9_hands", "Clock10_hands"};

static void onchange_analog_style(int value, SelectFromListMenuItem* source){
	analogClockStyleIndex = value;
}

static void onchange_analog_time_source(int value, SelectFromListMenuItem* source){
	analogClockTimeSourceIndex = value;
}

static void onchange_analog_pos_x(int value, SelectFromListMenuItem* source){
	analogClockPosX = (value * 5) / 1000.0f; // value是索引(步进5)
	// 任意移动X/Y即视为自定义
	analogClockPresetIndex = 4;
	if (gAnalogPresetItem) gAnalogPresetItem->value = 4;
}

static void onchange_analog_pos_y(int value, SelectFromListMenuItem* source){
	analogClockPosY = (value * 5) / 1000.0f;
	analogClockPresetIndex = 4;
	if (gAnalogPresetItem) gAnalogPresetItem->value = 4;
}

void process_analog_clock_menu(){
	std::vector<MenuItem<int>*> menuItems;
	ToggleMenuItem<int>* togItem;
	SelectFromListMenuItem* listItem;

	// 启用模拟时钟
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "启用模拟时钟";
	togItem->value = 0;
	togItem->toggleValue = &featureAnalogClockEnabled;
	togItem->toggleValueUpdated = &featureAnalogClockEnabledUpdated;
	menuItems.push_back(togItem);

	// 样式
	listItem = new SelectFromListMenuItem(ANALOG_STYLE_CAPTIONS, onchange_analog_style);
	listItem->wrap = false;
	listItem->caption = "模拟时钟样式";
	listItem->value = analogClockStyleIndex;
	menuItems.push_back(listItem);

	// 位置（将 0..1 映射到 0..1000 的整数，步进5）
	{
		std::vector<std::string> posx;
		for(int i=0;i<=1000;i+=5){ posx.push_back(std::to_string(i)); }
		int currentX = (int)(analogClockPosX * 1000.0f + 0.5f);
		SelectFromListMenuItem* posxItem = new SelectFromListMenuItem(posx, onchange_analog_pos_x);
		posxItem->wrap = false;
		posxItem->caption = "时钟左右位置";
		posxItem->value = currentX/5;
		menuItems.push_back(posxItem);
		gAnalogPosXItem = posxItem;
	}
	{
		std::vector<std::string> posy;
		for(int i=0;i<=1000;i+=5){ posy.push_back(std::to_string(i)); }
		int currentY = (int)(analogClockPosY * 1000.0f + 0.5f);
		SelectFromListMenuItem* posyItem = new SelectFromListMenuItem(posy, onchange_analog_pos_y);
		posyItem->wrap = false;
		posyItem->caption = "时钟上下位置";
		posyItem->value = currentY/5;
		menuItems.push_back(posyItem);
		gAnalogPosYItem = posyItem;
	}

	// 时钟尺寸（像素）：按屏幕分辨率1:1绘制，保证清晰与圆形
	{
		static const std::vector<int> PIX_SIZES{
			128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512
		};
		std::vector<std::string> captions; captions.reserve(PIX_SIZES.size());
		int currentIndex = 0;
		for(size_t i=0;i<PIX_SIZES.size();++i){
			captions.push_back(std::to_string(PIX_SIZES[i]));
			if(analogClockPixelSize == PIX_SIZES[i]) currentIndex = (int)i;
		}
		auto onchange_analog_size_pixels = [](int value, SelectFromListMenuItem*){
			static const std::vector<int> PIX_SIZES_LOCAL{
				128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512
			};
			int idx = value < 0 ? 0 : (value >= (int)PIX_SIZES_LOCAL.size()? (int)PIX_SIZES_LOCAL.size()-1 : value);
			analogClockPixelSize = PIX_SIZES_LOCAL[idx];
		};
		listItem = new SelectFromListMenuItem(captions, onchange_analog_size_pixels);
		listItem->wrap = false;
		listItem->caption = "时钟尺寸（像素）";
		listItem->value = currentIndex;
		menuItems.push_back(listItem);
	}

	// 显示时间来源
	listItem = new SelectFromListMenuItem(ANALOG_TIME_SOURCE_CAPTIONS, onchange_analog_time_source);
	listItem->wrap = false;
	listItem->caption = "时间/日期 来源";
	listItem->value = analogClockTimeSourceIndex;
	menuItems.push_back(listItem);

	// 预设位置（含“自定义”），与 X/Y 列表联动
	{
		static const std::vector<std::string> PRESET_POS{ "右上", "右下", "左上", "左下", "自定义" };
		auto onpos = [](int v, SelectFromListMenuItem*){
			analogClockPresetIndex = v;
			switch(v){
				case 0: analogClockPosX = 0.90f; analogClockPosY = 0.15f; break;//右上
				case 1: analogClockPosX = 0.90f; analogClockPosY = 0.85f; break;//右下
				case 2: analogClockPosX = 0.10f; analogClockPosY = 0.15f; break;//左上
				case 3: analogClockPosX = 0.10f; analogClockPosY = 0.85f; break;//左下
				case 4: /* 自定义：不改动坐标，仅记录 */ break;//自定义
			}
			int newX = (int)(analogClockPosX * 1000.0f + 0.5f);
			if(gAnalogPosXItem) gAnalogPosXItem->value = newX / 5;
			int newY = (int)(analogClockPosY * 1000.0f + 0.5f);
			if(gAnalogPosYItem) gAnalogPosYItem->value = newY / 5;
		};
        // 依据当前坐标推断预设索引（与 PRESET_POS 顺序一致：右上, 右下, 左上, 左下, 自定义）
        int curX = (int)(analogClockPosX * 1000.0f + 0.5f);
        int curY = (int)(analogClockPosY * 1000.0f + 0.5f);
        int inferred = 4;
        if (curX == 900 && curY == 150) inferred = 0; // 右上
        else if (curX == 900 && curY == 850) inferred = 1; // 右下
        else if (curX == 100 && curY == 150) inferred = 2; // 左上
        else if (curX == 100 && curY == 850) inferred = 3; // 左下
		analogClockPresetIndex = inferred;
		SelectFromListMenuItem* preset = new SelectFromListMenuItem(PRESET_POS, onpos);
		preset->wrap = false;
		preset->caption = "预设位置";
		preset->value = analogClockPresetIndex;
		menuItems.push_back(preset);
		gAnalogPresetItem = preset;
	}

	// 标签显示开关
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "显示来源标签";
	togItem->value = 0;
	togItem->toggleValue = &analogClockShowLabel;
	togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);

	// 数字时间开关
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "显示时间";
	togItem->value = 0;
	togItem->toggleValue = &analogClockShowDigital;
	togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);
	
	// 日期显示开关
	togItem = new ToggleMenuItem<int>();
	togItem->caption = "显示日期";
	togItem->value = 0;
	togItem->toggleValue = &analogClockShowDate;
	togItem->toggleValueUpdated = NULL;
	menuItems.push_back(togItem);

	draw_generic_menu<int>(menuItems, NULL, "模拟时钟选项", NULL, NULL, NULL, NULL);
}

// 释放模拟时钟纹理资源
static inline void release_clock_textures(){
	const char* dictName = "ENT_textures";
	if(GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictName)){
		GRAPHICS::SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED((char*)dictName);
	}
}

static inline void ensure_clock_textures_loaded(){
    static bool s_prev_enabled = false;
    
    // 检测功能开关状态变化
    if(s_prev_enabled != featureAnalogClockEnabled){
        s_prev_enabled = featureAnalogClockEnabled;
        // 如果是关闭操作，释放纹理资源
        if(!featureAnalogClockEnabled){
            release_clock_textures();
            return;
        } else {
            // 开启时立即请求并进行轻量预热，减少首次空白帧
            const char* dictWarm = "ENT_textures";
            GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictWarm, false);
            for (int i = 0; i < 3; ++i) {
                if (GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictWarm)) break;
                WAIT(0);
                GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictWarm, false);
            }
        }
    }
    
    // 功能未启用，不处理
    if(!featureAnalogClockEnabled) return;
    
    // 请求加载纹理字典；如果已加载则跳过重复请求（早退）
    const char* dictName = "ENT_textures";
    if (GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictName)) {
        return;
    }
    GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dictName, false);
}

static inline void draw_analog_clock(){
    int hour=0, minute=0, second=0;
    if(analogClockTimeSourceIndex==0){
        hour = TIME::GET_CLOCK_HOURS();
        minute = TIME::GET_CLOCK_MINUTES();
        second = TIME::GET_CLOCK_SECONDS();
    }else{
        time_t now = time(0);
        tm t; localtime_s(&t, &now);
        hour = t.tm_hour; minute = t.tm_min; second = t.tm_sec;
    }
    float hourDeg = (30.0f * (float)(hour % 12)) + (0.5f * (float)minute);
    float minDeg = 6.0f * (float)minute;
    float secDeg = 6.0f * (float)second;

    // 按屏幕分辨率进行1:1像素绘制，避免压缩与模糊
    int screenW = 0, screenH = 0;
    GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&screenW, &screenH);
    float onePixelW = screenW > 0 ? (1.0f / (float)screenW) : 0.0005f;
    float onePixelH = screenH > 0 ? (1.0f / (float)screenH) : 0.0005f;
    // 防御像素尺寸异常值（保持合理范围）
    int pxSize = analogClockPixelSize;
    if (pxSize < 128) pxSize = 128; else if (pxSize > 512) pxSize = 512;
    float sizeX = pxSize * onePixelW;
    float sizeY = pxSize * onePixelH;

	// 将中心点对齐到最近的像素，减少采样模糊
    float centerX = analogClockPosX;
    float centerY = analogClockPosY;
    // 归一化坐标夹取到屏幕范围
    if (centerX < 0.0f) centerX = 0.0f; else if (centerX > 1.0f) centerX = 1.0f;
    if (centerY < 0.0f) centerY = 0.0f; else if (centerY > 1.0f) centerY = 1.0f;
    centerX = ((float)((int)(centerX * screenW + 0.5f))) * onePixelW;
    centerY = ((float)((int)(centerY * screenH + 0.5f))) * onePixelH;

	// 字典未加载则跳过绘制但保持功能开启
	const char* dictCheck = "ENT_textures";
	if(!GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dictCheck)){
		return;
	}

    const char* dict = "ENT_textures"; // 必须与注册名一致
    int styleIdx = analogClockStyleIndex;
    // 防御样式索引越界（数组大小为11）
    if (styleIdx < 0) styleIdx = 0; else if (styleIdx > 10) styleIdx = 10;
    const char* face = kFACE_NAMES[styleIdx];
    const char* handh = kHANDH_NAMES[styleIdx];
    const char* handm = kHANDM_NAMES[styleIdx];
    const char* hands = kHANDS_NAMES[styleIdx];

	// 贴图存在性检查：找不到对应贴图则跳过该层绘制，避免出现白色占位图
	auto has_texture = [](const char* d, const char* n) -> bool {
		Vector3 res = GRAPHICS::GET_TEXTURE_RESOLUTION((char*)d, (char*)n);
		return (res.x > 0.0f && res.y > 0.0f);
	};

	if (has_texture(dict, face)) {
		GRAPHICS::DRAW_SPRITE((char*)dict, (char*)face, centerX, centerY, sizeX, sizeY, 0.0f, 255,255,255,255);
	}
	if (has_texture(dict, handh)) {
		GRAPHICS::DRAW_SPRITE((char*)dict, (char*)handh, centerX, centerY, sizeX, sizeY, hourDeg, 255,255,255,255);
	}
	if (has_texture(dict, handm)) {
		GRAPHICS::DRAW_SPRITE((char*)dict, (char*)handm, centerX, centerY, sizeX, sizeY, minDeg, 255,255,255,255);
	}
	if (has_texture(dict, hands)) {
		GRAPHICS::DRAW_SPRITE((char*)dict, (char*)hands, centerX, centerY, sizeX, sizeY, secDeg, 255,255,255,255);
	}

	// 下方标签与数字时间（固定字号，位置可自适应）
	if(analogClockShowLabel || analogClockShowDigital || analogClockShowDate){
		float bottomY = centerY + (sizeY * 0.5f);
		float topY    = centerY - (sizeY * 0.5f);
		const int textOffsetPx = 10;   // 钟与第一行文字距离（固定像素）
		const int lineSpacingPx = 40;  // 行间距（固定像素）

		// 需要的行数：第一行(标签/时间合并一行) + 日期(可选)
		int firstLineNeeded = (analogClockShowLabel || analogClockShowDigital) ? 1 : 0;
		int dateLineNeeded  = analogClockShowDate ? 1 : 0;
		int totalLines = firstLineNeeded + dateLineNeeded;

		// 选择在下方还是上方绘制
		bool drawBelow = true;
		if ((bottomY + ((textOffsetPx + totalLines*lineSpacingPx) * onePixelH)) > (1.0f - 6*onePixelH)){
			drawBelow = false; // 底部空间不足，移至上方
		}

		// 当需在上方绘制时，第一行应紧贴钟表上缘；其余行向上堆叠
		// 计算文本高度，用于上方绘制时避免文本与时钟重叠
		float firstLineHeight = UI::_GET_TEXT_SCALE_HEIGHT(0.38f, fontStatus);
		float labelY = drawBelow
			? (bottomY + (textOffsetPx * onePixelH))
			: (topY - (textOffsetPx * onePixelH) - firstLineHeight);
		labelY = ((float)((int)(labelY * screenH + 0.5f))) * onePixelH; // 像素对齐

		UI::SET_TEXT_FONT(fontStatus);
		UI::SET_TEXT_SCALE(0.0, 0.38);
		UI::SET_TEXT_PROPORTIONAL(1);
		UI::SET_TEXT_COLOUR(255, 242, 0, 220);
		UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
		UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
		UI::SET_TEXT_OUTLINE();
		UI::SET_TEXT_CENTRE(1);
		UI::_SET_TEXT_ENTRY("STRING");
		char line[64] = {0};
		if(analogClockShowLabel && analogClockShowDigital){
			snprintf(line, sizeof(line), "%s %02d:%02d:%02d", analogClockTimeSourceIndex==0? "游戏时间:" : "现实时间:", hour, minute, second);
		}else if(analogClockShowLabel){
			snprintf(line, sizeof(line), "%s", analogClockTimeSourceIndex==0? "游戏时间" : "现实时间");
		}else if(analogClockShowDigital){
			snprintf(line, sizeof(line), "%02d:%02d:%02d", hour, minute, second);
		}
		if(firstLineNeeded){
			UI::_ADD_TEXT_COMPONENT_SCALEFORM(line);
			UI::_DRAW_TEXT(centerX, labelY);
		}

		// 日期行（固定字号，固定行距）
		if(analogClockShowDate){
			// 当在上方绘制时，日期应位于第一行之上（向屏幕上方偏移）
			float dateY;
			if(firstLineNeeded){
				dateY = drawBelow
					? (labelY + (lineSpacingPx * onePixelH))
					: (labelY - (lineSpacingPx * onePixelH));
			}else{
				dateY = labelY;
			}
			dateY = ((float)((int)(dateY * screenH + 0.5f))) * onePixelH;
			UI::SET_TEXT_FONT(fontStatus);
			UI::SET_TEXT_SCALE(0.0, 0.38);
			UI::SET_TEXT_PROPORTIONAL(1);
			UI::SET_TEXT_COLOUR(255, 242, 0, 220);
			UI::SET_TEXT_EDGE(3, 0, 0, 0, 255);
			UI::SET_TEXT_DROPSHADOW(10, 10, 10, 10, 255);
			UI::SET_TEXT_OUTLINE();
			UI::SET_TEXT_CENTRE(1);
			UI::_SET_TEXT_ENTRY("STRING");
			char dateLine[64] = {0};
			if(analogClockTimeSourceIndex==0){
				int year = TIME::GET_CLOCK_YEAR();
				int month = TIME::GET_CLOCK_MONTH();
				int day = TIME::GET_CLOCK_DAY_OF_MONTH();
				std::string weekday = get_day_of_game_week();
				snprintf(dateLine, sizeof(dateLine), "%04d/%02d/%02d %s", year, month, day, weekday.c_str());
			}else{
				time_t now = time(0);
				tm t; localtime_s(&t, &now);
				const char* weekdays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
				snprintf(dateLine, sizeof(dateLine), "%04d/%02d/%02d %s", 
					t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, weekdays[t.tm_wday]);
			}
			UI::_ADD_TEXT_COMPONENT_SCALEFORM(dateLine);
			UI::_DRAW_TEXT(centerX, dateY);
		}
	}
}
