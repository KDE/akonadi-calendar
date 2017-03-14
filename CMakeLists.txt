cmake_minimum_required(VERSION 3.0)

set(PIM_VERSION "5.4.80")
project(Akonadi-Calendar  VERSION ${PIM_VERSION})

# ECM setup
set(KF5_VERSION "5.32.0")
find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMCoverageOption)


set(AKONADICALENDAR_LIB_VERSION ${PIM_VERSION})
set(CALENDARCORE_LIB_VERSION "5.4.40")
set(AKONADICONTACT_LIB_VERSION "5.4.40")
set(AKONADI_LIB_VERSION "5.4.40")
set(MAILTRANSPORT_LIB_VERSION "5.4.47")
set(KCONTACTS_LIB_VERSION "5.4.40")
set(CALENDARUTILS_LIB_VERSION "5.4.40")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.4.40")
ecm_setup_version(PROJECT VARIABLE_PREFIX AKONADICALENDAR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfigVersion.cmake"
                        SOVERSION 5
)

configure_file(akonadi-calendar-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/akonadi-calendar-version.h @ONLY)


########### Find packages ###########
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5MailTransportAkonadi ${MAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${CALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Akonadi ${AKONADI_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${AKONADICONTACT_LIB_VERSION} CONFIG REQUIRED)

########### Targets ###########

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5AkonadiCalendar")

configure_package_config_file(
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

