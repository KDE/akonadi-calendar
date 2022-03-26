# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5\")

add_library(KF5AkonadiCalendar)
add_library(KF5::AkonadiCalendar ALIAS KF5AkonadiCalendar)
target_sources(KF5AkonadiCalendar PRIVATE
    blockalarmsattribute.cpp
    blockalarmsattribute.h
    calendarbase.cpp
    calendarbase.h
    calendarbase_p.h
    calendarclipboard.cpp
    calendarclipboard.h
    calendarclipboard_p.h
    calendarmodel.cpp
    calendarmodel_p.h
    calfilterpartstatusproxymodel_p.cpp
    calfilterpartstatusproxymodel_p.h
    calfilterproxymodel_p.cpp
    calfilterproxymodel_p.h
    etmcalendar.cpp
    etmcalendar.h
    etmcalendar_p.h
    fetchjobcalendar.cpp
    fetchjobcalendar.h
    fetchjobcalendar_p.h
    freebusydownloadjob_p.cpp
    freebusydownloadjob_p.h
    freebusymanager.cpp
    freebusymanager.h
    freebusymanager_p.h
    freebusyproviderbase.cpp
    freebusyproviderbase.h
    freebusyproviderbase_p.h
    history.cpp
    history.h
    history_p.cpp
    history_p.h
    icalimporter.cpp
    icalimporter.h
    icalimporter_p.h
    incidencechanger.cpp
    incidencechanger.h
    incidencechanger_p.cpp
    incidencechanger_p.h
    incidencefetchjob_p.cpp
    incidencefetchjob_p.h
    incidencetreemodel.cpp
    incidencetreemodel.h
    incidencetreemodel_p.h
    itiphandler.cpp
    itiphandler.h
    itiphandlerhelper_p.cpp
    itiphandlerhelper_p.h
    itiphandler_p.cpp
    itiphandler_p.h
    kcolumnfilterproxymodel.cpp
    kcolumnfilterproxymodel_p.h
    mailclient_p.cpp
    mailclient_p.h
    mailscheduler_p.cpp
    mailscheduler_p.h
    publishdialog.cpp
    publishdialog.h
    publishdialog_p.cpp
    publishdialog_p.h
    scheduler_p.cpp
    scheduler_p.h
    standardcalendaractionmanager.cpp
    standardcalendaractionmanager.h
    todopurger.cpp
    todopurger.h
    todopurger_p.h
    utils_p.cpp
    utils_p.h
)

ecm_qt_declare_logging_category(KF5AkonadiCalendar HEADER akonadicalendar_debug.h IDENTIFIER AKONADICALENDAR_LOG CATEGORY_NAME org.kde.pim.akonadicalendar
        DESCRIPTION "akonadicalendar (pim lib)"
        OLD_CATEGORY_NAMES log_akonadicalendar
        EXPORT AKONADI-CALENDAR
    )

kconfig_add_kcfg_files(akonadicalendar_LIB_SRC calendarsettings.kcfgc)
ki18n_wrap_ui(akonadicalendar_LIB_SRC publishdialog_base.ui)

qt_add_dbus_adaptor( akonadicalendar_LIB_SRC interfaces/org.freedesktop.Akonadi.Resource.FreeBusyProvider.xml freebusyproviderbase_p.h Akonadi::FreeBusyProviderBasePrivate freebusyprovideradaptor  Akonadi__FreeBusyProviderAdaptor )
target_sources(KF5AkonadiCalendar PRIVATE ${akonadicalendar_LIB_SRC})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5AkonadiCalendar PROPERTIES UNITY_BUILD ON)
endif()
generate_export_header(KF5AkonadiCalendar BASE_NAME akonadi-calendar)


target_include_directories(KF5AkonadiCalendar INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiCalendar>")
target_include_directories(KF5AkonadiCalendar PUBLIC "$<BUILD_INTERFACE:${Akonadi-Calendar_SOURCE_DIR}/src;${Akonadi-Calendar_BINARY_DIR}/src>")

target_link_libraries(KF5AkonadiCalendar
PUBLIC
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::WidgetsAddons
    KF5::I18n
PRIVATE
    KF5::Codecs
    KF5::KIOWidgets
    KF5::XmlGui
    KF5::AkonadiContact
    KF5::CalendarUtils
    KF5::AkonadiWidgets
    KF5::MailTransportAkonadi
    KF5::MailTransport
    KF5::IdentityManagement
    KF5::Codecs
)

set_target_properties(KF5AkonadiCalendar PROPERTIES
    VERSION ${AKONADICALENDAR_VERSION}
    SOVERSION ${AKONADICALENDAR_SOVERSION}
    EXPORT_NAME AkonadiCalendar
)

ecm_generate_pri_file(BASE_NAME AkonadiCalendar
    LIB_NAME KF5AkonadiCalendar
    DEPS "AkonadiCore KCalendarCore" FILENAME_VAR PRI_FILENAME
)

install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

install(TARGETS
    KF5AkonadiCalendar
    EXPORT KF5AkonadiCalendarTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiCalendar_CamelCase_HEADERS
  HEADER_NAMES
  BlockAlarmsAttribute
  CalendarBase
  CalendarClipboard
  ETMCalendar
  FetchJobCalendar
  FreeBusyManager
  FreeBusyProviderBase
  History
  ICalImporter
  IncidenceChanger
  IncidenceTreeModel
  ITIPHandler
  PublishDialog
  StandardCalendarActionManager
  TodoPurger
  REQUIRED_HEADERS AkonadiCalendar_HEADERS
  PREFIX Akonadi
)

install( FILES
  ${AkonadiCalendar_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiCalendar/Akonadi COMPONENT Devel
)
install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_export.h
  ${AkonadiCalendar_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/calendarsettings.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiCalendar/akonadi COMPONENT Devel
)

