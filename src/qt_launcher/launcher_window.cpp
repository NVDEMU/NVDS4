// SPDX-License-Identifier: GPL-2.0-or-later

#include "launcher_window.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QSet>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QString coreExecutablePath() {
    QString name = QStringLiteral("nvds4");
#ifdef _WIN32
    name += QStringLiteral(".exe");
#endif
    return QDir(QCoreApplication::applicationDirPath()).filePath(name);
}

QIcon placeholderIcon() {
    QPixmap pixmap(256, 256);
    pixmap.fill(QColor("#141820"));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#242a35"));
    painter.drawRoundedRect(8, 8, 240, 240, 24, 24);

    painter.setPen(QColor("#ffffff"));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(42);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QStringLiteral("NVDS4"));
    return QIcon(pixmap);
}

QString gameTitleForPath(const QString& path) {
    const QFileInfo info(path);
    return info.fileName().isEmpty() ? info.absoluteFilePath() : info.fileName();
}

QIcon gameIconForPath(const QString& path) {
    const QFileInfo gameInfo(path);
    const QString iconPath =
        QDir(gameInfo.absoluteFilePath()).filePath(QStringLiteral("sce_sys/icon0.png"));
    if (QFileInfo::exists(iconPath)) {
        const QIcon icon(iconPath);
        if (!icon.isNull()) {
            return icon;
        }
    }
    return placeholderIcon();
}

QString gameKeyForPath(const QString& path) {
    return QFileInfo(path).canonicalFilePath();
}

} // namespace

