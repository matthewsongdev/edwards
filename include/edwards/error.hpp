#ifndef EDWARDS_ERROR_HPP
#define EDWARDS_ERROR_HPP

#include <boost/system/error_code.hpp>

namespace edwards {
    using boost::system::error_code;

    enum class error : int {
        invalid_command_for_object = 1,
        invalid_query = 2,
        missing_parameter = 3,
        out_of_range = 4,
        invalid_command_ = 5,
        checksum_ = 6,
        io_error = 7,
        timed_out = 8,
        invalid_config_id = 9
    };

    constexpr bool is_internal_logic_error(error code) noexcept {
        switch (code) {
            case error::missing_parameter:   [[fallthrough]];
            case error::checksum_:           [[fallthrough]];
            case error::invalid_config_id:
                return true;

            default:
                return false;
        }
    }

    const boost::system::error_category & edwards_category() noexcept;

    error_code make_error_code(error) noexcept;
}

template<>
struct boost::system::is_error_code_enum<edwards::error>
    : public std::true_type
{ };

#endif
