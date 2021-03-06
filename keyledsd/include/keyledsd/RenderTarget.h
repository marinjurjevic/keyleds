/* Keyleds -- Gaming keyboard tool
 * Copyright (C) 2017 Julien Hartmann, juli1.hartmann@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KEYLEDS_RENDER_TARGET_H_7E2781C6
#define KEYLEDS_RENDER_TARGET_H_7E2781C6

#include "keyledsd/tools/accelerated.h"
#include "keyledsd/colors.h"
#include <cassert>
#include <chrono>
#include <memory>
#include <utility>

namespace keyleds {

/****************************************************************************/

/** Rendering buffer for key colors
 *
 * Holds RGBA color entries for all keys of a device. All key blocks are in the
 * same memory area. Each block is contiguous, but padding keys may be inserted
 * in between blocks so blocks are SSE2-aligned. The buffers is addressed through
 * a 2-tuple containing the block index and key index within block. No ordering
 * is enforce on blocks or keys, but the for_device static method uses the same
 * order that is detected on the device by the keyleds::Device object.
 */
class RenderTarget final
{
public:
    using value_type = RGBAColor;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = value_type *;
    using const_iterator = const value_type *;
public:
                        RenderTarget() = default;
    explicit            RenderTarget(size_type);
                        RenderTarget(RenderTarget && other) noexcept
                         { swap(*this, other); }
    RenderTarget &      operator=(RenderTarget && other) noexcept
                         { if (m_colors) { clear(); } swap(*this, other); return *this; }
                        ~RenderTarget();

    iterator            begin() { return &m_colors[0]; }
    const_iterator      begin() const { return &m_colors[0]; }
    const_iterator      cbegin() const { return &m_colors[0]; }
    iterator            end() { return &m_colors[m_size]; }
    const_iterator      end() const { return &m_colors[m_size]; }
    const_iterator      cend() const { return &m_colors[m_size]; }
    reference           front() { return m_colors[0]; }
    const_reference     front() const { return m_colors[0]; }
    reference           back() { return m_colors[m_size - 1]; }
    const_reference     back() const { return m_colors[m_size - 1]; }
    bool                empty() const noexcept { return !m_colors; }
    size_type           size() const noexcept { return m_size; }
    size_type           capacity() const noexcept { return m_capacity; }
    value_type *        data() { return m_colors; }
    const value_type *  data() const { return m_colors; }
    reference           operator[](size_type idx) { return m_colors[idx]; }
    const_reference     operator[](size_type idx) const { return m_colors[idx]; }

private:
    void                clear() noexcept;
private:
    size_type           m_size = 0;         ///< Number of color entries
    size_type           m_capacity = 0;     ///< Number of allocated color entries
    RGBAColor *         m_colors = nullptr; ///< Color buffer. RGBAColor is a POD type

    friend void swap(RenderTarget &, RenderTarget &) noexcept;
};

void swap(RenderTarget &, RenderTarget &) noexcept;
void blend(RenderTarget &, const RenderTarget &) noexcept;
void multiply(RenderTarget &, const RenderTarget &) noexcept;

/****************************************************************************/

/** Renderer interface
 *
 * The interface an object must expose should it want to draw within a
 * RenderLoop
 */
class Renderer
{
protected:
    using milliseconds = std::chrono::duration<unsigned, std::milli>;
public:
    /// Modifies the target to reflect effect's display once the specified time has elapsed
    virtual void    render(milliseconds, RenderTarget & target) = 0;
protected:
    // Protect the destructor so we can leave it non-virtual
    ~Renderer() {}
};

/****************************************************************************/

inline void swap(RenderTarget & lhs, RenderTarget & rhs) noexcept
{
    using std::swap;
    swap(lhs.m_size, rhs.m_size);
    swap(lhs.m_capacity, rhs.m_capacity);
    swap(lhs.m_colors, rhs.m_colors);
}

inline void blend(RenderTarget & lhs, const RenderTarget & rhs) noexcept
{
    assert(lhs.capacity() == rhs.capacity());
    tools::blend(reinterpret_cast<uint8_t*>(lhs.data()),
                 reinterpret_cast<const uint8_t*>(rhs.data()), rhs.capacity());
}

template <typename A>
inline void blend(RenderTarget & lhs, const RenderTarget & rhs) noexcept
{
    assert(lhs.capacity() == rhs.capacity());
    A::blend(reinterpret_cast<uint8_t*>(lhs.data()),
             reinterpret_cast<const uint8_t*>(rhs.data()), rhs.capacity());
}

inline void multiply(RenderTarget & lhs, const RenderTarget & rhs) noexcept
{
    assert(lhs.capacity() == rhs.capacity());
    tools::multiply(reinterpret_cast<uint8_t*>(lhs.data()),
                    reinterpret_cast<const uint8_t*>(rhs.data()), rhs.capacity());
}

template <typename A>
inline void multiply(RenderTarget & lhs, const RenderTarget & rhs) noexcept
{
    assert(lhs.capacity() == rhs.capacity());
    A::multiply(reinterpret_cast<uint8_t*>(lhs.data()),
                reinterpret_cast<const uint8_t*>(rhs.data()), rhs.capacity());
}

} // keyleds

#endif
