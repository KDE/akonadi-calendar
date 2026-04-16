# SPDX-FileCopyrightText: 2026 Alan Thouvenin <alan.thouvenin@enioka.com>
# SPDX-License-Identifier: BSD-2-Clause

set(bindings_library "AkonadiCalendar")

set(wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h)
set(typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/typesystem_akonadi_calendar.xml)

set(dependencies
    KPim6::AkonadiCalendar
    KPim6::AkonadiCore
    KPim6::AkonadiWidgets
    KF6::CalendarCore
)

set(generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadicalendar_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_etmcalendar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_todopurger_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_history_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_incidencechanger_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_incidencetreemodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_searchcollectionhelper_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_freebusymanager_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_icalimporter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_collectioncalendar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_publishdialog_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_fetchjobcalendar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_blockalarmsattribute_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_calendarclipboard_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_standardcalendaractionmanager_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_calfilterproxymodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_calendarbase_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_calfilterpartstatusproxymodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_itiphandler_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_todomodel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/AkonadiCalendar/akonadi_calendarutils_wrapper.cpp
)

ecm_generate_python_bindings(
        PACKAGE_NAME ${bindings_library}
        VERSION ${PIM_VERSION}
        WRAPPED_HEADER ${wrapped_header}
        TYPESYSTEM ${typesystem_file}
        EXPORT_TYPESYSTEM
        GENERATED_SOURCES ${generated_sources}
        DEPENDENCIES ${dependencies}
        QT_VERSION ${QT_REQUIRED_VERSION}
        HOMEPAGE_URL "https://invent.kde.org/pim/akonadi-calendar"
        ISSUES_URL "https://bugs.kde.org/describecomponents.cgi?product=akonadi"
        AUTHOR "The KDE Community"
        README ../README.md
)

get_target_property(AKONADI_CORE_INCLUDE_DIR KPim6::AkonadiCore INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(CALENDAR_CORE_INCLUDE_DIR KF6::CalendarCore INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(
    ${bindings_library}
    PRIVATE
        ${AKONADI_CORE_INCLUDE_DIR}/akonadi
        ${CALENDAR_CORE_INCLUDE_DIR}/kcalendarcore
)

target_link_libraries(
    ${bindings_library}
    PRIVATE
        KPim6::AkonadiCalendar
        KPim6::AkonadiCore
        KPim6::AkonadiWidgets
        KF6::CalendarCore
)

execute_process(
    COMMAND
        ${Python_EXECUTABLE} -Esc
        "import sysconfig; print(sysconfig.get_path('platlib', vars={'platbase': '${CMAKE_INSTALL_PREFIX}', 'base': '${CMAKE_INSTALL_PREFIX}'}))"
    OUTPUT_VARIABLE sysconfig_output
)
string(STRIP ${sysconfig_output} PYTHON_INSTALL_DIR)

install(TARGETS ${bindings_library} LIBRARY DESTINATION "${PYTHON_INSTALL_DIR}")
