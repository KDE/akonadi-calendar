set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)



set( KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE )

set( PREVIOUS_EXEC_OUTPUT_PATH ../../tests )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include(../../AkonadiMacros.cmake)

add_definitions(-DITIP_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/itip_data\\"" )

set(common_sources unittestbase.cpp helper.cpp ../utils_p.cpp)

#add_akonadi_isolated_test_advanced( historytest.cpp "${common_sources}" "")
#add_akonadi_isolated_test( incidencechangertest.cpp )
#add_akonadi_isolated_test( calendarbasetest.cpp )
#add_akonadi_isolated_test( fetchjobcalendartest.cpp )
#add_akonadi_isolated_test( etmcalendartest.cpp )


if ( PLEASE_TEST_INVITATIONS )
  add_akonadi_isolated_test_advanced( itiphandlertest.cpp "${common_sources}" "")
  add_akonadi_isolated_test( mailclienttest.cpp )
endif()
