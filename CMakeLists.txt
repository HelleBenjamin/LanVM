cmake_minimum_required(VERSION 3.10)

project(lanvm)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall") # Optimize build

# Project's headers
include_directories(include)

# LanVM sources
file(GLOB VM_FILES src/lanvm/*.c)

# Lasm sources
file(GLOB ASM_FILES src/lasm/*.c)

# Glad source
file(GLOB GLAD_FILES src/glad/*.c)

option(BUILD_VM "Build VM" ON)
option(BUILD_ASM "Build ASM" ON)

if(BUILD_VM)
  add_executable(lanvm ${VM_FILES} ${GLAD_FILES})
endif()
if (BUILD_ASM)
  add_executable(lasm ${ASM_FILES})
endif()


if(UNIX)
  message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    target_link_directories(lanvm PRIVATE ${CMAKE_SOURCE_DIR}/lib/linux/aarch64)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    target_link_directories(lanvm PRIVATE ${CMAKE_SOURCE_DIR}/lib/linux/x86_64)
  endif()
  target_link_libraries(lanvm PRIVATE GL glfw3 m)
elseif(WIN32)
  target_link_directories(lanvm PRIVATE ${CMAKE_SOURCE_DIR}/lib/win32)
  target_link_libraries(lanvm PRIVATE win32 gdi32 opengl32 glfw)
endif()


set_target_properties(lanvm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})