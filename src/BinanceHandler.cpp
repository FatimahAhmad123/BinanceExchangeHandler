#include "BinanceHandler.h"
#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
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

extern std::shared_ptr<spdlog::logger> logger;

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

		// Return the response body as a string
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

void JSONParser::performJSONDataParsing(const std::string &jsonResponse)
{
	try
	{
		logger->info("PerformJSONDataParsing called.");
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

					// Using setter to set the symbol info
					setSymbolInfo(symbol, {{"status", status},
										   {"quoteAsset", quoteAsset},
										   {"tickSize", tickSize},
										   {"stepSize", stepSize}});
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

		// Log success
		logger->info("Successfully performed JSON data parsing");
	}
	catch (std::exception const &e)
	{
		// Log an error
		logger->error("Error: {}", e.what());
	}
}

// Getter method implementations
const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &JSONParser::getSymbolInfoMap() const
{
	return symbolInfoMap;
}

const std::unordered_map<std::string, std::string> &JSONParser::getSymbolInfo(const std::string &symbol) const
{
	return symbolInfoMap.at(symbol);
}

// Setter method implementations
void JSONParser::setSymbolInfoMap(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &symbolInfoMap)
{
	this->symbolInfoMap = symbolInfoMap;
}

void JSONParser::setSymbolInfo(const std::string &symbol, const std::unordered_map<std::string, std::string> &infoMap)
{
	symbolInfoMap[symbol] = infoMap;
	// logger->info("Symbol info map contents:");
	// for (const auto &entry : symbolInfoMap)
	// {
	// 	logger->info("Symbol: {}", entry.first);
	// 	for (const auto &info : entry.second)
	// 	{
	// 		logger->info("  {} : {}", info.first, info.second);
	// 	}
	// }
}

void JSONParser::handleDelete(const std::string &symbol)
{
	auto it = symbolInfoMap.find(symbol);
	if (it != symbolInfoMap.end())
	{
		// Symbol found, perform deletion
		symbolInfoMap.erase(it);
		logger->info("Symbol {} deleted.", symbol);
	}
	else
	{
		// Symbol not found
		logger->warn("Symbol {} not found for deletion.", symbol);
	}
}

void JSONParser::handleUpdate(const std::string &symbol, const std::unordered_map<std::string, std::string> &updatedInfo)
{
	auto it = symbolInfoMap.find(symbol);
	if (it != symbolInfoMap.end())
	{
		// Symbol found, perform update
		for (const auto &entry : updatedInfo)
		{
			it->second[entry.first] = entry.second;
			logger->info("Symbol: {}, Field: {} updated to {}", symbol, entry.first, entry.second);
		}
	}
	else
	{
		// Symbol not found
		logger->warn("Symbol {} not found for update.", symbol);
	}
}

int lastProcessedId = -1;

void QueryHandler::handleQueries(const std::string &queryFile, const JSONParser &jsonParser)
{
	try
	{
		std::ifstream file(queryFile);
		if (!file.is_open())
		{
			logger->error("Error opening query file.");
			return;
		}

		std::ostringstream queryContents;
		queryContents << file.rdbuf();
		file.close();

		rapidjson::Document queryDocument;
		queryDocument.Parse(queryContents.str().c_str());

		if (queryDocument.HasParseError())
		{
			logger->error("Error parsing JSON in query file. Parse error code: {}, Offset: {}", queryDocument.GetParseError(), queryDocument.GetErrorOffset());
			return;
		}

		if (queryDocument.HasMember("query") && queryDocument["query"].IsArray())
		{
			const rapidjson::Value &queryArray = queryDocument["query"];

			for (rapidjson::SizeType i = 0; i < queryArray.Size(); ++i)
			{
				const rapidjson::Value &queryObject = queryArray[i];

				if (queryObject.HasMember("id") && queryObject["id"].IsInt())
				{
					int id = queryObject["id"].GetInt();

					// Check if the query has changed
					if (id != lastProcessedId)
					{
						std::string queryType = queryObject["query_type"].GetString();

						if (queryType == "GET")
						{
							handleGetQuery(queryObject, jsonParser);
						}
						else if (queryType == "UPDATE")
						{
							JSONParser jsonParserCopy = jsonParser;
							handleUpdateQuery(queryObject, jsonParserCopy);
						}
						else if (queryType == "DELETE")
						{
							JSONParser jsonParserCopy = jsonParser;
							handleDeleteQuery(queryObject, jsonParserCopy);
						}
						else
						{
							logger->error("Invalid query type: {}", queryType);
						}

						// Update lastProcessedId
						lastProcessedId = id;
					}
				}
				else
				{
					logger->error("Missing or invalid 'id' in JSON query.");
				}
			}
		}
		else
		{
			logger->error("Missing or invalid 'query' array in JSON query file.");
		}
	}
	catch (std::exception const &e)
	{
		logger->error("Error: {}", e.what());
	}
}

