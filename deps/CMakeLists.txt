add_subdirectory(LinuxDetours)
#add_subdirectory(fifo_map)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_CJSON_TEST "Enable building cJSON test" OFF)
# 必须隐藏cJSON符号 避免与绿联的cJSON冲突导致崩溃
option(ENABLE_HIDDEN_SYMBOLS "Hide library symbols." ON)
add_subdirectory(cJSON)

string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} os)
set(logger_sources
    EasyLogger/easylogger/src/elog.c
    EasyLogger/easylogger/src/elog_async.c
#    EasyLogger/easylogger/src/elog_buf.c
    EasyLogger/easylogger/src/elog_utils.c
    EasyLogger/demo/os/${os}/easylogger/port/elog_port.c

    EasyLogger/easylogger/plugins/file/elog_file.c
    EasyLogger/demo/os/${os}/easylogger/port/elog_file_port.c
)

set(logger_definitions
    _ELOG_CFG_H_
    ELOG_FILTER_TAG_LVL_MAX_NUM=5
    ELOG_FILTER_TAG_MAX_LEN=30
    ELOG_FILTER_KW_MAX_LEN=16
    ELOG_OUTPUT_LVL=ELOG_LVL_VERBOSE

    _ELOG_FILE_CFG_H_
    ELOG_FILE_NAME="/tmp/ugreen_tools.log"
    ELOG_FILE_MAX_SIZE=1048576
    ELOG_FILE_MAX_ROTATE=5
)

if(LOGGER_ENABLED)
    list(APPEND logger_definitions
        ELOG_OUTPUT_ENABLE
        ELOG_COLOR_ENABLE
    )
endif()

add_library(logger STATIC ${logger_sources})
set_target_properties(logger 
    PROPERTIES
#    C_STANDARD 99
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
#    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(logger
    PUBLIC
    ${logger_definitions}
    PRIVATE
    ELOG_TERMINAL_ENABLE
    ELOG_FILE_ENABLE
    ELOG_FILE_FLUSH_CACHE_ENABLE
    ELOG_LINE_NUM_MAX_LEN=5
    ELOG_LINE_BUF_SIZE=1024
    ELOG_NEWLINE_SIGN="\\n"
)
target_include_directories(logger
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/EasyLogger/easylogger/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/EasyLogger/easylogger/plugins/file>
)