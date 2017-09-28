//          Copyright Thomas A Myles 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EDWARDS_CONFIG_HPP
#define EDWARDS_CONFIG_HPP

#if defined(EDWARDS_TEST)
// Mock version of asio used for unit testing
#elif defined(EDWARDS_USE_BOOST_ASIO)
#   include <boost/asio.hpp>
#   include <boost/system/system_error.hpp>
#   define EDWARDS_ASIO_NS ::boost::asio
#   define EDWARDS_ERROR_NS ::boost::system
#else
#   include <asio.hpp>
#   include <system_error>
#   define EDWARDS_ASIO_NS ::asio
#   define EDWARDS_ERROR_NS ::std
#endif

#endif //EDWARDS_CONFIG_HPP