void QueryHandler::handleGetQuery(const rapidjson::Value &queryObject, const JSONParser &jsonParser)
{
	if (!queryObject.IsObject())
	{
		logger->error("Invalid query object.");
		return;
	}

	if (!queryObject.HasMember("symbol") || !queryObject["symbol"].IsString())
	{
		logger->error("Missing or invalid 'symbol' in the query object.");
		return;
	}

	std::string symbol = queryObject["symbol"].GetString();

	logger->info("GET Query - Symbol: {}", symbol);

	const std::unordered_map<std::string, std::string> &symbolInfo = jsonParser.getSymbolInfo(symbol);

	rapidjson::Document answerDoc;
	answerDoc.SetObject();

	if (symbolInfo.find("status") != symbolInfo.end())
	{
		logger->info("GET Query - Symbol: {}, DataField: status, Value: {}", symbol, symbolInfo.at("status"));
		rapidjson::Value statusKey("status", answerDoc.GetAllocator());
		rapidjson::Value statusValue(symbolInfo.at("status").c_str(), answerDoc.GetAllocator());
		answerDoc.AddMember(statusKey, statusValue, answerDoc.GetAllocator());
	}
	else
	{
		logger->warn("GET Query - Symbol: {}, DataField: status not found.", symbol);
	}

	// Handle tickSize
	if (symbolInfo.find("tickSize") != symbolInfo.end())
	{
		logger->info("GET Query - Symbol: {}, DataField: tickSize, Value: {}", symbol, symbolInfo.at("tickSize"));
		rapidjson::Value tickSizeKey("tickSize", answerDoc.GetAllocator());
		rapidjson::Value tickSizeValue(symbolInfo.at("tickSize").c_str(), answerDoc.GetAllocator());
		answerDoc.AddMember(tickSizeKey, tickSizeValue, answerDoc.GetAllocator());
	}
	else
	{
		logger->warn("GET Query - Symbol: {}, DataField: tickSize not found.", symbol);
	}

	// Handle stepSize
	if (symbolInfo.find("stepSize") != symbolInfo.end())
	{
		logger->info("GET Query - Symbol: {}, DataField: stepSize, Value: {}", symbol, symbolInfo.at("stepSize"));
		rapidjson::Value stepSizeKey("stepSize", answerDoc.GetAllocator());
		rapidjson::Value stepSizeValue(symbolInfo.at("stepSize").c_str(), answerDoc.GetAllocator());
		answerDoc.AddMember(stepSizeKey, stepSizeValue, answerDoc.GetAllocator());
	}
	else
	{
		logger->warn("GET Query - Symbol: {}, DataField: stepSize not found.", symbol);
	}

	// Handle quoteAsset
	if (symbolInfo.find("quoteAsset") != symbolInfo.end())
	{
		logger->info("GET Query - Symbol: {}, DataField: quoteAsset, Value: {}", symbol, symbolInfo.at("quoteAsset"));
		rapidjson::Value quoteAssetKey("quoteAsset", answerDoc.GetAllocator());
		rapidjson::Value quoteAssetValue(symbolInfo.at("quoteAsset").c_str(), answerDoc.GetAllocator());
		answerDoc.AddMember(quoteAssetKey, quoteAssetValue, answerDoc.GetAllocator());
	}
	else
	{
		logger->warn("GET Query - Symbol: {}, DataField: quoteAsset not found.", symbol);
	}

	// Write the result to answers.json
	std::ofstream outputFile("answers.json");
	if (outputFile.is_open())
	{
		rapidjson::OStreamWrapper osw(outputFile);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		answerDoc.Accept(writer);
		outputFile.close();
	}
	else
	{
		logger->error("Failed to open answers.json for writing.");
	}
}

void QueryHandler::handleUpdateQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser)
{
	if (!queryObject.IsObject())
	{
		logger->error("Invalid query object.");
		return;
	}

	if (!queryObject.HasMember("symbol") || !queryObject["symbol"].IsString())
	{
		logger->error("Missing or invalid 'symbol' in the query object.");
		return;
	}

	std::string symbol = queryObject["symbol"].GetString();

	logger->info("UPDATE Query - Symbol: {}", symbol);

	if (!queryObject.HasMember("data"))
	{
		logger->error("Missing 'data' in the query object.");
		return;
	}

	const rapidjson::Value &dataObject = queryObject["data"];

	if (!dataObject.IsObject())
	{
		logger->error("'data' must be an object.");
		return;
	}

	// Convert the dataObject to an unordered_map
	std::unordered_map<std::string, std::string> updatedInfo;
	for (rapidjson::Value::ConstMemberIterator itr = dataObject.MemberBegin(); itr != dataObject.MemberEnd(); ++itr)
	{
		if (itr->value.IsString())
		{
			updatedInfo[itr->name.GetString()] = itr->value.GetString();
		}
	}

	// Call the handleUpdate method to perform the update
	jsonParser.handleUpdate(symbol, updatedInfo);
}
void QueryHandler::handleDeleteQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser)
{
	if (!queryObject.IsObject())
	{
		logger->error("Invalid query object.");
		return;
	}

	if (!queryObject.HasMember("symbol") || !queryObject["symbol"].IsString())
	{
		logger->error("Missing or invalid 'symbol' in the query object.");
		return;
	}

	std::string symbol = queryObject["symbol"].GetString();

	logger->info("DELETE Query - Symbol: {}", symbol);

	jsonParser.handleDelete(symbol);
}