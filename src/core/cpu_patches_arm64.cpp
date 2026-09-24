// SPDX-License-Identifier: GPL-2.0-or-later
// ARM64 host backend for the x86-64 guest CPU patch API.

#include "cpu_patches.h"

namespace Core::WindowsGuestRedZoneProtection {

void SetActiveMode(WindowsGuestRedZoneProtectionMode /*mode*/) noexcept {
    // Guest red-zone patching is an x86-64 host feature. ARM64 hosts do not
    // install the x86-64 instruction rewriter.
}

WindowsGuestRedZoneProtectionMode GetActiveMode() noexcept {
    return WindowsGuestRedZoneProtectionMode::Disabled;
}

bool IsStaticPatchingEnabled() noexcept {
    return false;
}

} // namespace Core::WindowsGuestRedZoneProtection

namespace Core {

void RegisterPatchModule(void* /*module_ptr*/, u64 /*module_size*/,
                         void* /*trampoline_area_ptr*/, u64 /*trampoline_area_size*/) {
    // No x86-64 instruction rewriting is installed on ARM64 hosts.
}

void PrePatchInstructions(u64 /*segment_addr*/, u64 /*segment_size*/) {
    // No-op on ARM64 hosts.
}

RedZonePatchResult PatchRedZoneMemoryInstructions(
    u64 /*segment_addr*/, u64 /*segment_size*/,
    std::span<const uintptr_t> /*function_starts*/) {
    return {};
}

} // namespace Core
