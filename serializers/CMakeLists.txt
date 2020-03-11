add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5-serializer\")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(akonadi_serializer_kcalcore_SRCS
    akonadi_serializer_kcalcore.cpp
)
ecm_qt_declare_logging_category(akonadi_serializer_kcalcore_SRCS HEADER serializer_debug.h IDENTIFIER AKONADI_SERIALIZER_CALENDAR_LOG CATEGORY_NAME org.kde.pim.calendar_serializer
        DESCRIPTION "calendar serializer (plugin)"
        EXPORT AKONADI-CALENDAR
    )


add_library(akonadi_serializer_kcalcore MODULE ${akonadi_serializer_kcalcore_SRCS})
target_link_libraries(akonadi_serializer_kcalcore
    KF5::CalendarUtils
    KF5::CalendarCore
    KF5::AkonadiCore
    KF5::I18n
)
install(TARGETS akonadi_serializer_kcalcore
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES akonadi_serializer_kcalcore.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/plugins/serializer
)
