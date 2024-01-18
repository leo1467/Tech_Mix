#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <ctime>
#include <exception>
#include <filesystem> // C++17以上才有的library
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "functions.h"
#include "company.h"

using namespace std;
using namespace chrono;
using namespace filesystem; // C++17以上才有的library

CompanyInfo::CompanyInfo(path pricePath, Info &info) : companyName_(pricePath.stem().string()), info_(&info)
{
    set_paths(paths_);
    store_date_price(pricePath);
    // create_folder(paths_);
    find_table_start_row();
    cout << companyName_ << endl;
}

void CompanyInfo::set_paths(Path &paths)
{ // 設定各種路徑
    for (auto tech : info_->allTech_)
    {
        paths.techOuputPaths_.emplace_back(info_->techFolder_ + tech + "/" + companyName_ + "/");

        paths.resultOutputPaths_.emplace_back(info_->rootFolder_ + "result_" + tech + "/");

        string companyRootFolder = info_->rootFolder_ + "result_" + tech + "/" + companyName_ + "/";
        paths.companyRootPaths_.emplace_back(companyRootFolder);

        paths.trainFilePaths_.emplace_back(companyRootFolder + "train/");
        paths.testFilePaths_.emplace_back(companyRootFolder + "test/");

        paths.trainTraditionFilePaths_.emplace_back(companyRootFolder + "trainTradition/");
        paths.testTraditionFilePaths_.emplace_back(companyRootFolder + "testTradition/");

        paths.trainHoldFilePaths_.emplace_back(companyRootFolder + "trainHold/");
        paths.testHoldFilePaths_.emplace_back(companyRootFolder + "testHold/");

        paths.trainTraditionHoldFilePaths_.emplace_back(companyRootFolder + "trainTraditionHold/");
        paths.testTraditionHoldFilePaths_.emplace_back(companyRootFolder + "testTraditionHold/");

        paths.trainBestHold_.emplace_back(companyRootFolder + "trainBestHold/");
        paths.testBestHold_.emplace_back(companyRootFolder + "testBestHold/");

        paths.trainTraditionBestHold_.emplace_back(companyRootFolder + "trainTraditionBestHold/");
        paths.testTraditionBestHold_.emplace_back(companyRootFolder + "testTraditionBestHold/");
    }
    if (info_->debug_)
    {
        paths.trainFilePaths_[info_->techIndex_].clear();
    }
}

void CompanyInfo::store_date_price(path priceFilePath)
{ // 讀股價
    vector<vector<string>> priceFile = read_data(priceFilePath);
    totalDays_ = (int)priceFile.size() - 1;
    date_.resize(totalDays_);
    price_.resize(totalDays_);
    for (int i = 1, j = 0; i <= totalDays_; i++)
    {
        date_[i - 1] = priceFile[i][0];
        if (!is_double(priceFile[i][4]))
        {
            price_[i - 1] = price_[i - 2];
        }
        else
        {
            price_[i - 1] = stod(priceFile[i][4]);
        }
        if (j == 0 && date_[i - 1].substr(0, 7) == info_->testStartYear_)
        {
            testStartRow_ = i - 1;
            j++;
        }
        else if (j == 1 && date_[i - 1].substr(0, 7) == info_->testEndYear_)
        {
            testEndRow_ = i - 2;
            j++;
        }
    }
    testDays_ = testEndRow_ - testStartRow_ + 1;
    oneYearDays_ = testDays_ / info_->testLength_;
}

