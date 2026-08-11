#pragma once

#include <SDL3/SDL_timer.h>

// May be faulty (TODO)
class DeltaTime {
private:
    DeltaTime() = default;
    ~DeltaTime() = default;
    inline static DeltaTime* _instance = nullptr;
    double delta_time;

public:
    static DeltaTime& instance() {
        if (!_instance) {
            _instance = new DeltaTime();
            _instance->delta_time = 0;
        }
        return *_instance;
    }

    DeltaTime(const DeltaTime&) = delete;
    DeltaTime& operator=(const DeltaTime&) = delete;

    static void reset() {
        delete _instance;
        _instance = nullptr;
    }

    double get() const noexcept {
        return delta_time;
    }

    void update() {
        static double last_frame = 0.0;
        double current_frame = static_cast<double>(SDL_GetTicks());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
    }
};
