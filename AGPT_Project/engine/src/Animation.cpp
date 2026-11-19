#include <xe/Animation.h>

#include <algorithm>

namespace xe {

void Animator::add(const std::string& name, Animation animation) {
    clips_[name] = std::move(animation);
}

void Animator::play(const std::string& name, bool restart) {
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
    const float step = std::max(current_->frameTime, 1e-4f);
    time_ += dt;
    while (time_ >= step) {
        time_ -= step;
        if (index_ + 1 < current_->frames.size()) {
            ++index_;
        } else if (current_->loop) {
            index_ = 0;
        } else {
            finished_ = true;   // one-shot: hold the last frame
            break;
        }
    }
}

int Animator::frame() const {
    if (!current_ || current_->frames.empty()) return 0;
    return current_->frames[index_];
}

}  // namespace xe
