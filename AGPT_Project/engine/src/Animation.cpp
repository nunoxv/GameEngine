#include <xe/Animation.h>
#include <algorithm>

namespace xe {

    Animator::Animator(const Animator& other)
        : clips_(other.clips_),
        name_(other.name_),
        time_(other.time_),
        index_(other.index_),
        finished_(other.finished_) {
        rebindCurrent();
    }

    Animator& Animator::operator=(const Animator& other) {
        if (this != &other) {
            clips_ = other.clips_;
            name_ = other.name_;
            time_ = other.time_;
            index_ = other.index_;
            finished_ = other.finished_;
            rebindCurrent();
        }
        return *this;
    }

    Animator::Animator(Animator&& other) noexcept
        : clips_(std::move(other.clips_)),
        name_(std::move(other.name_)),
        time_(other.time_),
        index_(other.index_),
        finished_(other.finished_) {
        other.current_ = nullptr;
        rebindCurrent();
    }

    Animator& Animator::operator=(Animator&& other) noexcept {
        if (this != &other) {
            clips_ = std::move(other.clips_);
            name_ = std::move(other.name_);
            time_ = other.time_;
            index_ = other.index_;
            finished_ = other.finished_;
            other.current_ = nullptr;
            rebindCurrent();
        }
        return *this;
    }

    void Animator::rebindCurrent() {
        if (name_.empty()) {
            current_ = nullptr;
            return;
        }
        auto it = clips_.find(name_);
        current_ = (it != clips_.end()) ? &it->second : nullptr;
    }

    void Animator::add(const std::string& name, Animation animation) {
        clips_[name] = std::move(animation);

        // If we just overwrote the currently playing clip, reset playback safely
        if (current_ && name == name_) {
            current_ = &clips_[name];
            time_ = 0.f;
            index_ = 0;
            finished_ = false;
        }
    }

    void Animator::play(const std::string& name, bool restart) {
        // Guard against resetting the animation every frame while a directional key is held down
        if (!restart && current_ && name == name_) return;

        auto it = clips_.find(name);
        if (it == clips_.end()) return;

        current_ = &it->second;
        name_ = name;
        time_ = 0.f;
        index_ = 0;
        finished_ = false;
    }

    void Animator::update(float dt) {
        if (!current_ || finished_ || current_->frames.empty()) return;

        // Clamp minimum frameTime to 0.1ms to prevent infinite loops if frameTime <= 0
        const float step = std::max(current_->frameTime, 1e-4f);
        time_ += dt;

        // Use a `while` loop rather than `if` so large `dt` spikes advance the exact number of frames
        while (time_ >= step) {
            time_ -= step;

            if (index_ + 1 < current_->frames.size()) {
                // Advance to the next frame in the sequence
                ++index_;
            }
            else if (current_->loop) {
                // Wrap around to the first frame for continuous animations (e.g., spinning projectiles)
                index_ = 0;
            }
            else {
                // One-shot animation (e.g., Explosion): freeze on the last frame and signal completion
                finished_ = true;
                break;
            }
        }
    }

    int Animator::frame() const {
        if (!current_ || current_->frames.empty()) return 0;
        return current_->frames[index_];
    }

}  // namespace xe