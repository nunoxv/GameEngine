#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace xe {

struct Animation {
    std::vector<int> frames;   // sprite-sheet frame indices
    float frameTime = 0.1f;    // seconds per frame
    bool loop = true;          // false => one-shot, holds the last frame and reports finished()
};

// Selects which frame an actor shows and when.
class Animator {
public:
    void add(const std::string& name, Animation animation);
    void play(const std::string& name, bool restart = false);
    void update(float dt);

    bool hasClip() const { return current_ != nullptr; }
    int frame() const;
    bool finished() const { return finished_; }
    const std::string& currentName() const { return name_; }

private:
    std::unordered_map<std::string, Animation> clips_;  // node-based: element pointers stay valid
    const Animation* current_ = nullptr;
    std::string name_;
    float time_ = 0.f;
    std::size_t index_ = 0;
    bool finished_ = false;
};

}  // namespace xe
