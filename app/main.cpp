#include "BinanceHandler.h" // Include the header file
#include <iostream>
#include <cstring>

int main()
{
	try
	{
		// Hardcoded values
		std::string host = "api.binance.com";
		std::string port = "443";
		std::string target = "/api/v1/exchangeInfo";
		int version = 11;

		// Create an object of HTTPRequest
		HTTPRequest binanceRequest;

		// Call performHttpRequest
		std::string response = binanceRequest.performBinanceAPIRequest(host, port, target, version);

		// Print the response
		std::cout << "Response from performBinanceAPIRequest:\n"
				  << response << std::endl;

		// ...
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
