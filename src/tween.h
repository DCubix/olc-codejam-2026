#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>
#include <cmath>
#include <numbers>

constexpr float pi = std::numbers::pi_v<float>;

template <typename T>
concept AddSub = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
};

template <typename T>
concept MulScalar = requires(T a, float b) {
    { a * b } -> std::convertible_to<T>;
};

template <typename T>
concept Tweenable = (std::is_arithmetic_v<T> || (AddSub<T> && MulScalar<T>));

using EaseFunction = std::function<float(float)>;

enum class Ease
{
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,
    EaseInQuint,
    EaseOutQuint,
    EaseInOutQuint,
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,
    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce
};

enum class Loop
{
    NoLoop,
    Repeat,
    PingPong
};

static float BounceOut(float t) {
    if (t < (4.0f / 11.0f)) {
        return (121.0f * t * t) / 16.0f;
    } else if (t < (8.0f / 11.0f)) {
        return (363.0f / 40.0f * t * t) - (99.0f / 10.0f * t) + 17.0f / 5.0f;
    } else if (t < (9.0f / 10.0f)) {
        return (4356.0f / 361.0f * t * t) - (35442.0f / 1805.0f * t) + 16061.0f / 1805.0f;
    } else {
        return (54.0f / 5.0f * t * t) - (513.0f / 25.0f * t) + 268.0f / 25.0f;
    }
}

inline static std::unordered_map<Ease, EaseFunction> easingFunctions = {
    { Ease::Linear, [](float t) { return t; } },
    { Ease::EaseInQuad, [](float t) { return t * t; } },
    { Ease::EaseOutQuad, [](float t) { return t * (2.0f - t); } },
    { Ease::EaseInOutQuad, [](float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; } },
    { Ease::EaseInCubic, [](float t) { return t * t * t; } },
    { Ease::EaseOutCubic, [](float t) { float t1 = t - 1.0f; return t1 * t1 * t1 + 1.0f; } },
    { Ease::EaseInOutCubic, [](float t) { return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f; } },
    { Ease::EaseInQuart, [](float t) { return t * t * t * t; } },
    { Ease::EaseOutQuart, [](float t) { float t1 = t - 1.0f; return 1.0f - t1 * t1 * t1 * t1; } },
    { Ease::EaseInOutQuart, [](float t) { if (t < 0.5f) return 8.0f * t * t * t * t; float t1 = t - 1.0f; return 1.0f - 8.0f * t1 * t1 * t1 * t1; } },
    { Ease::EaseInQuint, [](float t) { return t * t * t * t * t; } },
    { Ease::EaseOutQuint, [](float t) { float t1 = t - 1.0f; return 1.0f + t1 * t1 * t1 * t1 * t1; } },
    { Ease::EaseInOutQuint, [](float t) { if (t < 0.5f) return 16.0f * t * t * t * t * t; float t1 = t - 1.0f; return 1.0f + 16.0f * t1 * t1 * t1 * t1 * t1; } },
    { Ease::EaseInBack, [](float t) { constexpr float s = 1.70158f; return t * t * ((s + 1.0f) * t - s); } },
    { Ease::EaseOutBack, [](float t) { constexpr float s = 1.70158f; float t1 = t - 1.0f; return t1 * t1 * ((s + 1.0f) * t1 + s) + 1.0f; } },
    { Ease::EaseInOutBack, [](float t) {
        constexpr float s = 1.70158f * 1.525f;
        if (t < 0.5f) {
            float t2 = 2.0f * t;
            return 0.5f * (t2 * t2 * ((s + 1.0f) * t2 - s));
        } else {
            float t2 = 2.0f * t - 2.0f;
            return 0.5f * (t2 * t2 * ((s + 1.0f) * t2 + s) + 2.0f);
        }
    } },
    { Ease::EaseInElastic, [](float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return -std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.1f) * 5.0f * pi);
    } },
    { Ease::EaseOutElastic, [](float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.1f) * 5.0f * pi) + 1.0f;
    } },
    { Ease::EaseInOutElastic, [](float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        t = 2.0f * t;
        if (t < 1.0f) {
            return -0.5f * std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.1f) * 5.0f * pi);
        }
        return 0.5f * std::pow(2.0f, -10.0f * (t - 1.0f)) * std::sin((t - 1.1f) * 5.0f * pi) + 1.0f;
    } },
    { Ease::EaseInBounce, [](float t) {
        return 1.0f - BounceOut(1.0f - t);
    } },
    { Ease::EaseOutBounce, [](float t) {
        return BounceOut(t);
    } },
    { Ease::EaseInOutBounce, [](float t) {
        if (t < 0.5f) {
            return (1.0f - BounceOut(1.0f - 2.0f * t)) * 0.5f;
        }
        return (1.0f + BounceOut(2.0f * t - 1.0f)) * 0.5f;
    } }
};

class Animator;

struct IAnimation
{
    float duration{ 0.0f }, elapsed{ 0.0f }, delay{ 0.0f };
    std::string tag{};

    virtual ~IAnimation() = default;
    virtual bool Update(float deltaTime, bool accumulate) = 0; // Returns true if animation is complete
};

template <Tweenable T>
struct Tween : public IAnimation
{
    friend class TweenAnimator;
protected:
    T* target;
    T startValue, endValue;

