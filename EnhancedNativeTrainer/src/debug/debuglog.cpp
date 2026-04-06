/*
增强版原生训练器项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <cwctype>
#include <cstring>
#include <string>

#include "debuglog.h"
#include "../version.h"

namespace {
	const char* LOG_SEPARATOR = "---------------------------------------------------------------------------";

	enum GtaWindowType {
		GTA_WINDOW_UNKNOWN = 0,
		GTA_WINDOW_LEGACY = 1,
		GTA_WINDOW_ENHANCED = 2
	};

	std::wstring g_module_dir;
	std::wstring g_log_root_dir;
	std::wstring g_log_dir;
	std::wstring g_active_log_path;
	HANDLE g_log_file_handle = INVALID_HANDLE_VALUE;
	bool g_exit_hook_registered = false;
	bool g_log_initialized = false;
	int g_group_hour = -1;
	int g_group_minute = -1;
	int g_group_second = -1;
	std::mutex g_log_mutex;

	void close_log_file_handle();

	void close_log_file_handle_on_exit(){
		close_log_file_handle();
	}

	std::wstring get_module_directory(){
		wchar_t module_path[MAX_PATH] = {};
		DWORD path_len = GetModuleFileNameW(nullptr, module_path, MAX_PATH);
		if (path_len == 0 || path_len >= MAX_PATH) {
			wchar_t temp_path[MAX_PATH] = {};
			DWORD temp_len = GetTempPathW(MAX_PATH, temp_path);
			if (temp_len > 0 && temp_len < MAX_PATH) {
				std::wstring temp_dir(temp_path, temp_len);
				while (!temp_dir.empty() && (temp_dir.back() == L'\\' || temp_dir.back() == L'/')) {
					temp_dir.pop_back();
				}
				if (!temp_dir.empty()) {
					return temp_dir;
				}
			}
			wchar_t current_dir[MAX_PATH] = {};
			DWORD current_len = GetCurrentDirectoryW(MAX_PATH, current_dir);
			if (current_len > 0 && current_len < MAX_PATH) {
				return std::wstring(current_dir, current_len);
			}
			return L".";
		}

		std::wstring full_path(module_path, path_len);
		size_t pos = full_path.find_last_of(L"\\/");
		if (pos == std::wstring::npos) {
			return full_path;
		}
		return full_path.substr(0, pos);
	}

	void initialize_paths_if_needed(){
		if (!g_module_dir.empty()) {
			return;
		}

		g_module_dir = get_module_directory();
		g_log_root_dir = g_module_dir + L"\\Enhanced Native Trainer";
		g_log_dir = g_log_root_dir + L"\\Logs";
		if (!g_exit_hook_registered) {
			atexit(close_log_file_handle_on_exit);
			g_exit_hook_registered = true;
		}
	}

	std::wstring to_wide_text(const std::string& text){
		if (text.empty()) {
			return L"";
		}

		int utf8_required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1, nullptr, 0);
		if (utf8_required > 0) {
			std::wstring wide(static_cast<size_t>(utf8_required), L'\0');
			int converted = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1, &wide[0], utf8_required);
			if (converted <= 0) {
				return L"";
			}
			if (!wide.empty() && wide.back() == L'\0') {
				wide.pop_back();
			}
			return wide;
		}

		int acp_required = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
		if (acp_required <= 0) {
			return L"";
		}

		std::wstring wide(static_cast<size_t>(acp_required), L'\0');
		int converted = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, &wide[0], acp_required);
		if (converted <= 0) {
			return L"";
		}
		if (!wide.empty() && wide.back() == L'\0') {
			wide.pop_back();
		}
		return wide;
	}

	std::string to_utf8_text(const std::wstring& text){
		if (text.empty()) {
			return "";
		}

		int utf8_required = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (utf8_required <= 0) {
			return "";
		}

		std::string utf8(static_cast<size_t>(utf8_required), '\0');
		int converted = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, &utf8[0], utf8_required, nullptr, nullptr);
		if (converted <= 0) {
			return "";
		}
		if (!utf8.empty() && utf8.back() == '\0') {
			utf8.pop_back();
		}
		return utf8;
	}

	std::string normalize_log_text(const std::string& text){
		std::wstring wide_text = to_wide_text(text);
		if (wide_text.empty()) {
			return "";
		}

		size_t start = 0;
		size_t end = wide_text.size();
		while (start < end && iswspace(static_cast<wint_t>(wide_text[start])) != 0) {
			++start;
		}
		while (end > start && iswspace(static_cast<wint_t>(wide_text[end - 1])) != 0) {
			--end;
		}

		if (end <= start) {
			return "";
		}

		return to_utf8_text(wide_text.substr(start, end - start));
	}

	bool ensure_directory(const wchar_t* path){
		if (CreateDirectoryW(path, nullptr) != 0) {
			return true;
		}
		return GetLastError() == ERROR_ALREADY_EXISTS;
	}

	bool ensure_log_directories(){
		initialize_paths_if_needed();
		return ensure_directory(g_log_root_dir.c_str()) && ensure_directory(g_log_dir.c_str());
	}

	HANDLE open_log_file(const std::wstring& path, DWORD disposition){
		return CreateFileW(
			path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
	}

	void close_log_file_handle(){
		if (g_log_file_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(g_log_file_handle);
			g_log_file_handle = INVALID_HANDLE_VALUE;
		}
	}

	bool write_bytes(HANDLE handle, const char* data, DWORD data_size){
		DWORD total_written = 0;
		while (total_written < data_size) {
			DWORD just_written = 0;
			if (WriteFile(handle, data + total_written, data_size - total_written, &just_written, nullptr) == 0) {
				return false;
			}
			total_written += just_written;
		}
		return true;
	}

	bool append_utf8_line(const std::string& text){
		if (g_log_file_handle == INVALID_HANDLE_VALUE) {
			return false;
		}

		std::string line = text + "\r\n";
		if (!write_bytes(g_log_file_handle, line.c_str(), static_cast<DWORD>(line.size()))) {
			return false;
		}
		return FlushFileBuffers(g_log_file_handle) != 0;
	}

	std::string format_date_ymd(){
		SYSTEMTIME st = {};
		GetLocalTime(&st);
		char date_buffer[16] = {};
		sprintf_s(
			date_buffer,
			"%04u-%02u-%02u",
			static_cast<unsigned int>(st.wYear),
			static_cast<unsigned int>(st.wMonth),
			static_cast<unsigned int>(st.wDay)
		);
		return std::string(date_buffer);
	}

	bool write_log_header(HANDLE handle){
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		const std::string header = "// GTA V Enhanced Native Trainer " + VERSION_STRING + " [日期: " + format_date_ymd() + "]";
		const std::string separator = std::string(LOG_SEPARATOR) + "\r\n";
		const std::string header_line = header + "\r\n";
		bool success = write_bytes(handle, reinterpret_cast<const char*>(bom), static_cast<DWORD>(sizeof(bom)));
		if (success) {
			success = write_bytes(handle, header_line.c_str(), static_cast<DWORD>(header_line.size()));
		}
		if (success) {
			success = write_bytes(handle, separator.c_str(), static_cast<DWORD>(separator.size()));
		}
		return success;
	}

	bool initialize_log_header(const std::wstring& path, bool truncate_file){
		close_log_file_handle();
		g_log_file_handle = open_log_file(path, truncate_file ? CREATE_ALWAYS : OPEN_ALWAYS);
		HANDLE handle = g_log_file_handle;
		if (handle == INVALID_HANDLE_VALUE) {
			return false;
		}

		LARGE_INTEGER file_size = {};
		if (GetFileSizeEx(handle, &file_size) == 0) {
			close_log_file_handle();
			return false;
		}

		bool success = false;
		if (file_size.QuadPart == 0) {
			success = write_log_header(handle);
		}
		else {
			unsigned char existing_bom[3] = { 0, 0, 0 };
			LARGE_INTEGER seek_to_start = {};
			DWORD read_size = 0;
			success = SetFilePointerEx(handle, seek_to_start, nullptr, FILE_BEGIN) != 0;
			if (success) {
				success = ReadFile(handle, existing_bom, static_cast<DWORD>(sizeof(existing_bom)), &read_size, nullptr) != 0 && read_size == sizeof(existing_bom);
			}
			const bool bom_valid = success && existing_bom[0] == 0xEF && existing_bom[1] == 0xBB && existing_bom[2] == 0xBF;
			if (!bom_valid) {
				success = SetFilePointerEx(handle, seek_to_start, nullptr, FILE_BEGIN) != 0;
				if (success) {
					success = SetEndOfFile(handle) != 0;
				}
				if (success) {
					success = SetFilePointerEx(handle, seek_to_start, nullptr, FILE_BEGIN) != 0;
				}
				if (success) {
					success = write_log_header(handle);
				}
			}
			else {
				success = true;
			}
		}

		if (success) {
			LARGE_INTEGER seek_to_end = {};
			success = SetFilePointerEx(handle, seek_to_end, nullptr, FILE_END) != 0;
		}
		if (!success) {
			close_log_file_handle();
		}
		return success;
	}

	struct WindowSearchContext {
		DWORD process_id;
		const char* class_name;
		bool found;
	};

	BOOL CALLBACK enum_windows_callback(HWND hwnd, LPARAM l_param){
		WindowSearchContext* ctx = reinterpret_cast<WindowSearchContext*>(l_param);
		DWORD window_pid = 0;
		GetWindowThreadProcessId(hwnd, &window_pid);
		if (window_pid != ctx->process_id) {
			return TRUE;
		}

		char class_name[128] = {};
		if (GetClassNameA(hwnd, class_name, static_cast<int>(sizeof(class_name))) <= 0) {
			return TRUE;
		}
		if (strcmp(class_name, ctx->class_name) == 0) {
			ctx->found = true;
			return FALSE;
		}
		return TRUE;
	}

	bool has_process_window_class(DWORD process_id, const char* class_name){
		WindowSearchContext ctx = {};
		ctx.process_id = process_id;
		ctx.class_name = class_name;
		ctx.found = false;
		EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&ctx));
		return ctx.found;
	}

	GtaWindowType get_gta_window_type(){
		DWORD current_process_id = GetCurrentProcessId();
		if (has_process_window_class(current_process_id, "sgaWindow")) {
			return GTA_WINDOW_ENHANCED;
		}
		if (has_process_window_class(current_process_id, "grcWindow")) {
			return GTA_WINDOW_LEGACY;
		}
		return GTA_WINDOW_UNKNOWN;
	}

	std::wstring build_log_file_path(GtaWindowType window_type){
		initialize_paths_if_needed();
		if (window_type == GTA_WINDOW_ENHANCED) {
			return g_log_dir + L"\\ent-enhanced-cn.log";
		}
		if (window_type == GTA_WINDOW_LEGACY) {
			return g_log_dir + L"\\ent-legacy-cn.log";
		}
		return g_log_dir + L"\\ent-cn.log";
	}

	bool prepare_log_file(bool reset_for_legacy){
		if (!ensure_log_directories()) {
			return false;
		}

		GtaWindowType window_type = get_gta_window_type();
		g_active_log_path = build_log_file_path(window_type);
		if (!initialize_log_header(g_active_log_path, reset_for_legacy)) {
			return false;
		}

		g_group_hour = -1;
		g_group_minute = -1;
		g_group_second = -1;
		g_log_initialized = true;
		return true;
	}

	void get_current_time(int& hour, int& minute, int& second){
		SYSTEMTIME st = {};
		GetLocalTime(&st);
		hour = static_cast<int>(st.wHour);
		minute = static_cast<int>(st.wMinute);
		second = static_cast<int>(st.wSecond);
	}

	std::string format_time_hms(){
		int hour = 0;
		int minute = 0;
		int second = 0;
		get_current_time(hour, minute, second);

		char time_buffer[16] = {};
		sprintf_s(time_buffer, "%02d:%02d:%02d", hour, minute, second);
		return std::string(time_buffer);
	}
}

void prepare_log_file_for_new_session(){
	std::lock_guard<std::mutex> lock(g_log_mutex);
	prepare_log_file(true);
}

void clear_log_file(){
	prepare_log_file_for_new_session();
}

void write_text_to_log_file(const std::string &text){
	const std::string normalized_text = normalize_log_text(text);
	if (normalized_text.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lock(g_log_mutex);
	if (!g_log_initialized && !prepare_log_file(false)) {
		return;
	}

	int hour = 0;
	int minute = 0;
	int second = 0;
	get_current_time(hour, minute, second);

	if (g_group_hour < 0 || g_group_minute < 0) {
		g_group_hour = hour;
		g_group_minute = minute;
		g_group_second = second;
	}

	if (hour != g_group_hour || minute != g_group_minute || second != g_group_second) {
		if (!append_utf8_line(LOG_SEPARATOR)) {
			close_log_file_handle();
			if (!initialize_log_header(g_active_log_path, false) || !append_utf8_line(LOG_SEPARATOR)) {
				OutputDebugStringA("ENT 日志写入失败：分组分隔线写入失败。\n");
				return;
			}
		}
		g_group_hour = hour;
		g_group_minute = minute;
		g_group_second = second;
	}

	const std::string line = "[" + format_time_hms() + "]  " + normalized_text;
	if (!append_utf8_line(line)) {
		close_log_file_handle();
		if (!initialize_log_header(g_active_log_path, false) || !append_utf8_line(line)) {
			OutputDebugStringA("ENT 日志写入失败：日志正文写入失败。\n");
		}
	}
}
