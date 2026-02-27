/*
GDI 截图功能实现
使用 GDI BitBlt + GDI+ 进行窗口截图
保存为无损PNG格式
*/

#include "screenshot.h"
#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>

// 游戏内 UI 与音频
#include "../ui_support/menu_functions.h"
#include "../../inc/natives.h"
#include "../../inc/types.h"
#include "../../inc/enums.h"
#include "../io/config_io.h"

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

// ==================== 全局变量 ====================

// GDI+初始化状态
static bool g_gdi_screenshot_initialized = false;
static ULONG_PTR g_gdiplusToken = 0;

// 截图功能开关（默认开启）
bool featureScreenshotEnabled = true;

// 截图菜单活动行索引
int activeLineIndexScreenshot = 0;

// 截图按键索引（默认 F12）
int ScreenshotKeyIndex = 12;

// 截图按键是否已更改
bool ScreenshotKeyChanged = false;

// 截图按键列表标题
const std::vector<std::string> MISC_SCREENSHOT_KEY_CAPTIONS{ 
	"未绑定", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", 
	"Print Screen", "Scroll Lock", "Pause / Break", "Insert", "Home", "Num Lock", 
	"小键盘 /", "小键盘 *", "小键盘 -", "小键盘 +"
};

// 截图按键值列表
const int MISC_SCREENSHOT_KEY_VALUES[] = { 
	VK_NOTHING, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
	VK_SNAPSHOT, VK_SCROLL, VK_PAUSE, VK_INSERT, VK_HOME, VK_NUMLOCK, VK_DIVIDE, VK_MULTIPLY, VK_SUBTRACT, VK_ADD
};

// 截图消息显示系统
static std::string g_screenshot_message;
static DWORD g_screenshot_message_end_time = 0;
static bool g_screenshot_message_active = false;

// 截图冷却时间系统（防止连续截图时截到提示文字）
static DWORD g_last_screenshot_time = 0;
static const DWORD SCREENSHOT_COOLDOWN_MS = 2000; // 2.0秒冷却时间

// GDI+初始化
void init_gdiplus() {
	if (g_gdi_screenshot_initialized) {
		return;
	}
	
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
	g_gdi_screenshot_initialized = true;
}

// 清理GDI+
void cleanup_gdiplus() {
	if (g_gdi_screenshot_initialized) {
		GdiplusShutdown(g_gdiplusToken);
		g_gdi_screenshot_initialized = false;
	}
}

// 保存为PNG格式
bool save_bitmap_as_png(Gdiplus::Bitmap* bitmap, const std::string& filePath) {
	// 获取 PNG 编码器 CLSID（使用静态缓存避免重复枚举）
	static bool s_pngClsidCached = false;
	static CLSID s_pngClsid;
	static bool s_pngClsidOk = false;

	if (!s_pngClsidCached) {
		UINT num = 0;
		UINT size = 0;
		GetImageEncodersSize(&num, &size);
		if (size > 0 && num > 0) {
			// 使用C++风格的内存管理（RAII）
			std::vector<BYTE> buffer(size);
			ImageCodecInfo* codecInfo = reinterpret_cast<ImageCodecInfo*>(buffer.data());
			GetImageEncoders(num, size, codecInfo);
			for (UINT j = 0; j < num; ++j) {
				if (wcscmp(codecInfo[j].MimeType, L"image/png") == 0) {
					s_pngClsid = codecInfo[j].Clsid;
					s_pngClsidOk = true;
					break;
				}
			}
			// buffer会自动释放，无需手动free
		}
		s_pngClsidCached = true;
	}

	if (!s_pngClsidOk) return false;

	// 转换为宽字符路径并保存 PNG 文件
	std::wstring wFilePath(filePath.begin(), filePath.end());
	Status status = bitmap->Save(wFilePath.c_str(), &s_pngClsid, NULL);
	return (status == Ok);
}

// 初始化GDI截图系统
void init_gdi_screenshot_system() {
	init_gdiplus();
}

// 获取截图默认保存目录
std::string get_screenshot_directory() {
	// 确保父级目录存在，然后创建子目录
	std::string base = "Enhanced Native Trainer";
	CreateDirectoryA(base.c_str(), NULL);
	std::string dir = base + "\\Picture";
	CreateDirectoryA(dir.c_str(), NULL);
	return dir;
}