void CompanyInfo::create_folder(Path &paths)
{ // 用不到
    if (!info_->mixedTech_)
    {
        create_directories(paths.techOuputPaths_[info_->techIndex_]);
    }
    create_directories(paths.trainHoldFilePaths_[info_->techIndex_]);
    create_directories(paths.testHoldFilePaths_[info_->techIndex_]);
    create_directories(paths.trainTraditionHoldFilePaths_[info_->techIndex_]);
    create_directories(paths.testTraditionHoldFilePaths_[info_->techIndex_]);
    create_directories(paths.trainBestHold_[info_->techIndex_]);
    create_directories(paths.testBestHold_[info_->techIndex_]);
    create_directories(paths.trainTraditionBestHold_[info_->techIndex_]);
    create_directories(paths.testTraditionBestHold_[info_->techIndex_]);
    for (auto i : info_->slidingWindows_)
    {
        create_directories(paths.trainFilePaths_[info_->techIndex_] + i);
        create_directories(paths.testFilePaths_[info_->techIndex_] + i);
        create_directories(paths.trainTraditionFilePaths_[info_->techIndex_] + i);
        create_directories(paths.testTraditionFilePaths_[info_->techIndex_] + i);
    }
}

void CompanyInfo::find_table_start_row()
{ // TechTable要從date和price的哪一個row開始
    int longestTrainMonth = -1;
    for (auto windowComponent : info_->slidingWindowPairs_)
    {
        int trainMonth;
        if (get<1>(windowComponent.second) == 'M')
        {
            trainMonth = get<2>(windowComponent.second);
        }
        else
        {
            trainMonth = 12;
        }
        if (get<1>(windowComponent.second) == 'M' && trainMonth > longestTrainMonth)
        {
            longestTrainMonth = trainMonth;
        }
    }
    if (longestTrainMonth == -1)
    {
        longestTrainMonth = 12;
    }
    for (int i = testStartRow_ - 1, monthCount = 0; i >= 0; i--)
    {
        if (date_[i].substr(5, 2) != date_[i - 1].substr(5, 2))
        {
            monthCount++;
            if (monthCount == longestTrainMonth)
            {
                tableStartRow_ = i - 20;
                break;
            }
        }
    }
    if (tableStartRow_ == -1)
    {
        cout << "can't find longestTrainRow_" << endl;
        exit(1);
    }
}

void CompanyInfo::store_tech_to_vector(int techIndex)
{
    cout << "calculating " << companyName_ << " " << info_->techType_ << endl;
    vector<double> tmp;
    techTable_.emplace_back(tmp);
    (this->*cal_tech_[techIndex])(tmp); // or (*this.*cal_tech_[info_->techIndex_])(tmp);
    // outside the class (company.*(company.cal_tech_[company.info_.techIndex_]))(tmp);
    cout << "done calculating" << endl;
}

