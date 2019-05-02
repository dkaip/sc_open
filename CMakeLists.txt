cmake_minimum_required(VERSION 3.8)

# This batch of set statements will need their values changed to values
# that are appropriate for your development environment.
set(NormalSystemLibraries "/usr/lib64")

# Now get on with business and get the library built
project(sc)

# Sigh...the following must be after the project statement
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    set(CMAKE_C_FLAGS_RELEASE "-O3 -std=gnu18 -I/usr/include -Wall -Wextra")
    set(CMAKE_C_FLAGS_DEBUG "-g -std=gnu18 -I/usr/include -Wall -Wextra")

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")

    set(CMAKE_C_FLAGS_RELEASE "-O3 -std=gnu18 -I/usr/include -Wall -Wextra")
    set(CMAKE_C_FLAGS_DEBUG "-g -std=gnu18 -I/usr/include -Wall -Wextra")

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "HPUX")

    set(CMAKE_C_FLAGS_RELEASE "-O3 -Aa -DUNIX -DHP -DSYS5 -DHP700V807 -DHP9000v901 -DHP903 -DHP904 -D_HPUX_SOURCE -Wp -H20000")
    set(CMAKE_C_FLAGS_DEBUG "-g -Aa -DUNIX -DHP -DSYS5 -DHP700V807 -DHP9000v901 -DHP903 -DHP904 -D_HPUX_SOURCE -Wp -H20000")

endif()
file(GLOB sc_sources ./source/*.c)
set(sc_srcs ${sc_sources}) 

add_executable(sc ${sc_srcs})

# Add link library paths, BTW, this line needs to be before the add_library line below
link_directories(${NormalSystemLibraries})

target_include_directories(sc PRIVATE ./headers)

target_link_libraries(sc LINK_PRIVATE m)

message("--")
message("-- CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("-- OS is ${CMAKE_SYSTEM_NAME}")
message("--")
