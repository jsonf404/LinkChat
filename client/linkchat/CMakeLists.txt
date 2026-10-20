set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

file(GLOB LINKCHAT_CLIENT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB LINKCHAT_CLIENT_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB LINKCHAT_CLIENT_FORMS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")

add_executable(LinkChat WIN32
    ${LINKCHAT_CLIENT_SOURCES}
    ${LINKCHAT_CLIENT_HEADERS}
    ${LINKCHAT_CLIENT_FORMS}
    rc.qrc
)

target_link_libraries(LinkChat PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_compile_definitions(LinkChat PRIVATE QT_DEPRECATED_WARNINGS)

if(MSVC)
    target_compile_options(LinkChat PRIVATE /utf-8 /wd4819)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/static")
    add_custom_command(TARGET LinkChat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/static"
            "$<TARGET_FILE_DIR:LinkChat>/static"
        VERBATIM
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.ini")
    add_custom_command(TARGET LinkChat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"
            "$<TARGET_FILE_DIR:LinkChat>/config.ini"
        VERBATIM
    )
endif()
