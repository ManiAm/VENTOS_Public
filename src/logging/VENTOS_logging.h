
#ifndef VENTOSLOGGING_H
#define VENTOSLOGGING_H

#include "logging/vlog.h"
#include "logging/vglog.h"

namespace VENTOS {

// check if a particular logLevel is active?
#define LOG_ACTIVE(logLevel) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::ISLOGACTIVE(logLevel)

// logs are redirected to the standard output stream (std::cout)

#define LOG_WARNING \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::WARNING(__FILE__, __LINE__)

#define LOG_INFO \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::INFO(__FILE__, __LINE__)

#define LOG_ERROR \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::ERROR(__FILE__, __LINE__)

#define LOG_DEBUG \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::DEBUG(__FILE__, __LINE__)

#define LOG_FLUSH \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vlog::FLUSH()



// logs are redirected to the 'logWindow'

#define GLOG(tab, pane) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vglog::GLOGF(tab, pane)

#define GLOG_FLUSH(tab, pane) \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vglog::GFLUSH(tab, pane)

#define GLOG_FLUSH_ALL \
        std::lock_guard<std::mutex>{VENTOS::vlog::lock_log}, VENTOS::vglog::GFLUSHALL()
}

#endif
