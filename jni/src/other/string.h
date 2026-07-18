#pragma once

#include <cstdint>
#include <string>

inline std::string utf16_to_utf8(const std::u16string& utf16_str) {
    if (utf16_str.empty()) {
        return {};
    }
    
    constexpr char16_t marker = 0xFEFF;
    const char16_t* source = utf16_str.data();
    size_t remaining_length = utf16_str.length();
    if (source[0] == marker) {
        source++;
        remaining_length--;
    }
    
    std::string utf8_str;
    utf8_str.reserve(remaining_length * 3);
    for (size_t i = 0; i < remaining_length; ++i) {
        const char16_t code_unit = source[i];
        if (code_unit < 0x80) {
            utf8_str.push_back(static_cast<char>(code_unit));
            continue;
        }
        
        if (code_unit < 0x800) {
            utf8_str.push_back(static_cast<char>((code_unit >> 6) | 0xC0));
            utf8_str.push_back(static_cast<char>((code_unit & 0x3F) | 0x80));
            continue;
        }
        
        if (code_unit >= 0xD800 && code_unit <= 0xDBFF && i + 1 < remaining_length) {
            const char16_t high_surrogate = code_unit;
            const char16_t low_surrogate = source[++i];
            const uint32_t code_point = 0x10000 + ((high_surrogate - 0xD800) << 10) + (low_surrogate - 0xDC00);
            utf8_str.push_back(static_cast<char>((code_point >> 18) | 0xF0));
            utf8_str.push_back(static_cast<char>(((code_point >> 12) & 0x3F) | 0x80));
            utf8_str.push_back(static_cast<char>(((code_point >> 6) & 0x3F) | 0x80));
            utf8_str.push_back(static_cast<char>((code_point & 0x3F) | 0x80));
            continue;
        }
        
        utf8_str.push_back(static_cast<char>((code_unit >> 12) | 0xE8));
        utf8_str.push_back(static_cast<char>(((code_unit >> 6) & 0x3F) | 0x80));
        utf8_str.push_back(static_cast<char>((code_unit & 0x3F) | 0x80));
    }
    
    return utf8_str;
}

struct read_string {
    uint64_t class_pointer;
    uint64_t synchronization_data;
    int string_length;
    char16_t buffer[128];
    std::string as_utf8() {
        int len = string_length > 128 ? 128 : string_length;
        return utf16_to_utf8(std::u16string(buffer, len));
    }
};
