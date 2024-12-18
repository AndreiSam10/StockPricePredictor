#include "StockPricePredictor.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <dirent.h>
#include <time.h>

#define MAX_DATA_POINTS 10
#define EXCHANGE_NAME "LSE"
#define FROM_TIMESTAMP "10-09-2023"

using namespace std;

StockPricePredictor::StockPricePredictor(char numberOfFiles) : m_inputNumberOfFiles(numberOfFiles)
{}

StockPricePredictor::~StockPricePredictor() {}

void StockPricePredictor::readStockExchangeData(std::string exchangeName)
{
    if ((m_inputNumberOfFiles != 1) && (m_inputNumberOfFiles != 2))
    {
        std::cout << "Error: Invalid number of files to read!! (possible values: 1 or 2) " << endl;
        return;
    }

    m_exchangeName = exchangeName;
    int counter = 0;

    DIR* dir = opendir(m_exchangeName.c_str());

    struct dirent *d;
    while ((d = readdir(dir)) && (counter < m_inputNumberOfFiles))
    {
        std::string filename = d->d_name;
        std::cout << "Read file: " << filename.c_str() << std::endl;

        if (filename != "." && filename != "..")
        {
            m_keysOrder.clear();
            std::string filepath = m_exchangeName + "/" + filename;
            ifstream inputFile;
            try
            {
                inputFile.open(filepath);

                // Read data, line by line
                std::string line;
                int lineCount = 0;
                std::unordered_map<string, double> dataMap;
                std::string stockName;
                while (std::getline(inputFile, line))
                {
                    string stockId = "";
                    string timestamp = "";
                    double stockPrice = 0.0;

                    // Get Stock Name
                    std::stringstream ss(line);
                    std::getline(ss, stockId, ',');
                    stockName = stockId;
                    if (ss.peek() == ',') ss.ignore();

                    // Get timestamp
                    std::getline(ss, timestamp, ',');
                    if (timestamp.compare("") == 0)
                    {
                        std::cout << "Error: Invalid timestamp!!!" << endl;
                    }
                    if (ss.peek() == ',') ss.ignore();

                    // Get Stock price
                    string substr;
                    std::getline(ss, substr, ',');
                    stockPrice = stod(substr);
                    if (stockPrice < 0)
                    {
                        std::cout << "Error: Invalid stock price: " << stockPrice << endl;
                        continue;
                    }
                    dataMap[timestamp] = stockPrice;
                    m_keysOrder.push_back(timestamp);
                    lineCount++;
                }

                if (lineCount == 0)
                {
                    std::cout << "Error: File is empty!!!" << endl;
                    continue;
                }
                m_stockData[stockName] = dataMap;
            }
            catch (const ifstream::failure& e)
            {
                std::cout << "Exception opening/reading file: " << filepath << " detail: " << e.what();
            }
            inputFile.close();
            getConsecutiveDataPoints(FROM_TIMESTAMP);
            predictNextStockValues();
            m_extractedData.clear();
            m_stockData.clear();
            counter++;
        }
    }
    closedir(dir);
}

void StockPricePredictor::getConsecutiveDataPoints(string timestamp)
{
    m_extractedData.clear();
    for (auto exchangeData : m_stockData)
    {
        int counter = 0;
        unordered_map<string, double> extractedStockData;

        bool foundKey = false;
        auto it = m_keysOrder.begin();
        while (it != m_keysOrder.end())
        {
            if (counter == MAX_DATA_POINTS)
            {
                // Remove all the next data points
                m_keysOrder.erase(it, m_keysOrder.end());
                break;
            }
            
            if ((*it).compare(timestamp) == 0)
            {
                foundKey = true;
            }

            if (foundKey)
            {
                extractedStockData[*it] = exchangeData.second[*it];
                counter++;
            }
            else
            {
                // Remove data point if not reference timestamp
                m_keysOrder.erase(it);
                continue;
            }
            it++;
        }
        if (foundKey == false)
        {
            cout << "Error: Key not found: " << timestamp << endl;
            continue;
        }

        if (counter != MAX_DATA_POINTS)
        {
            // Couldn't find 10 entries
            extractedStockData.clear();
            std::cout << "Error: not enough data for Stock: " << exchangeData.first << " " << counter << std::endl;
        }
        else
        {
            m_extractedData[exchangeData.first] = extractedStockData;
        }
    }
}