// 获取 GTA5 窗口类型（0=未知, 1=传承版grcWindow, 2=增强版sgaWindow）
static int get_gta_window_type() {
	HWND hwnd = NULL;
	const char* gta5_title = "Grand Theft Auto V";
	HWND foregroundWnd = GetForegroundWindow();
	
	// 检查传承版窗口（类名：grcWindow）-> GTA5[Legacy]
	hwnd = FindWindowA("grcWindow", gta5_title);
	if (hwnd && hwnd == foregroundWnd) {
		return 1; // 传承版
	}
	
	// 检查增强版窗口（类名：sgaWindow）-> GTA5[Enhanced]
	hwnd = FindWindowA("sgaWindow", gta5_title);
	if (hwnd && hwnd == foregroundWnd) {
		return 2; // 增强版
	}
	
	// 仅通过类名检查传承版
	hwnd = FindWindowA("grcWindow", NULL);
	if (hwnd && hwnd == foregroundWnd) {
		return 1; // 传承版
	}
	
	// 仅通过类名检查增强版
	hwnd = FindWindowA("sgaWindow", NULL);
	if (hwnd && hwnd == foregroundWnd) {
		return 2; // 增强版
	}
	
	return 0; // 未知
}

// 生成截图文件名（带时间戳，根据窗口类型区分传承版和增强版）
// grcWindow类 -> GTA5[Legacy]_YYYY-MM-DD_HH.MM.png
// sgaWindow类 -> GTA5[Enhanced]_YYYY-MM-DD_HH.MM.png
std::string generate_screenshot_filename() {
	// 按分钟精度的基础名：GTA5[Legacy]_YYYY-MM-DD_HH.MM 或 GTA5[Enhanced]_YYYY-MM-DD_HH.MM
	time_t rawtime;
	struct tm timeinfo;
	char baseTimeStr[32];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(baseTimeStr, sizeof(baseTimeStr), "%Y-%m-%d_%H.%M", &timeinfo);

	// 根据窗口类型确定文件名前缀
	int windowType = get_gta_window_type();
	std::string prefix;
	if (windowType == 1) {
		prefix = "GTA5[Legacy]_"; // 传承版 grcWindow
	} else if (windowType == 2) {
		prefix = "GTA5[Enhanced]_"; // 增强版 sgaWindow
	} else {
		prefix = "GTA5_"; // 未知类型，使用默认前缀
	}

	std::string dir = get_screenshot_directory();
	std::string baseName = prefix + baseTimeStr;
	std::string path = dir + "\\" + baseName + ".png";

	// 如果同一分钟内多次截图，则追加 _01, _02 ...
	auto file_exists = [](const std::string& p) -> bool {
		DWORD attrs = GetFileAttributesA(p.c_str());
		return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
	};

	if (!file_exists(path)) {
		return path;
	}

	for (int i = 1; i <= 99; ++i) {
		char suffix[8];
		sprintf_s(suffix, "_%02d", i);
		std::string candidate = dir + "\\" + baseName + suffix + ".png";
		if (!file_exists(candidate)) {
			return candidate;
		}
	}

	// 兜底：若过多，则退化为“当前”秒级时间戳避免覆盖
	time_t nowRaw = time(nullptr);
	localtime_s(&timeinfo, &nowRaw);
	char fallbackStr[32];
	strftime(fallbackStr, sizeof(fallbackStr), "%Y-%m-%d_%H.%M.%S", &timeinfo);
	return dir + "\\" + prefix + fallbackStr + ".png";
}

// 清理GDI截图系统
void cleanup_gdi_screenshot_system() {
	cleanup_gdiplus();
}

// 获取 GTA5 窗口句柄（动态识别传承版和增强版）
static HWND get_gta_window_handle() {
	HWND hwnd = NULL;
	const char* gta5_title = "Grand Theft Auto V";
	
	// 尝试查找传承版窗口（类名：grcWindow）
	hwnd = FindWindowA("grcWindow", gta5_title);
	if (hwnd) {
		// 检查是否为前台活动窗口
		HWND foregroundWnd = GetForegroundWindow();
		if (hwnd == foregroundWnd) {
			return hwnd;
		}
	}
	
	// 尝试查找增强版窗口（类名：sgaWindow）
	hwnd = FindWindowA("sgaWindow", gta5_title);
	if (hwnd) {
		// 检查是否为前台活动窗口
		HWND foregroundWnd = GetForegroundWindow();
		if (hwnd == foregroundWnd) {
			return hwnd;
		}
	}
	
	// 如果上述方法都找不到，尝试仅通过类名查找
	hwnd = FindWindowA("grcWindow", NULL);
	if (hwnd) {
		HWND foregroundWnd = GetForegroundWindow();
		if (hwnd == foregroundWnd) {
			return hwnd;
		}
	}
	
	hwnd = FindWindowA("sgaWindow", NULL);
	if (hwnd) {
		HWND foregroundWnd = GetForegroundWindow();
		if (hwnd == foregroundWnd) {
			return hwnd;
		}
	}
	
	// 兜底：返回前台窗口（可能是游戏窗口）
	return GetForegroundWindow();
}

