// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QMainWindow>
#include <QStringList>

class QListWidget;
class QLineEdit;
class QLabel;
class QPushButton;

class LauncherWindow final : public QMainWindow {
public:
    explicit LauncherWindow(QWidget* parent = nullptr);

private:
    void loadSettings();
    void saveSettings() const;
    void refreshGames();
    void refreshVisibleGames();
    void addGameFolder();
    void showSettingsDialog();
    void launchSelectedGame();
    void launchGame(const QString& path);
    void updateSelection();

    QStringList gameFolders;
    QStringList gamePaths;

    QLineEdit* searchEdit{};
    QListWidget* gameList{};
    QLabel* selectionLabel{};
    QLabel* statusLabel{};
    QPushButton* launchButton{};
};