#include "Easing.h"
#include <cmath>
#include<numbers>



// ==========================//イーズイン//================================

float YoshidaMath::NormalizeAngle(float angle)
{

    while (angle > PI) angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;


    return angle;
}

float YoshidaMath::EaseInQuadT(const float& t)
{
    return  t * t;
}

float YoshidaMath::EaseInCubicT(const float& t)
{
    return t * t * t;
}

float YoshidaMath::EaseInQuartT(const float& t) {
    return t * t * t * t;
}

float YoshidaMath::EaseInQuIntT(const float& t)
{
    return t * t * t * t * t;
}

float YoshidaMath::EaseInSineT(const float& t)
{
    return 1.0f - std::cosf((t * PI) / 2.0f);
}

float YoshidaMath::EaseInCircT(const float& t)
{
    return 1.0f - std::sqrtf(1.0f - std::powf(t, 2.0f));
}

float YoshidaMath::EaseInElasticT(const float& t)
{
    const float c4 = (2.0f * PI) / 3.0f;

    if (t == 0.0f) {
        return 0.0f;
    } else if (t == 1.0f) {
        return 1.0f;
    } else {
        float powNum = 10.0f * t - 10.0f;
        float theta = (t * 10.0f - 10.75f) * c4;
        return -powf(2.0f, powNum) * sinf(theta);
    }

}

float YoshidaMath::EaseInExpoT(const float& t) {

    if (t == 0.0f) {
        return 0.0f;
    } else {

        float powNum = 10.0f * t - 10.0f;
        return  powf(2.0f, powNum);
    }

}

float YoshidaMath::EaseInBackT(const float& t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return c3 * t * t * t - c1 * t * t;
}


float YoshidaMath::EaseInBounceT(const float& t) {
    return 1.0f - EaseOutBounceT(1.0f - t);
}

// ==========================//イーズアウト//================================

float YoshidaMath::EaseOutQuadT(const float& t) {
    return 1.0f - powf(1.0f - t, 2.0f);
}

float YoshidaMath::EaseOutCubicT(const float& t)
{
    return 1.0f - powf(1.0f - t, 3.0f);
}

float YoshidaMath::EaseOutQuartT(const float& t) {
    return 1.0f - powf(1.0f - t, 4.0f);
}

float YoshidaMath::EaseOutQuintT(const float& t)
{
    return 1.0f - powf(1.0f - t, 5.0f);
}

float YoshidaMath::EaseOutSineT(const float& t)
{
    return std::sinf((t * PI) / 2.0f);
}

float YoshidaMath::EaseOutCircT(const float& t)
{
    return sqrtf(1.0f - powf(t - 1.0f, 2.0f));
}

float YoshidaMath::EaseOutElasticT(const float& t)
{
    const float c4 = (2.0f * PI) / 3.0f;

    if (t == 0.0f) {
        return 0.0f;
    } else if (t == 1.0f) {
        return 1.0f;
    } else {

        float powNum = -10.0f * t;
        float theta = (t * 10.0f - 0.75f) * c4;
        return powf(2.0f, powNum) * sinf(theta) + 1.0f;
    }

}

float YoshidaMath::EaseOutExpoT(const float& t) {
    float powNum = -10.0f * t;
    return (t == 1.0f) ? 1.0f : 1.0f - powf(2.0f, powNum);
}

float YoshidaMath::EaseOutBackT(const float& t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}


float YoshidaMath::EaseOutBounceT(const float& t) {

    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float newT = t;

    if (newT < 1.0f / d1) {
        return n1 * newT * newT;
    } else if (newT < 2.0f / d1) {
        return n1 * (newT -= 1.5f / d1) * newT + 0.75f;
    } else if (newT < 2.5f / d1) {
        return n1 * (newT -= 2.25f / d1) * newT + 0.9375f;
    } else {
        return n1 * (newT -= 2.625f / d1) * newT + 0.984375f;
    }
}

float YoshidaMath::EaseInOutT(const float& t)
{
    return -(std::cosf(PI * t) - 1.0f) / 2.0f;
}

float YoshidaMath::EaseInOutQuadT(const float& t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

float YoshidaMath::EaseInOutQuartT(const float& t) {
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

float YoshidaMath::EaseInOutSineT(const float& t)
{
    return-(cosf(PI * t) - 1.0f) / 2.0f;
}

float YoshidaMath::EaseInOutBackT(const float& x) {

    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    return x < 0.5f
        ? (std::powf(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
        : (std::powf(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

float YoshidaMath::EaseInOutCubicT(const float& x) {
    return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::powf(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

float YoshidaMath::EaseInOutExpoT(const float& t)
{
    if (t == 0.0f) {
        return 0.0f;
    } else if (t == 1.0f) {
        return 1.0f;
    } else {
        if (t < 0.5f) {
            float powNum = 20.0f * t - 10.0f;
            return powf(2.0f, powNum) / 2.0f;
        } else {
            float powNum = -20.0f * t + 10.0f;
            return (2.0f - powf(2.0f, powNum)) / 2.0f;
        }
    }

}

float YoshidaMath::EaseInOutQuintT(const float& t)
{
    return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

float YoshidaMath::EaseInOutCircT(const float& t) {
    return t < 0.5f
        ? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) / 2.0f
        : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

float YoshidaMath::EaseInOutElasticT(const float& t) {
    const float c5 = (2.0f * PI) / 4.5f;

    if (t == 0.0f) {
        return 0.0f;
    } else if (t == 1.0f) {
        return 1.0f;
    } else {
        float theta = (20.0f * t - 11.125f) * c5;
        if (t < 0.5f) {
            float powNum = 20.0f * t - 10.0f;
            return  -(powf(2.0f, powNum) * sinf(theta)) / 2.0f;
        } else {
            float powNum = -20.0f * t + 10.0f;
            return (powf(2.0f, powNum) * sinf(theta)) / 2.0f + 1.0f;
        }

    }


}

float YoshidaMath::EaseInOutBounceT(const float& t) {
    return t < 0.5f
        ? (1.0f - EaseOutBounceT(1.0f - 2.0f * t)) / 2.0f
        : (1.0f + EaseOutBounceT(2.0f * t - 1.0f)) / 2.0f;
}