#ifndef SDK_HPP
#define SDK_HPP

#include "offsets.hpp"
#include "memory/read.h"
#include "memory/memory.h"
#include <cstdint>
#include <string>
#include <imgui.h>

namespace sdk
{

    struct Vector3
    {
        float x, y, z;
    };

    struct ViewMatrix
    {
        float m11, m12, m13, m14;
        float m21, m22, m23, m24;
        float m31, m32, m33, m34;
        float m41, m42, m43, m44;
    };

    pid_t get_game_pid();
    uint64_t get_module_base(pid_t pid, const char *module_name);
    uint64_t read_uint64(pid_t pid, uint64_t address);
    int32_t read_int32(pid_t pid, uint64_t address);
    uint8_t read_byte(pid_t pid, uint64_t address);
    std::string read_string(pid_t pid, uint64_t address);
    Vector3 read_vector3(pid_t pid, uint64_t address);
    ViewMatrix read_view_matrix(pid_t pid, uint64_t address);

    uint64_t get_player_manager_static(pid_t pid, uint64_t lib_base, uintptr_t type_offset);
    uint64_t get_player_by_index(pid_t pid, uint64_t list_buffer, int index);
    Vector3 get_player_position(pid_t pid, uint64_t player);
    int get_player_health(pid_t pid, uint64_t player);
    std::string get_player_name(pid_t pid, uint64_t player);
    ViewMatrix get_view_matrix(pid_t pid, uint64_t local_player);

    bool world_to_screen(const Vector3 &pos, const ViewMatrix &m, ImVec2 &out, float screen_w, float screen_h);

}

#endif