    Ease easing{ Ease::Linear };
    Loop loop{ Loop::NoLoop };
    int32_t loopCount{ 0 }, // -1 for infinite
            currentLoop{ 0 };

    std::function<void()> onComplete{ nullptr };
    std::function<void(int32_t index)> onLoop{ nullptr };
    bool completed{ false }, reversing{ false };

    bool Update(float deltaTime, bool accumulate) override
    {
        if (!target) return true; // No target to animate
        if (completed) return true; // Already completed

        if (delay > 0.0f) {
            delay -= deltaTime;
            return false;
        }

        elapsed += deltaTime;
        float t = std::min(elapsed / duration, 1.0f);

        t = easingFunctions[easing](t);
        if (reversing) {
            t = 1.0f - t;
        }

        T newValue;
        if constexpr (std::is_integral_v<T>) {
            newValue = static_cast<T>(static_cast<float>(startValue) + static_cast<float>(endValue - startValue) * t);
        } else {
            newValue = startValue + (endValue - startValue) * t;
        }

        if (accumulate) {
            *target += newValue - (*target);
        } else {
            *target = newValue;
        }

        if (elapsed >= duration) {
            if (loop == Loop::NoLoop || (loopCount != -1 && currentLoop >= loopCount)) {
                *target = endValue;
                if (onComplete) onComplete();
                completed = true;
                return true; // Animation complete
            }

            currentLoop++;
            elapsed -= duration;

            if (loop == Loop::PingPong) {
                reversing = !reversing;
            }

            if (onLoop) onLoop(currentLoop - 1);
        }

        return false; // Animation still in progress
    }
};

class TweenAnimator
{
protected:
    std::vector<std::unique_ptr<IAnimation>> activeAnimations;
    std::unordered_map<void*, bool> updateStates;
    IAnimation* lastAddedAnimation{ nullptr };

public:
    template <Tweenable T>
    struct Builder {
        TweenAnimator& animator;
        std::unique_ptr<Tween<T>> animation;
        bool fromSpecified{ false };

        Builder(TweenAnimator& animator, T* target, const std::string& tag)
            : animator(animator),
              animation(std::make_unique<Tween<T>>())
        {
            animation->target = target;
            animation->tag = tag;
        }

        Builder& To(T endValue) {
            animation->endValue = endValue;
            return *this;
        }
        Builder& From(T startValue) {
            fromSpecified = true;
            animation->startValue = startValue;
            return *this;
        }
        Builder& For(float duration) {
            animation->duration = duration;
            return *this;
        }
        Builder& Wait(float delay) {
            animation->delay = delay;
            return *this;
        }
        Builder& Then() {
            if (animator.lastAddedAnimation) {
                animation->delay += animator.lastAddedAnimation->duration + animator.lastAddedAnimation->delay;
                if (!fromSpecified) {
                    auto anim = dynamic_cast<Tween<T>*>(animator.lastAddedAnimation);
                    return From(anim->endValue);
                }
            }
            return *this;
        }
        Builder& Ease(Ease easing) {
            animation->easing = easing;
            return *this;
        }
        Builder& Repeat(int32_t count = -1) {
            animation->loop = Loop::Repeat;
            animation->loopCount = count;
            return *this;
        }
        Builder& PingPong(int32_t count = -1) {
            animation->loop = Loop::PingPong;
            animation->loopCount = count;
            return *this;
        }
        Builder& OnComplete(std::function<void()> callback) {
            animation->onComplete = callback;
            return *this;
        }
        Builder& OnLoop(std::function<void(uint32_t index)> callback) {
            animation->onLoop = callback;
            return *this;
        }

        TweenAnimator& Start() {
            if (!fromSpecified) {
                animation->startValue = *animation->target;
            }

            // An empty tag is never deduplicated.
            bool exists = !animation->tag.empty() &&
                std::any_of(animator.activeAnimations.begin(), animator.activeAnimations.end(),
                    [&](const std::unique_ptr<IAnimation>& anim) {
                        auto casted = dynamic_cast<Tween<T>*>(anim.get());
                        return anim->tag == animation->tag && casted->target == animation->target;
                    });
            if (!exists) {
                animator.lastAddedAnimation = animation.get();
                animator.activeAnimations.push_back(std::move(animation));
            }
            return animator;
        }
    };

    template <Tweenable T>
    Builder<T> Animate(T* target, const std::string& tag = "") {
        if (activeAnimations.capacity() == 0) {
            activeAnimations.reserve(1000);
        }
        // updateStates[target] = false;
        return Builder<T>(*this, target, tag);
    }

    template <Tweenable T>
    void Staggered(const std::vector<T*>& targets, float duration, float staggerDelay, Ease easing = Ease::Linear) {
        for (size_t i = 0; i < targets.size(); i++) {
            Animate(targets[i])
                .For(duration)
                .Wait(i * staggerDelay)
                .Ease(easing)
            .Start();
        }
    }

    void Update(float deltaTime) {
        activeAnimations.erase(
            std::remove_if(
                activeAnimations.begin(), activeAnimations.end(),
                [&](const std::unique_ptr<IAnimation>& anim) {
                    // bool shouldAccumulate = updateStates[anim.get()];
                    // updateStates[anim.get()] = true;
                    return anim->Update(deltaTime, false);
                }
            ),
            activeAnimations.end()
        );
    }
};