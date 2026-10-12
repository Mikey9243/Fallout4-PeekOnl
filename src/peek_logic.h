#pragma once

namespace peekonly
{
    struct Config
    {
        float detectDistance{ 100.0F };
        float fullDistance{ 30.0F };
        float startAngleDegrees{ 5.0F };
        float fullAngleDegrees{ 30.0F };
        float hipFireScale{ 0.0F };
        float cameraOffset{ 60.0F };
        float cameraRollDegrees{ 10.0F };
        float cameraDrop{ 0.0F };
        float weaponOffset{ 65.0F };
        float weaponYawDegrees{ 6.0F };
        float weaponCantDegrees{ 8.0F };
        float fadeInRate{ 4.0F };
        float fadeOutRate{ 6.0F };
        float edgeMargin{ 16.0F };
    };

    // Runtime probes must supply geometry and input; this module only turns those
    // observations into a smoothed peek amount and signed camera/weapon targets.
    struct FrameInput
    {
        bool firstPerson{ false };
        bool aiming{ false };
        bool backingIntoCover{ false };
        bool validCorner{ false };
        bool cameraPathClear{ false };
        bool weaponPathClear{ false };
        int side{ 0 }; // -1 left, +1 right
        float wallDistance{ 0.0F };
        float intoWallAngleDegrees{ 0.0F };
        float deltaSeconds{ 0.0F };
    };

    struct Output
    {
        float strength{ 0.0F };
        float cameraSideOffset{ 0.0F };
        float cameraRollDegrees{ 0.0F };
        float cameraDrop{ 0.0F };
        float weaponSideOffset{ 0.0F };
        float weaponYawDegrees{ 0.0F };
        float weaponCantDegrees{ 0.0F };
    };

    [[nodiscard]] Output Update(const Config& config, const FrameInput& input, float previousStrength) noexcept;
}

