cmake_minimum_required(VERSION 3.16)
add_compile_options("-Wno-format")

include(\{IDF_PATH}/tools/cmake/project.cmake)
project(pc_monitor)

set(EXTRA_COMPONENT_DIRS ./components)