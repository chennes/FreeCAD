add_subdirectory(App)

if(BUILD_GUI)
    add_subdirectory(Gui)
endif(BUILD_GUI)

set(Assembly_Scripts
    Init.py
    Commands.py
    TestAssemblyWorkbench.py
    Preferences.py
)

if(BUILD_GUI)
    list (APPEND Assembly_Scripts InitGui.py)
endif(BUILD_GUI)

INSTALL(
    FILES
    ${Assembly_Scripts}
    DESTINATION
    Mod/Assembly
)

SET(AssemblyScripts_SRCS
    Assembly/__init__.py
)


SET(Tests_SRCS
    Tests/__init__.py
    Tests/TestCore.py
)


SET(all_files
    ${Tests_SRCS}
    ${AssemblyScripts_SRCS}
)

ADD_CUSTOM_TARGET(AssemblyScripts ALL
    SOURCES ${all_files}
)

SET(test_files
    ${Assembly_Scripts}
    ${Tests_SRCS}
)

ADD_CUSTOM_TARGET(AssemblyTests ALL
    SOURCES ${test_files}
)

fc_copy_sources(AssemblyScripts "${CMAKE_BINARY_DIR}/Mod/Assembly" ${all_files})
fc_copy_sources(AssemblyTests "${CMAKE_BINARY_DIR}/Mod/Assembly" ${test_files})


