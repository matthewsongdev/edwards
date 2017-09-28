#include <edwards/internal/dialog.hpp>

#include <algorithm>
#include <iterator>

#include <boost/asio.hpp>

namespace edwards::internal {
    dialog::dialog(boost::asio::serial_port & port) noexcept
        : _port{ std::addressof(port) }
        , _timer{ port.get_io_service() }
        , _message{ }
        , _result{ }
        , _resume_handle{ nullptr }
    { }

    auto dialog::get_io_service() noexcept -> boost::asio::io_service & {
        return _port->get_io_service();
    }

    auto dialog::await_ready() noexcept -> bool {
        return false;
    }

    auto dialog::await_suspend(std::experimental::coroutine_handle<> handle) -> void {
        _resume_handle = handle;

        // Start communication
        boost::asio::async_write(
            *_port,
            boost::asio::buffer(_message),
            [this](const error_code & ec, std::size_t written) { on_write_complete(ec, written); });
    }

    auto dialog::await_resume() noexcept -> dialog_result {
        return _result;
    }

    auto dialog::on_write_complete(const error_code & ec, std::size_t) noexcept -> void {
        if (ec) {
            // There was an error while sending the message
            signal_completion(ec);
            return;
        }

        // Read response from device
        start_read();

        // Setup timer which will cancel read operation if it takes too long to complete.
        _timer.expires_from_now(boost::posix_time::milliseconds{ 500 });
        _timer.async_wait([this](const error_code & ec) {
            on_timeout(ec);
        });
    }

    auto dialog::start_read() noexcept -> void {
        boost::asio::async_read(
            *_port,
            boost::asio::buffer(_result.response),
            [this](const error_code & ec, std::size_t read) { return on_read_packet(ec, read); },
            [this](const error_code & ec, std::size_t read) { on_read_complete(ec, read); }
        );
    }

    auto dialog::on_read_packet(const error_code & ec, std::size_t read) noexcept -> std::size_t {
        if (ec || _result.response[read - 1] == '\r') {
            return 0;
        }
        return max_message_size - read;
    }

    auto dialog::on_read_complete(const error_code & ec, std::size_t total_read) noexcept -> void {
        using namespace boost::asio::error;

        if (ec) {
            // An error occured during the read
            signal_completion(ec == operation_aborted ? timed_out : ec);
            return;
        }
        else if (_message[1] != _result.response[1] ||
                 _message[2] != _result.response[2]) {
            // We've read a message from someone else on the network.  Check to see if the response from
            // the client is in the buffer, if not clear the buffer and start the read operation again.
            const auto mesg = view_message(_result);
            if (const auto i = mesg.find_first_of("\r#"); i != std::string_view::npos) {

            }
            else {
                // Could not find a second message in the buffer, start another async read.
                std::fill_n(_result.response.begin(), total_read, '\0');
                start_read();
            }
        }
        else {
            // Read completed successfully, no longer need the timer running.
            _timer.cancel();
            signal_completion(error_code{ });
        }
    }

    auto dialog::on_timeout(const error_code & ec) noexcept -> void {
        if (!ec) {
            _port->cancel();
        }
    }

    auto dialog::signal_completion(const error_code & ec) -> void {
        _result.ec = ec;
        get_io_service().post(_resume_handle);
    }
} // namespace edwards::internal