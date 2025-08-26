cmake_minimum_required(VERSION 3.10)
project(multithreaded_browser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: where you might place prebuilt gumbo (headers + lib)
# e.g. multithreaded_browser/libs/gumbo/include and libs/gumbo/lib
set(GUMBO_HINT_ROOT "${CMAKE_SOURCE_DIR}/libs/gumbo" CACHE PATH "Path to gumbo root (include/, lib/)")

# Try to find Gumbo (optional)
find_path(GUMBO_INCLUDE_DIR
  NAMES gumbo.h
  PATHS "${GUMBO_HINT_ROOT}/include" /usr/include /usr/local/include
)

find_library(GUMBO_LIBRARY
  NAMES gumbo libgumbo
  PATHS "${GUMBO_HINT_ROOT}/lib" /usr/lib /usr/local/lib
)

if (GUMBO_INCLUDE_DIR AND GUMBO_LIBRARY)
  message(STATUS "Gumbo found: ${GUMBO_LIBRARY}")
  add_compile_definitions(USE_GUMBO)
else()
  message(STATUS "Gumbo NOT found. Falling back to naive HTML parser.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
if (GUMBO_INCLUDE_DIR)
  include_directories(${GUMBO_INCLUDE_DIR})
endif()

add_executable(browser
  src/main.cpp
  src/HttpClient.cpp
  src/HtmlParser.cpp
  src/Renderer.cpp
  src/Browser.cpp
)

if (WIN32)
  target_link_libraries(browser ws2_32)
endif()

if (GUMBO_LIBRARY)
  target_link_libraries(browser ${GUMBO_LIBRARY})
endif()
