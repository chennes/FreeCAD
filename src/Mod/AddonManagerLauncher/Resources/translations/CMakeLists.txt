SET(AddonManagerLauncherResourceFilesTranslations
)

ADD_CUSTOM_TARGET(AddonManagerLauncherTranslations ALL
    SOURCES ${AddonManagerLauncherResourceFilesTranslations}
)

fc_copy_sources(AddonManagerLauncherTranslations "${CMAKE_BINARY_DIR}/Mod/AddonManagerLauncher/Resources/Translations" ${AddonManagerLauncherResourceFilesTranslations})

INSTALL(FILES ${AddonManagerLauncherResourceFilesTranslations} DESTINATION Mod/AddonManagerLauncher/Resources/Translations)
