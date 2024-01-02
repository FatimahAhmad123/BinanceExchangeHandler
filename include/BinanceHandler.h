#ifndef BINANCE_HANDLER_H
#define BINANCE_HANDLER_H

#include <string>

class HTTPRequest
{
public:
	std::string performBinanceAPIRequest(const std::string &host, const std::string &port, const std::string &target, int version);
};

#endif
