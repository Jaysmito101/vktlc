#include "core/Uuid.hpp"

#include <random>
#include <chrono>

namespace tlc {
    uint32_t UUID::s_Counter = 0;

    UUID::UUID(void* data) {
        if (data) std::memcpy(m_Data.data(), static_cast<uint8_t*>(data), k_NumBytes);
		else std::memset(m_Data.data(), 0, k_NumBytes);
    }

    std::string UUID::ToString() const {
        std::string str;
        str.reserve(k_NumBytes * 2);
        for (uint8_t byte : m_Data) {
            str.push_back("0123456789abcdef"[byte >> 4]);
            str.push_back("0123456789abcdef"[byte & 0xf]);
            if (str.size() == 8 || str.size() == 13 || str.size() == 18 || str.size() == 23) {
                str.push_back('-');
            }
        }
        return str;
    }

    UUID UUID::FromBytes(const void* data) {
        return UUID(const_cast<void*>(data));
    }

    bool UUID::IsValid(std::string str)
    {
        if (str.size() != 36) {
            return false;
        }
        for (char c : str) {
            if (c == '-') {
                continue;
            }
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                return false;
            }
        }
        return true;
    }

    UUID UUID::FromString(std::string str) {
        UUID uuid = Zero();
        if (!IsValid(str)) {
            return uuid;
        }

        size_t i = 0;
        for (char c : str) {
            if (c == '-') {
                continue;
            }
            uint8_t byte = 0;
            if (c >= '0' && c <= '9') {
                byte = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                byte = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                byte = c - 'A' + 10;
            }
            if (i % 2 == 0) {
                byte <<= 4;
            }
            uuid.m_Data[i / 2] |= byte;
            i++;
        }
        return uuid;
    }

    UUID UUID::Zero() {
		return UUID(nullptr);
    }

    UUID UUID::New() {
        UUID uuid = Zero();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int32_t> dis(0, 255);
        for (size_t i = 0; i < k_NumBytes; i++) {
            uuid.m_Data[i] = static_cast<uint8_t>(dis(gen));
        }
        uuid.m_Data[4] = (s_Counter >> 8) & 0xff;
        uuid.m_Data[5] = s_Counter & 0xff;
        s_Counter++;

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto milis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        uuid.m_Data[10] = (milis >> 24) & 0xff;
        uuid.m_Data[11] = (milis >> 16) & 0xff;

        return uuid;
    }
       
}