LauncherWindow::LauncherWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("NVDS4"));
    setMinimumSize(1100, 720);
    resize(1400, 860);

    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget {
            background: #0f1218;
            color: #f4f6f8;
            font-family: "SF Pro Display", "Segoe UI", sans-serif;
            font-size: 14px;
        }
        QFrame#sidebar {
            background: #12161e;
            border-right: 1px solid #252b35;
        }
        QLabel#brand {
            color: #ffffff;
            font-size: 25px;
            font-weight: 700;
            padding: 6px 4px;
        }
        QLabel#subtitle {
            color: #7f8896;
            font-size: 12px;
            padding-bottom: 12px;
        }
        QPushButton#navButton {
            text-align: left;
            border: 0;
            border-radius: 8px;
            padding: 10px 12px;
            color: #b8c0cb;
            background: transparent;
        }
        QPushButton#navButton:hover {
            color: #ffffff;
            background: #1b212b;
        }
        QPushButton#navButton[selected="true"] {
            color: #ffffff;
            background: #242c38;
        }
        QLineEdit {
            background: #171c24;
            border: 1px solid #2a313d;
            border-radius: 8px;
            padding: 10px 12px;
            color: #ffffff;
            selection-background-color: #3b82f6;
        }
        QListWidget {
            background: #0f1218;
            border: 0;
            outline: none;
            padding: 8px;
        }
        QListWidget::item {
            border-radius: 10px;
            padding: 10px;
            margin: 8px;
            color: #d9dee5;
        }
        QListWidget::item:hover {
            background: #171d26;
        }
        QListWidget::item:selected {
            background: #222a36;
            border: 1px solid #394555;
        }
        QPushButton#primary {
            background: #3b82f6;
            color: white;
            border: 0;
            border-radius: 8px;
            padding: 10px 18px;
            font-weight: 600;
        }
        QPushButton#primary:hover {
            background: #4a8df7;
        }
        QPushButton#secondary {
            background: #1b212b;
            color: #d9dee5;
            border: 1px solid #2a313d;
            border-radius: 8px;
            padding: 10px 18px;
        }
        QPushButton#secondary:hover {
            background: #242c38;
        }
        QLabel#pageTitle {
            font-size: 24px;
            font-weight: 700;
        }
        QLabel#selection {
            color: #9aa3af;
        }
        QLabel#status {
            color: #697382;
        }
        QDialog {
            background: #12161e;
        }
    )"));

    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* sidebar = new QFrame;
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(230);

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(22, 24, 18, 20);
    sidebarLayout->setSpacing(6);

    auto* brand = new QLabel(QStringLiteral("NVDS4"));
    brand->setObjectName(QStringLiteral("brand"));
    sidebarLayout->addWidget(brand);

    auto* subtitle = new QLabel(QStringLiteral("PlayStation 4 Emulator"));
    subtitle->setObjectName(QStringLiteral("subtitle"));
    sidebarLayout->addWidget(subtitle);

    auto* gamesButton = new QPushButton(QStringLiteral("  Games"));
    gamesButton->setObjectName(QStringLiteral("navButton"));
    gamesButton->setProperty("selected", true);
    sidebarLayout->addWidget(gamesButton);

    auto* settingsButton = new QPushButton(QStringLiteral("  Settings"));
    settingsButton->setObjectName(QStringLiteral("navButton"));
    sidebarLayout->addWidget(settingsButton);

    sidebarLayout->addStretch();

    auto* addButton = new QPushButton(QStringLiteral("+  Add Game Folder"));
    addButton->setObjectName(QStringLiteral("secondary"));
    sidebarLayout->addWidget(addButton);

    root->addWidget(sidebar);

    auto* page = new QWidget;
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(30, 26, 30, 22);
    pageLayout->setSpacing(16);

    auto* header = new QHBoxLayout;
    auto* title = new QLabel(QStringLiteral("Games"));
    title->setObjectName(QStringLiteral("pageTitle"));
    header->addWidget(title);
    header->addStretch();

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(QStringLiteral("Search games..."));
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setFixedWidth(300);
    header->addWidget(searchEdit);

    auto* refreshButton = new QPushButton(QStringLiteral("Refresh"));
    refreshButton->setObjectName(QStringLiteral("secondary"));
    header->addWidget(refreshButton);
    pageLayout->addLayout(header);

    gameList = new QListWidget;
    gameList->setViewMode(QListView::IconMode);
    gameList->setIconSize(QSize(180, 180));
    gameList->setGridSize(QSize(230, 235));
    gameList->setResizeMode(QListView::Adjust);
    gameList->setMovement(QListView::Static);
    gameList->setSelectionMode(QAbstractItemView::SingleSelection);
    gameList->setSpacing(5);
    pageLayout->addWidget(gameList, 1);

    auto* footer = new QHBoxLayout;
    selectionLabel = new QLabel(QStringLiteral("No game selected"));
    selectionLabel->setObjectName(QStringLiteral("selection"));
    footer->addWidget(selectionLabel);

    statusLabel = new QLabel;
    statusLabel->setObjectName(QStringLiteral("status"));
    footer->addWidget(statusLabel);
    footer->addStretch();

    launchButton = new QPushButton(QStringLiteral("Launch"));
    launchButton->setObjectName(QStringLiteral("primary"));
    launchButton->setEnabled(false);
    footer->addWidget(launchButton);

    pageLayout->addLayout(footer);
    root->addWidget(page, 1);
    setCentralWidget(central);

    connect(addButton, &QPushButton::clicked, this, &LauncherWindow::addGameFolder);
    connect(settingsButton, &QPushButton::clicked, this, &LauncherWindow::showSettingsDialog);
    connect(refreshButton, &QPushButton::clicked, this, &LauncherWindow::refreshGames);
    connect(searchEdit, &QLineEdit::textChanged, this,
            [this] { refreshVisibleGames(); });
    connect(launchButton, &QPushButton::clicked, this, &LauncherWindow::launchSelectedGame);
    connect(gameList, &QListWidget::itemSelectionChanged, this, &LauncherWindow::updateSelection);
    connect(gameList, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem*) { launchSelectedGame(); });

    loadSettings();
    refreshGames();
}

void LauncherWindow::loadSettings() {
    QSettings settings(QStringLiteral("NVDEMU"), QStringLiteral("NVDS4"));
    gameFolders = settings.value(QStringLiteral("gameFolders")).toStringList();
}

void LauncherWindow::saveSettings() const {
    QSettings settings(QStringLiteral("NVDEMU"), QStringLiteral("NVDS4"));
    settings.setValue(QStringLiteral("gameFolders"), gameFolders);
    settings.sync();
}

void LauncherWindow::refreshGames() {
    gamePaths.clear();

    QSet<QString> seen;
    for (const QString& root : gameFolders) {
        const QFileInfo rootInfo(root);
        if (!rootInfo.isDir()) {
            continue;
        }

        QDirIterator iterator(root, QStringList{QStringLiteral("eboot.bin")}, QDir::Files,
                              QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const QFileInfo ebootInfo(iterator.next());
            QString gameDir = ebootInfo.absolutePath();

            if (QFileInfo(gameDir).fileName() == QStringLiteral("sce_sys")) {
                gameDir = QFileInfo(gameDir).absolutePath();
            }

            const QString key = gameKeyForPath(gameDir);
            if (!key.isEmpty() && !seen.contains(key)) {
                seen.insert(key);
                gamePaths.push_back(key);
            }
        }

        const QDir dir(root);
        const QFileInfoList archives =
            dir.entryInfoList(QStringList{QStringLiteral("*.zar")}, QDir::Files);
        for (const QFileInfo& archive : archives) {
            const QString key = gameKeyForPath(archive.absoluteFilePath());
            if (!key.isEmpty() && !seen.contains(key)) {
                seen.insert(key);
                gamePaths.push_back(key);
            }
        }
    }

    std::sort(gamePaths.begin(), gamePaths.end(),
              [](const QString& a, const QString& b) {
                  return QString::compare(gameTitleForPath(a), gameTitleForPath(b),
                                          Qt::CaseInsensitive) < 0;
              });

    refreshVisibleGames();
    statusLabel->setText(QStringLiteral("%1 game%2 found")
                             .arg(gamePaths.size())
                             .arg(gamePaths.size() == 1 ? "" : "s"));
}

