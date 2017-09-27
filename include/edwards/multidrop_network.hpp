#ifndef EDWARDS_SERIAL_INTERFACE_HPP
#define EDWARDS_SERIAL_INTERFACE_HPP

#include <common/config.hpp>

#include <cassert>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>

#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

#include <gsl/gsl>

#include <edwards/error.hpp>
#include <edwards/nEXT.hpp>
#include <edwards/units.hpp>
#include <edwards/internal/dialog_primatives.hpp>

namespace edwards {
    struct factory_default_t { };

    static constexpr auto factory_default = factory_default_t{};

    class multidrop_endpoint {
    public:
        constexpr multidrop_endpoint(int i) noexcept
            : _endpoint{ i }
        {
            assert(_endpoint >= 1 && _endpoint <= 99);
        }

        multidrop_endpoint(const multidrop_endpoint &) = default;
        multidrop_endpoint(multidrop_endpoint &&) = default;

        multidrop_endpoint & operator=(const multidrop_endpoint &) = default;
        multidrop_endpoint & operator=(multidrop_endpoint &&) = default;

        constexpr int get() const noexcept {
            return _endpoint;
        }

        constexpr bool is_wildcard() const noexcept {
            return _endpoint == 99;
        }

    private:
        int _endpoint;
    };

    static constexpr auto endpoint_wildcard = multidrop_endpoint{ 99 };

    class multidrop_network {
    public:
        multidrop_network(boost::asio::io_service & service, std::string_view rs485_port);

        auto get_io_service() noexcept -> boost::asio::io_service &;
        
        // 851
        auto pump_info(multidrop_endpoint pump) -> boost::future<edwards::pump_info>;
        auto pump_info(multidrop_endpoint pump, error_code & ec) -> boost::future<edwards::pump_info>;

        // 852
        auto start_pump(multidrop_endpoint pump) -> boost::future<void>;
        auto start_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void>;

        auto stop_pump(multidrop_endpoint pump) -> boost::future<void>;
        auto stop_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void>;
        
        auto pump_current_speed(multidrop_endpoint pump) -> boost::future<hertz_t>;
        auto pump_current_speed(multidrop_endpoint pump, error_code & ec) -> boost::future<hertz_t>;

        auto pump_status(multidrop_endpoint pump) -> boost::future<nEXT_status>;
        auto pump_status(multidrop_endpoint pump, error_code & ec) -> boost::future<nEXT_status>;

        // 853
        auto pump_vent_mode(multidrop_endpoint pump) -> boost::future<vent_mode>;
        auto pump_vent_mode(multidrop_endpoint pump, error_code & ec) -> boost::future<vent_mode>;

        auto pump_vent_mode(multidrop_endpoint pump, vent_mode new_mode) -> boost::future<void>;
        auto pump_vent_mode(multidrop_endpoint pump, vent_mode new_mode, error_code & ec) -> boost::future<void>;

        auto pump_vent_mode(multidrop_endpoint pump, factory_default_t) -> boost::future<void> {
            return pump_vent_mode(pump, vent_mode::_0);
        }
        auto pump_vent_mode(multidrop_endpoint pump, factory_default_t, error_code & ec) -> boost::future<void> {
            return pump_vent_mode(pump, vent_mode::_0, ec);
        }

        // 854
        auto pump_timer(multidrop_endpoint pump) -> boost::future<std::chrono::minutes>;
        auto pump_timer(multidrop_endpoint pump, error_code & ec) -> boost::future<std::chrono::minutes>;
        auto pump_timer(multidrop_endpoint pump, std::chrono::minutes new_timeout)->boost::future<void>;
        auto pump_timer(multidrop_endpoint pump, std::chrono::minutes new_timeout, error_code & ec) -> boost::future<void>;
        auto pump_timer(multidrop_endpoint pump, factory_default_t) -> boost::future<void> {
            return pump_timer(pump, 8min);
        }
        auto pump_timer(multidrop_endpoint pump, factory_default_t, error_code & ec) -> boost::future<void> {
            return pump_timer(pump, 8min, ec);
        }
 
        // 855
        auto pump_power_limit(multidrop_endpoint pump) -> boost::future<watt_t>;
        auto pump_power_limit(multidrop_endpoint pump, error_code & ec) -> boost::future<watt_t>;

        auto pump_power_limit(multidrop_endpoint pump, watt_t new_limit) -> boost::future<void>;
        auto pump_power_limit(multidrop_endpoint pump, watt_t new_limit, error_code & ec) -> boost::future<void>;

        auto pump_power_limit(multidrop_endpoint pump, factory_default_t) -> boost::future<void> {
            return pump_power_limit(pump, 160_W);
        }
        auto pump_power_limit(multidrop_endpoint pump, factory_default_t, error_code & ec) -> boost::future<void> {
            return pump_power_limit(pump, 160_W, ec);
        }

        // 859
        auto pump_temp(multidrop_endpoint pump) -> boost::future<pump_temperature>;
        auto pump_temp(multidrop_endpoint pump, error_code & ec) -> boost::future<pump_temperature>;

        // 867
        auto factory_reset_pump(multidrop_endpoint pump) -> boost::future<void>;
        auto factory_reset_pump(multidrop_endpoint pump, error_code & ec) -> boost::future<void>;
        
        auto pump_PIC_version(multidrop_endpoint pump) -> boost::future<std::string>;
        auto pump_PIC_version(multidrop_endpoint pump, error_code & ec) -> boost::future<std::string>;

        // 875
        auto close_vent_valve(multidrop_endpoint pump) -> boost::future<void>;
        auto close_vent_valve(multidrop_endpoint pump, error_code & ec) -> boost::future<void>;

        //std::tuple<std::chrono::hours, std::chrono::hours> controller_run_time(multidrop_endpoint pump);
        //std::tuple<std::chrono::hours, std::chrono::hours> pump_run_time(multidrop_endpoint pump);
        //std::tuple<std::chrono::hours, std::chrono::hours> bearing_run_time(multidrop_endpoint pump);

    private:
        template<typename... Args>
        auto send_message(error_code & ec, Args&&... args) -> boost::future<void>;
        template<typename... Args>
        auto send_query(error_code & ec, Args&&... args) -> boost::future<internal::message_buffer>;
        
        boost::asio::serial_port	_port;
    };
}

#endif
