#include "BinanceHandler.h"
#include <iostream>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

std::string HTTPRequest::performBinanceAPIRequest(const std::string &host, const std::string &port, const std::string &target, int version)
{
	try
	{
		// The io_context is required for all I/O
		net::io_context ioc;

		// SSL context
		ssl::context ctx(ssl::context::sslv23_client);

		// These objects perform our I/O
		tcp::resolver resolver(ioc);
		ssl::stream<tcp::socket> stream(ioc, ctx);

		// Look up the domain name
		auto const results = resolver.resolve(host, port);

		// Make the connection on the IP address we get from a lookup
		net::connect(stream.next_layer(), results.begin(), results.end());

		// Perform the SSL handshake
		stream.handshake(ssl::stream_base::client);

		// Set up an HTTP GET request message
		http::request<http::string_body> req{http::verb::get, target, version};
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		// Send the HTTP request to the remote host
		http::write(stream, req);

		// This buffer is used for reading and must be persisted
		beast::flat_buffer buffer;

		// Declare a container to hold the response
		http::response<http::dynamic_body> res;

		// Receive the HTTP response
		http::read(stream, buffer, res);

		// Write the message to standard out
		std::cout << res << std::endl;

		// Gracefully close the socket
		beast::error_code ec;
		stream.shutdown(ec);

		// not_connected happens sometimes
		// so don't bother reporting it.
		//
		if (ec && ec != beast::errc::not_connected)
			throw beast::system_error{ec};

		// If we get here then the connection is closed gracefully

		// Return the response as a string
		return boost::beast::buffers_to_string(res.body().data());
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return ""; // Return an empty string in case of an error
	}
}
