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
		// std::cout << boost::beast::buffers_to_string(res.body().data()) << std::endl;

		// Gracefully close the socket
		beast::error_code ec;
		stream.shutdown(ec);

		// Return the response body as a string
		return boost::beast::buffers_to_string(res.body().data());
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return ""; // Return an empty string in case of an error
	}
}

void HTTPRequest::performJSONDataParsing(const std::string &jsonResponse)
{
	rapidjson::Document document;
	document.Parse(jsonResponse.c_str());

	if (document.HasParseError())
	{
		std::cerr << "Error parsing JSON. Parse error code: " << document.GetParseError()
				  << ", Offset: " << document.GetErrorOffset() << std::endl;
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
				std::cerr << "Missing or invalid 'symbol' in JSON response." << std::endl;
			}
		}
	}
	else
	{
		std::cerr << "Missing or invalid 'symbols' array in JSON response." << std::endl;
	}

	// Print the entire unordered map for verification
	std::cout << "Symbol Info Map:" << std::endl;
	for (const auto &symbolPair : symbolInfoMap)
	{
		const std::string &symbol = symbolPair.first;
		const auto &infoMap = symbolPair.second;

		std::cout << "Symbol: " << symbol << std::endl;
		for (const auto &infoPair : infoMap)
		{
			const std::string &key = infoPair.first;
			const std::string &value = infoPair.second;
			std::cout << "  Key: " << key << ", Value: " << value << std::endl;
		}
	}
}
