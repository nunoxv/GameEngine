#include <xe/Animation.h>  // Match your header filename (Animation.h or Animator.h)

#include <algorithm>
#include <utility>

namespace xe {

    const Animation* Animator::currentClip() const {
        if (name_.empty()) return nullptr;
        const auto it = clips_.find(name_);
        return (it != clips_.end()) ? &it->second : nullptr;
    }

    void Animator::add(const std::string& name, Animation animation) {
        clips_[name] = std::move(animation);

        // Reset playback if the currently active clip was just overwritten
        if (name == name_) {
            time_ = 0.f;
            index_ = 0;
            finished_ = false;
        }
    }

    void Animator::play(const std::string& name, bool restart) {
        // Guard against resetting the animation every frame while a directional key is held down
        if (!restart && name == name_ && hasClip()) return;
        if (clips_.find(name) == clips_.end()) return;

        name_ = name;
        time_ = 0.f;
        index_ = 0;
        finished_ = false;
    }

    void Animator::update(float dt) {
        const Animation* clip = currentClip();
        if (!clip || finished_ || clip->frames.empty()) return;

        // Clamp minimum frameTime to 0.1ms to prevent infinite loops if frameTime <= 0
        const float step = std::max(clip->frameTime, 1e-4f);
        time_ += dt;

        // Loop rather than `if` so large `dt` spikes advance the exact number of frames
        while (time_ >= step) {
            time_ -= step;

            if (index_ + 1 < clip->frames.size()) {
                ++index_;
            }
            else if (clip->loop) {
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
        const Animation* clip = currentClip();
        if (!clip || clip->frames.empty()) return 0;
        return clip->frames[index_];
    }

}  // namespace xe