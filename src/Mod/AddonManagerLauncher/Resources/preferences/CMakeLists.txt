SET(AddonManagerLauncherResourceFilesPreferences
    addonmanagerlauncherprefs.ui
)

ADD_CUSTOM_TARGET(AddonManagerLauncherPreferences ALL
    SOURCES ${AddonManagerLauncherResourceFilesPreferences}
)

fc_copy_sources(AddonManagerLauncherPreferences "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher/Resources/Preferences" ${AddonManagerLauncherResourceFilesPreferences})

INSTALL(FILES ${AddonManagerLauncherResourceFilesPreferences} DESTINATION Mod/AddonManagerLauncher/Resources/Preferences)
