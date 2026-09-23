// SPDX-License-Identifier: GPL-2.0-or-later
#include "common/updater.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include "common/scm_rev.h"

namespace Common::Updater {
namespace {
constexpr std::string_view kReleaseApi="https://api.github.com/repos/NVDEMU/NVDS4/releases/tags/nightly";
std::string Curl(const std::string& url) {
#ifdef _WIN32
    FILE* p=_popen((R"(curl.exe -fsSL -A "NVDS4-Updater" ")" + url + R"(")").c_str(),"r");
#else
    FILE* p=popen(("curl -fsSL -A 'NVDS4-Updater' '" + url + "'").c_str(),"r");
#endif
    if (!p) return {};
    std::string out; char b[4096]; while (fgets(b,sizeof(b),p)) out+=b;
#ifdef _WIN32
    _pclose(p);
#else
    pclose(p);
#endif
    return out;
}
std::string OpenCommand(const std::string& url) {
#ifdef _WIN32
    return "start \"\" \"" + url + "\"";
#elif defined(__APPLE__)
    return "open \"" + url + "\"";
#else
    return "xdg-open \"" + url + "\"";
#endif
}
}
UpdateInfo CheckForUpdate() {
    UpdateInfo info;
    try {
        const auto j=nlohmann::json::parse(Curl(std::string{kReleaseApi}));
        info.release_url=j.value("html_url","");
        for (const auto& a:j.value("assets",nlohmann::json::array())) {
            const auto name=a.value("name","");
#if defined(_WIN32)
            if (name.ends_with(".zip") && name.find("windows")!=std::string::npos) {
#elif defined(__APPLE__)
            if (name.ends_with(".dmg") && name.find("macos")!=std::string::npos) {
#else
            if (name.ends_with(".tar.gz") && name.find("linux")!=std::string::npos) {
#endif
                info.download_url=a.value("browser_download_url","");
                break;
            }
        }
        info.remote_commit=j.value("target_commitish","");
        if (info.remote_commit.empty()) {
            const auto tag=nlohmann::json::parse(Curl("https://api.github.com/repos/NVDEMU/NVDS4/git/ref/tags/nightly"));
            info.remote_commit=tag["object"].value("sha","");
        }
        info.available=!info.remote_commit.empty() && info.remote_commit!=Common::g_scm_rev;
    } catch (...) { return {}; }
    return info;
}
bool OpenLatestDownload(const UpdateInfo& info) {
    const auto& url=info.download_url.empty()?info.release_url:info.download_url;
    return !url.empty() && std::system(OpenCommand(url).c_str())==0;
}
} // namespace Common::Updater
