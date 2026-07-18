#include "sdk.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>

namespace sdk
{

    pid_t get_game_pid()
    {
        FILE *fp = popen("su -c 'pidof com.axlebolt.standoff2'", "r");
        if (!fp)
            return -1;
        char buf[64];
        if (fgets(buf, sizeof(buf), fp))
        {
            pclose(fp);
            return atoi(buf);
        }
        pclose(fp);
        return -1;
    }

    uint64_t get_module_base(pid_t pid, const char *module_name)
    {
        return memory::GetModuleBase(pid, module_name);
    }

    uint64_t read_uint64(pid_t pid, uint64_t address)
    {
        uint64_t value = 0;
        memory::ReadProcessMemory(pid, address, &value, sizeof(value));
        return value;
    }

    int32_t read_int32(pid_t pid, uint64_t address)
    {
        int32_t value = 0;
        memory::ReadProcessMemory(pid, address, &value, sizeof(value));
        return value;
    }

    uint8_t read_byte(pid_t pid, uint64_t address)
    {
        uint8_t value = 0;
        memory::ReadProcessMemory(pid, address, &value, sizeof(value));
        return value;
    }

    std::string read_string(pid_t pid, uint64_t address)
    {
        int32_t length = read_int32(pid, address + offsets::String::Length);
        if (length <= 0 || length > 1024)
            return "";
        uint64_t data_ptr = read_uint64(pid, address + offsets::String::Data);
        if (!data_ptr)
            return "";
        std::vector<char> buffer(length * 2 + 1);
        if (!memory::ReadProcessMemory(pid, data_ptr, buffer.data(), static_cast<size_t>(length * 2)))
            return "";
        buffer[length * 2] = '\0';
        char16_t *utf16 = reinterpret_cast<char16_t *>(buffer.data());
        std::string result;
        for (int i = 0; i < length; i++)
        {
            char16_t c = utf16[i];
            if (c < 0x80)
                result += static_cast<char>(c);
            else if (c < 0x800)
            {
                result += static_cast<char>(0xC0 | (c >> 6));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
            else
            {
                result += static_cast<char>(0xE0 | (c >> 12));
                result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
        }
        return result;
    }

    Vector3 read_vector3(pid_t pid, uint64_t address)
    {
        Vector3 v{};
        memory::ReadProcessMemory(pid, address, &v, sizeof(v));
        return v;
    }

    ViewMatrix read_view_matrix(pid_t pid, uint64_t address)
    {
        ViewMatrix m{};
        memory::ReadProcessMemory(pid, address, &m, sizeof(m));
        return m;
    }

    uint64_t get_player_manager_static(pid_t pid, uint64_t lib_base, uintptr_t type_offset)
    {
        uint64_t cls = read_uint64(pid, lib_base + type_offset);
        if (!cls)
            return 0;
        uint64_t static_fields_obj = read_uint64(pid, cls + 0x148);
        if (!static_fields_obj)
            return 0;
        uint64_t fields_addr = read_uint64(pid, static_fields_obj + 0x10);
        if (!fields_addr)
            return 0;
        return read_uint64(pid, fields_addr);
    }

    uint64_t get_player_by_index(pid_t pid, uint64_t list_buffer, int index)
    {
        if (!list_buffer)
            return 0;
        uint64_t value_addr = list_buffer + offsets::Dictionary::ValueOffset + index * offsets::Dictionary::EntrySize;
        return read_uint64(pid, value_addr);
    }

    Vector3 get_player_position(pid_t pid, uint64_t player)
    {
        if (!player)
            return {0, 0, 0};
        uint64_t mov = read_uint64(pid, player + offsets::PlayerController::MovementController);
        if (!mov)
            return {0, 0, 0};
        uint64_t td = read_uint64(pid, mov + offsets::MovementController::TransformData);
        if (!td)
            return {0, 0, 0};
        return read_vector3(pid, td + offsets::TransformData::Position);
    }

    int get_player_health(pid_t pid, uint64_t player)
    {
        if (!player)
            return 0;
        uint64_t photon = read_uint64(pid, player + offsets::PlayerController::PhotonPlayer);
        if (!photon)
            return 0;
        uint64_t props = read_uint64(pid, photon + offsets::PhotonPlayer::CustomProperties);
        if (!props)
            return 0;
        int count = read_int32(pid, props + offsets::Dictionary::Count);
        uint64_t entries = read_uint64(pid, props + offsets::Dictionary::Entries);
        if (!entries || count <= 0)
            return 0;
        for (int i = 0; i < count; i++)
        {
            uint64_t key_addr = entries + offsets::Dictionary::KeyOffset + i * offsets::Dictionary::EntrySize;
            uint64_t key = read_uint64(pid, key_addr);
            if (!key)
                continue;
            std::string key_str = read_string(pid, key);
            if (key_str == "health")
            {
                uint64_t value_addr = entries + offsets::Dictionary::ValueOffset + i * offsets::Dictionary::EntrySize;
                uint64_t value_obj = read_uint64(pid, value_addr);
                if (!value_obj)
                    return 0;
                return read_int32(pid, value_obj + offsets::Dictionary::ValueUnboxOffset);
            }
        }
        return 0;
    }

    std::string get_player_name(pid_t pid, uint64_t player)
    {
        if (!player)
            return "";
        uint64_t photon = read_uint64(pid, player + offsets::PlayerController::PhotonPlayer);
        if (!photon)
            return "";
        uint64_t name_obj = read_uint64(pid, photon + offsets::PhotonPlayer::NickName);
        if (!name_obj)
            return "";
        return read_string(pid, name_obj);
    }

    ViewMatrix get_view_matrix(pid_t pid, uint64_t local_player)
    {
        ViewMatrix m{};
        if (!local_player)
            return m;
        uint64_t pmc = read_uint64(pid, local_player + offsets::PlayerController::PlayerMainCamera);
        if (!pmc)
            return m;
        uint64_t ct = read_uint64(pid, pmc + offsets::PlayerMainCamera::CameraTransform);
        if (!ct)
            return m;
        uint64_t cmd = read_uint64(pid, ct + offsets::CameraTransform::CameraMatrixData);
        if (!cmd)
            return m;
        return read_view_matrix(pid, cmd + offsets::CameraMatrixData::ViewMatrix);
    }

    bool world_to_screen(const Vector3 &pos, const ViewMatrix &m, ImVec2 &out, float screen_w, float screen_h)
    {
        float sx = m.m11 * pos.x + m.m21 * pos.y + m.m31 * pos.z + m.m41;
        float sy = m.m12 * pos.x + m.m22 * pos.y + m.m32 * pos.z + m.m42;
        float sw = m.m14 * pos.x + m.m24 * pos.y + m.m34 * pos.z + m.m44;

        if (sw <= 0.0001f)
        {
            out = ImVec2(-10000.f, -10000.f);
            return false;
        }

        float iw = 1.f / sw;
        float nx = sx * iw;
        float ny = sy * iw;

        out.x = (nx + 1.f) * 0.5f * screen_w;
        out.y = (1.f - ny) * 0.5f * screen_h;
        return true;
    }

}
