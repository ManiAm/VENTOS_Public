
namespace VENTOS {

enum logWindowCMD
{
    CMD_ADD_TAB,
    CMD_ADD_SUB_TEXTVIEW,
    CMD_INSERT_TXT,
    CMD_FLUSH,
};

#define   WARNING_LOG_VAL   0b00000001
#define   INFO_LOG_VAL      0b00000010
#define   ERROR_LOG_VAL     0b00000100
#define   DEBUG_LOG_VAL     0b00001000
#define   EVENT_LOG_VAL     0b00010000   // event log
#define   ALL_LOG_VAL       0b11111111
#define   NO_LOG_VAL        0b00000000

// logs redirected to the standard output stream

#define WARNING_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::WARNING()

#define INFO_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::INFO()

#define ERROR_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::ERROR()

#define DEBUG_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::DEBUG()

#define EVENT_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::EVENT()

#define FLUSH_LOG \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::FLUSH()

// logs redirected to the log window -- category andsubcategory

#define WARNING_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::WARNING(category, subcategory)

#define INFO_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::INFO(category, subcategory)

#define ERROR_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::ERROR(category, subcategory)

#define DEBUG_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::DEBUG(category, subcategory)

#define EVENT_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::EVENT(category, subcategory)

#define FLUSH_LOG_C(category, subcategory) \
        std::lock_guard<std::mutex>{vlog::lock_log}, vlog::FLUSH(category, subcategory)
}