void CompanyInfo::cal_SMA(vector<double> &tmp)
{ // 計算SMA
    for (int period = 1; period < 257; period++)
    {
        for (int dateRow = period - 1; dateRow < totalDays_; dateRow++)
        {
            double MARangePriceSum = 0;
            for (int i = dateRow, j = period; j > 0; i--, j--)
            {
                MARangePriceSum += price_[i];
            }
            tmp.emplace_back(MARangePriceSum / period);
        }
        techTable_.emplace_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_WMA(vector<double> &tmp)
{ // 計算WMA
    for (int period = 1; period < 257; period++)
    {
        for (int dateRow = period - 1; dateRow < totalDays_; dateRow++)
        {
            double weightedPriceSum = 0;
            int totalWeight = 0;
            for (int weight = period, tmpDateRow = dateRow; weight > 0; weight--, tmpDateRow--)
            {
                weightedPriceSum += price_[tmpDateRow] * weight;
                totalWeight += weight;
            }
            tmp.emplace_back(weightedPriceSum / totalWeight);
        }
        techTable_.emplace_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_EMA(vector<double> &tmp)
{ // 未完成，有bug
    for (int period = 1; period < 257; period++)
    {
        double alpha = 2.0 / (double(period) + 1.0);
        double EMA = 0;
        for (int dateRow = 0; dateRow < totalDays_; dateRow++)
        {
            if (dateRow == 0)
            {
                EMA = price_[dateRow];
            }
            else
            {
                EMA = price_[dateRow] * alpha + (1.0 - alpha) * tmp[dateRow - 1];
            }
            tmp.emplace_back(EMA);
        }
        techTable_.emplace_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_RSI(vector<double> &tmp)
{ // 計算RSI
    vector<double> priceGainLoss(totalDays_ - 1);
    for (int priceDateRow = 1; priceDateRow < totalDays_; priceDateRow++)
    {
        priceGainLoss[priceDateRow - 1] = price_[priceDateRow] - price_[priceDateRow - 1];
    }
    for (int RSIPeriod = 1; RSIPeriod < 257; RSIPeriod++)
    {
        double RSI, gain = 0, loss = 0, avgGain = 0, avgLoss = 0;
        for (int row = 0; row < RSIPeriod; row++)
        {
            if (priceGainLoss[row] >= 0)
            {
                gain += priceGainLoss[row];
            }
            else
            {
                loss += -priceGainLoss[row];
            }
        }
        avgGain = gain / RSIPeriod;
        avgLoss = loss / RSIPeriod;
        RSI = 100.0 - (100.0 / (1 + (avgGain / avgLoss)));
        tmp.emplace_back(RSI);
        double preAvgGain = avgGain, preAvgLoss = avgLoss;
        for (int i = RSIPeriod; i < totalDays_ - 1; i++)
        {
            if (priceGainLoss[i] >= 0)
            {
                RSI = 100.0 - (100.0 / (1 + (((preAvgGain * (RSIPeriod - 1) + priceGainLoss[i]) / (preAvgLoss * (RSIPeriod - 1))))));
                preAvgGain = (preAvgGain * (RSIPeriod - 1) + priceGainLoss[i]) / RSIPeriod;
                preAvgLoss = (preAvgLoss * (RSIPeriod - 1)) / RSIPeriod;
            }
            else
            {
                RSI = 100.0 - (100.0 / (1 + ((preAvgGain * (RSIPeriod - 1)) / (preAvgLoss * (RSIPeriod - 1) - priceGainLoss[i]))));
                preAvgGain = (preAvgGain * (RSIPeriod - 1)) / RSIPeriod;
                preAvgLoss = (preAvgLoss * (RSIPeriod - 1) - priceGainLoss[i]) / RSIPeriod;
            }
            if (isnan(RSI))
            {
                RSI = 100;
            }
            tmp.emplace_back(RSI);
        }
        techTable_.emplace_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::output_tech_file(int techIndex)
{ // 輸出技術指標資料
    create_directories(paths_.techOuputPaths_[techIndex]);
    store_tech_to_vector(techIndex);
    cout << "saving " << info_->techType_ << " file" << endl;
    for (int techPeriod = 1; techPeriod < 257; techPeriod++)
    {
        if (techPeriod % 10 == 0)
        {
            cout << ".";
        }
        ofstream out;
        set_techFile_title(out, techPeriod, techIndex);
        int techSize = (int)techTable_[techPeriod].size();
        int dateRow = 0;
        switch (techIndex)
        {
            case 0:
            case 1:
                dateRow = techPeriod - 1;
                break;
            case 2:
            {
                dateRow = 0;
                break;
            }
            case 3:
            {
                dateRow = techPeriod;
                break;
            }
            default:
            {
                cout << "output_tech exception" << endl;
                exit(1);
                break;
            }
        }
        for (int i = 0; i < techSize; i++, dateRow++)
        {
            out << date_[dateRow] << "," << set_precision(techTable_[techPeriod][i]) << endl;
        }
        out.close();
    }
    cout << endl;
    techTable_.clear();
}

void CompanyInfo::set_techFile_title(ofstream &out, int techPerid, int techIndex)
{
    string fileName = paths_.techOuputPaths_[techIndex] + companyName_ + "_" + info_->allTech_[techIndex];
    if (techPerid < 10)
    {
        out.open(fileName + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100)
    {
        out.open(fileName + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100)
    {
        out.open(fileName + "_" + to_string(techPerid) + ".csv");
    }
}
