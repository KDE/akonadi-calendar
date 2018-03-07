add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar5-serializer\")

set(akonadi_serializer_kcalcore_SRCS
    akonadi_serializer_kcalcore.cpp
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
