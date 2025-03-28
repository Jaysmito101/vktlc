#include "core/Types.hpp"

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define __get16bits(d) (*((const U16 *) (d)))
#endif

#if !defined (CGL_get16bits)
#define _get16bits(d) ((((U32)(((const U8 *)(d))[1])) << 8)\
                       +(U32)(((const U8 *)(d))[0]) )
#endif

namespace tlc
{

    struct Hash
    {
        U32 hash;

        static inline Hash Default()
        {
            return Hash{ 0 };
        }
    };

    namespace internal
    {

        inline Hash FastHashData(const U8* data, Size len)
        {
            U32 hash = (U32)len;
            U32 tmp = 0;
            I32 rem = 0;

            if (len <= 0 || data == NULL) {
                return Hash::Default();
            }

            rem = len & 3;
            len >>= 2;

            /* Main loop */
            for (; len > 0; len--)
            {
                hash += __get16bits(data);
                tmp = (__get16bits(data + 2) << 11) ^ hash;
                hash = (hash << 16) ^ tmp;
                data += 2 * sizeof(U16);
                hash += hash >> 11;
            }

            /* Handle end cases */
            switch (rem)
            {
            case 3:
                hash += __get16bits(data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof(U16)]) << 18;
                hash += hash >> 11;
                break;
            case 2:
                hash += __get16bits(data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
            case 1:
                hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
            }

            /* Force "avalanching" of final 127 bits */
            hash ^= hash << 3;
            hash += hash >> 5;
            hash ^= hash << 4;
            hash += hash >> 17;
            hash ^= hash << 25;
            hash += hash >> 6;

            return Hash { hash };
        }

    }

    template <typename T>
    requires (!std::is_pointer_v<T> && !std::is_reference_v<T>)
    inline Hash HashData(const T& data)
    {
        return internal::FastHashData(reinterpret_cast<const U8*>(&data), sizeof(T));
    }

    template <typename T>
    requires std::is_pointer_v<T>
    inline Hash HashData(const T& data, const Size& size)
    {
        return internal::FastHashData(reinterpret_cast<const U8*>(data), size);
    }
}

namespace std
{
    template <>
    struct hash<tlc::Hash>
    {
        size_t operator()(const tlc::Hash &hash) const
        {
            return hash.hash;
        }
    };
}