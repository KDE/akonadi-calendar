cmake_minimum_required(VERSION 2.8.12)

project(Akonadi-Calendar)

# ECM setup
find_package(ECM 5.9.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

set(KF5_VERSION "5.9.0")
set(AKONADICALENDAR_LIB_VERSION "4.79.0")
set(CALENDARCORE_LIB_VERSION "4.79.0")
set(AKONADICONTACT_LIB_VERSION "4.79.0")
set(AKONADI_LIB_VERSION "4.79.0")
set(MAILTRANSPORT_LIB_VERSION "4.79.0")
set(KCONTACTS_LIB_VERSION "4.79.0")
set(CALENDARUTILS_LIB_VERSION "4.79.0")
set(IDENTITYMANAGEMENT_LIB_VERSION "4.79.0")
ecm_setup_version(${AKONADICALENDAR_LIB_VERSION} VARIABLE_PREFIX AKONADICALENDAR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfigVersion.cmake"
                        SOVERSION 5
)

configure_file(akonadi-calendar-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/akonadi-calendar-version.h @ONLY)


########### Find packages ###########
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5MailTransport ${MAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${CALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Akonadi ${AKONADI_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${AKONADICONTACT_LIB_VERSION} CONFIG REQUIRED)
  calfilterpartstatusproxymodel_p.cpp

########### Targets ###########

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_subdirectory(src)
add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5AkonadiCalendar")

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5AkonadiCalendarConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5AkonadiCalendarTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5AkonadiCalendarTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

