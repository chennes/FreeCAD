pyside_wrap_rc(AddonManager_QRC_SRCS Resources/AddonManager.qrc)

set(AddonManagerGui_Python_SRCS
    addonmanager_connection_checker.py
    addonmanager_devmode.py
    addonmanager_devmode_add_content.py
    addonmanager_devmode_license_selector.py
    addonmanager_devmode_licenses_table.py
    addonmanager_devmode_metadata_checker.py
    addonmanager_devmode_person_editor.py
    addonmanager_devmode_people_table.py
    addonmanager_devmode_predictor.py
    addonmanager_devmode_validators.py
    addonmanager_firstrun.py
    addonmanager_installer_gui.py
    addonmanager_update_all_gui.py
    addonmanager_uninstaller_gui.py
    AddonManagerOptions.py
    change_branch.py
    compact_view.py
    expanded_view.py
    install_to_toolbar.py
    manage_python_dependencies.py
    package_details.py
    package_list.py
)
source_group("Python" FILES ${AddonManagerGui_Python_SRCS})

set(AddonManagerGui_UI_SRCS
    add_toolbar_button_dialog.ui
    AddonManager.ui
    AddonManagerOptions.ui
    AddonManagerOptions_AddCustomRepository.ui
    change_branch.ui
    dependency_resolution_dialog.ui
    developer_mode.ui
    developer_mode_add_content.ui
    developer_mode_advanced_freecad_versions.ui
    developer_mode_copyright_info.ui
    developer_mode_dependencies.ui
    developer_mode_edit_dependency.ui
    developer_mode_freecad_versions.ui
    developer_mode_license.ui
    developer_mode_licenses_table.ui
    developer_mode_people.ui
    developer_mode_people_table.ui
    developer_mode_select_from_list.ui
    developer_mode_tags.ui
    first_run.ui
    PythonDependencyUpdateDialog.ui
    select_toolbar_dialog.ui
    update_all.ui
)
source_group("UI" FILES ${AddonManagerGui_UI_SRCS})

set(AddonManagerGui_Other_SRCS
    loading.html
)
source_group("Other" FILES ${AddonManagerGui_Other_SRCS})

set(AddonManagerGui_SRCS
    ${AddonManagerGui_Python_SRCS}
    ${AddonManagerGui_UI_SRCS}
    ${AddonManagerGui_Other_SRCS}
    ${AddonManager_QRC_SRCS}
    )

add_custom_target(AddonManagerGui ALL
    SOURCES
    ${AddonManagerGui_SRCS}
)

fc_copy_sources(AddonManagerGui "${CMAKE_BINARY_DIR}/Mod/AddonManager/Gui" ${AddonManagerGui_SRCS})

fc_target_copy_resource(AddonManagerGui
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/Mod/AddonManager/Gui
    ${AddonManager_QRC_SRCS})

install(
    FILES
        ${AddonManagerGui_SRCS}
        ${AddonManager_QRC_SRCS}
    DESTINATION
        Mod/AddonManager/Gui
)
