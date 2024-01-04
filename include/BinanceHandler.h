#ifndef BINANCE_HANDLER_H
#define BINANCE_HANDLER_H

#include <string>
#include <unordered_map>
class HTTPRequest
{
public:
	std::string performBinanceAPIRequest(const std::string &host, const std::string &port, const std::string &target, int version);
	void performJSONDataParsing(const std::string &jsonResponse);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> symbolInfoMap;
};

#endif
