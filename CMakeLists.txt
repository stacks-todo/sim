cmake_minimum_required(VERSION 3.10)
project(lvgl)

option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype lib" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(SDL2 REQUIRED SDL2)

add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

# In git worktrees the submodule directory may be empty.
# Fall back to the parent repository's initialized lvgl copy.
if(EXISTS "${CMAKE_SOURCE_DIR}/lvgl/CMakeLists.txt")
    add_subdirectory(lvgl)
else()
    get_filename_component(LVGL_FALLBACK
        "${CMAKE_SOURCE_DIR}/../../../lvgl" ABSOLUTE)
    if(EXISTS "${LVGL_FALLBACK}/CMakeLists.txt")
        message(STATUS "lvgl submodule empty; using: ${LVGL_FALLBACK}")
        add_subdirectory(${LVGL_FALLBACK} ${CMAKE_BINARY_DIR}/lvgl)
    else()
        message(FATAL_ERROR
            "Cannot find lvgl. Run: git submodule update --init --recursive")
    endif()
endif()

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

set(STACKS_SOURCES
    src/main.c
    src/mouse_cursor_icon.c
    src/stacks_app.c
    src/screen_clock.c
    src/screen_pomodoro.c
    src/screen_stack.c
    src/screen_table.c
)

add_executable(main ${STACKS_SOURCES})

target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg
                      ${SDL2_LIBRARIES} m pthread)
add_custom_target(run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

if(LV_USE_DRAW_SDL)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
    find_package(SDL2_image REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_libraries(main ${SDL2_IMAGE_LIBRARIES})
endif()

if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIR})
    target_link_libraries(main ${PNG_LIBRARY})
endif()

if(LV_USE_LIBJPEG_TURBO)
    find_package(JPEG REQUIRED)
    target_include_directories(lvgl PUBLIC ${JPEG_INCLUDE_DIRS})
    target_link_libraries(main ${JPEG_LIBRARIES})
endif()

if(LV_USE_FFMPEG)
    target_link_libraries(main avformat avcodec avutil swscale)
endif()

if(LV_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_link_libraries(main ${FREETYPE_LIBRARIES})
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lvgl PRIVATE
        -pedantic-errors -Wall -Wextra -Wshadow -Wundef
        -Wuninitialized -Wreturn-type -Wno-pedantic
    )
    target_compile_options(main PRIVATE -fsanitize=address,leak,undefined)
    target_link_options(main PRIVATE -fsanitize=address,leak,undefined)
endif()
