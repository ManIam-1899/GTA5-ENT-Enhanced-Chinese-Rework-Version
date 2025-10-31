/*
GDI 截图功能
使用 GDI BitBlt + GDI+ 进行窗口截图
保存为无损PNG格式
*/

#pragma once

#include <windows.h>
#include <string>
#include <vector>

// 前置声明（来自 menu_functions.h）
template<typename T> class MenuItem;
class SelectFromListMenuItem;

// ==================== 截图系统初始化与清理 ====================

// 初始化GDI截图系统
void init_gdi_screenshot_system();

// 清理GDI截图系统
void cleanup_gdi_screenshot_system();

// ==================== 截图功能核心接口 ====================

// 获取截图默认保存目录
std::string get_screenshot_directory();

// 生成截图文件名（带时间戳，根据窗口类型区分传承版和增强版）
std::string generate_screenshot_filename();

// 根据游戏窗口客户区实际像素尺寸，截取游戏画面到 PNG 文件
// 返回是否保存成功
bool capture_game_client_area_to_png(const std::string& filePath);

// 进行一次游戏窗口截图（按当前游戏窗口客户区尺寸），并自动命名保存
// 返回是否保存成功
bool take_game_screenshot_with_auto_naming();

// ==================== 截图消息显示系统 ====================

// 更新并绘制屏幕正中间的截图消息（需要在主循环中每帧调用）
void update_screenshot_message();

// 设置截图消息（在屏幕中间显示，默认显示2秒）
void set_screenshot_message(const std::string& message, DWORD displayTimeMs = 2000);

// ==================== 截图菜单与设置 ====================

// 处理截图菜单
void process_misc_screenshot_menu();

// 截图菜单确认处理函数
bool onconfirm_misc_screenshot_menu(MenuItem<int> choice);

// 截图按键修改回调函数
void onchange_misc_screenshot_key_index(int value, SelectFromListMenuItem* source);

// 执行游戏截图（检查功能是否启用）
void take_screenshot();

// ==================== 截图功能状态与设置 ====================

// 重置截图功能设置为默认值
void reset_screenshot_settings();

// 截图功能是否启用
extern bool featureScreenshotEnabled;

// 截图按键索引
extern int ScreenshotKeyIndex;

// 截图按键是否已更改
extern bool ScreenshotKeyChanged;

// 截图菜单活动行索引
extern int activeLineIndexScreenshot;

// 截图按键列表标题
extern const std::vector<std::string> MISC_SCREENSHOT_KEY_CAPTIONS;

// 截图按键值列表
extern const int MISC_SCREENSHOT_KEY_VALUES[];

