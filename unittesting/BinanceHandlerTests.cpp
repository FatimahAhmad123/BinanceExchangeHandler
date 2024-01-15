#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "BinanceHandler.h"
#include <fstream>
#include <sstream>

std::shared_ptr<spdlog::logger> logger;

TEST(BinanceHandlerTests, ConnectionAndDataRetrieval)
{
	HTTPRequest httpRequest;

	std::string responseData = httpRequest.performBinanceAPIRequest("fapi.binance.com", "443", "/fapi/v1/exchangeInfo", 11); // hard code values
	ASSERT_FALSE(responseData.empty());
}

TEST(BinanceHandlerTests, DataParsing)
{
	HTTPRequest httpRequest;
	std::string jsonResponse = httpRequest.performBinanceAPIRequest("fapi.binance.com", "443", "/fapi/v1/exchangeInfo", 11);

	ASSERT_FALSE(jsonResponse.empty());

	JSONParser jsonParser;
	jsonParser.performJSONDataParsing(jsonResponse);

	auto symbolInfo = jsonParser.getSymbolInfo("BTCUSDT");

	ASSERT_TRUE(symbolInfo.find("status") != symbolInfo.end());
	ASSERT_EQ(symbolInfo.find("status")->second, "TRADING");

	ASSERT_TRUE(symbolInfo.find("tickSize") != symbolInfo.end());
	ASSERT_EQ(symbolInfo.find("tickSize")->second, "0.10");
}

TEST(QueryHandlerTests, HandleGetQuery)
{
	JSONParser jsonParser;

	jsonParser.setSymbolInfoMap({
		{"BTCUSDT", {{"status", "TRADING"}, {"tickSize", "0.01"}, {"stepSize", "0.001"}, {"quoteAsset", "USDT"}}},
		{"ETHUSDT", {{"status", "TRADING"}, {"tickSize", "0.02"}, {"stepSize", "0.002"}, {"quoteAsset", "USDT"}}},

	});

	rapidjson::Document queryObject(rapidjson::kObjectType);
	queryObject.AddMember("id", 1, queryObject.GetAllocator());
	queryObject.AddMember("query_type", "GET", queryObject.GetAllocator());
	queryObject.AddMember("symbol", "BTCUSDT", queryObject.GetAllocator());

	QueryHandler queryHandler;
	queryHandler.handleGetQuery(queryObject, jsonParser);

	ASSERT_EQ(jsonParser.getSymbolInfoMap().size(), 2);

	std::ifstream outputFile("answers.json");
	std::string line;
	bool statusFieldFound = false;
	while (std::getline(outputFile, line))
	{
		if (line.find("status") != std::string::npos)
		{
			statusFieldFound = true;
			break;
		}
	}
	outputFile.close();
	ASSERT_TRUE(statusFieldFound);
}

TEST(QueryHandlerTests, HandleUpdateQuery)
{

	JSONParser jsonParser;
	jsonParser.setSymbolInfoMap({
		{"BTCUSDT", {{"status", "TRADING"}, {"tickSize", "0.01"}, {"stepSize", "0.001"}, {"quoteAsset", "USDT"}}},
		{"ETHUSDT", {{"status", "TRADING"}, {"tickSize", "0.02"}, {"stepSize", "0.002"}, {"quoteAsset", "USDT"}}},

	});

	rapidjson::Document queryObject(rapidjson::kObjectType);
	queryObject.AddMember("id", 2, queryObject.GetAllocator());
	queryObject.AddMember("query_type", "UPDATE", queryObject.GetAllocator());
	queryObject.AddMember("symbol", "BTCUSDT", queryObject.GetAllocator());

	rapidjson::Value dataObject(rapidjson::kObjectType);
	dataObject.AddMember("status", "PENDING", queryObject.GetAllocator());
	dataObject.AddMember("tickSize", "0.0001", queryObject.GetAllocator());
	queryObject.AddMember("data", dataObject, queryObject.GetAllocator());

	QueryHandler queryHandler;
	queryHandler.handleUpdateQuery(queryObject, jsonParser);

	const std::unordered_map<std::string, std::string> &btcusdInfo = jsonParser.getSymbolInfo("BTCUSDT");
	ASSERT_EQ(btcusdInfo.at("status"), "PENDING");
	ASSERT_EQ(btcusdInfo.at("tickSize"), "0.0001");
}

