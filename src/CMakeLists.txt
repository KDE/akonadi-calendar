add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5\")


set(akonadicalendar_LIB_SRC
  blockalarmsattribute.cpp
  calendarbase.cpp
  calendarclipboard.cpp
  calendarmodel.cpp
  calfilterproxymodel_p.cpp
  etmcalendar.cpp
  history.cpp
  history_p.cpp
  icalimporter.cpp
  incidencefetchjob_p.cpp
  incidencechanger.cpp
  incidencechanger_p.cpp
  itiphandler.cpp
  itiphandler_p.cpp
  itiphandlerhelper_p.cpp
  kcolumnfilterproxymodel.cpp
  fetchjobcalendar.cpp
  freebusydownloadjob_p.cpp
  freebusymanager.cpp
  freebusyproviderbase.cpp
  mailclient_p.cpp
  mailscheduler_p.cpp
  publishdialog.cpp
  publishdialog_p.cpp
  scheduler_p.cpp
  standardcalendaractionmanager.cpp
  todopurger.cpp
  utils_p.cpp
  calfilterpartstatusproxymodel_p.cpp
)

if (ECM_VERSION VERSION_LESS "5.68.0")
    ecm_qt_declare_logging_category(akonadicalendar_LIB_SRC HEADER akonadicalendar_debug.h IDENTIFIER AKONADICALENDAR_LOG CATEGORY_NAME org.kde.pim.akonadicalendar)
else()
    ecm_qt_declare_logging_category(akonadicalendar_LIB_SRC HEADER akonadicalendar_debug.h IDENTIFIER AKONADICALENDAR_LOG CATEGORY_NAME org.kde.pim.akonadicalendar
        DESCRIPTION "akonadicalendar (pim lib)"
        OLD_CATEGORY_NAMES log_akonadicalendar
        EXPORT AKONADI-CALENDAR
    )
endif()

kconfig_add_kcfg_files(akonadicalendar_LIB_SRC calendarsettings.kcfgc)
ki18n_wrap_ui(akonadicalendar_LIB_SRC publishdialog_base.ui)

qt5_add_dbus_adaptor( akonadicalendar_LIB_SRC interfaces/org.freedesktop.Akonadi.Resource.FreeBusyProvider.xml freebusyproviderbase_p.h Akonadi::FreeBusyProviderBasePrivate freebusyprovideradaptor  Akonadi__FreeBusyProviderAdaptor )
add_library(KF5AkonadiCalendar ${akonadicalendar_LIB_SRC})

generate_export_header(KF5AkonadiCalendar BASE_NAME akonadi-calendar)

add_library(KF5::AkonadiCalendar ALIAS KF5AkonadiCalendar)

target_include_directories(KF5AkonadiCalendar INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/Akonadi/Calendar;${KF5_INCLUDE_INSTALL_DIR}/akonadi/calendar>")
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
    KF5::IdentityManagement
    KF5::Codecs
)

set_target_properties(KF5AkonadiCalendar PROPERTIES
    VERSION ${AKONADICALENDAR_VERSION_STRING}
    SOVERSION ${AKONADICALENDAR_SOVERSION}
    EXPORT_NAME AkonadiCalendar
)

ecm_generate_pri_file(BASE_NAME AkonadiCalendar
    LIB_NAME KF5AkonadiCalendar
    DEPS "AkonadiCore CalendarCore" FILENAME_VAR PRI_FILENAME
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
  ITIPHandler
  PublishDialog
  StandardCalendarActionManager
  TodoPurger
  REQUIRED_HEADERS AkonadiCalendar_HEADERS
  PREFIX Akonadi/Calendar
)

install( FILES
  ${AkonadiCalendar_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/Akonadi/Calendar COMPONENT Devel
)
install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_export.h
  ${AkonadiCalendar_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/calendarsettings.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/akonadi/calendar COMPONENT Devel
)

