# SPDX-FileCopyrightText: 2023-2025 Laurent Montel <montel@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause

add_executable(
    notification_gui
    notification_gui.cpp
    notificationobject.h
    notificationobject.cpp
)

target_link_libraries(
    notification_gui
    KF6::Notifications
    Qt::Widgets
)

install(FILES notification_gui.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
