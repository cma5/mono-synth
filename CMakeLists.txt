cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "path/to/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(modeld
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "modeld VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(modeld
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(modeld
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(modeld
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(modeld
        RESOURCES
            "resource/myplugineditor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (modeld
    RESOURCES
        resource/693E2A0AA5495623B6D69DD23B8FE3DF_snapshot.png
        resource/693E2A0AA5495623B6D69DD23B8FE3DF_snapshot_2.0x.png
)

target_link_libraries(modeld
    PRIVATE
        sdk
)

smtg_target_configure_version_file(modeld)

if(SMTG_MAC)
    smtg_target_set_bundle(modeld
        BUNDLE_IDENTIFIER de.cma5.modeld
        COMPANY_NAME "cma5"
    )
    smtg_target_set_debug_executable(modeld
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(modeld PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT modeld)

        smtg_target_set_debug_executable(modeld
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
