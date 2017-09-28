#include <edwards/error.hpp>

#include <exception>
#include <string>

namespace edwards {
    namespace {
        class category_impl
            : public EDWARDS_ERROR_NS::error_category
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

            virtual bool equivalent(const EDWARDS_ERROR_NS::error_code & code, int ev) final {
                switch (static_cast<error>(ev)) {
                    case error::invalid_command_for_object:
                        return code == EDWARDS_ERROR_NS::errc::function_not_supported ||
                               code == EDWARDS_ERROR_NS::errc::not_supported;

                    case error::invalid_query:
                        return false;

                    case error::missing_parameter:
                        return false;

                    case error::out_of_range:
                        return code == EDWARDS_ERROR_NS::errc::argument_out_of_domain ||
                               code == EDWARDS_ERROR_NS::errc::invalid_argument;

                    case error::invalid_command_:
                        return false;

                    case error::checksum_:
                        return code == EDWARDS_ERROR_NS::errc::protocol_error;

                    case error::io_error:
                        return code == EDWARDS_ERROR_NS::errc::io_error;

                    case error::timed_out:
                        return code == EDWARDS_ERROR_NS::errc::timed_out;

                    case error::invalid_config_id:
                        return false;

                    default:
                        std::terminate();
                }
            }
        };
    }

    const EDWARDS_ERROR_NS::error_category & edwards_category() noexcept {
        static const category_impl category{};
        return category;
    }

    EDWARDS_ERROR_NS::error_code make_error_code(error e) noexcept {
        return { static_cast<int>(e), edwards_category() };
    }
} // namespace edwards