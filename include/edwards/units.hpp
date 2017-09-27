#pragma once

#include <units.h>
#include <chrono>

namespace edwards {
    using namespace units::literals;
    using namespace std::literals::chrono_literals;

    using units::temperature::celsius_t;
    using units::power::watt_t;
    using units::voltage::volt_t;
    using units::current::ampere_t;
    using units::frequency::hertz_t;
} // edwards