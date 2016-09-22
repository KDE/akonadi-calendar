include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.6.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)



set( KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE )

set( PREVIOUS_EXEC_OUTPUT_PATH ../../tests )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DITIP_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/itip_data\\"" )

set(common_sources unittestbase.cpp helper.cpp ${Akonadi-Calendar_BINARY_DIR}/src/akonadicalendar_debug.cpp ${Akonadi-Calendar_SOURCE_DIR}/src/utils_p.cpp)

# the tests need the ical resource, which we might not have at this point (e.g. on the CI)
find_program(AKONADI_ICAL_RESOURCE NAMES akonadi_ical_resource)
if (AKONADI_ICAL_RESOURCE)
add_akonadi_isolated_test_advanced( historytest.cpp "${common_sources}" "KF5::AkonadiCalendar;KF5::CalendarCore;KF5::Mime;KF5::IdentityManagement;KF5::AkonadiWidgets")
add_akonadi_isolated_test_advanced( incidencechangertest.cpp "" "KF5::AkonadiCalendar" )
add_akonadi_isolated_test_advanced( calendarbasetest.cpp "" "KF5::AkonadiCalendar" )
add_akonadi_isolated_test_advanced( fetchjobcalendartest.cpp "" "KF5::AkonadiCalendar" )
add_akonadi_isolated_test_advanced( etmcalendartest.cpp "" "KF5::AkonadiCalendar" )


if ( PLEASE_TEST_INVITATIONS )
  add_akonadi_isolated_test_advanced( itiphandlertest.cpp "${common_sources}" "")
  add_akonadi_isolated_test_advanced( mailclienttest.cpp "" "KF5::AkonadiCalendar")
endif()
endif()
