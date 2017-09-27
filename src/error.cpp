#include "error.hpp"

#include <exception>
#include <string>

namespace edwards {
    using namespace boost::system;

    namespace {
        class category_impl
            : public error_category
        {
        public:
            virtual const char * name() const noexcept final {
                return "Edwards";
            }

            virtual std::string message(int ev) const final {
                switch (static_cast<error>(ev)) {
                    case error::invalid_command_for_object:
                        return "Invalid command for object";

                    case error::invalid_query:
                        return "Invalid query or command";

                    case error::missing_parameter:
                        return "Missing parameter";

                    case error::out_of_range:
                        return "Parameter out of range";

                    case error::invalid_command_:
                        return "Invalid command in current state";

                    case error::checksum_:
                        return "";

                    case error::io_error:
                        return "EEPROM read or write error";

                    case error::timed_out:
                        return "Operation took too long";

                    case error::invalid_config_id:
                        return "Invalid config ID";

                    default:
                        std::terminate();
                }
            }

            virtual bool equivalent(const error_code & code, int ev) final {
                switch (static_cast<error>(ev)) {
                    case error::invalid_command_for_object:
                        return code == errc::function_not_supported ||
                               code == errc::not_supported;

                    case error::invalid_query:
                        return false;

                    case error::missing_parameter:
                        return false;

                    case error::out_of_range:
                        return code == errc::argument_out_of_domain ||
                               code == errc::invalid_argument;

                    case error::invalid_command_:
                        return false;

                    case error::checksum_:
                        return code == errc::protocol_error;

                    case error::io_error:
                        return code == errc::io_error;

                    case error::timed_out:
                        return code == errc::timed_out;

                    case error::invalid_config_id:
                        return false;

                    default:
                        std::terminate();
                }
            }
        };
    }

    const error_category & edwards_category() noexcept {
        static const category_impl category{};
        return category;
    }

    error_code make_error_code(error e) noexcept {
        return { static_cast<int>(e), edwards_category() };
    }
}