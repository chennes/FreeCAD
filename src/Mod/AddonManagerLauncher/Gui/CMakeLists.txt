
SET(AddonManagerLauncher_Gui_SRCS
    __init__.py
    Launcher.py
    Downloader.py
)

ADD_CUSTOM_TARGET(AddonManagerLauncherGui ALL
    SOURCES ${AddonManagerLauncher_Gui_SRCS}
)

fc_copy_sources(AddonManagerLauncherGui "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher/Gui" ${AddonManagerLauncher_Gui_SRCS})

INSTALL(FILES ${AddonManagerLauncher_Gui_SRCS} DESTINATION Mod/AddonManagerLauncher/Gui)
