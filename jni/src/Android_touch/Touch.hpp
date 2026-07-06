#ifndef TOUCH_HPP
#define TOUCH_HPP

#include <cstdint>
namespace touch
{
    bool init(int32_t screen_w, int32_t screen_h, uint8_t orientation);
    void update(int32_t screen_w, int32_t screen_h, uint8_t orientation);
    void updateOrientation(uint8_t orientation);
    void shutdown();
}
#endif