// 截取游戏窗口客户区到 PNG
bool capture_game_client_area_to_png(const std::string& filePath) {
	if (!g_gdi_screenshot_initialized) {
		init_gdiplus();
	}

	HWND hwnd = get_gta_window_handle();
	if (!hwnd) {
		return false;
	}

	RECT clientRect{};
	if (!GetClientRect(hwnd, &clientRect)) {
		return false;
	}

	POINT ptClientOrigin{0, 0};
	if (!ClientToScreen(hwnd, &ptClientOrigin)) {
		return false;
	}

	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	if (width <= 0 || height <= 0) {
		return false;
	}

	// 屏幕与内存 DC
	HDC screenDC = GetDC(NULL);
	if (!screenDC) return false;
	HDC memDC = CreateCompatibleDC(screenDC);
	if (!memDC) {
		ReleaseDC(NULL, screenDC);
		return false;
	}

	HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, width, height);
	if (!hBitmap) {
		DeleteDC(memDC);
		ReleaseDC(NULL, screenDC);
		return false;
	}
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

	// 从客户区屏幕坐标复制像素
	if (!BitBlt(memDC, 0, 0, width, height, screenDC, ptClientOrigin.x, ptClientOrigin.y, SRCCOPY)) {
		SelectObject(memDC, oldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(memDC);
		ReleaseDC(NULL, screenDC);
		return false;
	}

	Gdiplus::Bitmap bitmap(hBitmap, NULL);
	bool result = save_bitmap_as_png(&bitmap, filePath);

	SelectObject(memDC, oldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(memDC);
	ReleaseDC(NULL, screenDC);
	return result;
}

// 自动命名并完成一次截图，含提示音与屏幕正中间消息
bool take_game_screenshot_with_auto_naming() {
	if (!g_gdi_screenshot_initialized) {
		init_gdi_screenshot_system();
	}

	// 检查截图冷却时间（防止短时间内多次截图导致截到提示文字）
	DWORD currentTime = GetTickCount();
	if (currentTime - g_last_screenshot_time < SCREENSHOT_COOLDOWN_MS) {
		// 冷却中，静默拒绝（不显示提示）
		return false;
	}

	std::string path = generate_screenshot_filename();

	// 播放快门声
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Camera_Shoot", "Phone_SoundSet_Michael", true);

	bool ok = capture_game_client_area_to_png(path);
	
	// 更新最后截图时间
	g_last_screenshot_time = GetTickCount();
	
	// 截图完成后显示消息
	if (ok) {
		// 提取文件名（不含路径）
		size_t lastSlash = path.find_last_of("\\/");
		std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
		
		// 只显示文件名，不显示冷却时间
		std::string message = "截图已保存: " + filename;
		set_screenshot_message(message, 1800); // 显示1.8秒
	} else {
		// 失败时只显示一行
		set_screenshot_message("游戏截图失败！", 1800); // 显示1.8秒
	}
	return ok;
}

// ==================== 截图消息显示系统 ====================

// 设置截图消息（在屏幕中间显示，默认2秒）
void set_screenshot_message(const std::string& message, DWORD displayTimeMs) {
	g_screenshot_message = message;
	g_screenshot_message_end_time = GetTickCount() + displayTimeMs;
	g_screenshot_message_active = true;
}

// 更新并绘制屏幕正中间的截图消息
void update_screenshot_message() {
	if (!g_screenshot_message_active) {
		return;
	}

	// 检查是否超时
	DWORD currentTime = GetTickCount();
	if (currentTime >= g_screenshot_message_end_time) {
		g_screenshot_message_active = false;
		return;
	}

	// 绘制文本到屏幕正中间
	// 位置：X=0.5（屏幕水平中间），Y=0.5（屏幕垂直中间）
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(0.45f, 0.45f);
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	UI::SET_TEXT_WRAP(0.0f, 1.0f);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 0, 0, 0, 255);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((char*)g_screenshot_message.c_str());
	UI::_DRAW_TEXT(0.5f, 0.5f);
}

