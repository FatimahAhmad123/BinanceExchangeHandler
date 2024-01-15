#include "BinanceHandler.h"
#include "rapidjson/document.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Initialize a global logger with a file sink
auto logger = spdlog::basic_logger_mt("binance_logger", "log/binance_log.txt");

// Function to read the logging level from the config file using RapidJSON
std::string readLoggingLevel(const std::string &configFile)
{
	std::ifstream file(configFile);
	if (!file.is_open())
	{
		logger->error("Error opening config file.");
		return "error";
	}

	std::ostringstream configContents;
	configContents << file.rdbuf();
	file.close();

	rapidjson::Document document;
	document.Parse(configContents.str().c_str());

	if (!document.HasParseError() && document.HasMember("logging") && document["logging"].HasMember("level"))
	{
		return document["logging"]["level"].GetString();
	}

	return "error";
}

int main()
{
	// Initialize the logger
	spdlog::set_default_logger(logger);

	// Read logging level from config.json
	std::string logLevel = readLoggingLevel("config.json");

	// Set the logging level based on the configuration
	if (logLevel == "error" || logLevel == "info" || logLevel == "warn" || logLevel == "trace" || logLevel == "fatal")
	{
		spdlog::set_level(spdlog::level::from_str(logLevel));
		logger->info("Logging level set to: {}", logLevel);
	}
	else
	{
		std::cout << "Error reading log level from config." << std::endl;
		return EXIT_FAILURE;
	}
	try
	{
		// Read URL from config.json
		std::ifstream configFile("config.json");
		if (!configFile.is_open())
		{
			logger->error("Error opening config file.");
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
			logger->error("Error parsing JSON in config file. Parse error code: {}, Offset: {}", configDocument.GetParseError(), configDocument.GetErrorOffset());
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

					// Create an object of JSONParser
					JSONParser jsonParser;

					// Call performJSONDataParsing with the response
					jsonParser.performJSONDataParsing(response);

					// Create an object of QueryHandling
					QueryHandling queryHandler;

					// Call handleQueries with the query file path
					queryHandler.handleQueries("query.json");
				}
				else
				{
					logger->error("Invalid URL format: {}", apiUrl);
					return EXIT_FAILURE;
				}
			}
			else
			{
				logger->error("Invalid URL format: {}", apiUrl);
				return EXIT_FAILURE;
			}
		}
		else
		{
			logger->error("Missing or invalid 'exchange_info_url' in config file.");
			return EXIT_FAILURE;
		}
	}
	catch (std::exception const &e)
	{
		// Log errors
		logger->error("Error: {}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
