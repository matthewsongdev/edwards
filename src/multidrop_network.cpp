#include <edwards/multidrop_network.hpp>
#include <edwards/internal/dialog.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>
#include <future>
#include <condition_variable>
#include <optional>
#include <variant>

#include <boost/asio/read_until.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/format.h>

#include <common/coroutines.hpp>

using namespace std::chrono_literals;

namespace edwards {
    namespace {
        error_code check_response(std::string_view response) {
            assert(response.length() >= 12);

            if (response[6] != '*') {
                const auto l = response.length();

                // Parse one or two digit code into an int
                const auto code = std::atoi(&response[l - 3]);

                // If not the OK code, create an error code
                if (code != 0) {
                    return { code, edwards_category() };
                }
            }

            return { };
        }
    }

    multidrop_network::multidrop_network(boost::asio::io_service & service,
                                         std::string_view rs485_port)
        : _port{ service, rs485_port.data() }
    {
        _port.set_option(boost::asio::serial_port::baud_rate{ 9600 });
    }

    template<typename... Args>
    auto multidrop_network::send_message(error_code & ec, Args&&... args) -> boost::future<void> {
        const auto result = co_await internal::dialog{ _port, std::forward<Args>(args)... };
        if (result.ec) {
            // Error during communication
            ec = result.ec;
        }
        else {
            // No communication error, get error code in response.
            ec = check_response(internal::view_message(result));
        }
    }

    template<typename... Args>
    auto multidrop_network::send_query(error_code & ec, Args&&... args) -> boost::future<internal::message_buffer> {
        const auto result = co_await internal::dialog{ _port, std::forward<Args>(args)... };
        if (result.ec) {
            ec = result.ec;
        }
        else {
            ec = check_response(internal::view_message(result));
        }
        co_return result.response;
    }

    //boost::future<pump_info> multidrop_network::pump_info(multidrop_endpoint pump, error_code & ec) {
    //    //const auto response = send_message(fmt::format("#{:02d}:00?S851\r", pump.get()));
    //    //check_response(response);
    //    //
    //    //// Successful message format:
    //    ////   #00:ii?S851 xxxxxxx;yyyyyyyyyy;zzzz\r
    //    ////            '         '         '
    //    ////   i - endpoint of the pump we're communicating with
    //    ////   x - pump type
    //    ////   y - DSP software version
    //    ////   z - full speed rpm
    //    //assert(response.length() == 36);


    //    //return { std::string{ response.begin() + 13, response.begin() + 20 },
    //    //         std::string{ response.begin() + 21, response.begin() + 31 },
    //    //         std::atoi(response.data() + 32) };
    //}

    //auto multidrop_network::pump_power_limit(multidrop_endpoint pump, error_code & ec) -> boost::future<watt_t> {
    //    dialog dia{ *this, fmt::format("#{:02d}:00?S855\r", pump.get()) };
    //    if (ec = co_await dia.run()) {
    //        // Error in serial communication
    //        co_return watt_t{};
    //    }
    //    if (ec = dia.response_code()) {
    //        // Error returned from pump
    //        co_return watt_t{};
    //    }

    //    co_return watt_t{ std::strtoll(dia.response_data().data(), nullptr, 10) };
    //}

    auto multidrop_network::get_io_service() noexcept -> boost::asio::io_service & {
        return _port.get_io_service();
    }

    auto multidrop_network::pump_info(multidrop_endpoint pump, error_code & ec) -> boost::future<::edwards::pump_info> {
        const auto response = co_await send_query(ec, "#{:02d}:00!S851 1\r", pump.get());
        auto info = edwards::pump_info{};
        if (!ec) {
            const auto data = internal::view_data(response);
            if (data[8] != ';' || data[18] != ';') {
                ec.assign(boost::system::errc::protocol_error,
                          boost::system::generic_category());
            }
            else {
                info.type.assign(data.substr(0, 7));
                info.DSP_version.assign(data.substr(8, 10));
                info.max_speed = static_cast<hertz_t>(std::strtoul(&data[19], nullptr, 10));
            }
        }
        co_return info;
    }

    auto multidrop_network::pump_info(multidrop_endpoint pump) -> boost::future<::edwards::pump_info> {
        auto ec = error_code{};
        const auto info = co_await pump_info(pump, ec);
        if (ec) {
            throw boost::system::system_error{ ec };
        }
        co_return info;
    }

    auto multidrop_network::start_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void> {
        return send_message(ec, "#{:02d}:00!C852 1\r", pump.get());
    }

