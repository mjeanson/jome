# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

# Qt5
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
find_package (Qt5Network CONFIG REQUIRED)

# jome-wake-up program
add_executable (
    jome-wake-up
    jome-wake-up.cpp
    q-wake-up-client.cpp
)
target_link_libraries (
    jome-wake-up
    Qt5::Network
)
target_compile_definitions (
    jome-wake-up PRIVATE
    "-DJOME_VERSION=\"${PROJECT_VERSION}\""
)
install (
    TARGETS jome-wake-up
    RUNTIME DESTINATION bin
)
