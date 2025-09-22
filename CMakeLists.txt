cmake_minimum_required(VERSION 3.16)
project(bs_pricer_ql LANGUAGES CXX)

# 切到 C++17（QuantLib 1.39 需要）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bs_pricer_ql
  src/main.cpp
  src/csv.cpp
  src/pricer.cpp
)

# 让编译器找到我们自己的头
target_include_directories(bs_pricer_ql PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 手动包含 & 链接 Homebrew 的 QuantLib（不依赖 QuantLibConfig.cmake）
include_directories(/opt/homebrew/include)
find_library(QUANTLIB_LIB QuantLib HINTS /opt/homebrew/lib)
if(NOT QUANTLIB_LIB)
  message(FATAL_ERROR "Could not find libQuantLib in /opt/homebrew/lib")
endif()
target_link_libraries(bs_pricer_ql PRIVATE ${QUANTLIB_LIB})
