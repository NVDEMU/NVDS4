// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>

#include "launcher_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("NVDS4"));
    QApplication::setOrganizationName(QStringLiteral("NVDEMU"));
    QApplication::setApplicationDisplayName(QStringLiteral("NVDS4"));
    QApplication::setStyle(QStringLiteral("Fusion"));

    LauncherWindow window;
    window.show();

    return app.exec();
}