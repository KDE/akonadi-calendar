# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)



set( KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE )

set( PREVIOUS_EXEC_OUTPUT_PATH ../../tests )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(common_sources unittestbase.cpp helper.cpp ${Akonadi-Calendar_BINARY_DIR}/src/akonadicalendar_debug.cpp ${Akonadi-Calendar_SOURCE_DIR}/src/utils_p.cpp)
set(common_libs "KF5::AkonadiCalendar;KF5::CalendarCore;KF5::Mime;KF5::IdentityManagement;KF5::AkonadiWidgets;KF5::Codecs")

# the tests need the ical resource, which we might not have at this point (e.g. on the CI)
find_program(AKONADI_ICAL_RESOURCE NAMES akonadi_ical_resource)
if (AKONADI_ICAL_RESOURCE)
    add_akonadi_isolated_test_advanced( historytest.cpp "${common_sources}" "${common_libs}")
    add_akonadi_isolated_test_advanced( incidencechangertest.cpp "" "KF5::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( calendarbasetest.cpp "" "KF5::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( fetchjobcalendartest.cpp "" "KF5::AkonadiCalendar" )
    add_akonadi_isolated_test_advanced( etmcalendartest.cpp "" "KF5::AkonadiCalendar;KF5::ConfigCore" )

    add_akonadi_isolated_test_advanced( itiphandlertest.cpp "${common_sources}" "${common_libs};KF5::MailTransportAkonadi")
    add_akonadi_isolated_test_advanced( mailclienttest.cpp "" "KF5::AkonadiCalendar;KF5::Mime;KF5::MailTransportAkonadi;KF5::IdentityManagement")

endif()



ecm_qt_declare_logging_category(akonadi_serializer_kcalcore_autotest_SRCS HEADER serializer_debug.h IDENTIFIER AKONADI_SERIALIZER_CALENDAR_LOG CATEGORY_NAME org.kde.pim.calendar_serializer)

ecm_add_test(kcalcoreserializertest.cpp ../serializers/akonadi_serializer_kcalcore.cpp ${akonadi_serializer_kcalcore_autotest_SRCS}
    LINK_LIBRARIES KF5::CalendarUtils KF5::CalendarCore KF5::AkonadiCore KF5::I18n Qt${QT_MAJOR_VERSION}::Test
    TEST_NAME kcalcoreserializertest
)
