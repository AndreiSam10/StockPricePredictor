import os
import csv
import datetime

#Configure below the Number of files, Exchange Name and Reference Timestamp
numberOfFiles = 2
exchangeName = 'LSE'
referenceTimestamp = '10-09-2023'

def getFirstPrediction(values):
    maxPrice = 0
    secondMaxPrice = 0
    for stockEntry in values:
        stockPrice = float(stockEntry.get('stockPrice'))
        if stockPrice > maxPrice:
            maxPrice = stockPrice

    for stockEntry in values:
        stockPrice = float(stockEntry.get('stockPrice'))
        if (stockPrice > secondMaxPrice) and (stockPrice != maxPrice):
            secondMaxPrice = stockPrice

    return secondMaxPrice

def predictNextStockPrice(lastTenValues):
    stockId = lastTenValues[-1].get('stockId')
    lastStockPrice = float(lastTenValues[-1].get('stockPrice'))

    # Get first prediction as the second max value out of the referenced ten entries
    firstPrediction = getFirstPrediction(lastTenValues)
    timestamp = datetime.datetime.strptime(lastTenValues[-1].get('timestamp'),'%d-%m-%Y') + datetime.timedelta(days=1)
    timestamp = datetime.datetime.strftime(timestamp, '%d-%m-%Y')
    #print(str(firstPrediction) + ' ' + timestamp)
    lastTenValues.append({'stockId' : stockId, 'timestamp' : timestamp, 'stockPrice' : firstPrediction})

    # Get second prediction by adding a variation from the first prediction of half the difference between last stock price
    # and first prediction
    secondPrediction = firstPrediction + ((lastStockPrice - firstPrediction) / 2)
    timestamp = datetime.datetime.strptime(timestamp,'%d-%m-%Y') + datetime.timedelta(days=1)
    timestamp = datetime.datetime.strftime(timestamp, '%d-%m-%Y')
    #print(str(secondPrediction) + ' ' + timestamp)
    lastTenValues.append({'stockId' : stockId, 'timestamp' : timestamp, 'stockPrice' : secondPrediction})

    # Get third prediction by adding a variation from the second prediction of quarter the difference between first prediction
    # and second prediction
    thirdPrediction = secondPrediction + (round((firstPrediction - secondPrediction) / 4.0, ndigits=10))
    timestamp = datetime.datetime.strptime(timestamp,'%d-%m-%Y') + datetime.timedelta(days=1)
    timestamp = datetime.datetime.strftime(timestamp, '%d-%m-%Y')
    #print(str(thirdPrediction) + ' ' + timestamp)
    lastTenValues.append({'stockId' : stockId, 'timestamp' : timestamp, 'stockPrice' : thirdPrediction})

    try:
        with open(os.getcwd() + '/StockPricePredictor/' + exchangeName + '/' + stockId + '_Prediction.csv', 'w+') as file:
            cnt = 1
            for row in lastTenValues:
                if cnt == 1:
                    file.write(row.get('stockId') + ',' + row.get('timestamp') + ',' + str(row.get('stockPrice')))
                else:
                    file.write('\n' + row.get('stockId') + ',' + row.get('timestamp') + ',' + str(row.get('stockPrice')))
                cnt += 1
            file.close()
    except PermissionError:
        print('Error! You do not have the permissions to write to this file ' + file.name)
    except IOError:
        print('An error has occured while writing to file: ' + file.name)

    #print(lastTenValues)

def readFiles():
    print('Reading ' + str(numberOfFiles) + ' files from Exchange: [' + exchangeName + '] starting date: [' + referenceTimestamp + ']')
    fileCount = 0
    for filename in os.listdir(os.getcwd() + '/StockPricePredictor/' + exchangeName):
        if fileCount == numberOfFiles:
            break
        f = os.path.join(os.getcwd() + '/StockPricePredictor/' + exchangeName, filename)
        if os.path.isfile(f):
            print('Input file: [' + f + ']')
            fileCount += 1
            try:
                with open(f) as file:
                    reader = csv.DictReader(file, fieldnames=['stockId', 'timestamp', 'stockPrice'])
                    rowCount = 0 # counter for the ten reference entries
                    cnt = 0 # global counter to check empty files
                    foundTimestamp = False
                    resultList = list(dict())
                    for row in reader:
                        cnt += 1
                        if row.get('timestamp') == referenceTimestamp:
                            foundTimestamp = True
                        if foundTimestamp == True:
                            rowCount += 1
                            #print(row)
                            entryDictionary = dict()

                            stockId = row.get('stockId')
                            if stockId is None or stockId == '':
                                print('Error! Invalid stockId!!!')
                                continue
                            else:
                                entryDictionary['stockId'] = stockId
                                
                            timestamp = row.get('timestamp')
                            if timestamp is None or timestamp == '':
                                print('Error! Invalid timestamp!!!')
                                continue
                            else:
                                entryDictionary['timestamp'] = timestamp

                            stockPrice = row.get('stockPrice')
                            if stockPrice is None or stockPrice == '' or float(stockPrice) < 0:
                                print('Error! Invalid stockPrice')
                                continue
                            else:
                                entryDictionary['stockPrice'] = stockPrice
                            resultList.append(entryDictionary)
                        if rowCount == 10:
                            break
                    if cnt == 0:
                        print('Error! Empty file!!')
                        continue
                    if foundTimestamp == False:
                        print('Error! Could not find the required timestamp!!!')
                        continue
                    if rowCount != 10:
                        print('Error! Could not read the first 10 rows!')
                        continue
                    #print(resultList)
                    file.close()
                    predictNextStockPrice(resultList)
            except FileNotFoundError:
                print('Error! The file does not exist: ' + f)
            except PermissionError:
                print('Error! You do not have the permissions to read this file ' + f)
            except IOError:
                print('An error has occured while reading ' + f)
    if fileCount == 0:
        print('Error! No files available!')

def main():
    readFiles()

main()
