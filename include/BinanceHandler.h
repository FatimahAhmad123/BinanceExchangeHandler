#ifndef BINANCE_HANDLER_H
#define BINANCE_HANDLER_H

#include <string>
#include <unordered_map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "rapidjson/document.h"

extern std::shared_ptr<spdlog::logger> logger;

class HTTPRequest
{
public:
	HTTPRequest();
	std::string performBinanceAPIRequest(const std::string &host, const std::string &port, const std::string &target, int version);
};

class JSONParser
{
public:
	void performJSONDataParsing(const std::string &jsonResponse);
	void handleDelete(const std::string &symbol);
	void handleUpdate(const std::string &symbol, const std::unordered_map<std::string, std::string> &updatedInfo);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> symbolInfoMap;

public:
	// Getter methods
	const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &getSymbolInfoMap() const;
	const std::unordered_map<std::string, std::string> &getSymbolInfo(const std::string &symbol) const;

	// Setter methods
	void setSymbolInfoMap(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &symbolInfoMap);
	void setSymbolInfo(const std::string &symbol, const std::unordered_map<std::string, std::string> &infoMap);
};

class QueryHandler
{
public:
	void handleQueries(const std::string &queryFile, JSONParser &jsonParser);
	void handleGetQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser);
	void handleUpdateQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser);
	void handleDeleteQuery(const rapidjson::Value &queryObject, JSONParser &jsonParser);
};

#endif
