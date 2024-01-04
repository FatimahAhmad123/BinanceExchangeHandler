#ifndef BINANCE_HANDLER_H
#define BINANCE_HANDLER_H

#include <string>
#include <unordered_map>

class HTTPRequest
{
public:
	HTTPRequest();
	std::string performBinanceAPIRequest(const std::string &host, const std::string &port, const std::string &target, int version);
};

class JSONParser
{
public:
	JSONParser();
	void performJSONDataParsing(const std::string &jsonResponse);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> symbolInfoMap;
};

#endif
