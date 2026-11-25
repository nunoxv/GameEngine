#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace xe {

    // Describes a single animation clip (e.g., "idle", "left", "right", "boom").
    struct Animation {
        std::vector<int> frames;   // Ordered list of tile indices inside a SpriteSheet
        float frameTime = 0.1f;    // Duration (in seconds) each frame stays on screen
        bool loop = true;          // true = repeat continuously; false = one-shot (stops on last frame)
    };

    // State machine that tracks elapsed time and selects the active sprite-sheet frame for an Actor.
    // Stores the active clip by name (`name_`) rather than raw pointer so default copy/move work safely.
    class Animator {
    public:
        // Registers or replaces a named animation clip.
        void add(const std::string& name, Animation animation);

        // Switches to a named clip. If already playing that clip and `restart` is false,
        // does nothing so continuous movement inputs (e.g., holding Left) don't reset frame 0 every tick.
        void play(const std::string& name, bool restart = false);

        // Advances the animation timer by `dt` seconds and steps frame indices when needed.
        void update(float dt);

        bool hasClip() const { return currentClip() != nullptr; }
        int frame() const;
        bool finished() const { return finished_; }
        const std::string& currentName() const { return name_; }

    private:
        const Animation* currentClip() const;

        std::unordered_map<std::string, Animation> clips_;
        std::string name_;                   // Name of the active clip
        float time_ = 0.f;                   // Accumulated time within the current frame
        std::size_t index_ = 0;              // Current index into the active clip's frames
        bool finished_ = false;              // True when a one-shot clip reaches its final frame
    };

}  // namespace xe