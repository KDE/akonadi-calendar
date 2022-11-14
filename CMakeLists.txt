# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(PIM_VERSION "5.21.90")
project(Akonadi-Calendar  VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.99.0")
find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(QT_REQUIRED_VERSION "5.15.2")
if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)



set(AKONADICALENDAR_LIB_VERSION ${PIM_VERSION})
set(AKONADICONTACT_LIB_VERSION "5.21.90")
set(AKONADI_LIB_VERSION "5.21.90")
set(MAILTRANSPORT_LIB_VERSION "5.21.90")
set(CALENDARUTILS_LIB_VERSION "5.21.90")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.21.90")

ecm_setup_version(PROJECT VARIABLE_PREFIX AKONADICALENDAR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5AkonadiCalendarConfigVersion.cmake"
                        SOVERSION 5
)

configure_file(akonadi-calendar-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/akonadi-calendar-version.h @ONLY)

if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif()

########### Find packages ###########
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5WidgetsAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5XmlGui ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Codecs ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Notifications ${KF5_MIN_VERSION} CONFIG REQUIRED)

find_package(KF5MailTransportAkonadi ${MAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5IdentityManagement ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)

find_package(KF5Akonadi ${AKONADI_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5AkonadiContact ${AKONADICONTACT_LIB_VERSION} CONFIG REQUIRED)

########### Targets ###########


ecm_set_disabled_deprecation_versions(QT 5.15.2 KF 5.99.0)



option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
    add_definitions(-DCOMPILE_WITH_UNITY_CMAKE_SUPPORT)
endif()

add_subdirectory(src)
add_subdirectory(serializers)
add_subdirectory(reminder-daemon)
add_subdirectory(calendar-plugin)
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
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiCalendar COMPONENT Devel
)
ecm_qt_install_logging_categories(
        EXPORT AKONADI-CALENDAR
        FILE akonadi-calendar.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

