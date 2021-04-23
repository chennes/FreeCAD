
SET(ThemeTemplates_Files
    Arch_Colors_theme_template.cfg
    Console_Colors_theme_template.cfg
    Draft_Colors_theme_template.cfg
    Editor_Colors_theme_template.cfg
    Path_Colors_theme_template.cfg
    Sketcher_Colors_theme_template.cfg
    Start_Colors_theme_template.cfg
    TechDraw_Colors_theme_template.cfg
    Window_Colors_theme_template.cfg
)

ADD_CUSTOM_TARGET(ThemeTemplates_data ALL
    SOURCES ${ThemeTemplates_Files}
)

fc_copy_sources(ThemeTemplates_data "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Gui/ThemeTemplates"
                                  ${ThemeTemplates_Files})

INSTALL(
    FILES
        ${ThemeTemplates_Files}
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Gui/ThemeTemplates
)