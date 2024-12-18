#pragma once

#include <vector>
#include <unordered_map>

class StockPricePredictor
{
public:

	StockPricePredictor(char numberOfFiles);
	virtual ~StockPricePredictor();

    void readStockExchangeData(std::string exchangeName);
    void getConsecutiveDataPoints(std::string timestamp);
    void predictNextStockValues();

private:
    double getFirstPrediction(std::unordered_map<std::string, double> stockData);
    void writePredictions();

    char m_inputNumberOfFiles;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> m_stockData;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> m_extractedData;
    std::vector<std::string> m_keysOrder;
    std::string m_exchangeName;
};