TEST(QueryHandlerTests, HandleDeleteQuery)
{

	JSONParser jsonParser;
	jsonParser.setSymbolInfoMap({
		{"BTCUSDT", {{"status", "TRADING"}, {"tickSize", "0.01"}, {"stepSize", "0.001"}, {"quoteAsset", "USDT"}}},
		{"ETHUSDT", {{"status", "TRADING"}, {"tickSize", "0.02"}, {"stepSize", "0.002"}, {"quoteAsset", "USDT"}}},

	});

	rapidjson::Document queryObject(rapidjson::kObjectType); // wll represent a json object
	queryObject.AddMember("id", 3, queryObject.GetAllocator());
	queryObject.AddMember("query_type", "DELETE", queryObject.GetAllocator());
	queryObject.AddMember("symbol", "BTCUSDT", queryObject.GetAllocator());

	// Call handleDeleteQuery
	QueryHandler queryHandler;
	queryHandler.handleDeleteQuery(queryObject, jsonParser);

	ASSERT_TRUE(jsonParser.getSymbolInfo("BTCUSDT").empty());
}

TEST(QueryHandlerTests, DeleteAndGetQuerySequence)
{

	JSONParser jsonParser;
	jsonParser.setSymbolInfoMap({
		{"BTCUSDT", {{"status", "TRADING"}, {"tickSize", "0.01"}, {"stepSize", "0.001"}, {"quoteAsset", "USDT"}}},
		{"ETHUSDT", {{"status", "TRADING"}, {"tickSize", "0.02"}, {"stepSize", "0.002"}, {"quoteAsset", "USDT"}}},

	});
	QueryHandler queryHandler;

	std::string symbolToDelete = "BTCUSDT";
	rapidjson::Document deleteDocument(rapidjson::kObjectType);
	deleteDocument.AddMember("id", 4, deleteDocument.GetAllocator());
	deleteDocument.AddMember("query_type", "DELETE", deleteDocument.GetAllocator());
	deleteDocument.AddMember("symbol", rapidjson::Value(symbolToDelete.c_str(), deleteDocument.GetAllocator()).Move(), deleteDocument.GetAllocator());
	queryHandler.handleDeleteQuery(deleteDocument, jsonParser);

	rapidjson::Document getDocument(rapidjson::kObjectType);
	getDocument.AddMember("id", 5, getDocument.GetAllocator());
	getDocument.AddMember("query_type", "GET", getDocument.GetAllocator());
	getDocument.AddMember("symbol", rapidjson::Value(symbolToDelete.c_str(), getDocument.GetAllocator()).Move(), getDocument.GetAllocator());
	queryHandler.handleGetQuery(getDocument, jsonParser);

	ASSERT_TRUE(jsonParser.getSymbolInfo(symbolToDelete).empty());
}

TEST(QueryHandlerTests, GetQueryAfterUpdateQuery)
{

	JSONParser jsonParser;
	jsonParser.setSymbolInfoMap({
		{"BTCUSDT", {{"status", "TRADING"}, {"tickSize", "0.1"}, {"stepSize", "0.1"}, {"quoteAsset", "USDT"}}},
		{"ETHUSDT", {{"status", "TRADING"}, {"tickSize", "0.02"}, {"stepSize", "0.002"}, {"quoteAsset", "USDT"}}},

	});
	QueryHandler queryHandler;

	rapidjson::Document updateDocument(rapidjson::kObjectType);
	updateDocument.AddMember("id", 4, updateDocument.GetAllocator());
	updateDocument.AddMember("query_type", "UPDATE", updateDocument.GetAllocator());
	updateDocument.AddMember("symbol", "BTCUSDT", updateDocument.GetAllocator());

	rapidjson::Value dataObject(rapidjson::kObjectType);
	dataObject.AddMember("tickSize", "0.01", updateDocument.GetAllocator());
	dataObject.AddMember("stepSize", "0.001", updateDocument.GetAllocator());
	updateDocument.AddMember("data", dataObject, updateDocument.GetAllocator());

	queryHandler.handleUpdateQuery(updateDocument, jsonParser);

	rapidjson::Document getDocument(rapidjson::kObjectType);
	getDocument.AddMember("id", 5, getDocument.GetAllocator());
	getDocument.AddMember("query_type", "GET", getDocument.GetAllocator());
	getDocument.AddMember("symbol", "BTCUSDT", getDocument.GetAllocator());
	queryHandler.handleGetQuery(getDocument, jsonParser);

	const std::unordered_map<std::string, std::string> &updatedInfo = jsonParser.getSymbolInfo("BTCUSDT");

	ASSERT_EQ(updatedInfo.at("tickSize"), "0.01");
	ASSERT_EQ(updatedInfo.at("stepSize"), "0.001");
}

int main(int argc, char **argv)
{
	setenv("GTEST_LOG", "INFO", 1);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