    auto multidrop_network::start_pump(multidrop_endpoint pump) -> boost::future<void> {
        auto ec = error_code{};
        co_await start_pump(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::stop_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void> {
        return send_message(ec, "#{:02d}:00!C852 0\r", pump.get());
    }

    auto multidrop_network::stop_pump(multidrop_endpoint pump) -> boost::future<void> {
        auto ec = error_code{};
        co_await stop_pump(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::pump_current_speed(multidrop_endpoint pump, error_code & ec) -> boost::future<hertz_t> {
        const auto response = co_await send_query(ec, "#{:02d}:00?V852\r", pump.get());
        if (!ec) {
            const auto resp_data = internal::view_data(response);
            co_return static_cast<hertz_t>(std::strtoul(resp_data.data(), nullptr, 10));
        }
        co_return hertz_t{ std::numeric_limits<double>::quiet_NaN() };
    }

    auto multidrop_network::pump_current_speed(multidrop_endpoint pump) -> boost::future<hertz_t> {
        auto ec = error_code{};
        const auto speed = co_await pump_current_speed(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
        co_return speed;
    }

    auto multidrop_network::pump_status(multidrop_endpoint pump, error_code & ec) -> boost::future<nEXT_status> {
        const auto response = co_await send_query(ec, "#{:02d}:00?V852\r", pump.get());
        if (!ec) {
            const auto resp_data = internal::view_data(response);
            const auto status_start = resp_data.find_first_of(';');
            if (status_start != std::string_view::npos) {
                co_return static_cast<nEXT_status>(std::strtoul(&resp_data[status_start + 1], nullptr, 16));
            }
            else {
                ec.assign(boost::system::errc::protocol_error,
                          boost::system::generic_category());
            }
        }
        co_return nEXT_status{ 0 };
    }

    auto multidrop_network::pump_status(multidrop_endpoint pump) -> boost::future<nEXT_status> {
        auto ec = error_code{};
        const auto status = co_await pump_status(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
        co_return status;
    }

    auto multidrop_network::pump_vent_mode(multidrop_endpoint pump, error_code & ec) -> boost::future<vent_mode> {
        const auto response = co_await send_query(ec, "#{:02d}:00?S853\r", pump.get());
        if (!ec) {
            const auto data = internal::view_data(response);
            if (data.size() != 1 || !std::isdigit(data[0])) {
                ec.assign(boost::system::errc::protocol_error,
                          boost::system::generic_category());
            }
            co_return static_cast<vent_mode>(data[0] - '0');
        }
        co_return vent_mode::_0;
    }

    auto multidrop_network::pump_vent_mode(multidrop_endpoint pump) -> boost::future<vent_mode> {
        auto ec = error_code{};
        const auto mode = co_await pump_vent_mode(pump, ec);
        if (ec) {
            throw boost::system::system_error{ ec };
        }
        co_return mode;
    }

    auto multidrop_network::pump_vent_mode(multidrop_endpoint pump, vent_mode new_mode, error_code & ec) -> boost::future<void> {
        return send_message(ec, "#{:02d}:00!S853 {}\r", pump.get(), static_cast<int>(new_mode));
    }

    auto multidrop_network::pump_vent_mode(multidrop_endpoint pump, vent_mode new_mode) -> boost::future<void> {
        auto ec = error_code{};
        co_await pump_vent_mode(pump, new_mode, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::pump_timer(multidrop_endpoint pump, std::chrono::minutes new_timeout, error_code & ec) -> boost::future<void> {
        assert(new_timeout >= 1min && new_timeout <= 30min);

        return send_message(ec, "#{:02d}:00!S854 {}\r", pump.get(), new_timeout.count());
    }

    auto multidrop_network::pump_timer(multidrop_endpoint pump, std::chrono::minutes new_timeout) -> boost::future<void> {
        auto ec = error_code{};
        co_await pump_timer(pump, new_timeout, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::pump_power_limit(multidrop_endpoint pump, watt_t new_limit, error_code & ec) -> boost::future<void> {
        assert(new_limit >= 50_W && new_limit <= 200_W);

        return send_message(ec, "#{:02d}:00!S855 {}\r", pump.get(), units::unit_cast<int>(new_limit));
    }

    auto multidrop_network::pump_power_limit(multidrop_endpoint pump, watt_t new_limit) -> boost::future<void> {
        auto ec = error_code{};
        co_await pump_power_limit(pump, new_limit, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::factory_reset_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void> {
        return send_message(ec, "#{:02d}:00!S867 1\r", pump.get());
    }

    auto multidrop_network::factory_reset_pump(multidrop_endpoint pump) -> boost::future<void> {
        auto ec = error_code{};
        co_await factory_reset_pump(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }

    auto multidrop_network::close_vent_valve(multidrop_endpoint pump, error_code & ec) -> boost::future<void> {
        return send_message(ec, "#{:02d}:00!C875 1\r", pump.get());
    }

    auto multidrop_network::close_vent_valve(multidrop_endpoint pump) -> boost::future<void> {
        auto ec = error_code{};
        co_await close_vent_valve(pump, ec);
        if (ec) {
            throw boost::system::error_code{ ec };
        }
    }
}