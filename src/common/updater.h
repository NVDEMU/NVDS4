// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <string>
namespace Common::Updater {
struct UpdateInfo { bool available{}; std::string remote_commit; std::string download_url; std::string release_url; };
UpdateInfo CheckForUpdate();
bool OpenLatestDownload(const UpdateInfo& info);
} // namespace Common::Updater