void LauncherWindow::refreshVisibleGames() {
    const QString filter = searchEdit ? searchEdit->text().trimmed() : QString();
    gameList->clear();

    for (const QString& path : gamePaths) {
        const QString title = gameTitleForPath(path);
        if (!filter.isEmpty() && !title.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        auto* item = new QListWidgetItem(gameIconForPath(path), title);
        item->setData(Qt::UserRole, path);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        gameList->addItem(item);
    }

    updateSelection();
}

void LauncherWindow::addGameFolder() {
    const QString folder =
        QFileDialog::getExistingDirectory(this, QStringLiteral("Select PS4 Game Folder"));
    if (folder.isEmpty()) {
        return;
    }

    const QString canonical = gameKeyForPath(folder);
    if (canonical.isEmpty() || gameFolders.contains(canonical)) {
        return;
    }

    gameFolders.push_back(canonical);
    saveSettings();
    refreshGames();
}

void LauncherWindow::showSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("NVDS4 Settings"));
    dialog.resize(700, 430);

    auto* layout = new QVBoxLayout(&dialog);
    auto* title = new QLabel(QStringLiteral("Game Library"));
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 700;"));
    layout->addWidget(title);

    auto* list = new QListWidget;
    list->addItems(gameFolders);
    layout->addWidget(list, 1);

    auto* buttons = new QHBoxLayout;
    auto* add = new QPushButton(QStringLiteral("Add Folder"));
    auto* remove = new QPushButton(QStringLiteral("Remove"));
    auto* refresh = new QPushButton(QStringLiteral("Refresh Library"));
    auto* close = new QPushButton(QStringLiteral("Close"));

    buttons->addWidget(add);
    buttons->addWidget(remove);
    buttons->addWidget(refresh);
    buttons->addStretch();
    buttons->addWidget(close);
    layout->addLayout(buttons);

    connect(add, &QPushButton::clicked, [&] {
        const QString folder =
            QFileDialog::getExistingDirectory(&dialog, QStringLiteral("Select PS4 Game Folder"));
        if (folder.isEmpty()) {
            return;
        }

        const QString canonical = gameKeyForPath(folder);
        if (!canonical.isEmpty() && !gameFolders.contains(canonical)) {
            gameFolders.push_back(canonical);
            list->addItem(canonical);
            saveSettings();
        }
    });

    connect(remove, &QPushButton::clicked, [&] {
        if (!list->currentItem()) {
            return;
        }
        gameFolders.removeAll(list->currentItem()->text());
        delete list->takeItem(list->currentRow());
        saveSettings();
    });

    connect(refresh, &QPushButton::clicked, this, &LauncherWindow::refreshGames);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
    refreshGames();
}

void LauncherWindow::updateSelection() {
    const auto* item = gameList->currentItem();
    if (!item) {
        selectionLabel->setText(QStringLiteral("No game selected"));
        launchButton->setEnabled(false);
        return;
    }

    selectionLabel->setText(item->text());
    launchButton->setEnabled(true);
}

void LauncherWindow::launchSelectedGame() {
    auto* item = gameList->currentItem();
    if (!item) {
        return;
    }

    launchGame(item->data(Qt::UserRole).toString());
}

void LauncherWindow::launchGame(const QString& path) {
    const QString core = coreExecutablePath();
    if (!QFileInfo::exists(core)) {
        QMessageBox::critical(
            this, QStringLiteral("NVDS4"),
            QStringLiteral("Could not find the NVDS4 emulator core beside the launcher.\n\n%1")
                .arg(core));
        return;
    }

    if (!QProcess::startDetached(core, {QStringLiteral("--game"), path},
                                 QCoreApplication::applicationDirPath())) {
        QMessageBox::critical(
            this, QStringLiteral("NVDS4"),
            QStringLiteral("NVDS4 could not be started.\n\n%1").arg(core));
    }
}