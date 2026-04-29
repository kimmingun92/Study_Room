cmake_minimum_required(VERSION 3.19)
project(StudyRoom_qt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(StudyRoom_qt
    # WIN32 MACOSX_BUNDLE
    main.cpp
    mainserver.cpp
    mainserver.h
    mainserver.ui
)

target_link_libraries(StudyRoom_qt
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

install(TARGETS StudyRoom_qt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET StudyRoom_qt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
