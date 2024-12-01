#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <format>
#include <iostream>

namespace tlc {
    class UUID {
        private:
            UUID(void* data);

        public:
            ~UUID() = default;

            std::string ToString() const;
            const uint8_t* ToBytes() const { return m_Data.data(); }

            static bool IsValid(std::string str);
            static UUID FromBytes(const void* data);
            static UUID FromString(std::string str);
            static UUID New();
            static UUID Zero();

            static constexpr size_t GetNumBytes() { return k_NumBytes; }
        private:
            static uint32_t s_Counter;

            static constexpr size_t k_NumBytes = 16;
            std::array<uint8_t, k_NumBytes> m_Data;
    };
}

inline bool operator==(const tlc::UUID& lhs, const tlc::UUID& rhs) {
    return std::equal(lhs.ToBytes(), lhs.ToBytes() + tlc::UUID::GetNumBytes(), rhs.ToBytes());
}

inline bool operator!=(const tlc::UUID& lhs, const tlc::UUID& rhs) {
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const tlc::UUID& uuid) {
    return os << uuid.ToString();
}

namespace std {
    template <>
    struct hash<tlc::UUID> {
        size_t operator()(const tlc::UUID& uuid) const {
            return std::hash<std::string>{}(uuid.ToString());
        }
    };

    template <>
    struct formatter<tlc::UUID> : formatter<string> {
        template<typename ParseContext>
        auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const tlc::UUID& uuid, FormatContext& ctx) const {
            return format_to(ctx.out(), "{}", uuid.ToString());
        }
    };
}