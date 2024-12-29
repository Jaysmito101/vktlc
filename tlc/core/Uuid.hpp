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
			inline UUID() : UUID(nullptr) {}
            ~UUID() = default;

            std::string ToString() const;
            const uint8_t* ToBytes() const { return m_Data.data(); }

            static bool IsValid(std::string str);
            static UUID FromBytes(const void* data);
            static UUID FromString(std::string str);
            static UUID New();
            static UUID Zero();

            static constexpr size_t GetNumBytes() { return k_NumBytes; }

            inline bool operator<(const UUID& other) const {
                return std::lexicographical_compare(m_Data.begin(), m_Data.end(), other.m_Data.begin(), other.m_Data.end());
            }

            inline bool operator>(const UUID& other) const {
                return other < *this;
            }

            inline bool operator==(const UUID& other) const {
                return std::equal(m_Data.begin(), m_Data.end(), other.m_Data.begin());
            }

            inline bool operator!=(const UUID& other) const {
                return !(*this == other);
            }

        private:
            static uint32_t s_Counter;

            static constexpr size_t k_NumBytes = 16;
            std::array<uint8_t, k_NumBytes> m_Data;
    };
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