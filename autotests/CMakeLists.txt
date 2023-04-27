# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

add_subdirectory(gnupg_home)
include(${CMAKE_SOURCE_DIR}/cmake/modules/kdepim_add_gpg_crypto_test.cmake)

set( KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE )

set( PREVIOUS_EXEC_OUTPUT_PATH ../../tests )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(common_sources unittestbase.cpp helper.cpp ${Akonadi-Calendar_BINARY_DIR}/src/akonadicalendar_debug.cpp ${Akonadi-Calendar_SOURCE_DIR}/src/utils_p.cpp)
set(common_libs "KPim${KF_MAJOR_VERSION}::AkonadiCalendar;KF${KF_MAJOR_VERSION}::CalendarCore;KPim${KF_MAJOR_VERSION}::Mime;KPim${KF_MAJOR_VERSION}::IdentityManagement;KPim${KF_MAJOR_VERSION}::AkonadiWidgets;KF${KF_MAJOR_VERSION}::Codecs;KF${KF_MAJOR_VERSION}::ConfigCore")

# the tests need the ical resource, which we might not have at this point (e.g. on the CI)
find_program(AKONADI_ICAL_RESOURCE NAMES akonadi_ical_resource)
if (AKONADI_ICAL_RESOURCE)
    add_akonadi_isolated_test_advanced( historytest.cpp "${common_sources}" "${common_libs}")
    add_akonadi_isolated_test_advanced( incidencechangertest.cpp "" "KPim${KF_MAJOR_VERSION}::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( calendarbasetest.cpp "" "KPim${KF_MAJOR_VERSION}::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( fetchjobcalendartest.cpp "" "KPim${KF_MAJOR_VERSION}::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( etmcalendartest.cpp "" "KPim${KF_MAJOR_VERSION}::AkonadiCalendar;KF${KF_MAJOR_VERSION}::ConfigCore" )

    add_akonadi_isolated_test_advanced( itiphandlertest.cpp "${common_sources}" "${common_libs};KPim${KF_MAJOR_VERSION}::AkonadiMime")

    add_executable( mailclienttest )
    target_sources( mailclienttest PRIVATE mailclienttest.cpp )
    target_link_libraries( mailclienttest PRIVATE
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Test
        KPim${KF_MAJOR_VERSION}::AkonadiCalendar
        KPim${KF_MAJOR_VERSION}::Mime
        KF${KF_MAJOR_VERSION}::Contacts
        KPim${KF_MAJOR_VERSION}::AkonadiMime
        KPim${KF_MAJOR_VERSION}::IdentityManagement
        KPim${KF_MAJOR_VERSION}::MessageComposer
        -lgpgmepp
    )
    add_gpg_crypto_akonadi_test( mailclienttest "mailclienttest" )
endif()



ecm_qt_declare_logging_category(akonadi_serializer_kcalcore_autotest_SRCS HEADER serializer_debug.h IDENTIFIER AKONADI_SERIALIZER_CALENDAR_LOG CATEGORY_NAME org.kde.pim.calendar_serializer)

ecm_add_test(kcalcoreserializertest.cpp ../serializers/akonadi_serializer_kcalcore.cpp ${akonadi_serializer_kcalcore_autotest_SRCS}
    LINK_LIBRARIES KPim${KF_MAJOR_VERSION}::CalendarUtils KF${KF_MAJOR_VERSION}::CalendarCore KPim${KF_MAJOR_VERSION}::AkonadiCore KF${KF_MAJOR_VERSION}::I18n Qt::Test
    TEST_NAME kcalcoreserializertest
)
