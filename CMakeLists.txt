cmake_minimum_required(VERSION 2.8.12)

project(Akonadi-Calendar)

# ECM setup
find_package(ECM 0.0.15 CONFIG REQUIRED)
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

set(KF5_VERSION "5.0.0")
set(KDEPIMLIBS_VERSION "4.70.0")

ecm_setup_version(${KDEPIMLIBS_VERSION} VARIABLE_PREFIX AKONADICALENDAR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)

if(${CMAKE_SOURCE_DIR} STREQUAL ${Akonadi-Calendar_SOURCE_DIR})
  set(AkonadiServer_MIN_VERSION "1.72.43")
  find_package(Akonadi ${AkonadiServer_MIN_VERSION} CONFIG REQUIRED)

  find_package(KF5MailTransport ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5Akonadi CONFIG REQUIRED)
  find_package(KF5PimIdentities ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5CalendarCore ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5CalendarUtils ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5AkonadiContact ${KF5_VERSION} CONFIG REQUIRED)
  find_package(KF5PimUtils ${KF5_VERSION} CONFIG REQUIRED)
endif()

########### Targets ###########

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5\")

add_subdirectory(src)
# TODO: Port autotests
#add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5AkonadiCalendar")

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

#QT5 REACTIVATE IT
install(EXPORT KF5AkonadiCalendarTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5AkonadiCalendarTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

