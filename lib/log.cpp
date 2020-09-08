/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 16:03:53
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-08 16:40:41
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/log.cpp
 */

#include "lib/log.h"

#include <syslog.h>

#include <sstream>

namespace Kiran
{
Log::Log() : log_level_(G_LOG_LEVEL_WARNING),
             logger_(nullptr)
{
}

Log::~Log()
{
    if (this->logger_)
    {
        delete this->logger_;
    }
}

Log *Log::instance_ = nullptr;
void Log::global_init()
{
    RETURN_IF_TRUE(instance_);
    instance_ = new Log();
    instance_->init();
}

void Log::global_deinit()
{
    delete instance_;
    instance_ = nullptr;
}

void Log::try_append(GLogLevelFlags log_level, const char *format, ...)
{
    if (log_level > this->log_level_)
    {
        return;
    }

    int32_t priority;
    std::ostringstream oss;

    switch (log_level & G_LOG_LEVEL_MASK)
    {
    case G_LOG_FLAG_FATAL:
        priority = LOG_EMERG;
        oss << "[FATAL]";
        break;
    case G_LOG_LEVEL_ERROR:
        priority = LOG_ERR;
        oss << "[ERROR]";
        break;
    case G_LOG_LEVEL_CRITICAL:
        priority = LOG_CRIT;
        oss << "[CRITICAL]";
        break;
    case G_LOG_LEVEL_WARNING:
        priority = LOG_WARNING;
        oss << "[WARNING]";
        break;
    case G_LOG_LEVEL_MESSAGE:
        priority = LOG_NOTICE;
        oss << "[MESSAGE]";
        break;
    case G_LOG_LEVEL_INFO:
        priority = LOG_INFO;
        oss << "[INFO]";
        break;
    case G_LOG_LEVEL_DEBUG:
        priority = LOG_DEBUG;
        oss << "[DEBUG]";
        break;
    default:
        priority = LOG_DEBUG;
        oss << "[UNKNOWN]";
        break;
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsnprintf(this->message_, Log::kMessageSize, format, arg_ptr);
    va_end(arg_ptr);

    oss << ": " << this->message_ << "\n";

    auto log_content = oss.str();
    syslog(priority, "%s", log_content.c_str());
    if (this->logger_)
    {
        this->logger_->write_log(log_content.c_str(), log_content.length());
    }
}

void Log::init()
{
    // g_log_set_default_handler(log_handler, this);
}

void Log::log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    Log *log = (Log *)user_data;
    if (!log)
    {
        return;
    }

    log->try_append(log_level, message);
}

}  // namespace Kiran