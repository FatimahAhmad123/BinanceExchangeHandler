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
#include <unordered_set>

std::unordered_set<int> processedIds;

void QueryHandler::handleQueries(const std::string &queryFile, JSONParser &jsonParser)
{
	try
	{
		std::ifstream file(queryFile);
		if (!file.is_open())
		{
			logger->error("Error opening query file.");
			return;
		}

		std::ostringstream queryContents; // converting contents of file into a string
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
					if (processedIds.find(id) == processedIds.end())
					{
						// Process the query

						std::string queryType = queryObject["query_type"].GetString();

						if (queryType == "GET")
						{
							handleGetQuery(queryObject, jsonParser);
						}
						else if (queryType == "UPDATE")
						{

							handleUpdateQuery(queryObject, jsonParser);
						}
						else if (queryType == "DELETE")
						{

							handleDeleteQuery(queryObject, jsonParser);
						}
						else
						{
							logger->error("Invalid query type: {}", queryType);
						}

						// Update processed IDs
						processedIds.insert(id);
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

void QueryHandler::handleGetQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser) // jsonparcer object to call symbolinfo
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
	logger->info("Before processing query. SymbolInfoMap size: {}", jsonParser.getSymbolInfoMap().size());
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

	if (!symbolInfo.empty())
	{
		// Check if the symbol is deleted
		const auto &deletedIt = std::find_if(symbolInfo.begin(), symbolInfo.end(),
											 [](const auto &entry)
											 { return entry.second.empty(); });

		if (deletedIt != symbolInfo.end())
		{
			logger->error("GET Query - Symbol: {}, Error: Symbol is deleted.", symbol);
			return;
		}
	}
	logger->info("After processing query. SymbolInfoMap size: {}", jsonParser.getSymbolInfoMap().size());

	std::ofstream outputFile("answers.json", std::ios::app);
	if (outputFile.is_open())
	{
		rapidjson::OStreamWrapper osw(outputFile);
		rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
		answerDoc.Accept(writer);
		outputFile << "," << std::endl; // Adding a newline to separate entries
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