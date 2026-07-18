#ifndef OFFSETS_HPP
#define OFFSETS_HPP

#include <cstdint>

namespace offsets
{

    namespace PlayerManager
    {
        constexpr uintptr_t TypeOffset = 0xADFBC78;
        constexpr uintptr_t LocalPlayer = 0x70;
        constexpr uintptr_t PlayerList = 0x28;
    }

    namespace Dictionary
    {
        constexpr uintptr_t Count = 0x20;
        constexpr uintptr_t Entries = 0x18;
        constexpr uintptr_t EntrySize = 0x18;
        constexpr uintptr_t KeyOffset = 0x28;
        constexpr uintptr_t ValueOffset = 0x30;
        constexpr uintptr_t ValueUnboxOffset = 0x10;
    }

    namespace PlayerController
    {
        constexpr uintptr_t MovementController = 0x98;
        constexpr uintptr_t PhotonPlayer = 0x160;
        constexpr uintptr_t PlayerMainCamera = 0xE8;
    }

    namespace MovementController
    {
        constexpr uintptr_t TransformData = 0xB0;
    }

    namespace TransformData
    {
        constexpr uintptr_t Position = 0x44;
    }

    namespace PlayerMainCamera
    {
        constexpr uintptr_t CameraTransform = 0x20;
    }

    namespace CameraTransform
    {
        constexpr uintptr_t CameraMatrixData = 0x10;
    }

    namespace CameraMatrixData
    {
        constexpr uintptr_t ViewMatrix = 0xF0;
    }

    namespace PhotonPlayer
    {
        constexpr uintptr_t NickName = 0x20;
        constexpr uintptr_t CustomProperties = 0x38;
    }

    namespace String
    {
        constexpr uintptr_t Length = 0x10;
        constexpr uintptr_t Data = 0x14;
    }

}

#endif
