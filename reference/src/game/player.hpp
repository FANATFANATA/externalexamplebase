#pragma once

#include "game.hpp"
#include "../other/vector3.h"
#include "../other/string.h"
#include "../protect/oxorany.hpp"
#include <cstring>
#include <string>

namespace player {
    inline Vector3 position(uint64_t p) noexcept {
        uint64_t MovementController = rpm<uint64_t>(p + oxorany(0x98));
        if (!MovementController) return Vector3(0, 0, 0);

        uint64_t TransformData = rpm<uint64_t>(MovementController + oxorany(0xB0));
        if (!TransformData) return Vector3(0, 0, 0);

        return rpm<Vector3>(TransformData + oxorany(0x44));
    }

    inline uint64_t photon_ptr(uint64_t p) noexcept {
        return rpm<uint64_t>(p + oxorany(0x160));
    }

    template<typename T>
    inline T property(uint64_t p, const char* tag) noexcept {
        T result{};
        uint64_t PhotonPlayer = photon_ptr(p);
        if (!PhotonPlayer) return result;

        uint64_t PropertiesRegistry = rpm<uint64_t>(PhotonPlayer + oxorany(0x38));
        if (!PropertiesRegistry) return result;

        int Count = rpm<int>(PropertiesRegistry + oxorany(0x20));
        uint64_t PropertiesList = rpm<uint64_t>(PropertiesRegistry + oxorany(0x18));

        for (int i = 0; i < Count; i++) {
            uint64_t Key = rpm<uint64_t>(PropertiesList + oxorany(0x28) + oxorany(0x18) * i);
            uint64_t Value = rpm<uint64_t>(PropertiesList + oxorany(0x30) + oxorany(0x18) * i);

            if (!Key) continue;

            std::string KeyString = rpm<read_string>(Key).as_utf8();
            if (strstr(KeyString.c_str(), tag)) {
                result = rpm<T>(Value + oxorany(0x10));
                break;
            }
        }

        return result;
    }

    inline int health(uint64_t p) noexcept {
        return property<int>(p, oxorany("health"));
    }

    inline read_string name(uint64_t p) noexcept {
        uint64_t PhotonPlayer = photon_ptr(p);
        if (!PhotonPlayer) return {};
        return rpm<read_string>(rpm<uint64_t>(PhotonPlayer + oxorany(0x20)));
    }

    inline matrix view_matrix(uint64_t p) noexcept {
        uint64_t PlayerMainCamera = rpm<uint64_t>(p + oxorany(0xE8));
        if (!PlayerMainCamera) return {};

        uint64_t CameraTransform = rpm<uint64_t>(PlayerMainCamera + oxorany(0x20));
        if (!CameraTransform) return {};

        uint64_t CameraMatrix = rpm<uint64_t>(CameraTransform + oxorany(0x10));
        if (!CameraMatrix) return {};

        return rpm<matrix>(CameraMatrix + oxorany(0xF0));
    }
}
