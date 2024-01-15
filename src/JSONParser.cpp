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

void JSONParser::performJSONDataParsing(const std::string &jsonResponse)
{
	try
	{
		logger->info("PerformJSONDataParsing called.");
		rapidjson::Document document;
		document.Parse(jsonResponse.c_str()); // returns char pointer to string array

		if (document.HasParseError())
		{
			// Log an error
			logger->error("Error parsing JSON. Parse error code: {}, Offset: {}", document.GetParseError(), document.GetErrorOffset());
			return;
		}

		if (document.HasMember("symbols") && document["symbols"].IsArray()) // check for null value in array
		{
			const rapidjson::Value &symbolsArray = document["symbols"]; // accessing highest symbols array without making a copy, checks for symbol key

			for (rapidjson::SizeType i = 0; i < symbolsArray.Size(); ++i)
			{
				const rapidjson::Value &symbolObject = symbolsArray[i]; // getting value against symbols key

				if (symbolObject.HasMember("symbol") && symbolObject["symbol"].IsString()) // checking inner sysmbols array
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
	auto it = symbolInfoMap.find(symbol);
	if (it != symbolInfoMap.end())
	{
		return it->second;
	}
	else
	{
		// Symbol not found, return an empty map or handle appropriately
		static const std::unordered_map<std::string, std::string> emptyMap;
		return emptyMap;
	}
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
	logger->info("Before deletion. SymbolInfoMap size: {}", symbolInfoMap.size());
	const auto &it = symbolInfoMap.find(symbol);
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
	logger->info("After deletion. SymbolInfoMap size: {}", symbolInfoMap.size());
}

void JSONParser::handleUpdate(const std::string &symbol, const std::unordered_map<std::string, std::string> &updatedInfo)
{

	const auto &it = symbolInfoMap.find(symbol);
	if (it != symbolInfoMap.end())
	{
		for (const auto &entry : updatedInfo)
		{
			logger->info("Before update. SymbolInfoMap: {}", it->second[entry.first]);
			it->second[entry.first] = entry.second;
			logger->info("Symbol: {}, Field: {} updated to {}", symbol, entry.first, entry.second);
			logger->info("After update. SymbolInfoMap: {}", it->second[entry.first]);
		}
	}
	else
	{
		// Symbol not found
		logger->warn("Symbol {} not found for update.", symbol);
	}
}