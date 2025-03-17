# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar6\")

add_library(KPim6AkonadiCalendarCore)
add_library(KPim6::AkonadiCalendarCore ALIAS KPim6AkonadiCalendarCore)

target_sources(KPim6AkonadiCalendarCore PRIVATE
    freebusyproviderbase.cpp
    freebusyproviderbase.h
    freebusyproviderbase_p.h
)

ecm_qt_declare_logging_category(KPim6AkonadiCalendarCore
    HEADER akonadicalendarcore_debug.h
    IDENTIFIER AKONADICALENDAR_LOG
    CATEGORY_NAME org.kde.pim.akonadicalendarcore
    DESCRIPTION "akonadicalendarcore (pim lib)"
    EXPORT AKONADI-CALENDAR
)

qt_add_dbus_adaptor(akonadicalendarcore_LIB_SRC
    interfaces/org.freedesktop.Akonadi.Resource.FreeBusyProvider.xml
    freebusyproviderbase_p.h
    Akonadi::FreeBusyProviderBasePrivate
    freebusyprovideradaptor
    Akonadi__FreeBusyProviderAdaptor
)

target_sources(KPim6AkonadiCalendarCore PRIVATE ${akonadicalendarcore_LIB_SRC})
if(COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6AkonadiCalendarCore PROPERTIES UNITY_BUILD ON)
endif()

ecm_generate_export_header(KPim6AkonadiCalendarCore
    VERSION ${PIM_VERSION}
    BASE_NAME akonadi_calendar_core
    EXPORT_FILE_NAME akonadi-calendar-core_export.h
)


target_include_directories(KPim6AkonadiCalendarCore
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendarCore>"
    PUBLIC
        "$<BUILD_INTERFACE:${Akonadi-Calendar-Calendar_SOURCE_DIR}/src;${Akonadi-Calendar-Calendar_BINARY_DIR}/src>"
)

target_link_libraries(KPim6AkonadiCalendarCore PUBLIC
    KPim6::AkonadiCore
)

set_target_properties(KPim6AkonadiCalendarCore PROPERTIES
    VERSION ${AKONADICALENDAR_VERSION}
    SOVERSION ${AKONADICALENDAR_SOVERSION}
    EXPORT_NAME AkonadiCalendarCore
)

install(TARGETS KPim6AkonadiCalendarCore
    EXPORT KPim6AkonadiCalendarCoreTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiCalendarCore_CamelCase_HEADERS
    HEADER_NAMES
        FreeBusyProviderBase
    REQUIRED_HEADERS AkonadiCalendarCore_HEADERS
    PREFIX Akonadi
)

install(FILES ${AkonadiCalendarCore_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendarCore/Akonadi
    COMPONENT Devel
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar-core_export.h
    ${AkonadiCalendarCore_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendarCore/akonadi COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KPim6AkonadiCalendarCore_QCH
        NAME AkonadiCalendar
        BASE_NAME KPim6AkonadiCalendarCore
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
  	    ${AkonadiCalendarCore_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            AKONADI_CALENDAR_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