void StockPricePredictor::predictNextStockValues()
{
    for (auto stockData : m_extractedData)
    {
        // Get last stock entry timestamp and add 1 day
        string prevKey = m_keysOrder.back();
        std::istringstream ss(m_keysOrder.back());
        struct std::tm  ts{};
        ss >> std::get_time(&ts, "%d-%m-%Y");
        char buf[80];
        std::time_t rawtime = mktime(&ts);
        rawtime += 86400;

        // Format time, "dd-mm-yyyy"
        ts = *localtime(&rawtime);
        strftime(buf, sizeof(buf), "%d-%m-%Y", &ts);
        string newTimestamp(buf);

        // Get first prediction and add to map
        double firstPrediction = getFirstPrediction(stockData.second);
        std::pair<string, double> prediction1(newTimestamp, firstPrediction);
        m_extractedData[stockData.first].insert(prediction1);
        m_keysOrder.push_back(newTimestamp);

        // Add 1 day to first prediction date and convert to string
        rawtime += 86400;

        // Format time, "dd-mm-yyyy"
        ts = *localtime(&rawtime);
        strftime(buf, sizeof(buf), "%d-%m-%Y", &ts);
        newTimestamp = buf;

        // Calculate second prediction and add to map
        double secondPrediction = firstPrediction + ((m_extractedData[stockData.first][prevKey] - firstPrediction) / 2.0f);
        std::pair<string, double> prediction2(newTimestamp, secondPrediction);
        m_extractedData[stockData.first].insert(prediction2);
        m_keysOrder.push_back(newTimestamp);

        // Add 1 day to first prediction date and convert to string
        rawtime += 86400;

        // Format time, "dd-mm-yyyy"
        ts = *localtime(&rawtime);
        strftime(buf, sizeof(buf), "%d-%m-%Y", &ts);
        newTimestamp = buf;

        // Calculate third prediction and add to map
        double thirdPrediction = secondPrediction + ((firstPrediction - secondPrediction) / 4.0f);
        std::pair<string, double> prediction3(newTimestamp, thirdPrediction);
        m_extractedData[stockData.first].insert(prediction3);
        m_keysOrder.push_back(newTimestamp);
        cout << " " << firstPrediction << " " << secondPrediction << " " << thirdPrediction << endl;
    }
    writePredictions();
}

double StockPricePredictor::getFirstPrediction(unordered_map<string, double> stockData)
{
    double maxValue = 0.0;
    double secondHighest = 0.0;

    // Get maximum stock value
    for (auto data : stockData)
    {
        if (data.second > maxValue)
        {
            maxValue = data.second;
        }
    }

    // Get second maximum value
    for (auto data : stockData)
    {
        if ((data.second > secondHighest) && (data.second != maxValue))
        {
            secondHighest = data.second;
        }
    }

    return secondHighest;
}

void StockPricePredictor::writePredictions()
{
    for (auto extractedData : m_extractedData)
    {
        ofstream outputFile;
        try
        {
            outputFile.open(m_exchangeName + "/" + extractedData.first + "_Prediction.csv");
            for (auto key : m_keysOrder)
            {
                outputFile << extractedData.first << "," << key << "," << extractedData.second[key] << "\n";
            }
        }
        catch (const ofstream::failure& e)
        {
            std::cout << "Exception opening/writing file: " << e.what();
        }
        outputFile.close();
    }
}

int main(int argc, char* argv[]) 
{
    int numberOfFiles = 2;

    StockPricePredictor* stockPredictor = new StockPricePredictor(numberOfFiles);
    stockPredictor->readStockExchangeData(EXCHANGE_NAME);
    return 0;
}