//          Copyright Thomas A Myles 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EDWARDS_INTERNAL_DIALOG_PRIMATIVES_HPP
#define EDWARDS_INTERNAL_DIALOG_PRIMATIVES_HPP

#include <array>
#include <cstdint>
#include <string_view>

namespace edwards::internal {
    // The maximum length in bytes that a message (either send to received) can be.
    static constexpr std::size_t max_message_size = 80;

    using message_buffer = std::array<char, max_message_size>;

    constexpr auto view_message(const message_buffer & message) noexcept -> std::string_view {
        const auto view = std::string_view{ message.data(), message.size() };
        if (const auto end = view.find_last_of('\r'); end != view.npos) {
            return view.substr(0, end + 1);
        }
        // No carriage return in buffer, therefore there is no (complete) message
        // stored.  Return an empty view.
        return {};
    }

    constexpr auto view_data(const message_buffer & buffer) noexcept -> std::string_view {
        constexpr auto data_start = std::size_t{ 12 };

        const auto message = view_message(buffer);
        if (message.empty()) {
            return {};
        }
        return message.substr(data_start, message.size() - data_start - 1);
    }
} // namespace edwards::internal

#endif // EDWARDS_INTERNAL_DIALOG_PRIMATIVES_HPP