#include "pch.h"

#include <MinHook.h>
#include <Windows.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iterator>
#include <string>

namespace
{
    using CameraUpdate = void(__fastcall*)(RE::TESCamera*);
    CameraUpdate g_originalCameraUpdate{};
    std::atomic_bool g_enabled{ true };
    std::atomic_bool g_aimOnly{ true };
    std::atomic_int g_aimKey{ VK_RBUTTON };
    std::atomic_int g_leftKey{ 'A' };
    std::atomic_int g_rightKey{ 'D' };
    float g_detectDistance{ 100.0F };
    float g_fullDistance{ 30.0F };
    float g_sideProbe{ 60.0F };
    float g_cameraOffset{ 60.0F };
    float g_fadeIn{ 4.0F };
    float g_fadeOut{ 6.0F };
    float g_strength{};
    std::uint64_t g_lastProbe{};
    std::uint64_t g_lastFrame{};
    int g_cachedSide{};
    float g_cachedWallDistance{};
    bool g_cachedCorner{};

    [[nodiscard]] RE::NiPoint3 Add(const RE::NiPoint3& a, const RE::NiPoint3& b) noexcept
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    [[nodiscard]] RE::NiPoint3 Scale(const RE::NiPoint3& a, float s) noexcept
    {
        return { a.x * s, a.y * s, a.z * s };
    }

    [[nodiscard]] float Clamp01(float value) noexcept
    {
        return std::clamp(value, 0.0F, 1.0F);
    }

    [[nodiscard]] bool RayHit(RE::TESObjectCELL* cell, const RE::NiPoint3& start,
        const RE::NiPoint3& end, float& fraction) noexcept
    {
        if (!cell) {
            return false;
        }
        RE::bhkPickData pick;
        pick.SetStartEnd(start, end);
        const auto hit = cell->Pick(pick) != nullptr || pick.HasHit();
        fraction = hit ? std::clamp(pick.GetHitFraction(), 0.0F, 1.0F) : 1.0F;
        return hit;
    }

