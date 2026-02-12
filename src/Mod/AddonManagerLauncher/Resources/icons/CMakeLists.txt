SET(AddonManagerLauncherResourceFilesIcons
    addon_manager_launcher.svg
)

ADD_CUSTOM_TARGET(AddonManagerLauncherIcons ALL
    SOURCES ${AddonManagerLauncherResourceFilesIcons}
)

fc_copy_sources(AddonManagerLaucherIcons "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher/Resources/icons" ${AddonManagerLauncherResourceFilesIcons})

INSTALL(FILES ${AddonManagerLauncherResourceFilesIcons} DESTINATION Mod/AddonManagerLauncher/Resources/icons)
