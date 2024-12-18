# StockPricePredictor
Download StockPricePredictor.cpp and StockPricePredictor.h
Place the relevant Exchange folders containing the Input Files in the same directory with the source and header files.

For the sake of simplicity please set the following defines in StockPricePredictor.cpp similar to below:
  #define EXCHANGE_NAME "LSE" - this will represent the Exchange folder name where the input CSV files can be found (set this accordingly)
  #define FROM_TIMESTAMP "10-09-2023" - please set this with the reference timestamp (random) from which the analysis will start. Format should be DD-MM-YYYY

Building the application:
Go to the folder where the source and header files are downloaded and run:
g++ -o StockPricePredictor StockPricePredictor.cpp

Running the application:
./StockPricePredictor

Go to the exchange folder that is analysed and check for the generated output files:
e.g. FLTR_Prediction.csv (StockName_Prediction.csv format)
