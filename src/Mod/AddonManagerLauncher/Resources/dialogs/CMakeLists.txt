SET(AddonManagerLauncherResourceFilesDialogs
        installation_warning.ui
        installing.ui
)

ADD_CUSTOM_TARGET(AddonManagerLauncherDialogs ALL
    SOURCES ${AddonManagerLauncherResourceFilesDialogs}
)

fc_copy_sources(AddonManagerLauncherDialogs "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher/Resources/Dialogs" ${AddonManagerLauncherResourceFilesDialogs})

INSTALL(FILES ${AddonManagerLauncherResourceFilesDialogs} DESTINATION Mod/AddonManagerLauncher/Resources/Dialogs)
