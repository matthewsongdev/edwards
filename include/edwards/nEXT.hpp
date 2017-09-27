#pragma once

#include <cstdint>
#include <string>
#include <type_traits>

#include <edwards/units.hpp>

namespace edwards {
    struct pump_info {
        std::string type;
        std::string DSP_version;
        hertz_t     max_speed;
    };

    enum class nEXT_status
        : std::uint32_t
    {
        // Fail status condition active
        fail                = 1 << 0,
        // Below stopped speed
        stopped_speed       = 1 << 1,
        // Above normal speed
        normal_speed        = 1 << 2,
        // Vent valve energised
        vent_valve          = 1 << 3,
        // Start command active
        start               = 1 << 4,
        // Serial enable active
        serial_enabled      = 1 << 5,
        // Standby active
        standby             = 1 << 6,
        // Above 50% full rotational speed
        half_speed          = 1 << 7,
        // Exclusive parallel control mode active
        parallel_control    = 1 << 8,
        // Exclusive serial control mode active
        serial_control      = 1 << 9,
        // Controller internal software mismatch
        invalid_software    = 1 << 10,
        // Controller failed internal configuration and calibration operation
        upload_incomplete   = 1 << 11,
        // Failure to reach or maintain half full speed within the timer setting value
        timer_expired       = 1 << 12,
        // Overspeed or overcurrent trip activated
        hardware_trip       = 1 << 13,
        // Pump internal temperature measurement system disconnected or damaged
        thermistor_error    = 1 << 14,
        // Serial enable is inactive following a serial 'Start' command
        serial_interlock    = 1 << 15
    };

    constexpr bool has_flag(nEXT_status status, nEXT_status flag) noexcept {
        using U = std::underlying_type_t<nEXT_status>;
        return (static_cast<U>(status) & static_cast<U>(flag)) == static_cast<U>(flag);
    }

    enum class vent_mode {
        // Hard vent when below 50% full speed for either stop command or fail condition. (Factory default).
        _0,
        // Controlled vent between 100-50% full speed, hard vent below 50% for either stop command or fail condition.
        _1,
        // Hard vent immediately on stop command or when fail condition and rotor below 50% full speed.
        _2,
        // Hard vent immediately on stop command. On fail condition controlled venting between 100-50% full speed, hard vent below 50%.
        _3,
        // Hard vent immediately on fail condition, or when rotor below 50% full speed on stop command.
        _4,
        // Hard vent immediately on fail condition. On stop command controlled venting between 100-50% full speed, hard vent below 50%.
        _5,
        // Hard vent immediately on either stop command or fail condition.
        _6,
        // Same as Option 6.
        _7
    };

    enum class service_status
        : std::uint32_t
    {
        // Oil cartridge 
        oil_due         = 1 << 0,
        bearing_due     = 1 << 1,
        pump_due        = 1 << 2,
        controller_due  = 1 << 3
    };

    struct pump_temperature {
        celsius_t motor;
        celsius_t controller;
    };

    enum class pump_speed {
        full,
        standby
    };
}