    [[nodiscard]] bool IsDown(int key) noexcept
    {
        return key > 0 && (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    void LoadConfig() noexcept
    {
        constexpr wchar_t path[] = L"Data\\F4SE\\Plugins\\PeekOnly.ini";
        const auto getFloat = [path](const wchar_t* key, float fallback) {
            wchar_t value[64]{};
            GetPrivateProfileStringW(L"PeekOnly", key, L"", value, static_cast<DWORD>(std::size(value)), path);
            if (value[0] == L'\0') {
                return fallback;
            }
            try {
                return std::stof(value);
            } catch (...) {
                return fallback;
            }
        };
        g_enabled = GetPrivateProfileIntW(L"PeekOnly", L"Enabled", 1, path) != 0;
        g_aimOnly = GetPrivateProfileIntW(L"PeekOnly", L"AimOnly", 1, path) != 0;
        g_aimKey = GetPrivateProfileIntW(L"Keys", L"Aim", VK_RBUTTON, path);
        g_leftKey = GetPrivateProfileIntW(L"Keys", L"Left", 'A', path);
        g_rightKey = GetPrivateProfileIntW(L"Keys", L"Right", 'D', path);
        g_detectDistance = std::max(1.0F, getFloat(L"DetectDistance", 100.0F));
        g_fullDistance = std::clamp(getFloat(L"FullDistance", 30.0F), 0.0F, g_detectDistance - 0.01F);
        g_sideProbe = std::max(1.0F, getFloat(L"SideProbe", 60.0F));
        g_cameraOffset = std::max(0.0F, getFloat(L"CameraOffset", 60.0F));
        g_fadeIn = std::max(0.01F, getFloat(L"FadeIn", 4.0F));
        g_fadeOut = std::max(0.01F, getFloat(L"FadeOut", 6.0F));
        REX::INFO("PeekOnly config loaded: enabled={}, aimOnly={}, detectDistance={}, cameraOffset={}",
            g_enabled.load(), g_aimOnly.load(), g_detectDistance, g_cameraOffset);
    }

    void ProbeCorner(RE::PlayerCamera* camera, RE::TESObjectCELL* cell,
        const RE::NiPoint3& position, const RE::NiPoint3& forward, const RE::NiPoint3& right) noexcept
    {
        g_cachedCorner = false;
        g_cachedSide = 0;
        g_cachedWallDistance = 0.0F;

        float centerFraction{};
        if (!RayHit(cell, position, Add(position, Scale(forward, g_detectDistance)), centerFraction)) {
            return;
        }
        const float wallDistance = centerFraction * g_detectDistance;
        if (wallDistance > g_detectDistance || wallDistance < 1.0F) {
            return;
        }

        const float sideOffset = std::min(g_sideProbe, 48.0F);
        const auto leftStart = Add(position, Scale(right, -sideOffset));
        const auto rightStart = Add(position, Scale(right, sideOffset));
        float leftFraction{};
        float rightFraction{};
        const bool leftBlocked = RayHit(cell, leftStart, Add(leftStart, Scale(forward, g_detectDistance)), leftFraction);
        const bool rightBlocked = RayHit(cell, rightStart, Add(rightStart, Scale(forward, g_detectDistance)), rightFraction);

        // A corner is accepted only when the forward ray meets a near wall and
        // one shoulder lane has visibly more clearance than the other.
        const float openThreshold = centerFraction + (g_sideProbe / g_detectDistance);
        const bool leftOpen = !leftBlocked || leftFraction > openThreshold;
        const bool rightOpen = !rightBlocked || rightFraction > openThreshold;
        if (leftOpen == rightOpen) {
            return;
        }
        g_cachedSide = leftOpen ? -1 : 1;
        g_cachedWallDistance = wallDistance;
        g_cachedCorner = true;
        (void)camera;
    }

    void __fastcall CameraUpdateHook(RE::TESCamera* camera)
    {
        g_originalCameraUpdate(camera);
        if (camera != RE::PlayerCamera::GetSingleton()) {
            return;
        }

        const auto now = GetTickCount64();
        const float dt = g_lastFrame == 0 ? (1.0F / 60.0F) :
            std::clamp(static_cast<float>(now - g_lastFrame) / 1000.0F, 0.0F, 0.1F);
        g_lastFrame = now;

        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* root = camera->cameraRoot.get();
        const bool firstPerson = camera->QCameraEquals(RE::CameraState::kFirstPerson) ||
            camera->QCameraEquals(RE::CameraState::kIronSights);
        const bool aiming = IsDown(g_aimKey.load());
        if (!g_enabled.load() || !firstPerson || !player || !root || (g_aimOnly.load() && !aiming)) {
            const float blend = 1.0F - std::exp(-g_fadeOut * dt);
            g_strength += (0.0F - g_strength) * blend;
            return;
        }

        const auto rotation = root->GetWorldRotate();
        // The camera root's second matrix row is its forward axis in FO4.
        const RE::NiPoint3 forward{ rotation[1][0], rotation[1][1], rotation[1][2] };
        const RE::NiPoint3 right{ rotation[0][0], rotation[0][1], rotation[0][2] };
        const auto position = root->GetWorldTranslate();
        auto* cell = player->GetParentCell();
        if (now - g_lastProbe >= 50 || g_lastProbe == 0) {
            ProbeCorner(RE::PlayerCamera::GetSingleton(), cell, position, forward, right);
            g_lastProbe = now;
        }

        const bool towardWall = (g_cachedSide < 0 && IsDown(g_leftKey.load())) ||
            (g_cachedSide > 0 && IsDown(g_rightKey.load()));
        float target = 0.0F;
        if (g_cachedCorner && !towardWall) {
            target = Clamp01((g_detectDistance - g_cachedWallDistance) /
                std::max(g_detectDistance - g_fullDistance, 0.01F));
        }
        const float rate = target > g_strength ? g_fadeIn : g_fadeOut;
        const float blend = 1.0F - std::exp(-rate * dt);
        g_strength = Clamp01(g_strength + (target - g_strength) * blend);
        if (g_strength < 0.001F) {
            return;
        }

        auto offset = Scale(right, static_cast<float>(g_cachedSide) * g_cameraOffset * g_strength);
        const auto targetPosition = Add(position, offset);
        float obstruction{};
        if (RayHit(cell, position, targetPosition, obstruction)) {
            const float safeFraction = std::max(0.0F, obstruction - (12.0F / std::max(g_cameraOffset, 1.0F)));
            offset = Scale(offset, safeFraction);
        }
        // This runs after the game's TESCamera update has authored the base
        // transform, so the lateral shift is freshly based each frame.
        root->SetWorldTranslate(Add(position, offset));
    }

    [[nodiscard]] bool InstallCameraHook() noexcept
    {
        auto* camera = RE::PlayerCamera::GetSingleton();
        if (!camera) {
            REX::ERROR("PeekOnly: PlayerCamera singleton is not available; hook not installed");
            return false;
        }
        auto** vtable = *reinterpret_cast<void***>(camera);
        void* update = vtable[3];  // TESCamera::Update, vtable slot 03.
        auto status = MH_Initialize();
        if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
            REX::ERROR("PeekOnly: MinHook initialization failed ({})", static_cast<int>(status));
            return false;
        }
        status = MH_CreateHook(update, &CameraUpdateHook, reinterpret_cast<void**>(&g_originalCameraUpdate));
        if (status != MH_OK) {
            REX::ERROR("PeekOnly: camera hook creation failed ({})", static_cast<int>(status));
            return false;
        }
        status = MH_EnableHook(update);
        if (status != MH_OK) {
            REX::ERROR("PeekOnly: camera hook enable failed ({})", static_cast<int>(status));
            return false;
        }
        REX::INFO("PeekOnly: TESCamera::Update hook installed at {:X}", reinterpret_cast<std::uintptr_t>(update));
        return true;
    }
}

F4SE_PLUGIN_PRELOAD(const F4SE::PreLoadInterface* f4se)
{
    F4SE::Init(f4se);
    REX::INFO("PeekOnly preloaded");
    return true;
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* f4se)
{
    F4SE::Init(f4se);
    LoadConfig();
    return InstallCameraHook();
}

