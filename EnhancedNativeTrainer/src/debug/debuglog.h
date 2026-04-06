/*
增强版原生训练器项目的一部分。
https://github.com/gtav-ent/GTAV-EnhancedNativeTrainer
(C) Rob Pridham 及其他贡献者 2015
*/

#include <string>

#pragma once

#ifdef _DEBUG
const bool DEBUG_LOG_ENABLED = true;
#endif
#ifndef _DEBUG
const bool DEBUG_LOG_ENABLED = false;
#endif

/**向日志文件追加一行文本，自动过滤空内容并格式化输出*/
void write_text_to_log_file(const std::string &text);

/**初始化新会话日志文件。按窗口类型使用固定文件名并重置文件内容*/
void clear_log_file();

/**为新会话准备日志文件*/
void prepare_log_file_for_new_session();
