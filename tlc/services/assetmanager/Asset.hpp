#pragma once
#include "core/Core.hpp"

namespace tlc 
{
    enum class AssetTags : U32
    {
        None                 = 0b00000000000000000000000000000000,
        Shader               = 0b00000000000000000000000000000001,
        Image                = 0b00000000000000000000000000000010,
        Audio                = 0b00000000000000000000000000000100,
        Font                 = 0b00000000000000000000000000001000,
    };

    inline AssetTags operator|(AssetTags a, AssetTags b) {
        return static_cast<AssetTags>(static_cast<U32>(a) | static_cast<U32>(b));
    }

    inline AssetTags operator&(AssetTags a, AssetTags b) {
        return static_cast<AssetTags>(static_cast<U32>(a) & static_cast<U32>(b));
    }

    inline AssetTags operator~(AssetTags a) {
        return static_cast<AssetTags>(~static_cast<U32>(a));
    }

    inline String AssetTagsToStringSingle(AssetTags tags) {
        switch (tags) {
            case AssetTags::None: return "None";
            case AssetTags::Image: return "Image";
            case AssetTags::Audio: return "Audio";
            case AssetTags::Font: return "Font";
            case AssetTags::Shader: return "Shader";
            default: return "Unknown";
        }
    }

    inline String AssetTagsToString(AssetTags tags) {
        String result = "";
        if (tags == AssetTags::None) {
            return "None";
        }

        // loop through all the bits in the tags
        for (U32 i = 0; i < 32; i++) {
            if ((static_cast<U32>(tags) & (1 << i)) != 0) {
                if (!result.empty()) {
                    result += ", ";
                }
                result += AssetTagsToStringSingle(static_cast<AssetTags>(1 << i));
            }
        }

        return result;
    }

    struct Asset {
        String Path = "";
        String Address = "";
        UUID UUID = UUID::Zero();
        Raw<U8> Data = nullptr;
        Size Offset = 0;
        Size Size = 0;
        AssetTags Tags = AssetTags::None;
        U32 Hash = 0;
    };
}

namespace std {
    template <>
    struct formatter<tlc::AssetTags> : formatter<string> {
        template<typename ParseContext>
        auto parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const tlc::AssetTags& tags, FormatContext& ctx) const {
            return format_to(ctx.out(), "{}", tlc::AssetTagsToString(tags));
        }
    };

}