// ==================== 截图功能重置 ====================

// 外部函数声明（来自misc.cpp）
extern char* keyValToName(int keyValue);

// 只更新XML配置文件中的单个按键配置（避免影响其他按键设置）
static void update_single_key_in_xml(const std::string& keyFunction) {
	HRESULT hrInit = CoInitialize(NULL);
	
	// 创建XML文档
	MSXML2::IXMLDOMDocumentPtr spXMLDoc;
	HRESULT hr = spXMLDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (FAILED(hr)) {
		if (SUCCEEDED(hrInit)) CoUninitialize();
		return;
	}
	
	// 设置preserveWhiteSpace为true以保留空行和格式
	spXMLDoc->put_preserveWhiteSpace(VARIANT_TRUE);
	
	// 加载现有的XML文件
	if (!spXMLDoc->load("Enhanced Native Trainer/ent-config.xml")) {
		if (SUCCEEDED(hrInit)) CoUninitialize();
		return;
	}
	
	// 获取按键配置管理器
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) {
		if (SUCCEEDED(hrInit)) CoUninitialize();
		return;
	}
	
	// 获取当前按键配置
	KeyConfig* key = keyConfig->get_key(keyFunction);
	if (key == NULL) {
		if (SUCCEEDED(hrInit)) CoUninitialize();
		return;
	}
	
	// 构建XPath查询，查找特定function的按键节点
	std::wstring xpath = L"//ent-config/keys/key[@function='" + 
		std::wstring(keyFunction.begin(), keyFunction.end()) + L"']";
	
	IXMLDOMNodePtr node = spXMLDoc->selectSingleNode(xpath.c_str());
	
	bool nodeExists = (node != NULL);
	
	if (nodeExists) {
		// 如果节点存在，更新它的value属性
		IXMLDOMNamedNodeMap* attribs = nullptr;
		node->get_attributes(&attribs);
		
		if (attribs != NULL) {
			// 更新value属性
			IXMLDOMNode* valueNode = nullptr;
			attribs->getNamedItem(L"value", &valueNode);
			
			if (valueNode != NULL) {
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
			
			attribs->Release();
		}
	}
	
	// 如果节点不存在，需要手动插入到正确位置（在freecam_toggle之后）
	if (!nodeExists) {
		// 释放DOM资源（因为不需要使用DOM方式）
		spXMLDoc.Release();
		if (SUCCEEDED(hrInit)) CoUninitialize();
		
		// 重新初始化COM（用于文件操作）
		hrInit = CoInitialize(NULL);
		
		// 直接读取原始XML文件
		std::ifstream origFile("Enhanced Native Trainer/ent-config.xml", std::ios::binary);
		if (!origFile.is_open()) {
			if (SUCCEEDED(hrInit)) CoUninitialize();
			return;
		}
		std::string content((std::istreambuf_iterator<char>(origFile)), std::istreambuf_iterator<char>());
		origFile.close();
		
		// 获取按键值名称
		std::string keyValueName = keyValToName(key->keyCode);
		
		// 获取按键显示名称（用于注释）
		std::string keyDisplayName = "F12"; // 默认显示名称
		if (ScreenshotKeyIndex >= 0 && ScreenshotKeyIndex < (int)MISC_SCREENSHOT_KEY_CAPTIONS.size()) {
			keyDisplayName = MISC_SCREENSHOT_KEY_CAPTIONS[ScreenshotKeyIndex];
		}
		
		// 构建要插入的内容（保持与原文件相同的格式）
		std::string insertContent = "\r\n\t<key function=\"screenshot\" value=\"" + keyValueName + "\"/>\r\n";
		insertContent += "\t<!-- 游戏全屏截图，按键为 " + keyDisplayName + " -->\r\n";
		
		// 查找freecam_toggle注释的位置（在这之后插入）
		size_t insertPos = content.find("<!-- 开启/关闭，自由相机模式，按键为 F7 -->");
		
		if (insertPos != std::string::npos) {
			// 找到注释行的结束位置
			size_t lineEndPos = content.find('\n', insertPos);
			if (lineEndPos != std::string::npos) {
				// 在注释行之后插入
				content.insert(lineEndPos + 1, insertContent);
			}
		}
		
		// 直接保存处理后的内容
		std::ofstream outFile("Enhanced Native Trainer/ent-config.xml", std::ios::binary);
		if (outFile.is_open()) {
			outFile.write(content.c_str(), content.length());
			outFile.close();
		}
		
		if (SUCCEEDED(hrInit)) CoUninitialize();
		return;
	}
	
	// 如果节点存在，保存XML文件到临时文件
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
	if (xmlDeclPos != std::string::npos) {
		xmlDeclPos += 2; // 移动到?>后面
		// 检查后面是否有换行符
		if (xmlDeclPos < content.length() && content[xmlDeclPos] != '\n' && content[xmlDeclPos] != '\r') {
			// 添加换行符（使用Windows CRLF格式）
			content.insert(xmlDeclPos, "\r\n");
		}
	}
	
	// 确保<ent-config>前面有换行符
	size_t entConfigPos = content.find("<ent-config>");
	if (entConfigPos != std::string::npos && entConfigPos > 0) {
		// 检查前面是否有换行符
		if (content[entConfigPos - 1] != '\n' && content[entConfigPos - 1] != '\r') {
			// 添加换行符（使用Windows CRLF格式）
			content.insert(entConfigPos, "\r\n");
		}
	}
	
	// 将所有LF转换为CRLF（强制二进制模式写入并手动转换换行符）
	std::string result;
	result.reserve(content.length() * 2); // 预分配足够空间
	for (size_t i = 0; i < content.length(); i++) {
		if (content[i] == '\n' && (i == 0 || content[i - 1] != '\r')) {
			// 发现单独的LF，转换为CRLF
			result += "\r\n";
		} else {
			// 保持原字符
			result += content[i];
		}
	}
	
	// 确保文件末尾有一个空行（</ent-config>后面）
	if (!result.empty() && result[result.length() - 1] != '\n') {
		result += "\r\n";
	}
	
	// 以二进制模式写入最终文件，确保CRLF格式
	std::ofstream outFile("Enhanced Native Trainer/ent-config.xml", std::ios::binary);
	outFile.write(result.c_str(), result.length());
	outFile.close();
	
	if (SUCCEEDED(hrInit)) CoUninitialize();
}

