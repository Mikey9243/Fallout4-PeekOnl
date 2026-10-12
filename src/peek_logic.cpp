#include "peek_logic.h"

#include <algorithm>
#include <cmath>

namespace peekonly
{
    namespace
    {
        [[nodiscard]] float Clamp01(float value) noexcept
        {
            return std::clamp(value, 0.0F, 1.0F);
        }

        [[nodiscard]] float SmoothToward(float current, float target, float rate, float dt) noexcept
        {
            if (rate <= 0.0F || dt <= 0.0F) {
                return target;
            }
            const float blend = 1.0F - std::exp(-rate * dt);
            return current + (target - current) * blend;
        }
    }

    Output Update(const Config& config, const FrameInput& input, float previousStrength) noexcept
    {
        Output result{};
        const int side = input.side < 0 ? -1 : (input.side > 0 ? 1 : 0);
        const bool permitted = input.firstPerson && input.validCorner && side != 0 &&
            input.cameraPathClear && input.weaponPathClear && !input.backingIntoCover;

        float target = 0.0F;
        if (permitted && (input.aiming || config.hipFireScale > 0.0F)) {
            const float distanceRange = std::max(config.detectDistance - config.fullDistance, 0.001F);
            const float distanceFactor = Clamp01((config.detectDistance - input.wallDistance) / distanceRange);
            const float angleRange = std::max(config.fullAngleDegrees - config.startAngleDegrees, 0.001F);
            const float angleFactor = Clamp01((input.intoWallAngleDegrees - config.startAngleDegrees) / angleRange);
            const float stanceScale = input.aiming ? 1.0F : Clamp01(config.hipFireScale);
            target = distanceFactor * angleFactor * stanceScale;
        }

        const float rate = target > previousStrength ? config.fadeInRate : config.fadeOutRate;
        result.strength = Clamp01(SmoothToward(previousStrength, target, rate, input.deltaSeconds));
        const float signedStrength = static_cast<float>(side) * result.strength;
        result.cameraSideOffset = signedStrength * config.cameraOffset;
        result.cameraRollDegrees = signedStrength * config.cameraRollDegrees;
        result.cameraDrop = result.strength * config.cameraDrop;
        result.weaponSideOffset = signedStrength * config.weaponOffset;
        result.weaponYawDegrees = signedStrength * config.weaponYawDegrees;
        result.weaponCantDegrees = signedStrength * config.weaponCantDegrees;
        return result;
    }
}

