#pragma once

#include <array>
#include <experimental/coroutine>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/serial_port.hpp>
#include <gsl/gsl>
#include <fmt/format.h>

#include <edwards/error.hpp>
#include <edwards/internal/dialog_primatives.hpp>

namespace edwards::internal {
    struct dialog_result {
        message_buffer response;
        error_code     ec;
    };

    constexpr auto view_message(const dialog_result & result) noexcept {
        return view_message(result.response);
    }

    constexpr auto view_data(const dialog_result & result) noexcept {
        return view_data(result.response);
    }
    
    class dialog {
    public:
        dialog(boost::asio::serial_port & port) noexcept;
        template<typename... Args>
        dialog(boost::asio::serial_port & port, Args&&... args)
            : dialog{ port }
        {
            format_message(std::forward<Args>(args)...);
        }

        /// Formats the message to send at the beginning of the dialog.  Arguments should following
        /// fmt's write api
        template<typename... Args>
        auto format_message(Args&&... args) -> void {
            fmt::ArrayWriter mesg{ _message.data(), _message.size() };
            mesg.write(std::forward<Args>(args)...);
        }

        // Generic asio interface

        auto get_io_service() noexcept -> boost::asio::io_service &;

        // Coroutines interface

        auto await_ready() noexcept -> bool;
        auto await_suspend(std::experimental::coroutine_handle<> handle) -> void;
        auto await_resume() noexcept -> dialog_result;

    private:
        /// Executed when the asynchronous write operation is complete.  Will queue the
        /// following asynchronous read to get the response from the network device or
        /// resume continuation on error.
        auto on_write_complete(const error_code & ec, std::size_t written) noexcept -> void;

        auto start_read() noexcept -> void;

        /// Called during the async read to determine if the entire response has been received.
        /// If the response is complete function returns 0, otherwise it will return the maximum
        /// number of bytes that can still be read into the buffer. 
        auto on_read_packet(const error_code & ec, std::size_t read) noexcept -> std::size_t;

        auto on_read_complete(const error_code & ec, std::size_t read) noexcept -> void;

        auto on_timeout(const error_code & ec) noexcept -> void;

        auto complete(const error_code & code) -> void;

        gsl::not_null<boost::asio::serial_port*> _port;
        boost::asio::deadline_timer              _timer;
        message_buffer                           _message;
        dialog_result                            _result;
        std::experimental::coroutine_handle<>    _resume_handle;
    };
}