// 更新XML配置文件中的截图按键（只更新截图按键，不影响其他配置）
static void update_screenshot_key_in_xml() {
	// 获取按键配置管理器
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig == NULL) {
		return;
	}
	
	// 获取当前截图按键值并转换为键名
	int keyValue = MISC_SCREENSHOT_KEY_VALUES[ScreenshotKeyIndex];
	char* keyValueName = keyValToName(keyValue);
	
	// 更新内存中的按键配置
	keyConfig->set_key((char*)KeyConfig::KEY_SCREENSHOT.c_str(), keyValueName, false, false, false);
	
	// 只更新XML中的截图按键配置（不影响其他按键）
	update_single_key_in_xml(KeyConfig::KEY_SCREENSHOT);
}

// 重置截图功能设置为默认值
void reset_screenshot_settings() {
	featureScreenshotEnabled = true; // 默认开启
	ScreenshotKeyIndex = 12; // 默认 F12
	ScreenshotKeyChanged = false;
	activeLineIndexScreenshot = 0;
	
	// 重置截图消息和冷却时间
	g_screenshot_message_active = false;
	g_last_screenshot_time = 0;
	
	// 更新KeyInputConfig中的截图按键配置为默认F12
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		int keyValue = MISC_SCREENSHOT_KEY_VALUES[12]; // F12的键值
		char* keyValueName = keyValToName(keyValue);
		keyConfig->set_key((char*)KeyConfig::KEY_SCREENSHOT.c_str(), keyValueName, false, false, false);
	}
	
	// 更新XML配置文件中的截图按键（将按键重置为默认F12）
	update_screenshot_key_in_xml();
}

// ==================== 截图菜单与设置 ====================

// 从按键值反推索引（用于菜单显示）
static int keyValueToIndex(int keyValue) {
	for (size_t i = 0; i < sizeof(MISC_SCREENSHOT_KEY_VALUES) / sizeof(int); i++) {
		if (MISC_SCREENSHOT_KEY_VALUES[i] == keyValue) {
			return (int)i;
		}
	}
	return 12; // 默认F12
}

// 截图按键修改回调函数
void onchange_misc_screenshot_key_index(int value, SelectFromListMenuItem* source) {
	ScreenshotKeyIndex = value;
	ScreenshotKeyChanged = true;
	
	// 立即更新KeyInputConfig以使更改生效
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	if (keyConfig != NULL) {
		int keyValue = MISC_SCREENSHOT_KEY_VALUES[value];
		char* keyValueName = keyValToName(keyValue);
		keyConfig->set_key((char*)KeyConfig::KEY_SCREENSHOT.c_str(), keyValueName, false, false, false);
	}
	
	// 不立即写入XML，等退出菜单时再写入
}

