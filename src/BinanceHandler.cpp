#include "BinanceHandler.h"
#include "rapidjson/document.h"
#include <iostream>
#include <string>
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
	// Initialize the logger
	auto console_logger = spdlog::stdout_color_mt("console");
	auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/binary_exchange_handler_log.txt");
	spdlog::set_default_logger(file_logger);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
	spdlog::flush_on(spdlog::level::info);
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

		// Return the response body as a string
		auto response = boost::beast::buffers_to_string(res.body().data());

		// Log success
		logger->info("Successfully performed Binance API request to {}:{}{}", host, port, target);

		return response;
	}
	catch (std::exception const &e)
	{
		// Log an error
		logger->error("Error: {}", e.what());
		return ""; // Return an empty string in case of an error
	}
}

void HTTPRequest::performJSONDataParsing(const std::string &jsonResponse)
{
	rapidjson::Document document;
	document.Parse(jsonResponse.c_str());

	if (document.HasParseError())
	{
		// Log an error
		logger->error("Error parsing JSON. Parse error code: {}, Offset: {}", document.GetParseError(), document.GetErrorOffset());
		return;
	}

	if (document.HasMember("symbols") && document["symbols"].IsArray())
	{
		const rapidjson::Value &symbolsArray = document["symbols"];

		for (rapidjson::SizeType i = 0; i < symbolsArray.Size(); ++i)
		{
			const rapidjson::Value &symbolObject = symbolsArray[i];

			if (symbolObject.HasMember("symbol") && symbolObject["symbol"].IsString())
			{
				std::string symbol = symbolObject["symbol"].GetString();
				std::string quoteAsset = symbolObject["quoteAsset"].GetString();
				std::string status = symbolObject["status"].GetString();
				std::string tickSize = "";
				std::string stepSize = "";

				if (symbolObject.HasMember("filters") && symbolObject["filters"].IsArray())
				{
					const rapidjson::Value &filtersArray = symbolObject["filters"];

					for (rapidjson::SizeType j = 0; j < filtersArray.Size(); ++j)
					{
						const rapidjson::Value &filterObject = filtersArray[j];

						if (filterObject.HasMember("filterType") && filterObject["filterType"].IsString())
						{
							std::string filterType = filterObject["filterType"].GetString();

							if (filterType == "PRICE_FILTER" && filterObject.HasMember("tickSize") &&
								filterObject["tickSize"].IsString())
							{
								tickSize = filterObject["tickSize"].GetString();
							}
							else if (filterType == "LOT_SIZE" && filterObject.HasMember("stepSize") &&
									 filterObject["stepSize"].IsString())
							{
								stepSize = filterObject["stepSize"].GetString();
							}
						}
					}
				}

				// Storing in unordered
				symbolInfoMap[symbol]["status"] = status;
				symbolInfoMap[symbol]["quoteAsset"] = quoteAsset;
				symbolInfoMap[symbol]["tickSize"] = tickSize;
				symbolInfoMap[symbol]["stepSize"] = stepSize;
			}
			else
			{
				// Log an error
				logger->error("Missing or invalid 'symbol' in JSON response.");
			}
		}
	}
	else
	{
		// Log an error
		logger->error("Missing or invalid 'symbols' array in JSON response.");
	}

	// Log the entire unordered map for verification
	logger->info("Symbol Info Map:");
	for (const auto &symbolPair : symbolInfoMap)
	{
		const std::string &symbol = symbolPair.first;
		const auto &infoMap = symbolPair.second;

		logger->info("Symbol: {}", symbol);
		for (const auto &infoPair : infoMap)
		{
			const std::string &key = infoPair.first;
			const std::string &value = infoPair.second;
			logger->info("  Key: {}, Value: {}", key, value);
		}
	}

	// Log success
	logger->info("Successfully performed JSON data parsing");
}