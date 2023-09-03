set(AddonManagerApp_SRCS
    Addon.py
    addonmanager_dependency_installer.py
    addonmanager_freecad_interface.py
    addonmanager_git.py
    addonmanager_installer.py
    addonmanager_macro.py
    addonmanager_macro_parser.py
    addonmanager_metadata.py
    addonmanager_pyside_interface.py
    addonmanager_uninstaller.py
    addonmanager_utilities.py
    addonmanager_workers_installation.py
    addonmanager_workers_startup.py
    addonmanager_workers_utility.py
    NetworkManager.py
)
source_group("" FILES ${AddonManagerApp_SRCS})

add_custom_target(AddonManagerApp ALL
    SOURCES ${AddonManagerApp_SRCS}
)

fc_copy_sources(AddonManagerApp "${CMAKE_BINARY_DIR}/Mod/AddonManager/App" ${AddonManagerApp_SRCS})

install(
    FILES
        ${AddonManagerApp_SRCS}
    DESTINATION
        Mod/AddonManager/App
)
