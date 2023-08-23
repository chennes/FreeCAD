
#set (Assembly_TR_QRC ${CMAKE_CURRENT_BINARY_DIR}/Resources/Assembly_translation.qrc)
#qt_find_and_add_translation(QM_SRCS "Resources/translations/*_*.ts"
#    ${CMAKE_CURRENT_BINARY_DIR}/Resources/translations)
#qt_create_resource_file(${Assembly_TR_QRC} ${QM_SRCS})
qt_add_resources(AssemblyResource_SRCS Resources/Assembly.qrc ${Assembly_TR_QRC})

# SOURCE_GROUP("Resources" FILES ${PathResource_SRCS})

# # set(PathGui_UIC_SRCS
# #     DlgSettingsPathColor.ui
# #     TaskDlgPathCompound.ui
# #     DlgProcessorChooser.ui
# # )

# SET(PathGui_SRCS_Module
#     Command.cpp
#     AppPathGui.cpp
#     AppPathGuiPy.cpp
#     Resources/Path.qrc
#     PreCompiled.cpp
#     PreCompiled.h
#     DlgSettingsPathColor.ui
#     DlgSettingsPathColor.cpp
#     DlgSettingsPathColor.h
#     TaskDlgPathCompound.ui
#     TaskDlgPathCompound.cpp
#     TaskDlgPathCompound.h
#     DlgProcessorChooser.ui
#     DlgProcessorChooser.cpp
#     DlgProcessorChooser.h
# )

# SOURCE_GROUP("Module" FILES ${PathGui_SRCS_Module})


# SET(AssemblyGui_SRCS
#     ${PathResource_SRCS}
#     ${PathGui_UIC_HDRS}
#     ${PathGui_SRCS_Module}
#     ${PathGui_SRCS_ViewProvider}
# )


SET(AssemblyGuiIcon_SVG
    Resources/icons/AssemblyWorkbench.svg
)

add_library(AssemblyGui SHARED ${AssemblyGui_SRCS} ${AssemblyGuiIcon_SVG})
target_link_libraries(AssemblyGui ${PathGui_LIBS})

SET_BIN_DIR(AssemblyGui AssemblyGui /Mod/Path)
SET_PYTHON_PREFIX_SUFFIX(AssemblyGui)

fc_copy_sources(AssemblyGui "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Mod/Assembly" ${AssemblyGuiIcon_SVG})

INSTALL(TARGETS AssemblyGui DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL(FILES ${AssemblyGuiIcon_SVG} DESTINATION "${CMAKE_INSTALL_DATADIR}/Mod/Assembly/Resources/icons")
