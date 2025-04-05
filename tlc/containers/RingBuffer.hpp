#pragma once

#include "core/Core.hpp"

namespace tlc {

    template<typename T, Size N>
    class RingBuffer
    {
    public:
        RingBuffer() = default;
        ~RingBuffer() = default;

        inline void Push(const T& item)
        {
            m_Buffer[m_Tip] = item;
            m_Tip = (m_Tip + 1) % N;
            if (m_Tip == m_Start) {
                m_Start = (m_Start + 1) % N; // Overwrite the oldest item
            }
        }

        inline T Pop()
        {
            if (m_Start == m_Tip) {
                throw std::out_of_range("Buffer is empty");
            }
            T item = m_Buffer[m_Start];
            m_Start = (m_Start + 1) % N;
            return item;
        }

        inline const T& Peek() const
        {
            if (m_Start == m_Tip) {
                throw std::out_of_range("Buffer is empty");
            }
            return m_Buffer[m_Start];
        }

        inline T& Peek()
        {
            if (m_Start == m_Tip) {
                throw std::out_of_range("Buffer is empty");
            }
            return m_Buffer[m_Start];
        }

        inline const T& Top() const
        {
            if (m_Start == m_Tip) {
                throw std::out_of_range("Buffer is empty");
            }
            return m_Buffer[(m_Tip - 1 + N) % N];
        }

        inline const T& Bottom() const
        {
            if (m_Start == m_Tip) {
                throw std::out_of_range("Buffer is empty");
            }
            return m_Buffer[m_Start];
        }

        inline const T& Get(Size index) const
        {
            if (index >= Length()) {
                throw std::out_of_range("Index out of range");
            }
            return m_Buffer[(m_Start + index) % N];
        }

        inline T& Get(Size index)
        {
            if (index >= Length()) {
                throw std::out_of_range("Index out of range");
            }
            return m_Buffer[(m_Start + index) % N];
        }

        inline Size Length() const
        {
            if (m_Tip >= m_Start) {
                return m_Tip - m_Start;
            }
            return N - m_Start + m_Tip;
        }
        
        inline Size Capacity() const
        {
            return N;
        }

        inline Bool IsEmpty() const
        {
            return m_Start == m_Tip;
        }   

        inline Bool IsFull() const
        {
            return (m_Tip + 1) % N == m_Start;
        }

        inline void Clear()
        {
            m_Start = 0;
            m_Tip = 0;
        }

        inline void CopyToArray(Raw<T> array, Raw<Size> size) const
        {
            if (size && *size < N) {
                throw std::out_of_range("Array size is too small");
            }

            if (m_Start < m_Tip) {
                std::copy(m_Buffer.begin() + m_Start, m_Buffer.begin() + m_Tip, array);
            }
            else {
                std::copy(m_Buffer.begin() + m_Start, m_Buffer.end(), array);
                std::copy(m_Buffer.begin(), m_Buffer.begin() + m_Tip, array + (N - m_Start));
            }
            
            if (size) {
                *size = Length();
            }
        }

        inline void Fill(const T& value, Size count)
        {
            for (Size i = 0; i < count; ++i) {
                Push(value);
            }
        }

    private:
        std::array<T, N> m_Buffer;
        Size m_Start = 0;
        Size m_Tip = 0;
    };
}