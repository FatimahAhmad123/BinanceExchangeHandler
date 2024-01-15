#include "BinanceHandler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "rapidjson/document.h"
int main()
{
	try
	{
		// Read URL from config.json
		std::ifstream configFile("config.json");
		if (!configFile.is_open())
		{
			std::cerr << "Error opening config file." << std::endl;
			return EXIT_FAILURE;
		}

		// Read the entire contents of the config file into a string
		std::ostringstream configContents;
		configContents << configFile.rdbuf();
		configFile.close();

		// Parse JSON using RapidJSON
		rapidjson::Document configDocument;
		configDocument.Parse(configContents.str().c_str());

		// Check if parsing succeeded
		if (configDocument.HasParseError())
		{
			std::cerr << "Error parsing JSON in config file. Parse error code: " << configDocument.GetParseError()
					  << ", Offset: " << configDocument.GetErrorOffset() << std::endl;
			return EXIT_FAILURE;
		}

		// Check for the presence of the "exchange_info_url" key
		if (configDocument.HasMember("exchange_info_url") && configDocument["exchange_info_url"].IsString())
		{
			std::string apiUrl = configDocument["exchange_info_url"].GetString();

			// Extract host and target from the URL
			std::string protocolDelimiter = "://";
			size_t posProtocol = apiUrl.find(protocolDelimiter);
			if (posProtocol != std::string::npos)
			{
				posProtocol += protocolDelimiter.length();

				size_t posPath = apiUrl.find('/', posProtocol);
				if (posPath != std::string::npos)
				{
					std::string host = apiUrl.substr(posProtocol, posPath - posProtocol);
					std::string target = apiUrl.substr(posPath);

					// Hard-coded values for demonstration purposes
					std::string port = "443";
					int version = 11; // HTTP version 1.1

					// Create an object of HTTPRequests
					HTTPRequest binanceRequest;

					// Call performBinanceAPIRequest with the parsed URL
					std::string response = binanceRequest.performBinanceAPIRequest(host, port, target, version);

					// Perform JSON data parsing
					binanceRequest.performJSONDataParsing(response);
				}
				else
				{
					std::cerr << "Invalid URL format: " << apiUrl << std::endl;
					return EXIT_FAILURE;
				}
			}
			else
			{
				std::cerr << "Invalid URL format: " << apiUrl << std::endl;
				return EXIT_FAILURE;
			}
		}
		else
		{
			std::cerr << "Missing or invalid 'exchange_info_url' in config file." << std::endl;
			return EXIT_FAILURE;
		}
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
