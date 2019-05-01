cmake_minimum_required(VERSION 3.8)

# This batch of set statements will need their values changed to values
# that are appropriate for your development environment.
set(NormalSystemLibraries "/usr/lib64")

set(CMAKE_C_FLAGS "-std=c11 -I/usr/include -I. -Wall")

# Now get on with business and get the library built
project(sc_open)

message("CFLAGS for this build are ${CMAKE_C_FLAGS}")

file(GLOB sc_open_sources ./source/*.c)
set(sc_open_srcs ${sc_open_sources}) 

add_executable(sc_open ${sc_open_srcs})

# Add link library paths, BTW, this line needs to be before the add_library line below
link_directories(${NormalSystemLibraries})

target_include_directories(sc_open PRIVATE ./headers)


