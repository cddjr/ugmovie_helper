cmake_minimum_required(VERSION 3.13)

project(ugmovie_helper LANGUAGES C CXX)

if(NOT(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION MATCHES "8.4.0"))
  # 确保与绿联用的gcc版本一致
  message(FATAL_ERROR "仅支持GCC 8.4.0")
endif()

include(CMakeDependentOption)

option(LOGGER_ENABLED "Enable Logger" ON)
option(ENHANCED_MANUAL_SEARCH_ENABLED "Enable Enhanced Manual Search" OFF)
option(HIGH_RES_DOUBAN_ENABLED "Enable Hi-Res Douban Cover" OFF)
option(USE_DOUBAN_WECHAT_API "Use APIs based on Douban WeChat" ON)
CMAKE_DEPENDENT_OPTION(USE_DOUBAN_WEB_API "Use APIs based on Douban Web" OFF "USE_DOUBAN_WECHAT_API" ON)

add_subdirectory(deps)

set(sources
  src/main.cc
  src/hooks.cc

  src/network/http_client.cc

  src/tools/utils.cc
  src/tools/symbol_lookup.cc

  src/scraper/douban/api.cc
  src/scraper/douban/parser.cc
  src/scraper/tmdb/parser.cc
  src/scraper/cache.cc
  src/scraper/region.cc
  src/scraper/label.cc

  src/hooks/check_api_token.cc
  src/hooks/douban.cc
  src/hooks/tmdb_api.cc
  src/hooks/sqlite3_wraper_tv_film.cc
  src/hooks/sqlite3_wraper_label.cc
  src/hooks/HttpResponseData.cc
  src/hooks/handleMoviesEpsCreate.cc
  src/hooks/refresh_movies_func.cc
  src/hooks/download_file.cc

  src/ugreen/sqlite3_wraper_label.cc
)

set(libraries
  logger
  linuxDetours
  cjson
  curl
  event

  # libasan.a
)

set(definitions
  _GLIBCXX_USE_CXX11_ABI=0
  $<$<CONFIG:DEBUG>:DEBUG=1>
  $<$<CONFIG:DEBUG>:LOG_LVL=ELOG_LVL_VERBOSE>
  $<$<CONFIG:RelWithDebInfo>:DEBUG=2>
  $<$<CONFIG:RelWithDebInfo>:LOG_LVL=ELOG_LVL_DEBUG>
  $<$<CONFIG:RELEASE>:LOG_LVL=ELOG_LVL_INFO>
)

if(ENHANCED_MANUAL_SEARCH_ENABLED)
  list(APPEND definitions
    ENHANCED_MANUAL_SEARCH=1
  )
endif()

if(HIGH_RES_DOUBAN_ENABLED)
  list(APPEND definitions
    HIGH_RES_DOUBAN=1
  )
endif()

if(USE_DOUBAN_WECHAT_API)
  list(APPEND definitions
    USE_DOUBAN_WECHAT_API=1
  )
elseif(USE_DOUBAN_WEB_API)
  list(APPEND definitions
    USE_DOUBAN_WEB_API=1
  )
endif()

add_library(${PROJECT_NAME} SHARED ${sources})

target_compile_options(${PROJECT_NAME}
  PRIVATE

  # -fsanitize=address
  # -fno-omit-frame-pointer
  # -static-libasan
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  C_STANDARD 99
  C_VISIBILITY_PRESET hidden
  CXX_STANDARD 20
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  ${definitions}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/cJSON
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/fifo_map/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${libraries})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:-s>)
endif()

add_executable(dltest test/dltest.c)
target_link_libraries(dltest PRIVATE dl)

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/out/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${PROJECT_SOURCE_DIR}/injector.sh"
  "$<TARGET_FILE:${PROJECT_NAME}>"
  "$<TARGET_FILE:linux_injector>"
  "${PROJECT_BINARY_DIR}/out/"
)