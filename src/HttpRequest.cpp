#include "BinanceHandler.h"
#include <iostream>
#include <fstream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <cstdlib>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

HTTPRequest::HTTPRequest()
{
	// Ensure the global logger is initialized
	if (!logger)
	{
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
		logger = spdlog::basic_logger_mt("console", "logs/binary_exchange_logs.log");
		spdlog::flush_on(spdlog::level::info);
	}
}
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

		// Gracefully close the socket
		beast::error_code ec;
		stream.shutdown(ec);

		// Converting the response body as a string
		auto response = boost::beast::buffers_to_string(res.body().data());

		// Log success
		logger->info("Successfully performed Binance API request to {}:{}{}", host, port, target);

		// Create an object of JSONParser
		JSONParser jsonParser;

		return response;
	}
	catch (std::exception const &e)
	{
		// Log an error
		logger->error("Error: {}", e.what());
		return ""; // Return an empty string in case of an error
	}
}
