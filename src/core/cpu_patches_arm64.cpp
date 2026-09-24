// SPDX-License-Identifier: GPL-2.0-or-later
// ARM64 host backend for the x86-64 guest CPU patch API.

#include "cpu_patches.h"

namespace Core::WindowsGuestRedZoneProtection {

void SetActiveMode(WindowsGuestRedZoneProtectionMode /*mode*/) noexcept {
    // Guest red-zone patching is an x86-64 host feature. ARM64 hosts do not
    // install the x86-64 instruction rewriter, but the public API is kept
    // available so the emulator core remains portable.
}

WindowsGuestRedZoneProtectionMode GetActiveMode() noexcept {
    return WindowsGuestRedZoneProtectionMode::Disabled;
}

bool IsStaticPatchingEnabled() noexcept {
    return false;
}

} // namespace Core::WindowsGuestRedZoneProtection
