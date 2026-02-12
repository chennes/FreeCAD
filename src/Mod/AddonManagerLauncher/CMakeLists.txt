# SPDX-License-Identifier: LGPL-2.1-or-later
# SPDX-FileNotice: Part of the FreeCAD Project.

IF (BUILD_GUI)
    add_subdirectory(Gui)
ENDIF (BUILD_GUI)

add_subdirectory(Resources)

SET(AddonManagerLauncher_SRCS
    __init__.py
    Init.py
    InitGui.py
)

SOURCE_GROUP("" FILES ${AddonManagerLauncher_SRCS})

ADD_CUSTOM_TARGET(AddonManagerLauncher ALL
    SOURCES ${AddonManagerLauncher_SRCS}
)

fc_copy_sources(AddonManagerLauncher "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher" ${AddonManagerLauncher_SRCS})

INSTALL(FILES ${AddonManagerLauncher_SRCS} DESTINATION Mod/AddonManagerLauncher)
