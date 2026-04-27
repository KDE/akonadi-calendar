# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt6Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

add_subdirectory(gnupg_home)
set(GNUPGHOME ${CMAKE_SOURCE_DIR}/autotests/gnupg_home)
include(${LIBKLEO_TEST_MODULE_PATH}/KleoAddGpgCryptoTest.cmake)

set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)

set(PREVIOUS_EXEC_OUTPUT_PATH ../../tests)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(common_sources
    unittestbase.cpp
    helper.cpp
    ${Akonadi-Calendar_BINARY_DIR}/src/akonadicalendar_debug.cpp
    ${Akonadi-Calendar_SOURCE_DIR}/src/utils_p.cpp
)
set(common_libs
    KPim6::AkonadiCalendar
    KF6::CalendarCore
    KF6::Mime
    KPim6::IdentityManagementCore
    KPim6::AkonadiWidgets
    KF6::Codecs
    KF6::ConfigCore
)

# the tests need the ical resource, which we might not have at this point (e.g. on the CI)
find_program(AKONADI_ICAL_RESOURCE NAMES akonadi_ical_resource)
if(AKONADI_ICAL_RESOURCE)
    add_akonadi_isolated_test(SOURCE historytest.cpp ADDITIONAL_SOURCES ${common_sources} LINK_LIBRARIES ${common_libs})
    add_akonadi_isolated_test(SOURCE incidencechangertest.cpp LINK_LIBRARIES KPim6::AkonadiCalendar)
    add_akonadi_isolated_test(SOURCE calendarbasetest.cpp LINK_LIBRARIES KPim6::AkonadiCalendar)
    add_akonadi_isolated_test(SOURCE fetchjobcalendartest.cpp LINK_LIBRARIES KPim6::AkonadiCalendar)
    add_akonadi_isolated_test(SOURCE etmcalendartest.cpp LINK_LIBRARIES KPim6::AkonadiCalendar KF6::ConfigCore)
    add_akonadi_isolated_test(SOURCE collectioncalendartest.cpp LINK_LIBRARIES KPim6::AkonadiCalendar KPim6::AkonadiPrivate)

    add_akonadi_isolated_test(SOURCE itiphandlertest.cpp ADDITIONAL_SOURCES ${common_sources} LINK_LIBRARIES ${common_libs} KPim6::AkonadiMime)

    add_executable(mailclienttest)
    target_sources(mailclienttest PRIVATE mailclienttest.cpp)
    target_link_libraries(
        mailclienttest
        PRIVATE
            Qt::DBus
            Qt::Test
            KPim6::AkonadiCalendar
            KF6::Mime
            KF6::Contacts
            KPim6::AkonadiMime
            KPim6::IdentityManagementCore
            KPim6::MessageComposer
    )
    add_gpg_crypto_akonadi_test( mailclienttest "mailclienttest")
endif()

ecm_qt_declare_logging_category(akonadi_serializer_kcalcore_autotest_SRCS HEADER serializer_debug.h IDENTIFIER AKONADI_SERIALIZER_CALENDAR_LOG CATEGORY_NAME org.kde.pim.calendar_serializer)

ecm_add_test(kcalcoreserializertest.cpp ../serializers/akonadi_serializer_kcalcore.cpp ${akonadi_serializer_kcalcore_autotest_SRCS}
    LINK_LIBRARIES KPim6::CalendarUtils KF6::CalendarCore KPim6::AkonadiCore KF6::I18n Qt::Test
    TEST_NAME kcalcoreserializertest
)