// 截图菜单退出处理函数
void onexit_misc_screenshot_menu(bool result) {
	// 清除菜单每帧回调（避免影响其他菜单）
	clear_menu_per_frame_call();
	
	// 如果按键已更改，写入XML配置文件
	if (ScreenshotKeyChanged) {
		update_screenshot_key_in_xml();
		ScreenshotKeyChanged = false; // 重置更改标志
	}
}

// 截图菜单确认处理函数
bool onconfirm_misc_screenshot_menu(MenuItem<int> choice) {
	return false;
}

// 截图菜单中用于跟踪功能开关状态的变量
static bool g_last_screenshot_enabled_state = false;

// 截图菜单每帧更新回调（检查开关状态变化并显示提示）
static void screenshot_menu_per_frame_call() {
	// 如果状态发生变化，显示提示
	if (featureScreenshotEnabled != g_last_screenshot_enabled_state) {
		if (featureScreenshotEnabled) {
			set_status_text("游戏截图功能-已启用");
		} else {
			set_status_text("游戏截图功能-已关闭");
		}
		g_last_screenshot_enabled_state = featureScreenshotEnabled;
	}
}

// 截图菜单实现
void process_misc_screenshot_menu() {
	const std::string caption = "游戏截图设置";
	
	// 记录进入菜单时的状态
	g_last_screenshot_enabled_state = featureScreenshotEnabled;
	
	// 设置每帧回调，用于检查开关状态变化
	set_menu_per_frame_call(screenshot_menu_per_frame_call);
	
	// 从XML实际配置中获取当前按键值，反推索引（确保菜单显示和XML一致）
	KeyInputConfig* keyConfig = get_config()->get_key_config();
	KeyConfig* screenshotKeyConfig = keyConfig->get_key(KeyConfig::KEY_SCREENSHOT);
	int actualKeyValue = (screenshotKeyConfig != NULL) ? screenshotKeyConfig->keyCode : VK_F12;
	int actualKeyIndex = keyValueToIndex(actualKeyValue);
	
	// 如果索引和XML不一致，更新索引（保持同步）
	if (actualKeyIndex != ScreenshotKeyIndex) {
		ScreenshotKeyIndex = actualKeyIndex;
	}
	
	std::vector<MenuItem<int>*> menuItems;
	SelectFromListMenuItem *listItem;
	
	// 添加启用截图功能复选框
	ToggleMenuItem<int>* toggleItem = new ToggleMenuItem<int>();
	toggleItem->caption = "启用游戏截图";
	toggleItem->toggleValue = &featureScreenshotEnabled;
	menuItems.push_back(toggleItem);
	
	// 添加截图快捷键设置（显示实际应用的按键索引）
	listItem = new SelectFromListMenuItem(MISC_SCREENSHOT_KEY_CAPTIONS, onchange_misc_screenshot_key_index);
	listItem->wrap = false;
	listItem->caption = "截图快捷键设置";
	listItem->value = ScreenshotKeyIndex; // 使用从XML反推的索引
	menuItems.push_back(listItem);
	
	// 添加截图冷却时间显示（只读文本，不可操作）
	MenuItem<int>* cooldownItem = new MenuItem<int>();
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1) << (SCREENSHOT_COOLDOWN_MS / 1000.0f);
	cooldownItem->caption = "截图冷却时间: " + oss.str() + " 秒";
	cooldownItem->isLeaf = true;
	cooldownItem->value = 0;
	menuItems.push_back(cooldownItem);
	
	// 添加截图目录显示（只读文本，不可操作）
	MenuItem<int>* dirItem = new MenuItem<int>();
	dirItem->caption = "截图目录: Enhanced Native Trainer\\Picture";
	dirItem->isLeaf = true;
	dirItem->value = 0;
	menuItems.push_back(dirItem);
	
	draw_generic_menu<int>(menuItems, &activeLineIndexScreenshot, caption, onconfirm_misc_screenshot_menu, NULL, onexit_misc_screenshot_menu);
}

// 执行游戏截图
void take_screenshot() {
	// 检查功能是否启用
	if (!featureScreenshotEnabled) {
		return;
	}
	
	// 自动命名并按游戏分辨率（客户区大小）截图
	take_game_screenshot_with_auto_naming();
}

