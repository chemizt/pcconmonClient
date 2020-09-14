cmake_minimum_required(VERSION 3.5)

project(pcconmonClient LANGUAGES CXX)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
    set(ADDITIONAL_LIBRARIES pthread)
else()
    if(MINGW)
        set(CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
        set(ADDITIONAL_LIBRARIES ws2_32)
    elseif(WIN32)
        set(CMAKE_CXX_FLAGS_DEBUG "/Od /Wall /std:c++17")
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ot /GL /W0 /std:c++17")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "/O2 /Oi /Ot /GL /W0 /std:c++17")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
        set(ADDITIONAL_LIBRARIES ws2_32)
    endif()
endif()

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

add_subdirectory(external/pcconmonLibs)

add_executable(pcConMonClient WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_include_directories(pcConMonClient PUBLIC external/pcconmonLibs/src/lib/structureLib/include)

target_link_libraries(pcConMonClient PUBLIC structureLib PRIVATE Qt5::Widgets Qt5::Network)
