#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

//#include "CompanyInfo.h"
//#include "TechTable.h"
#include "functions.h"

using namespace std;
using namespace chrono;
using namespace filesystem;

class Info {
   public:
    int mode_ = 10;
    string setCompany_ = "AAPL";  //AAPL to JPM, KO to ^NYA
    string setWindow_ = "all";

    vector<int> techIndexs_ = {0};
    bool mixedTech_ = false;
    int techIndex_;
    vector<string> allTech_ = {"SMA", "WMA", "EMA", "RSI"};
    string techType_;

    int algoIndex_ = 2;
    vector<string> allAlgo_ = {"QTS", "GQTS", "GNQTS", "KNQTS"};
    string algoType_;

    double delta_ = 0.0002;
    int expNum_ = 50;
    int genNum_ = 10000;
    int particleNum_ = 10;
    double totalCapitalLV_ = 10000000;

    bool debug_ = false;
    
    int testDeltaLoop_ = 0;
    double testDeltaGap_ = 0.00001;
    double multiplyUp_ = 1.01;
    double multiplyDown_ = 0.99;
    int compareMode_ = 0;

    string testStartYear_ = "2012-01";
    string testEndYear_ = "2021-01";
    double testLength_;

    path pricePath_ = "price";
    string rootFolder_ = "result";

    vector<string> slidingWindows_ = {"A2A", "YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2" /* , "4D4" */, "4D3", "4D2" /* , "3D3" */, "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    vector<string> slidingWindowsEx_ = {"A2A", "36M36", "36M24", "36M18", "36M12", "36M6", "36M3", "36M1", "24M24", "24M18", "24M12", "24M6", "24M3", "24M1", "18M18", "18M12", "18M6", "18M3", "18M1", "12M12", "12M6", "12M3", "12M1", "6M6", "6M3", "6M1", "3M3", "3M1", "1M1", "6M", "3M", "1M", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2" /* , "4D4" */, "4D3", "4D2" /* , "3D3" */, "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    map<string, string> slidingWindowPairs = {};

    int windowNumber_;

    void set_techIndex_and_techType() {
        sort(techIndexs_.begin(), techIndexs_.end());
        vector<string> mixTech = cut_string(techType_, '_');
        for (auto &techIndex : techIndexs_) {
            techType_ += allTech_[techIndex] + "_";
        }
        techType_.pop_back();
        algoType_ = allAlgo_[algoIndex_];
        testLength_ = stod(testEndYear_) - stod(testStartYear_);
        if (techIndexs_.size() == 1) {
            techIndex_ = techIndexs_[0];
            mixedTech_ = false;
        }
        else {
            techIndex_ = (int)allTech_.size();
            allTech_.push_back(techType_);
            mixedTech_ = true;
        }
    }

    void set_slidingWindow() {
        if (setWindow_ != "all") {
            size_t startIndex = find_index_of_string_in_vec(slidingWindows_, cut_string(setWindow_).front());
            size_t endIndex = find_index_of_string_in_vec(slidingWindows_, cut_string(setWindow_).back()) + 1;
            slidingWindows_ = set_certain_range_of_vec(setWindow_, slidingWindows_);
            vector<string> tmp(slidingWindowsEx_.begin() + startIndex, slidingWindowsEx_.begin() + endIndex);
            slidingWindowsEx_ = tmp;
        }
    }

    Info() {
        set_techIndex_and_techType();
        set_slidingWindow();
        windowNumber_ = (int)slidingWindows_.size();
        for (size_t i = 0; i < windowNumber_; i++) {
            slidingWindowPairs.insert({slidingWindows_[i], slidingWindowsEx_[i]});
        }
    }
} _info;

class CompanyInfo {
   public:
    class Path {
       public:
        vector<string> techOuputPaths_;

        vector<string> resultOutputPaths_;
        vector<string> companyRootPaths_;

        vector<string> trainFilePaths_;
        vector<string> testFilePaths_;

        vector<string> trainTraditionFilePaths_;
        vector<string> testTraditionFilePaths_;

        vector<string> trainHoldFilePaths_;
        vector<string> testHoldFilePaths_;

        vector<string> trainTraditionHoldFilePaths_;
        vector<string> testTraditionHoldFilePaths_;

        vector<string> trainBestHold_;
        vector<string> testBestHold_;

        vector<string> trainTraditionBestHold_;
        vector<string> testTraditionBestHold_;
    };
    Info &info_;
    string companyName_;
    Path paths_;

    int totalDays_;
    vector<string> date_;
    vector<double> price_;
    int testStartRow_ = -1;
    int testEndRow_ = -1;
    vector<vector<double>> techTable_;
    int tableStartRow_ = -1;

    vector<void (CompanyInfo::*)(vector<double> &)> cal_tech_{&CompanyInfo::cal_SMA, &CompanyInfo::cal_WMA, &CompanyInfo::cal_EMA, &CompanyInfo::cal_RSI};  // outside the class (company.*(company.cal_tech_[company.info_.techIndex_]))(tmp);

    void set_paths(Path &paths);
    void store_date_price(path priceFilePath);
    void create_folder(Path &paths);
    void find_table_start_row();
    void store_tech_to_vector();
    void cal_SMA(vector<double> &tmp);
    void cal_WMA(vector<double> &tmp);
    void cal_EMA(vector<double> &tmp);
    void cal_RSI(vector<double> &tmp);
    void output_Tech();
    void set_techFile_title(ofstream &out, int techPerid);

    CompanyInfo(path pricePath, Info &info);
};

CompanyInfo::CompanyInfo(path pricePath, Info &info) : companyName_(pricePath.stem().string()), info_(info) {
    set_paths(paths_);
    store_date_price(pricePath);
    create_folder(paths_);
    find_table_start_row();
    cout << companyName_ << endl;
}

void CompanyInfo::set_paths(Path &paths) {
    for (auto tech : info_.allTech_) {
        paths.techOuputPaths_.push_back("tech/" + tech + "/" + companyName_ + "/");

        paths.resultOutputPaths_.push_back(info_.rootFolder_ + "/" + tech + "_result/");

        string companyRootFolder = info_.rootFolder_ + "/" + tech + "_result/" + companyName_ + "/";
        paths.companyRootPaths_.push_back(companyRootFolder);

        paths.trainFilePaths_.push_back(companyRootFolder + "train/");
        paths.testFilePaths_.push_back(companyRootFolder + "test/");

        paths.trainTraditionFilePaths_.push_back(companyRootFolder + "trainTradition/");
        paths.testTraditionFilePaths_.push_back(companyRootFolder + "testTradition/");

        paths.trainHoldFilePaths_.push_back(companyRootFolder + "trainHold/");
        paths.testHoldFilePaths_.push_back(companyRootFolder + "testHold/");

        paths.trainTraditionHoldFilePaths_.push_back(companyRootFolder + "trainTraditionHold/");
        paths.testTraditionHoldFilePaths_.push_back(companyRootFolder + "testTraditionHold/");

        paths.trainBestHold_.push_back(companyRootFolder + "trainBestHold/");
        paths.testBestHold_.push_back(companyRootFolder + "testBestHold/");

        paths.trainTraditionBestHold_.push_back(companyRootFolder + "trainTraditionBestHold/");
        paths.testTraditionBestHold_.push_back(companyRootFolder + "testTraditionBestHold/");
    }
    if (info_.debug_) {
        paths.trainFilePaths_[info_.techIndex_].clear();
    }
}

void CompanyInfo::store_date_price(path priceFilePath) {
    vector<vector<string>> priceFile = read_data(priceFilePath);
    totalDays_ = (int)priceFile.size() - 1;
    date_.resize(totalDays_);
    price_.resize(totalDays_);
    for (int i = 1, j = 0; i <= totalDays_; i++) {
        date_[i - 1] = priceFile[i][0];
        if (!is_double(priceFile[i][4])) {
            price_[i - 1] = price_[i - 2];
        }
        else {
            price_[i - 1] = stod(priceFile[i][4]);
        }
        if (j == 0 && date_[i - 1].substr(0, 7) == info_.testStartYear_) {
            testStartRow_ = i - 1;
            j++;
        }
        else if (j == 1 && date_[i - 1].substr(0, 7) == info_.testEndYear_) {
            testEndRow_ = i - 2;
            j++;
        }
    }
}

void CompanyInfo::create_folder(Path &paths) {
    if (!info_.mixedTech_) {
        create_directories(paths.techOuputPaths_[info_.techIndex_]);
    }
    // create_directories(paths.trainHoldFilePaths_[info_.techIndex_]);
    create_directories(paths.testHoldFilePaths_[info_.techIndex_]);
    // create_directories(paths.trainTraditionHoldFilePaths_[info_.techIndex_]);
    create_directories(paths.testTraditionHoldFilePaths_[info_.techIndex_]);
    // create_directories(paths.trainBestHold_[info_.techIndex_]);
    // create_directories(paths.testBestHold_[info_.techIndex_]);
    // create_directories(paths.trainTraditionBestHold_[info_.techIndex_]);
    // create_directories(paths.testTraditionBestHold_[info_.techIndex_]);
    for (auto i : info_.slidingWindows_) {
        create_directories(paths.trainFilePaths_[info_.techIndex_] + i);
        create_directories(paths.testFilePaths_[info_.techIndex_] + i);
        create_directories(paths.trainTraditionFilePaths_[info_.techIndex_] + i);
        create_directories(paths.testTraditionFilePaths_[info_.techIndex_] + i);
    }
}

void CompanyInfo::find_table_start_row() {
    char delimiter;
    int longestTrainMonth = -1;
    for (int i = 0; i < info_.windowNumber_; i++) {
        vector<int> trainTest = find_train_and_test_len(info_.slidingWindowsEx_[i], delimiter);
        int trainMonth;
        if (trainTest.size() == 1) {
            trainMonth = 12;
        }
        else {
            trainMonth = trainTest[0];
        }
        if (delimiter == 'M' && trainMonth > longestTrainMonth) {
            longestTrainMonth = trainMonth;
        }
    }
    if (longestTrainMonth == -1) {
        longestTrainMonth = 12;
    }
    for (int i = testStartRow_ - 1, monthCount = 0; i >= 0; i--) {
        if (date_[i].substr(5, 2) != date_[i - 1].substr(5, 2)) {
            monthCount++;
            if (monthCount == longestTrainMonth) {
                tableStartRow_ = i - 20;
                break;
            }
        }
    }
    if (tableStartRow_ == -1) {
        cout << "can't find longestTrainRow_" << endl;
        exit(1);
    }
}

void CompanyInfo::store_tech_to_vector() {
    cout << "calculating " << companyName_ << " " << info_.techType_ << endl;
    vector<double> tmp;
    techTable_.push_back(tmp);
    (this->*cal_tech_[info_.techIndex_])(tmp);  // or (*this.*cal_tech_[info_.techIndex_])(tmp);
    cout << "done calculating" << endl;
}

void CompanyInfo::cal_SMA(vector<double> &tmp) {
    for (int MA = 1; MA < 257; MA++) {
        for (int dateRow = MA - 1; dateRow < totalDays_; dateRow++) {
            double MARangePriceSum = 0;
            for (int i = dateRow, j = MA; j > 0; i--, j--) {
                MARangePriceSum += price_[i];
            }
            tmp.push_back(MARangePriceSum / MA);
        }
        techTable_.push_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_WMA(vector<double> &tmp) {
    for (int WMA = 1; WMA < 257; WMA++) {
        for (int dateRow = WMA - 1; dateRow < totalDays_; dateRow++) {
            double weightedPriceSum = 0;
            int totalWeight = 0;
            for (int weight = WMA, tmpDateRow = dateRow; weight > 0; weight--, tmpDateRow--) {
                weightedPriceSum += price_[tmpDateRow] * weight;
                totalWeight += weight;
            }
            tmp.push_back(weightedPriceSum / totalWeight);
        }
        techTable_.push_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_EMA(vector<double> &tmp) {
}

void CompanyInfo::cal_RSI(vector<double> &tmp) {
    vector<double> priceGainLoss(totalDays_ - 1);
    for (int priceDateRow = 1; priceDateRow < totalDays_; priceDateRow++) {
        priceGainLoss[priceDateRow - 1] = price_[priceDateRow] - price_[priceDateRow - 1];
    }
    for (int RSIPeriod = 1; RSIPeriod < 257; RSIPeriod++) {
        double RSI, gain = 0, loss = 0, avgGain = 0, avgLoss = 0;
        for (int row = 0; row < RSIPeriod; row++) {
            if (priceGainLoss[row] >= 0) {
                gain += priceGainLoss[row];
            }
            else {
                loss += -priceGainLoss[row];
            }
        }
        avgGain = gain / RSIPeriod;
        avgLoss = loss / RSIPeriod;
        RSI = 100.0 - (100.0 / (1 + (avgGain / avgLoss)));
        tmp.push_back(RSI);
        double preAvgGain = avgGain, preAvgLoss = avgLoss;
        for (int i = RSIPeriod; i < totalDays_ - 1; i++) {
            if (priceGainLoss[i] >= 0) {
                RSI = 100.0 - (100.0 / (1 + (((preAvgGain * (RSIPeriod - 1) + priceGainLoss[i]) / (preAvgLoss * (RSIPeriod - 1))))));
                preAvgGain = (preAvgGain * (RSIPeriod - 1) + priceGainLoss[i]) / RSIPeriod;
                preAvgLoss = (preAvgLoss * (RSIPeriod - 1)) / RSIPeriod;
            }
            else {
                RSI = 100.0 - (100.0 / (1 + ((preAvgGain * (RSIPeriod - 1)) / (preAvgLoss * (RSIPeriod - 1) - priceGainLoss[i]))));
                preAvgGain = (preAvgGain * (RSIPeriod - 1)) / RSIPeriod;
                preAvgLoss = (preAvgLoss * (RSIPeriod - 1) - priceGainLoss[i]) / RSIPeriod;
            }
            if (isnan(RSI)) {
                RSI = 100;
            }
            tmp.push_back(RSI);
        }
        techTable_.push_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::output_Tech() {
    store_tech_to_vector();
    cout << "saving " << info_.techType_ << " file" << endl;
    for (int techPeriod = 1; techPeriod < 257; techPeriod++) {
        if (techPeriod % 10 == 0) {
            cout << ".";
        }
        ofstream out;
        set_techFile_title(out, techPeriod);
        int techSize = (int)techTable_[techPeriod].size();
        int dateRow = 0;
        switch (info_.techIndex_) {
            case 0:
            case 1:
            case 2: {
                dateRow = techPeriod - 1;
                break;
            }
            case 3: {
                dateRow = techPeriod;
                break;
            }
            default: {
                cout << "output_tech exception" << endl;
                exit(1);
                break;
            }
        }
        for (int i = 0; i < techSize; i++, dateRow++) {
            out << date_[dateRow] << "," << set_precision(techTable_[techPeriod][i]) << endl;
        }
        out.close();
    }
    cout << endl;
}

void CompanyInfo::set_techFile_title(ofstream &out, int techPerid) {
    if (techPerid < 10) {
        out.open(paths_.techOuputPaths_[info_.techIndex_] + companyName_ + "_" + info_.techType_ + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100) {
        out.open(paths_.techOuputPaths_[info_.techIndex_] + companyName_ + "_" + info_.techType_ + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100) {
        out.open(paths_.techOuputPaths_[info_.techIndex_] + companyName_ + "_" + info_.techType_ + "_" + to_string(techPerid) + ".csv");
    }
}

class TechTable {
   private:
    CompanyInfo *company_;

   public:
    int techIndex_;
    string techType_;
    int days_;
    vector<string> date_;
    vector<double> price_;
    vector<vector<double>> techTable_;

    void create_techTable();
    void ask_generate_tech_file();
    void read_techFile(vector<path> &techFilePath, int techFilePathSize);
    void output_techTable();

    TechTable(CompanyInfo *company, int techIndex);
};

TechTable::TechTable(CompanyInfo *company, int techIndex) : company_(company), techIndex_(techIndex), techType_(company->info_.allTech_[techIndex]), days_(company->totalDays_ - company->tableStartRow_) {
    create_techTable();
}

void TechTable::create_techTable() {
    vector<path> techFilePath = get_path(company_->paths_.techOuputPaths_[techIndex_]);
    int techFilePathSize = (int)techFilePath.size();
    while (techFilePathSize == 0) {
        cout << "no " << techType_ << " tech file" << endl;
        ask_generate_tech_file();
        techFilePath = get_path(company_->paths_.techOuputPaths_[techIndex_]);
        techFilePathSize = (int)techFilePath.size();
    }
    if ((int)read_data(techFilePath.back()).size() - days_ < 0) {
        company_->tableStartRow_ = find_index_of_string_in_vec(company_->date_, read_data(techFilePath.back())[0][0]);
        days_ = company_->totalDays_ - company_->tableStartRow_;
    }
    date_.resize(days_);
    price_.resize(days_);
    for (int i = company_->tableStartRow_, j = 0; i < company_->totalDays_; i++, j++) {
        date_[j] = company_->date_[i];
        price_[j] = company_->price_[i];
    }
    techTable_.resize(days_);
    for (int i = 0; i < days_; i++) {
        techTable_[i].resize(257);
    }
    read_techFile(techFilePath, techFilePathSize);
    cout << endl;
}

void TechTable::ask_generate_tech_file() {
    cout << "generate new tech file now?\n1.y\n2.n" << endl;
    char choose;
    cin >> choose;
    switch (choose) {
        case 'y': {
            company_->output_Tech();
            break;
        }
        case 'n': {
            cout << "exit program" << endl;
            exit(1);
            break;
        }
        default: {
            cout << "wrong choise" << endl;
        }
    }
}

void TechTable::read_techFile(vector<path> &techFilePath, int techFilePathSize) {
    cout << "reading " << techType_ << " files";
    for (int i = 0; i < techFilePathSize; i++) {
        if (i % 16 == 0) {
            cout << ".";
        }
        vector<vector<string>> techFile = read_data(techFilePath[i]);
        int techFileSize = (int)techFile.size();
        if (i == 0 && techFile.back()[0] != date_.back()) {
            cout << "last date of price file and techFile are different, need to generate new techFile" << endl;
            ask_generate_tech_file();
            i--;
            continue;
        }
        for (int j = 0, k = techFileSize - days_; k < techFileSize; j++, k++) {
            techTable_[j][i + 1] = stod(techFile[k][1]);
        }
    }
}

void TechTable::output_techTable() {
    ofstream out;
    out.open(company_->companyName_ + "_" + company_->info_.techType_ + "_table.csv");
    for (int i = 1; i < 257; i++) {
        out << "," << i;
    }
    out << endl;
    for (int i = 0; i < days_; i++) {
        out << date_[i] << ",";
        for (int j = 1; j < 257; j++) {
            out << set_precision(techTable_[i][j]) << ",";
        }
        out << endl;
    }
    out.close();
}

class TechTables {
    vector<TechTable> tables_;
    int tableSize_;
};

class TestWindow {
   protected:
    CompanyInfo &company_;

   public:
    string windowName_;
    string windowNameEx_;
    int tableStartRow_ = -1;
    int trainLength_ = -1;
    int testLength_ = -1;
    char windowType_;
    vector<int> interval_;
    int intervalSize_;

    void find_test_interval();
    void find_M_test();
    void find_W_test();
    void find_D_test();
    void print_test();

    TestWindow(CompanyInfo &company, string window);
};

TestWindow::TestWindow(CompanyInfo &company, string window) : company_(company), windowName_(window), windowNameEx_(company.info_.slidingWindowPairs.at(window)), tableStartRow_(company.tableStartRow_) {
    if (windowName_ != "A2A") {
        find_test_interval();
    }
    else {
        interval_.push_back(company_.testStartRow_);
        interval_.push_back(company_.testEndRow_);
    }
    for (auto &i : interval_) {
        i -= tableStartRow_;
    }
    intervalSize_ = (int)interval_.size();
}

void TestWindow::find_test_interval() {
    vector<int> trainTestType = find_train_and_test_len(windowNameEx_, windowType_);
    if (trainTestType.size() == 0) {
        cout << "testType.size() cant be 0" << endl;
        exit(1);
    }
    if (trainTestType.size() == 1) {
        trainLength_ = trainTestType[0];
        testLength_ = trainLength_;
        windowType_ = 'S';
    }
    else {
        trainLength_ = trainTestType[0];
        testLength_ = trainTestType[1];
    }
    if (testLength_ == -1) {
        cout << "cant find testLength" << endl;
        exit(1);
    }
    switch (windowType_) {
        case 'M':
        case 'S': {
            find_M_test();
            break;
        }
        case 'W': {
            find_W_test();
            break;
        }
        case 'D': {
            find_D_test();
            break;
        }
        default: {
            cout << "test delimiter wrong" << endl;
            exit(1);
        }
    }
}

void TestWindow::find_M_test() {
    vector<int> startRow, endRow;
    for (int dateRow = company_.testStartRow_, monthCnt = testLength_ - 1; dateRow <= company_.testEndRow_; dateRow++) {
        if (company_.date_[dateRow - 1].substr(5, 2) != company_.date_[dateRow].substr(5, 2)) {
            monthCnt++;
            if (monthCnt == testLength_) {
                startRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    for (int dateRow = company_.testStartRow_, monthCnt = 0; dateRow <= company_.testEndRow_; dateRow++) {
        if (company_.date_[dateRow].substr(5, 2) != company_.date_[dateRow + 1].substr(5, 2)) {
            monthCnt++;
            if (monthCnt == testLength_ || dateRow == company_.testEndRow_) {
                endRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::find_W_test() {
    vector<int> startRow, endRow;
    int smallWeekDay = -1, bigWeekDay = -1;
    for (int dateRow = company_.testStartRow_, monthCnt = testLength_ - 1; dateRow < company_.testEndRow_; dateRow++) {
        smallWeekDay = cal_weekday(company_.date_[dateRow - 1]);
        bigWeekDay = cal_weekday(company_.date_[dateRow]);
        if (is_week_changed(company_.date_, bigWeekDay, smallWeekDay, dateRow, dateRow - 1)) {
            monthCnt++;
            if (monthCnt == testLength_) {
                startRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    for (int dateRow = company_.testStartRow_, monthCnt = 0; dateRow <= company_.testEndRow_; dateRow++) {
        smallWeekDay = cal_weekday(company_.date_[dateRow]);
        bigWeekDay = cal_weekday(company_.date_[dateRow + 1]);
        if (is_week_changed(company_.date_, bigWeekDay, smallWeekDay, dateRow + 1, dateRow)) {
            monthCnt++;
            if (monthCnt == testLength_ || dateRow == company_.testEndRow_) {
                endRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::find_D_test() {
    vector<int> startRow, endRow;
    for (int dateRow = company_.testStartRow_; dateRow <= company_.testEndRow_; dateRow += testLength_) {
        startRow.push_back(dateRow);
    }
    for (int dateRow = company_.testStartRow_ + testLength_ - 1; dateRow <= company_.testEndRow_; dateRow += testLength_) {
        endRow.push_back(dateRow);
    }
    if (startRow.size() > endRow.size()) {
        endRow.push_back(company_.testEndRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::print_test() {
    cout << "test window: " << windowName_ << "=" << windowNameEx_ << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company_.date_[*it + tableStartRow_] << "~" << company_.date_[*(++it) + tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class TrainWindow : public TestWindow {
   public:
    vector<int> interval_;

    void find_train_interval();
    void find_M_train();
    void find_regular_M_train(vector<int> &endRow, vector<int> &startRow);
    void find_star_train(vector<int> &endRow, vector<int> &startRow);
    void find_W_train();
    void find_D_train();
    void print_train();

    TrainWindow(CompanyInfo &company, string window);
};

TrainWindow::TrainWindow(CompanyInfo &company, string window) : TestWindow(company, window) {
    if (TestWindow::windowName_ != "A2A") {
        find_train_interval();
        for (auto &i : interval_) {
            i -= tableStartRow_;
        }
    }
    else {
        interval_ = TestWindow::interval_;
    }
}

void TrainWindow::find_train_interval() {
    switch (windowType_) {
        case 'M':
        case 'S': {
            find_M_train();
            break;
        }
        case 'W': {
            find_W_train();
            break;
        }
        case 'D': {
            find_D_train();
            break;
        }
        default: {
            cout << "trainWindow exception" << endl;
            exit(1);
        }
    }
}

void TrainWindow::find_M_train() {
    vector<int> startRow, endRow;
    switch (windowType_) {
        case 'M': {
            find_regular_M_train(endRow, startRow);
            break;
        }
        case 'S': {
            find_star_train(endRow, startRow);
            break;
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::find_regular_M_train(vector<int> &endRow, vector<int> &startRow) {
    for (int intervalIndex = 0; intervalIndex < intervalSize_; intervalIndex += 2) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + tableStartRow_, monthCnt = 0; dateRow > 0; dateRow--) {
            if (company_.date_[dateRow].substr(5, 2) != company_.date_[dateRow - 1].substr(5, 2)) {
                monthCnt++;
                if (monthCnt == trainLength_) {
                    startRow.push_back(dateRow);
                    break;
                }
            }
        }
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + tableStartRow_);
    }
}

void TrainWindow::find_star_train(vector<int> &endRow, vector<int> &startRow) {
    for (int intervalIndex = 0; intervalIndex < intervalSize_; intervalIndex++) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + tableStartRow_, monthCnt = 0; dateRow > 0; dateRow--) {
            if (company_.date_[dateRow].substr(5, 2) != company_.date_[dateRow - 1].substr(5, 2)) {
                monthCnt++;
                if (monthCnt == 12 && intervalIndex % 2 == 0) {
                    startRow.push_back(dateRow);
                    break;
                }
                else if (monthCnt == 12 && intervalIndex % 2 == 1) {
                    endRow.push_back(dateRow - 1);
                    break;
                }
            }
        }
    }
}

void TrainWindow::find_W_train() {
    vector<int> startRow, endRow;
    int smallWeekDay = -1, bigWeekDay = -1;
    for (int intervalIndex = 0; intervalIndex < intervalSize_; intervalIndex += 2) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + tableStartRow_, weekCnt = 0; dateRow > 0; dateRow--) {
            smallWeekDay = cal_weekday(company_.date_[dateRow - 1]);
            bigWeekDay = cal_weekday(company_.date_[dateRow]);
            if (is_week_changed(company_.date_, bigWeekDay, smallWeekDay, dateRow, dateRow - 1)) {
                weekCnt++;
                if (weekCnt == trainLength_) {
                    startRow.push_back(dateRow);
                    break;
                }
            }
        }
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + tableStartRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::find_D_train() {
    vector<int> startRow, endRow;
    for (int intervalIndex = 0; intervalIndex < intervalSize_; intervalIndex += 2) {
        startRow.push_back(TestWindow::interval_[intervalIndex] - trainLength_ + tableStartRow_);
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + tableStartRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::print_train() {
    cout << "train window: " << windowName_ << "=" << windowNameEx_ << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company_.date_[*it + tableStartRow_] << "~" << company_.date_[*(++it) + tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class MA {
   public:
    static const inline vector<int> eachVariableBitsNum_ = {8, 8, 8, 8};
    static const inline vector<vector<int>> traditionStrategy_ = {{5, 10, 5, 10}, {5, 20, 5, 20}, {5, 60, 5, 60}, {10, 20, 10, 20}, {10, 60, 10, 60}, {20, 60, 20, 60}, {120, 240, 120, 240}};

    static bool buy_condition0(vector<TechTable> *tables, vector<int> &decimal, int i) {
        double MAbuy1PreDay = (*tables)[0].techTable_[i - 1][decimal[0]];
        double MAbuy2PreDay = (*tables)[0].techTable_[i - 1][decimal[1]];
        double MAbuy1Today = (*tables)[0].techTable_[i][decimal[0]];
        double MAbuy2Today = (*tables)[0].techTable_[i][decimal[1]];
        return MAbuy1PreDay <= MAbuy2PreDay && MAbuy1Today > MAbuy2Today;
    }

    static bool sell_condition0(vector<TechTable> *tables, vector<int> &decimal, int i) {
        double MAsell1PreDay = (*tables)[0].techTable_[i - 1][decimal[2]];
        double MAsell2PreDay = (*tables)[0].techTable_[i - 1][decimal[3]];
        double MAsell1Today = (*tables)[0].techTable_[i][decimal[2]];
        double MAsell2Today = (*tables)[0].techTable_[i][decimal[3]];
        return MAsell1PreDay >= MAsell2PreDay && MAsell1Today < MAsell2Today;
    }
};

class RSI {
   public:
    static const inline vector<int> eachVariableBitsNum_ = {8, 7, 7};
    static const inline vector<vector<int>> traditionStrategy_ = {{5, 20, 80}, {5, 30, 70}, {6, 20, 80}, {6, 30, 70}, {14, 20, 80}, {14, 30, 70}};

    static bool buy_condition0(vector<TechTable> *tables, vector<int> &decimal, int i) {
        double RSI = (*tables)[0].techTable_[i][decimal[0]];
        return RSI <= decimal[1];
    }

    static bool sell_condition0(vector<TechTable> *tables, vector<int> &decimal, int i) {
        double RSI = (*tables)[0].techTable_[i][decimal[0]];
        return RSI >= decimal[2];
    }
};

class BetaMatrix {
   public:
    int variableNum_ = 0;
    vector<int> eachVariableBitsNum_;
    int bitsNum_ = 0;
    vector<double> matrix_;

    void reset();
    void print(ostream &out);
};

void BetaMatrix::reset() {
    fill(matrix_.begin(), matrix_.end(), 0.5);
}

void BetaMatrix::print(ostream &out = cout) {
    out << "beta matrix" << endl;
    for (int variableIndex = 0, bitIndex = 0; variableIndex < variableNum_; variableIndex++) {
        for (int fakeBitIndex = 0; fakeBitIndex < eachVariableBitsNum_[variableIndex]; fakeBitIndex++, bitIndex++) {
            out << matrix_[bitIndex] << ",";
        }
        out << " ,";
    }
    out << endl;
}

class Particle {
   private:
    CompanyInfo *company_;

   public:
    int techIndex_;
    string techType_;

    vector<int> eachVariableBitsNum_;
    int bitsNum_ = 0;
    vector<int> binary_;
    int variableNum_ = 0;
    vector<int> decimal_;

    vector<TechTable> *tables_ = nullptr;

    double remain_ = 0;
    double RoR_ = 0;
    int buyNum_ = 0;
    int sellNum_ = 0;
    vector<string> tradeRecord_;
    int gen_ = 0;
    int exp_ = 0;
    int bestCnt_ = 0;
    bool isRecordOn_ = false;

    double actualDelta_ = -1;

    typedef vector<bool (*)(vector<TechTable> *, vector<int> &, int)> buy_sell;
    buy_sell buy{&MA::buy_condition0, &MA::buy_condition0, &MA::buy_condition0, &RSI::buy_condition0};
    buy_sell sell{&MA::sell_condition0, &MA::sell_condition0, &MA::sell_condition0, &RSI::sell_condition0};

    vector<vector<int>> allTechEachVariableBitsNum_{MA::eachVariableBitsNum_, MA::eachVariableBitsNum_, MA::eachVariableBitsNum_, RSI::eachVariableBitsNum_};

    void instant_trade(string startDate, string endDate, bool hold = false);
    void push_holdInfo_column_Name(bool hold, vector<string> &holdInfo, vector<string> *&holdInfoPtr);
    string set_title_variables();
    void set_instant_trade_file(ofstream &out, const string &startDate, const string &endDate);
    void print_trade_record(ofstream &out);
    void print_instant_trade_holdInfo(bool hold, const vector<string> &holdInfo, const string &startDate, const string &endDate);
    void ini_buyNum_sellNum();
    void trade(int startRow, int endRow, bool lastRecord = false, vector<string> *holdInfoPtr = nullptr);
    void set_buy_sell_condition(bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow);
    void push_holdInfo_date_price(vector<string> *holdInfoPtr, int i);
    void push_tradeInfo_column_name();
    void push_tradeInfo_buy(int stockHold, int i);
    void push_holdInfo_buy(vector<string> *holdInfoPtr, int i);
    void push_extra_techInfo(int i, string &push);
    void push_tradeInfo_sell(int stockHold, int i);
    void push_holdInfo_sell(int endRow, vector<string> *holdInfoPtr, int i);
    void push_holdInfo_holding(vector<string> *holdInfoPtr, int i);
    void push_holdInfo_not_holding(vector<string> *holdInfoPtr, int i);
    void push_tradeInfo_last(bool lastRecord);
    void check_buyNum_sellNum();
    void reset(double RoR = 0);
    void measure(BetaMatrix &betaMatrix);
    void convert_bi_dec();
    void print(ostream &out);
    void print_train_test_data(string windowName, string outputPath, int actualStartRow, int actualEndRow, vector<string> *holdInfoPtr = nullptr, vector<vector<string>> *trainFile = nullptr);
    string set_output_filePath(string windowName, string &outputPath, int actualStartRow, int actualEndRow);

    Particle(CompanyInfo *company, int techIndex_, bool isRecordOn = false, vector<int> variables = {});
};

Particle::Particle(CompanyInfo *company, int techIndex, bool isRecordOn, vector<int> variables) : company_(company), techIndex_(techIndex), techType_(company_->info_.techType_), remain_(company->info_.totalCapitalLV_), isRecordOn_(isRecordOn) {
    eachVariableBitsNum_ = allTechEachVariableBitsNum_[techIndex_];
    bitsNum_ = accumulate(eachVariableBitsNum_.begin(), eachVariableBitsNum_.end(), 0);
    binary_.resize(bitsNum_);
    variableNum_ = (int)eachVariableBitsNum_.size();
    decimal_.resize(variableNum_);
    for (int i = 0; i < variables.size(); i++) {
        decimal_[i] = variables[i];
    }
}

void Particle::instant_trade(string startDate, string endDate, bool hold) {
    vector<TechTable> tmp = {TechTable(company_, techIndex_)};
    tables_ = &tmp;
    int startRow = -1, endRow = -1;
    startRow = find_index_of_string_in_vec((*tables_)[0].date_, startDate);
    endRow = find_index_of_string_in_vec((*tables_)[0].date_, endDate);
    vector<string> holdInfo;
    vector<string> *holdInfoPtr = nullptr;
    push_holdInfo_column_Name(hold, holdInfo, holdInfoPtr);
    trade(startRow, endRow, true, holdInfoPtr);
    ofstream out;
    set_instant_trade_file(out, startDate, endDate);
    print_trade_record(out);
    out.close();
    print_instant_trade_holdInfo(hold, holdInfo, startDate, endDate);
}

void Particle::push_holdInfo_column_Name(bool hold, vector<string> &holdInfo, vector<string> *&holdInfoPtr) {
    if (hold) {
        holdInfo.clear();
        holdInfo.push_back("Date,Price,Hold,buy,sell date,sell " + techType_ + ",");
        if (company_->info_.techIndex_ == techIndex_) {
            switch (techIndex_) {
                case 0:
                case 1:
                case 2: {
                    holdInfo.push_back("buy 1,buy 2,sell 1,sell 2\n");
                    break;
                }
                case 3: {
                    holdInfo.push_back("RSI,overSold,overbought\n");
                    break;
                }
            }
        }
        else {
            holdInfo.push_back("\n");
        }
        holdInfoPtr = &holdInfo;
    }
}

void Particle::set_instant_trade_file(ofstream &out, const string &startDate, const string &endDate) {
    string titleVariables = set_title_variables();
    string showVariablesInFile;
    for (auto i : decimal_) {
        showVariablesInFile += ",";
        showVariablesInFile += to_string(i);
    }
    out.open(company_->companyName_ + "_" + techType_ + titleVariables + "_instantTrade_" + startDate + "_" + endDate + ".csv");
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            out << "company,startDate,endDate,buy1,buy2,sell1,sell2" << endl;
            break;
        }
        case 3: {
            out << "company,startDate,endDate,period,overSold,overBought" << endl;
            break;
        }
        default: {
            cout << "set_instant_trade_file exception" << endl;
            exit(1);
        }
    }
    out << company_->companyName_ << "," << startDate << "," << endDate << showVariablesInFile << "\n\n";
}

string Particle::set_title_variables() {
    string titleVariables;
    for (auto i : decimal_) {
        titleVariables += "_";
        titleVariables += to_string(i);
    }
    return titleVariables;
}

void Particle::print_trade_record(ofstream &out) {
    for (auto record : tradeRecord_) {
        out << record;
    }
}

void Particle::print_instant_trade_holdInfo(bool hold, const vector<string> &holdInfo, const string &startDate, const string &endDate) {
    if (hold) {
        string titleVariables = set_title_variables();
        ofstream holdFile(company_->companyName_ + "_" + company_->info_.techType_ + titleVariables + "_hold_" + startDate + "_" + endDate + ".csv");
        for (auto info : holdInfo) {
            holdFile << info;
        }
        holdFile.close();
    }
}

void Particle::ini_buyNum_sellNum() {
    if (buyNum_ != 0 || sellNum_ != 0) {
        buyNum_ = 0;
        sellNum_ = 0;
    }
}

void Particle::trade(int startRow, int endRow, bool lastRecord, vector<string> *holdInfoPtr) {
    int stockHold = 0;
    push_tradeInfo_column_name();
    ini_buyNum_sellNum();
    bool buyCondition = false;
    bool sellCondition = false;
    for (int i = startRow; i <= endRow; i++) {
        set_buy_sell_condition(buyCondition, sellCondition, stockHold, i, endRow);
        push_holdInfo_date_price(holdInfoPtr, i);
        if (buyCondition) {
            stockHold = floor(remain_ / (*tables_)[0].price_[i]);
            remain_ = remain_ - stockHold * (*tables_)[0].price_[i];
            buyNum_++;
            push_tradeInfo_buy(stockHold, i);
            push_holdInfo_buy(holdInfoPtr, i);
        }
        else if (sellCondition) {
            remain_ = remain_ + (double)stockHold * (*tables_)[0].price_[i];
            stockHold = 0;
            sellNum_++;
            push_tradeInfo_sell(stockHold, i);
            push_holdInfo_sell(endRow, holdInfoPtr, i);
        }
        else if (holdInfoPtr != nullptr && stockHold != 0) {
            push_holdInfo_holding(holdInfoPtr, i);
        }
        else if (holdInfoPtr != nullptr && stockHold == 0) {
            push_holdInfo_not_holding(holdInfoPtr, i);
        }
    }
    check_buyNum_sellNum();
    RoR_ = (remain_ - company_->info_.totalCapitalLV_) / company_->info_.totalCapitalLV_ * 100.0;
    push_tradeInfo_last(lastRecord);
}

// bool RSIhold = true;
// bool MAhold = false;
// int buy1 = 10;
// int buy2 = 20;
// int sell1 = 10;
// int sell2 = 20;
void Particle::set_buy_sell_condition(bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow) {
    buyCondition = !stockHold && (*buy[techIndex_])(tables_, decimal_, i) && i != endRow;
    sellCondition = stockHold && ((*sell[techIndex_])(tables_, decimal_, i) || i == endRow);

    // 以RSI訓練期的資料為底，測試期的時候再加上MA的條件
    // if ((*tables_).size() == 1) {
    //     buyCondition = !stockHold && (*buy[techIndex_])(tables_, decimal_, i) && i != endRow;
    //     sellCondition = stockHold && ((*sell[techIndex_])(tables_, decimal_, i) || i == endRow);
    // }
    // else {
    //     bool RSIbuy = false;
    //     bool RSIsell = false;
    //     bool MAbuy = false;
    //     bool MAsell = false;
    //     if (!RSIhold) {
    //         RSIbuy = !RSIhold && (*buy[info_.techIndex_])(tables_, decimal_, i) && i != endRow;
    //         if (RSIbuy)
    //             RSIhold = true;
    //     }
    //     if (!MAhold) {
    //         MAbuy = !MAhold && ((*tables_)[1].techTable_[i][buy1] >= (*tables_)[1].techTable_[i][buy2]) && i != endRow;
    //         if (MAbuy)
    //             MAhold = true;
    //     }

    //     RSIsell = RSIhold && ((*sell[info_.techIndex_])(tables_, decimal_, i) || i == endRow);
    //     MAsell = MAhold && ((*tables_)[1].techTable_[i][sell1] <= (*tables_)[1].techTable_[i][sell2] || i == endRow);

    //     buyCondition = !stockHold && RSIhold && MAhold && i != endRow;
    //     sellCondition = stockHold && (RSIsell || MAsell || i == endRow);

    //     if (RSIsell) {
    //         RSIhold = false;
    //     }
    //     if (MAsell) {
    //         MAhold = false;
    //     }
    // }
}

void Particle::push_holdInfo_date_price(vector<string> *holdInfoPtr, int i) {
    if (holdInfoPtr != nullptr) {
        string push;
        push += (*tables_)[0].date_[i];
        push += ",";
        push += set_precision((*tables_)[0].price_[i]);
        push += ",";
        (*holdInfoPtr).push_back(push);
    }
}

void Particle::push_tradeInfo_column_name() {
    if (isRecordOn_) {
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_.push_back(",date,price,preday 1,preday 2,today 1,today 2,stockHold,remain,capital lv\n");
                break;
            }
            case 3: {
                tradeRecord_.push_back(",date,price,RSI,stockHold,remain,capital lv\n");
                break;
            }
        }
    }
}

void Particle::push_tradeInfo_buy(int stockHold, int i) {
    if (isRecordOn_) {
        string push;
        push += "buy,";
        push += (*tables_)[0].date_[i] + ",";
        push += set_precision((*tables_)[0].price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                push += set_precision((*tables_)[0].techTable_[i - 1][decimal_[0]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i - 1][decimal_[1]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i][decimal_[1]]) + ",";
                break;
            }
            case 3: {
                push += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        push += to_string(stockHold) + ",";
        push += set_precision(remain_) + ",";
        push += set_precision(remain_ + stockHold * (*tables_)[0].price_[i]) + "\n";
        tradeRecord_.push_back(push);
    }
}

void Particle::push_extra_techInfo(int i, string &push) {
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            for (auto variable : decimal_) {
                push += set_precision((*tables_)[0].techTable_[i][variable]);
                push += ",";
            }
            break;
        }
        case 3: {
            push += set_precision((*tables_)[0].techTable_[i][decimal_[0]]);
            push += ",";
            push += to_string(decimal_[1]);
            push += ",";
            push += to_string(decimal_[2]);
            push += ",";
            // push += set_precision((*tables_)[1].techTable_[i][buy1]);
            // push += ",";
            // push += set_precision((*tables_)[1].techTable_[i][buy2]);
            // push += ",";
            // push += set_precision((*tables_)[1].techTable_[i][sell1]);
            // push += ",";
            // push += set_precision((*tables_)[1].techTable_[i][sell2]);
            break;
        }
    }
}

void Particle::push_holdInfo_buy(vector<string> *holdInfoPtr, int i) {
    if (holdInfoPtr != nullptr) {
        string push;
        push += set_precision((*tables_)[0].price_[i]);
        push += ",";
        push += set_precision((*tables_)[0].price_[i]);
        push += ",,,";
        push_extra_techInfo(i, push);
        push += "\n";
        (*holdInfoPtr).push_back(push);
    }
}

void Particle::push_tradeInfo_sell(int stockHold, int i) {
    if (isRecordOn_) {
        string push;
        push += "sell,";
        push += (*tables_)[0].date_[i] + ",";
        push += set_precision((*tables_)[0].price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                push += set_precision((*tables_)[0].techTable_[i - 1][decimal_[2]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i - 1][decimal_[3]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i][decimal_[2]]) + ",";
                push += set_precision((*tables_)[0].techTable_[i][decimal_[3]]) + ",";
                break;
            }
            case 3: {
                push += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        push += to_string(stockHold) + ",";
        push += set_precision(remain_) + ",";
        push += set_precision(remain_ + stockHold * (*tables_)[0].price_[i]) + "\n\n";
        tradeRecord_.push_back(push);
    }
}

void Particle::push_holdInfo_sell(int endRow, vector<string> *holdInfoPtr, int i) {
    if (holdInfoPtr != nullptr) {
        string push;
        push += set_precision((*tables_)[0].price_[i]);
        if (i == endRow) {
            push += ",,";
            push += set_precision((*tables_)[0].price_[i]);
            push += ",,";
        }
        else {
            push += ",,,";
            push += set_precision((*tables_)[0].price_[i]);
            push += ",";
        }
        push_extra_techInfo(i, push);
        push += "\n";
        (*holdInfoPtr).push_back(push);
    }
}

void Particle::push_holdInfo_holding(vector<string> *holdInfoPtr, int i) {
    string push;
    push += set_precision((*tables_)[0].price_[i]);
    push += ",,,,";
    push_extra_techInfo(i, push);
    push += "\n";
    (*holdInfoPtr).push_back(push);
}

void Particle::push_holdInfo_not_holding(vector<string> *holdInfoPtr, int i) {
    string push;
    push += ",,,,";
    push_extra_techInfo(i, push);
    push += "\n";
    (*holdInfoPtr).push_back(push);
}

void Particle::check_buyNum_sellNum() {
    if (buyNum_ != sellNum_) {
        cout << "particle.buyNum_ = " << buyNum_ << ", particle.sellNum_ = " << sellNum_ << endl;
        for (auto &record : tradeRecord_) {
            cout << record << endl;
        }
        exit(1);
    }
}

void Particle::push_tradeInfo_last(bool lastRecord) {
    if (isRecordOn_ && lastRecord) {
        tradeRecord_.push_back("buyNum," + to_string(buyNum_) + ",sellNum," + to_string(sellNum_) + "\nremain," + set_precision(remain_) + "\nreturn rate," + set_precision(RoR_) + "%\n");
    }
}

void Particle::reset(double RoR) {
    fill(binary_.begin(), binary_.end(), 0);
    fill(decimal_.begin(), decimal_.end(), 0);
    buyNum_ = 0;
    sellNum_ = 0;
    remain_ = company_->info_.totalCapitalLV_;
    RoR_ = RoR;
    tradeRecord_.clear();
    gen_ = 0;
    exp_ = 0;
    isRecordOn_ = false;
    bestCnt_ = 0;
}

void Particle::measure(BetaMatrix &betaMatrix) {
    double r;
    for (int i = 0; i < betaMatrix.bitsNum_; i++) {
        r = (double)rand() / (double)RAND_MAX;
        if (r < betaMatrix.matrix_[i]) {
            binary_[i] = 1;
        }
        else {
            binary_[i] = 0;
        }
    }
}

void Particle::convert_bi_dec() {
    for (int variableIndex = 0, bitIndex = 0; variableIndex < variableNum_; variableIndex++) {
        for (int fakeIndex = 0, variableBitPosition = bitIndex, power = eachVariableBitsNum_[variableIndex] - 1; fakeIndex < eachVariableBitsNum_[variableIndex]; fakeIndex++, variableBitPosition++, power--) {
            decimal_[variableIndex] += pow(2, power) * binary_[variableBitPosition];
        }
        decimal_[variableIndex]++;
        bitIndex += eachVariableBitsNum_[variableIndex];
    }
    if (techIndex_ == 3) {
        decimal_[1]--;
        decimal_[2]--;
    }
}

void Particle::print(ostream &out = cout) {
    for (int variableIndex = 0, bitIndex = 0; variableIndex < variableNum_; variableIndex++) {
        for (int fakeBitIndex = 0; fakeBitIndex < eachVariableBitsNum_[variableIndex]; fakeBitIndex++, bitIndex++) {
            out << binary_[bitIndex] << ",";
        }
        out << " ,";
    }
    for (int variableIndex = 0; variableIndex < variableNum_; variableIndex++) {
        out << decimal_[variableIndex] << ",";
    }
    out << set_precision(RoR_) << "%," << endl;
}

void Particle::print_train_test_data(string windowName, string outputPath, int actualStartRow, int actualEndRow, vector<string> *holdInfoPtr, vector<vector<string>> *trainFile) {
    string filePath = set_output_filePath(windowName, outputPath, actualStartRow, actualEndRow);
    isRecordOn_ = true;
    remain_ = company_->info_.totalCapitalLV_;
    if (decimal_[0]) {
        trade(actualStartRow, actualEndRow, false, holdInfoPtr);
    }
    else {
        reset();
    }
    if (holdInfoPtr == nullptr || company_->info_.mode_ == 1 || company_->info_.mode_ == 3 || company_->info_.mode_ == 10) {
        ofstream out;
        out.open(filePath);
        if (trainFile != nullptr) {
            for (size_t row = 0; row < 6; row++) {
                for (size_t col = 0; col < (*trainFile)[row].size(); col++) {
                    out << (*trainFile)[row][col] << ",";
                }
                out << endl;
            }
            out << endl;
        }
        else {
            out << "tech type," << techType_ << endl;
            out << "algo," << company_->info_.algoType_ << endl;
            out << "delta," << set_precision(company_->info_.delta_) << endl;
            out << "exp," << company_->info_.expNum_ << endl;
            out << "gen," << company_->info_.genNum_ << endl;
            out << "p number," << company_->info_.particleNum_ << endl;
            out << endl;
        }
        out << "initial capital," << set_precision(company_->info_.totalCapitalLV_) << endl;
        out << "final capital," << set_precision(remain_) << endl;
        out << "final return," << set_precision(remain_ - company_->info_.totalCapitalLV_) << endl;
        out << "return rate"
            << "," << set_precision(RoR_) << "%" << endl;
        out << endl;
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                out << "buy1," << decimal_[0] << endl;
                out << "buy1," << decimal_[1] << endl;
                out << "sell1," << decimal_[2] << endl;
                out << "sell2," << decimal_[3] << endl;
                break;
            }
            case 3: {
                out << "period," << decimal_[0] << endl;
                out << "overSold," << decimal_[1] << endl;
                out << "overBought," << decimal_[2] << endl;
                break;
            }
            default: {
                cout << "print_train_test_data exception" << endl;
                exit(1);
            }
        }
        out << endl;
        out << "best exp," << exp_ << endl;
        out << "best gen," << gen_ << endl;
        out << "best cnt," << bestCnt_ << endl;
        out << endl;
        out << "trade," << sellNum_ << endl;
        print_trade_record(out);
        out.close();
    }
}

string Particle::set_output_filePath(string windowName, string &outputPath, int actualStartRow, int actualEndRow) {
    if (outputPath != "") {
        if (company_->info_.testDeltaLoop_ > 0) {
            string folderName = windowName + "_" + to_string(actualDelta_);
            create_directories(folderName);
            outputPath = folderName;
        }
        outputPath += "/";
    }
    else {
        string delta = set_precision(actualDelta_);
        delta.erase(delta.find_last_not_of('0') + 1, std::string::npos);
        outputPath = techType_ + "_";
        outputPath += company_->companyName_ + "_";
        outputPath += company_->info_.algoType_ + "_";
        outputPath += delta + "_";
    }
    outputPath += (*tables_)[0].date_[actualStartRow] + "_";
    outputPath += (*tables_)[0].date_[actualEndRow] + ".csv";
    return outputPath;
}

class TrainAPeriod {
   private:
    CompanyInfo &company_;
    vector<TechTable> &tables_;

    vector<Particle> particles_;
    vector<Particle> globalP_;  // 0:best,1:globalBest,2:globalWorst,3:localBest,4:localWorst
    BetaMatrix betaMatrix_;
    
    int startRow_ = -1;
    int endRow_ = -1;

    double actualDelta_ = -1;
    int compareNew_ = -1;
    int compareOld_ = -1;
    
    ofstream debugOut_;
    bool debug_ = false;
    bool record_ = false;

    void create_particles();
    void create_betaMatrix();
    ofstream set_debug_file();
    void start_exp(int expCnt);
    void initialize_KNQTS();
    void print_debug_exp(int expCnt);
    void evolve_particles(int i);
    void start_gen(int expCnt, int genCnt);
    void print_debug_gen(int genCnt);
    void print_debug_particle(int i);
    void store_exp_gen(int expCnt, int genCnt);
    void update_local();
    void update_global();
    void run_algo();
    void QTS();
    void GQTS();
    void GNQTS();
    void KNQTScompare();
    void KNQTSmultiply();
    void print_debug_beta();
    void update_best(int renewBest);

   public:
    TrainAPeriod(CompanyInfo &company, vector<TechTable> &tables, int startRow, int endRow, string windowName = "", bool record = false);
};

TrainAPeriod::TrainAPeriod(CompanyInfo &company, vector<TechTable> &tables, int startRow, int endRow, string windowName, bool record) : company_(company), tables_{tables}, startRow_(startRow), endRow_(endRow), actualDelta_(company.info_.delta_), debug_(company.info_.debug_), record_(record) {
    cout << tables_[0].date_[startRow_] << "~" << tables_[0].date_[endRow] << endl;
    create_particles();
    create_betaMatrix();
    globalP_[0].reset();
    debugOut_ = set_debug_file();
    for (int expCnt = 0; expCnt < company_.info_.expNum_; expCnt++) {
        start_exp(expCnt);
    }
    debugOut_.close();
    globalP_[0].print_train_test_data(windowName, company_.paths_.trainFilePaths_[company_.info_.techIndex_] + windowName, startRow_, endRow_);
    cout << globalP_[0].RoR_ << "%" << endl;
    cout << "==========" << endl;
}


void TrainAPeriod::create_particles() {
    Particle p(&company_, company_.info_.techIndex_, debug_);
    p.tables_ = &tables_;
    p.actualDelta_ = actualDelta_;
    for (int i = 0; i < company_.info_.particleNum_; i++) {
        particles_.push_back(p);
    }
    for (int i = 0; i < 5; i++) {
        globalP_.push_back(p);
    }
}

void TrainAPeriod::create_betaMatrix() {
    betaMatrix_.variableNum_ = particles_[0].variableNum_;
    betaMatrix_.eachVariableBitsNum_ = particles_[0].eachVariableBitsNum_;
    betaMatrix_.matrix_.resize(particles_[0].bitsNum_);
    betaMatrix_.bitsNum_ = particles_[0].bitsNum_;
}

ofstream TrainAPeriod::set_debug_file() {
    ofstream out;
    if (debug_) {
        string delta = set_precision(actualDelta_);
        delta.erase(delta.find_last_not_of('0') + 1, std::string::npos);
        string title;
        title += "debug_";
        title += company_.companyName_ + "_";
        title += company_.info_.techType_ + "_";
        title += company_.info_.algoType_ + "_";
        title += delta + "_";
        title += tables_[0].date_[startRow_] + "_";
        title += tables_[0].date_[endRow_] + ".csv";
        out.open(title);
    }
    return out;
}

void TrainAPeriod::start_exp(int expCnt) {
    print_debug_exp(expCnt);
    globalP_[1].reset();
    betaMatrix_.reset();
    initialize_KNQTS();
    for (int genCnt = 0; genCnt < company_.info_.genNum_; genCnt++) {
        start_gen(expCnt, genCnt);
    }
    update_best(0);
}

void TrainAPeriod::initialize_KNQTS() {
    actualDelta_ = company_.info_.delta_;
    compareNew_ = 0;
    compareOld_ = 0;
}

void TrainAPeriod::print_debug_exp(int expCnt) {
    if (debug_)
        debugOut_ << "exp:" << expCnt << ",==========,==========" << endl;
}

void TrainAPeriod::start_gen(int expCnt, int genCnt) {
    print_debug_gen(genCnt);
    globalP_[3].reset();
    globalP_[4].reset(company_.info_.totalCapitalLV_);
    for (int i = 0; i < company_.info_.particleNum_; i++) {
        evolve_particles(i);
    }
    store_exp_gen(expCnt, genCnt);
    update_local();
    update_global();
    run_algo();
    print_debug_beta();
}

void TrainAPeriod::print_debug_gen(int genCnt) {
    if (debug_)
        debugOut_ << "gen:" << genCnt << ",=====" << endl;
}

void TrainAPeriod::evolve_particles(int i) {
    particles_[i].reset();
    particles_[i].measure(betaMatrix_);
    particles_[i].convert_bi_dec();
    particles_[i].trade(startRow_, endRow_);
    print_debug_particle(i);
}

void TrainAPeriod::print_debug_particle(int i) {
    if (debug_)
        particles_[i].print(debugOut_);
}

void TrainAPeriod::store_exp_gen(int expCnt, int genCnt) {
    for_each(particles_.begin(), particles_.end(), [genCnt, expCnt](auto &p) {
        p.exp_ = expCnt;
        p.gen_ = genCnt;
    });
}

void TrainAPeriod::update_local() {
    for (auto &p : particles_) {
        if (p.RoR_ > globalP_[3].RoR_) {
            globalP_[3] = p;
        }
        if (p.RoR_ < globalP_[4].RoR_) {
            globalP_[4] = p;
        }
    }
}

void TrainAPeriod::update_global() {
    if (globalP_[3].RoR_ > globalP_[1].RoR_) {
        globalP_[1] = globalP_[3];
    }
}

void TrainAPeriod::run_algo() {
    switch (company_.info_.algoIndex_) {
        case 0: {
            if (globalP_[3].RoR_ > 0) {
                QTS();
            }
            break;
        }
        case 1: {
            if (globalP_[1].RoR_ > 0) {
                GQTS();
            }
            break;
        }
        case 2: {
            if (globalP_[1].RoR_ > 0) {
                GNQTS();
            }
            break;
        }
        case 3: {
            if (globalP_[1].RoR_ > 0) {
                GNQTS();
            }
            KNQTScompare();
            KNQTSmultiply();
            break;
        }
        default: {
            cout << "wrong algo" << endl;
            exit(1);
        }
    }
}

void TrainAPeriod::QTS() {
    for (int i = 0; i < betaMatrix_.bitsNum_; i++) {
        if (globalP_[3].binary_[i] == 1) {
            betaMatrix_.matrix_[i] += actualDelta_;
        }
        if (globalP_[4].binary_[i] == 1) {
            betaMatrix_.matrix_[i] -= actualDelta_;
        }
    }
}

void TrainAPeriod::GQTS() {
    for (int i = 0; i < betaMatrix_.bitsNum_; i++) {
        if (globalP_[1].binary_[i] == 1) {
            betaMatrix_.matrix_[i] += actualDelta_;
        }
        if (globalP_[4].binary_[i] == 1) {
            betaMatrix_.matrix_[i] -= actualDelta_;
        }
    }
}

void TrainAPeriod::GNQTS() {
    for (int i = 0; i < betaMatrix_.bitsNum_; i++) {
        if (globalP_[1].binary_[i] == 1 && globalP_[4].binary_[i] == 0 && betaMatrix_.matrix_[i] < 0.5) {
            betaMatrix_.matrix_[i] = 1.0 - betaMatrix_.matrix_[i];
        }
        if (globalP_[1].binary_[i] == 0 && globalP_[4].binary_[i] == 1 && betaMatrix_.matrix_[i] > 0.5) {
            betaMatrix_.matrix_[i] = 1.0 - betaMatrix_.matrix_[i];
        }
    }
    GQTS();
}

void TrainAPeriod::KNQTScompare() {
    switch (company_.info_.compareMode_) {
        case 0: {
            auto localBestBinaryIter = globalP_[3].binary_.begin(), localWorstBinaryIter = globalP_[4].binary_.begin();
            for (; localBestBinaryIter != globalP_[3].binary_.end();) {
                if (*localBestBinaryIter != *localWorstBinaryIter) {
                    compareNew_++;
                }
                localBestBinaryIter++;
                localWorstBinaryIter++;
            }
            break;
        }
        case 1: {
            auto localBestDecimalIter = globalP_[3].decimal_.begin(), localWorstDecimalIter = globalP_[4].decimal_.begin();
            for (; localBestDecimalIter != globalP_[3].decimal_.end() && localWorstDecimalIter != globalP_[4].decimal_.end();) {
                compareNew_ += abs(*localBestDecimalIter - *localWorstDecimalIter);
                localBestDecimalIter++;
                localWorstDecimalIter++;
            }
            break;
        }
    }
}

void TrainAPeriod::KNQTSmultiply() {
    if (compareNew_ > compareOld_) {
        actualDelta_ *= company_.info_.multiplyUp_;
    }
    else {
        actualDelta_ *= company_.info_.multiplyDown_;
    }
    compareOld_ = compareNew_;
    compareNew_ = 0;
}

void TrainAPeriod::print_debug_beta() {
    if (debug_) {
        switch (company_.info_.algoIndex_) {
            case 0: {
                debugOut_ << "local best" << endl;
                globalP_[3].print(debugOut_);
                debugOut_ << "local worst" << endl;
                globalP_[4].print(debugOut_);
                break;
            }
            case 1:
            case 2: {
                debugOut_ << "global best" << endl;
                globalP_[1].print(debugOut_);
                debugOut_ << "local worst" << endl;
                globalP_[4].print(debugOut_);
                break;
            }
            case 3: {
                debugOut_ << "global best" << endl;
                globalP_[1].print(debugOut_);
                debugOut_ << "local best" << endl;
                globalP_[3].print(debugOut_);
                debugOut_ << "local worst" << endl;
                globalP_[4].print(debugOut_);
                debugOut_ << actualDelta_ << endl;
                break;
            }
        }
        betaMatrix_.print(debugOut_);
    }
}

void TrainAPeriod::update_best(int renewBest) {
    if (globalP_[0].RoR_ < globalP_[1].RoR_) {
        globalP_[0] = globalP_[1];
    }
    switch (renewBest) {
        case 0: {
            if (globalP_[1].binary_ == globalP_[0].binary_) {
                globalP_[0].bestCnt_++;
            }
            break;
        }
        case 1: {
            if (globalP_[1].RoR_ == globalP_[0].RoR_) {
                globalP_[0].bestCnt_++;
            }
            break;
        }
        default: {
            cout << "" << endl;
            exit(1);
        }
    }
}

class Train {
    public:
    CompanyInfo &company_;
    vector<TechTable> tables_;
    
    void train_a_window(string windowName);
    void train_a_company();
    
    Train(CompanyInfo &company);
    Train(CompanyInfo &company, string startDate, string endDate);
};

Train::Train(CompanyInfo &company) : company_(company), tables_({TechTable(&company, company.info_.techIndex_)}) {
    if (company_.info_.testDeltaLoop_ == 0) {
        train_a_company();
    }
    else {
        for (int loop = 0; loop < company.info_.testDeltaLoop_; loop++) {
            train_a_company();
            company_.info_.delta_ -= company_.info_.testDeltaGap_;
        }
    }
}

Train::Train(CompanyInfo &company, string startDate, string endDate) : company_(company), tables_({TechTable(&company, company.info_.techIndex_)}) {
    int startRow = find_index_of_string_in_vec(tables_[0].date_, startDate);
    int endRow = find_index_of_string_in_vec(tables_[0].date_, endDate);
    srand(343);
    company_.paths_.trainFilePaths_[company_.info_.techIndex_].clear();
    TrainAPeriod period(company, tables_, startRow, endRow);
}

void Train::train_a_company() {
    for (auto windowName : company_.info_.slidingWindows_) {
        train_a_window(windowName);
    }
}

void Train::train_a_window(string windowName) {
    TrainWindow window(company_, windowName);
    if (window.interval_[0] >= 0) {
        window.print_train();
        srand(343);
        for (auto intervalIter = window.interval_.begin(); intervalIter != window.interval_.end(); intervalIter += 2) {
            TrainAPeriod period(company_, tables_, *intervalIter, *(intervalIter + 1), window.windowName_);
        }
    }
    else {
        cout << "train window is too old, skip this window" << endl;
    }
}

class Test {
   private:
    class Path {
       public:
        vector<string> trainFilePaths_;
        string testFileOutputPath_;
    };
    CompanyInfo &company_;
    Path paths_;
    Particle p_;
    vector<TechTable> tables_;
    bool tradition_ = false;
    bool hold_ = false;
    vector<string> holdInfo_;
    vector<string> *holdInfoPtr_ = nullptr;

    void add_tables(vector<int> addtionTable);
    void create_particle();
    void set_train_test_file_path();
    void set_normal_paths();
    void set_mixedTech_paths();
    void test_a_window(TestWindow &window);
    TestWindow set_window(string &targetWindow, string &actualWindow, int &windowIndex);
    void check_exception(vector<vector<path>> &eachTrainFilePath, TestWindow &window);
    size_t set_variables(vector<vector<vector<string>>> &thisTrainFile);
    void print_test_holdInfo(TestWindow &window);

   public:
    Test(CompanyInfo &company, string targetWindow = "all", bool tradition = false, bool hold = false, vector<int> additionTable = {});
};

Test::Test(CompanyInfo &company, string targetWindow, bool tradition, bool hold, vector<int> additionTable) : company_(company), p_(&company, 0), tradition_(tradition), hold_(hold) {
    add_tables(additionTable);
    create_particle();
    set_train_test_file_path();
    for (int windowIndex = 0; windowIndex < company_.info_.windowNumber_; windowIndex++) {
        string actualWindow = company_.info_.slidingWindows_[windowIndex];
        if (actualWindow != "A2A") {
            TestWindow window = set_window(targetWindow, actualWindow, windowIndex);
            test_a_window(window);
        }
    }
}

void Test::add_tables(vector<int> addtionTable) {
    if (!company_.info_.mixedTech_) {
        tables_.push_back(TechTable(&company_, company_.info_.techIndex_));
    }
    else {
        for (auto &techIndex : company_.info_.techIndexs_) {
            tables_.push_back(TechTable(&company_, techIndex));
        }
    }
    if (addtionTable.size() > 0) {
        for (int i = 0; i < addtionTable.size(); i++) {
            tables_.push_back(TechTable(&company_, addtionTable[i]));
        }
    }
}

void Test::create_particle() {
    if (!company_.info_.mixedTech_) {
        switch (company_.info_.techIndex_) {
            case 3: {
                p_ = Particle(&company_, company_.info_.techIndex_);
                break;
            }
        }
    }
    p_.tables_ = &tables_;
}

void Test::set_train_test_file_path() {
    if (!company_.info_.mixedTech_) {
        set_normal_paths();
    }
    else if (company_.info_.mixedTech_) {
        set_mixedTech_paths();
    }
    cout << "test " << company_.companyName_ << endl;
}

void Test::set_normal_paths() {
    if (tradition_) {
        paths_.trainFilePaths_.push_back(company_.paths_.trainTraditionFilePaths_[company_.info_.techIndex_]);
        paths_.testFileOutputPath_ = company_.paths_.testTraditionFilePaths_[company_.info_.techIndex_];
    }
    else {
        paths_.trainFilePaths_.push_back(company_.paths_.trainFilePaths_[company_.info_.techIndex_]);
        paths_.testFileOutputPath_ = company_.paths_.testFilePaths_[company_.info_.techIndex_];
    }
}

void Test::set_mixedTech_paths() {
    if (tradition_) {
        for (auto &techIndex : company_.info_.techIndexs_) {
            paths_.trainFilePaths_.push_back(company_.paths_.trainTraditionFilePaths_[techIndex]);
        }
        paths_.testFileOutputPath_ = company_.paths_.testTraditionFilePaths_[company_.info_.techIndex_];
    }
    else {
        for (auto &techIndex : company_.info_.techIndexs_) {
            paths_.trainFilePaths_.push_back(company_.paths_.trainFilePaths_[techIndex]);
        }
        paths_.testFileOutputPath_ = company_.paths_.testFilePaths_[company_.info_.techIndex_];
    }
}

TestWindow Test::set_window(string &targetWindow, string &actualWindow, int &windowIndex) {
    if (targetWindow != "all") {
        actualWindow = targetWindow;
        windowIndex = company_.info_.windowNumber_;
    }
    TestWindow window(company_, actualWindow);
    cout << window.windowName_ << endl;
    return window;
}

void Test::test_a_window(TestWindow &window) {
    vector<vector<path>> eachTrainFilePath;
    for (auto &trainPath : paths_.trainFilePaths_) {
        eachTrainFilePath.push_back(get_path(trainPath + window.windowName_));
    }
    for (auto &eachPath : eachTrainFilePath) {
        if (eachPath.size() == 0)
            break;
        check_exception(eachTrainFilePath, window);
        p_.push_holdInfo_column_Name(hold_, holdInfo_, holdInfoPtr_);
        for (int intervalIndex = 0, trainFileIndex = 0; intervalIndex < window.intervalSize_; intervalIndex += 2, trainFileIndex++) {
            int startRow = window.interval_[intervalIndex];
            int endRow = window.interval_[intervalIndex + 1];
            vector<vector<vector<string>>> thisTrainFile;
            for (auto &diffTrainFilePath : eachTrainFilePath) {
                thisTrainFile.push_back(read_data(diffTrainFilePath[trainFileIndex]));
            }
            p_.reset();
            size_t bestIndex = set_variables(thisTrainFile);
            p_.print_train_test_data(window.windowName_, paths_.testFileOutputPath_ + window.windowName_, startRow, endRow, holdInfoPtr_, &thisTrainFile[bestIndex]);
            print_test_holdInfo(window);
        }
    }
}

void Test::check_exception(vector<vector<path>> &eachTrainFilePath, TestWindow &window) {
    for (auto &trainFilePath : eachTrainFilePath) {
        if (trainFilePath.size() != window.intervalSize_ / 2) {
            cout << company_.companyName_ << " " << window.windowName_ << " test interval number is not equal to train fle number" << endl;
            exit(1);
        }
    }
}

size_t Test::set_variables(vector<vector<vector<string>>> &thisTrainFile) {
    size_t bestRoRIndex = 0;
    if (company_.info_.mixedTech_) {
        vector<double> everyRoR;
        for (auto &trainFile : thisTrainFile) {
            everyRoR.push_back(stod(trainFile[10][1]));
        }
        bestRoRIndex = distance(everyRoR.begin(), max_element(everyRoR.begin(), everyRoR.end()));
        p_ = Particle(&company_, company_.info_.techIndexs_[bestRoRIndex]);
        p_.tables_ = &tables_;
    }
    for (int i = 0; i < p_.variableNum_; i++) {
        p_.decimal_[i] = stoi(thisTrainFile[bestRoRIndex][i + 12][1]);
    }
    return bestRoRIndex;
}

void Test::print_test_holdInfo(TestWindow &window) {
    if (hold_) {
        ofstream holdFile;
        if (!tradition_) {
            holdFile.open(company_.paths_.testHoldFilePaths_[company_.info_.techIndex_] + company_.companyName_ + "_" + window.windowName_ + ".csv");
        }
        else if (tradition_) {
            holdFile.open(company_.paths_.testTraditionHoldFilePaths_[company_.info_.techIndex_] + company_.companyName_ + "_" + window.windowName_ + ".csv");
        }
        for (auto info : holdInfo_) {
            holdFile << info;
        }
        holdFile.close();
    }
}

class BH {
   public:
    double BHRoR;
    BH(CompanyInfo &company, string startDate, string endDate) {
        int startRow = find_index_of_string_in_vec(company.date_, startDate);
        int endRow = find_index_of_string_in_vec(company.date_, endDate);
        int stockHold = company.info_.totalCapitalLV_ / company.price_[startRow];
        double remain = company.info_.totalCapitalLV_ - stockHold * company.price_[startRow];
        remain += stockHold * company.price_[endRow];
        BHRoR = ((remain - company.info_.totalCapitalLV_) / company.info_.totalCapitalLV_);
    }
};

class Tradition {
   private:
    CompanyInfo &company_;
    vector<TechTable> tables_;
    vector<Particle> particles_;
    vector<vector<int>> traditionStrategy_;
    int traditionStrategyNum_ = -1;

    vector<vector<vector<int>>> allTraditionStrategy_{MA::traditionStrategy_, MA::traditionStrategy_, MA::traditionStrategy_, RSI::traditionStrategy_};

    void create_particles();
    void set_strategy();
    TrainWindow set_window(string &targetWindow, int &windowIndex);
    void train_a_tradition_window(TrainWindow &window);
    void set_variables(int index);

   public:
    Tradition(CompanyInfo &company, string targetWindow = "all");
};

Tradition::Tradition(CompanyInfo &company, string targetWindow) : company_(company), tables_{TechTable(&company, company.info_.techIndex_)} {
    set_strategy();
    create_particles();
    cout << "train " << company_.companyName_ << " tradition" << endl;
    for (int windowIndex = 0; windowIndex < company_.info_.windowNumber_; windowIndex++) {
        TrainWindow window = set_window(targetWindow, windowIndex);
        if (window.interval_[0] < 0) {
            cout << "train window is too old, skip this window" << endl;
            continue;
        }
        train_a_tradition_window(window);
    }
}

void Tradition::set_strategy() {
    traditionStrategy_ = allTraditionStrategy_[company_.info_.techIndex_];
    traditionStrategyNum_ = (int)traditionStrategy_.size();
}

void Tradition::create_particles() {
    Particle p(&company_, company_.info_.techIndex_);
    p.tables_ = &tables_;
    for (int i = 0; i < traditionStrategyNum_; i++) {
        particles_.push_back(p);
    }
}

TrainWindow Tradition::set_window(string &targetWindow, int &windowIndex) {
    string actualWindow = company_.info_.slidingWindows_[windowIndex];
    if (targetWindow != "all") {
        actualWindow = targetWindow;
        windowIndex = company_.info_.windowNumber_;
    }
    TrainWindow window(company_, actualWindow);
    cout << actualWindow << endl;
    return window;
}

void Tradition::train_a_tradition_window(TrainWindow &window) {
    string outputPath = company_.paths_.trainTraditionFilePaths_[company_.info_.techIndex_] + window.windowName_;
    for (int intervalIndex = 0; intervalIndex < window.intervalSize_; intervalIndex += 2) {
        int startRow = window.interval_[intervalIndex];
        int endRow = window.interval_[intervalIndex + 1];
        for (int i = 0; i < traditionStrategyNum_; i++) {
            particles_[i].reset();
            set_variables(i);
            particles_[i].trade(startRow, endRow);
        }
        stable_sort(particles_.begin(), particles_.end(), [](const Particle &a, const Particle &b) { return a.RoR_ > b.RoR_; });
        particles_[0].print_train_test_data(window.windowName_, outputPath, startRow, endRow);
    }
}

void Tradition::set_variables(int index) {
    for (int i = 0; i < particles_[i].variableNum_; i++) {
        particles_[index].decimal_[i] = traditionStrategy_[index][i];
    }
}

class CalIRR {
   public:
    class WindowIRR {
       public:
        string windowName_;
        double algoIRR_;
        double traditionIRR_;
        int rank_;
        vector<vector<int>> techChooseTimes_;

        void resize_vec(size_t allTechSize) {
            techChooseTimes_.resize(2);
            for (auto &row : techChooseTimes_) {
                row.resize(allTechSize);
            }
        }

        void fill_zero() {
            for (auto &row : techChooseTimes_) {
                fill(row.begin(), row.end(), 0);
            }
        }
    };

    class CompanyWindowIRRContainer {
       public:
        string companyName_;
        vector<WindowIRR> windowsIRR_;
    };

    class Rank {
       public:
        string companyName_;
        vector<int> algoWindowRank_;
        vector<int> traditionWindowRank_;
    };

    class CompanyAllRoRInfo {
       public:
        vector<string> algoRoRoutInfo_;
        vector<string> traditionRoRoutInfo_;
    };

    class CalOneCompanyIRR {
       public:
        CompanyInfo &company_;
        vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR_;
        vector<Rank> &allCompanyWindowRank_;
        CompanyWindowIRRContainer thisCompanyWindowIRR_;
        vector<size_t> eachVariableNum_{MA::eachVariableBitsNum_.size(), MA::eachVariableBitsNum_.size(), MA::eachVariableBitsNum_.size(), RSI::eachVariableBitsNum_.size()};
        WindowIRR tmpWinodwIRR_;
        bool tradition_ = false;

        double cal_one_window_IRR(vector<string> &RoRoutInfo, string &window, string &stratgyFilePath);
        string compute_and_record_window_RoR(vector<path> &strategyPaths, const vector<path>::iterator &filePathIter, double &totalRoR);
        WindowIRR cal_BH_IRR();
        void rank_algo_and_tradition_window(vector<WindowIRR> &windowsIRR);
        void sort_by_tradition_IRR(vector<WindowIRR> &windowsIRR);
        void rank_window(vector<WindowIRR> &windowsIRR);
        void sort_by_window_name(vector<WindowIRR> &windowsIRR);
        void sort_by_algo_IRR(vector<WindowIRR> &windowsIRR);

        CalOneCompanyIRR(CompanyInfo &company, vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR, vector<Rank> &allCompanyWindowRank);
    };

    Info info_;
    vector<path> &companyPricePaths_;
    vector<Rank> allCompanyWindowRank_;
    vector<CompanyWindowIRRContainer> allCompanyWindowsIRR_;

    void output_all_IRR();
    void output_IRR(ofstream &IRRout);
    void output_all_window_rank();
    vector<string> remove_A2A_and_sort();

    CalIRR(vector<path> &companyPricePaths);
};

CalIRR::CalIRR(vector<path> &companyPricePaths) : companyPricePaths_(companyPricePaths) {
    for (auto &companyPricePath : companyPricePaths_) {
        CompanyInfo company(companyPricePath, info_);
        CalOneCompanyIRR calOneCompanyIRR(company, allCompanyWindowsIRR_, allCompanyWindowRank_);
    }
    output_all_IRR();
    output_all_window_rank();
}

void CalIRR::output_all_IRR() {  //輸出以視窗名稱排序以及IRR排序
    ofstream IRRout(info_.rootFolder_ + "/" + info_.techType_ + "_test_IRR_sorted_by_name.csv");
    output_IRR(IRRout);
    for_each(allCompanyWindowsIRR_.begin(), allCompanyWindowsIRR_.end(), [](CompanyWindowIRRContainer &eachCompanyContainer) {
        sort(eachCompanyContainer.windowsIRR_.begin(), eachCompanyContainer.windowsIRR_.end(), [](const WindowIRR &w1, const WindowIRR &w2) {
            return w1.algoIRR_ > w2.algoIRR_;
        });
    });
    IRRout.open(info_.rootFolder_ + "/" + info_.techType_ + "_test_IRR_sorted_by_IRR.csv");
    output_IRR(IRRout);
}

void CalIRR::output_IRR(ofstream &IRRout) {
    for (auto &company : allCompanyWindowsIRR_) {
        IRRout << "=====" << company.companyName_ << "=====";
        IRRout << "," << info_.techType_ << "_algo," << info_.techType_ << " Tradition,";
        if (info_.mixedTech_) {
            for (int loop = 0; loop < 2; loop++) {
                for_each(info_.techIndexs_.begin(), info_.techIndexs_.end(), [&](const int &techIndex) {
                    IRRout << info_.allTech_[techIndex] << ",";
                });
            }
        }
        IRRout << "\n";
        for (auto &eachIRR : company.windowsIRR_) {
            IRRout << eachIRR.windowName_ << ",";
            IRRout << set_precision(eachIRR.algoIRR_) << ",";
            IRRout << set_precision(eachIRR.traditionIRR_) << ",";
            if (info_.mixedTech_) {
                for_each(eachIRR.techChooseTimes_.begin(), eachIRR.techChooseTimes_.end(), [&](const vector<int> &i) {
                    for_each(info_.techIndexs_.begin(), info_.techIndexs_.end(), [&](const int &j) {
                        IRRout << i[j] << ",";
                    });
                });
            }
            IRRout << "\n";
        }
    }
    IRRout.close();
}

void CalIRR::output_all_window_rank() {
    vector<string> windowSort = remove_A2A_and_sort();
    ofstream rankOut(info_.rootFolder_ + "/" + info_.techType_ + "_windowRank.csv");
    rankOut << "algo window rank\n,";
    for (auto &windowName : windowSort) {
        rankOut << windowName << ",";
    }
    rankOut << endl;
    for (auto &i : allCompanyWindowRank_) {
        rankOut << i.companyName_ << ",";
        for (auto &j : i.algoWindowRank_) {
            rankOut << j << ",";
        }
        rankOut << endl;
    }
    rankOut << "\n\ntradition window rank\n";
    for (auto &i : allCompanyWindowRank_) {
        rankOut << i.companyName_ << ",";
        for (auto &j : i.traditionWindowRank_) {
            rankOut << j << ",";
        }
        rankOut << endl;
    }
    rankOut.close();
}

vector<string> CalIRR::remove_A2A_and_sort() {
    vector<string> windowSort = info_.slidingWindows_;
    auto A2Aiter = find(windowSort.begin(), windowSort.end(), "A2A");
    windowSort.erase(A2Aiter);
    windowSort.push_back("B&H");
    sort(windowSort.begin(), windowSort.end());
    return windowSort;
}

CalIRR::CalOneCompanyIRR::CalOneCompanyIRR(CompanyInfo &company, vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR, vector<Rank> &allCompanyWindowRank) : company_(company), allCompanyWindowsIRR_(allCompanyWindowsIRR), allCompanyWindowRank_(allCompanyWindowRank) {
    thisCompanyWindowIRR_.companyName_ = company_.companyName_;
    CompanyAllRoRInfo allRoRInfo;
    if (company_.info_.mixedTech_) {
        tmpWinodwIRR_.resize_vec(company_.info_.allTech_.size());
    }
    for (auto &window : company_.info_.slidingWindows_) {
        if (window != "A2A") {
            cout << window << endl;
            tradition_ = false;
            tmpWinodwIRR_.algoIRR_ = cal_one_window_IRR(allRoRInfo.algoRoRoutInfo_, window, company_.paths_.testFilePaths_[company_.info_.techIndex_]);
            tradition_ = true;
            tmpWinodwIRR_.traditionIRR_ = cal_one_window_IRR(allRoRInfo.traditionRoRoutInfo_, window, company_.paths_.testTraditionFilePaths_[company_.info_.techIndex_]);  //若還沒有傳統的test這行可以註解
            tmpWinodwIRR_.windowName_ = window;
            thisCompanyWindowIRR_.windowsIRR_.push_back(tmpWinodwIRR_);
            tmpWinodwIRR_.fill_zero();
        }
    }
    thisCompanyWindowIRR_.windowsIRR_.push_back(cal_BH_IRR());
    rank_algo_and_tradition_window(thisCompanyWindowIRR_.windowsIRR_);
    auto output_company_all_window_IRR = [](ofstream &out, const vector<string> &outputInfo) {
        for (auto &info : outputInfo)
            out << info;
        out.close();
    };
    ofstream out(company_.paths_.companyRootPaths_[company_.info_.techIndex_] + company_.companyName_ + "_testRoR.csv");
    output_company_all_window_IRR(out, allRoRInfo.algoRoRoutInfo_);
    out.open(company_.paths_.companyRootPaths_[company_.info_.techIndex_] + company_.companyName_ + "_traditionRoR.csv");
    output_company_all_window_IRR(out, allRoRInfo.traditionRoRoutInfo_);
    allCompanyWindowsIRR_.push_back(thisCompanyWindowIRR_);
}

double CalIRR::CalOneCompanyIRR::cal_one_window_IRR(vector<string> &RoRoutInfo, string &window, string &stratgyFilePath) {
    RoRoutInfo.push_back("," + window + "\n");
    double totalRoR = 0;
    vector<path> strategyPaths = get_path(stratgyFilePath + window);
    for (auto filePathIter = strategyPaths.begin(); filePathIter != strategyPaths.end(); filePathIter++) {
        RoRoutInfo.push_back(compute_and_record_window_RoR(strategyPaths, filePathIter, totalRoR));
    }
    double windowIRR = pow(totalRoR--, 1.0 / (double)company_.info_.testLength_) - 1.0;
    RoRoutInfo.push_back(",,,,,," + window + "," + set_precision(totalRoR) + "," + set_precision(windowIRR) + "\n");
    return windowIRR;
}

string CalIRR::CalOneCompanyIRR::compute_and_record_window_RoR(vector<path> &strategyPaths, const vector<path>::iterator &filePathIter, double &totalRoR) {
    vector<vector<string>> file = read_data(*filePathIter);
    double RoR = stod(file[10][1]);
    if (filePathIter == strategyPaths.begin()) {
        totalRoR = RoR / 100.0 + 1.0;
    }
    else {
        totalRoR = totalRoR * (RoR / 100.0 + 1.0);
    }
    string push = filePathIter->stem().string() + ",";
    int techIndex = -1;
    if (!company_.info_.mixedTech_) {
        techIndex = company_.info_.techIndex_;
    }
    else {
        techIndex = find_index_of_string_in_vec(company_.info_.allTech_, file[0][1]);
    }
    for (int i = 0; i < eachVariableNum_[techIndex]; i++) {
        push += file[i + 12][1] + ",";
    }
    if (techIndex == 3) {
        push += ",";
    }
    push += file[10][1] + "\n";
    if (company_.info_.mixedTech_) {
        if (!tradition_) {
            tmpWinodwIRR_.techChooseTimes_[0][techIndex]++;
        }
        else {
            tmpWinodwIRR_.techChooseTimes_[1][techIndex]++;
        }
    }
    return push;
}

CalIRR::WindowIRR CalIRR::CalOneCompanyIRR::cal_BH_IRR() {
    BH bh(company_, company_.date_[company_.testStartRow_], company_.date_[company_.testEndRow_]);
    WindowIRR tmp;
    tmp.windowName_ = "B&H";
    tmp.algoIRR_ = pow(bh.BHRoR + 1.0, 1.0 / company_.info_.testLength_) - 1.0;
    tmp.traditionIRR_ = tmp.algoIRR_;
    tmp.resize_vec(company_.info_.allTech_.size());
    return tmp;
}

void CalIRR::CalOneCompanyIRR::rank_algo_and_tradition_window(vector<WindowIRR> &windowsIRR) {
    Rank tmpRank;
    tmpRank.companyName_ = company_.companyName_;
    sort_by_tradition_IRR(windowsIRR);  //將視窗按照傳統的IRR排序
    rank_window(windowsIRR);
    sort_by_window_name(windowsIRR);  //將視窗按照名字排序
    for (auto &windowIRR : windowsIRR) {
        tmpRank.traditionWindowRank_.push_back(windowIRR.rank_);
    }
    sort_by_algo_IRR(windowsIRR);  //將視窗按照用演算法的IRR排序
    rank_window(windowsIRR);
    sort_by_window_name(windowsIRR);  //將視窗按照名字排序
    for (auto &windowIRR : windowsIRR) {
        tmpRank.algoWindowRank_.push_back(windowIRR.rank_);
    }
    // sort_by_algo_IRR(windowsIRR);
    allCompanyWindowRank_.push_back(tmpRank);
}

void CalIRR::CalOneCompanyIRR::sort_by_tradition_IRR(vector<WindowIRR> &windowsIRR) {
    sort(windowsIRR.begin(), windowsIRR.end(), [](const WindowIRR &w1, const WindowIRR &w2) { return w1.traditionIRR_ > w2.traditionIRR_; });
}

void CalIRR::CalOneCompanyIRR::rank_window(vector<WindowIRR> &windowsIRR) {
    for (int i = 0; i < windowsIRR.size(); i++) {
        windowsIRR[i].rank_ = i + 1;
    }
}

void CalIRR::CalOneCompanyIRR::sort_by_window_name(vector<WindowIRR> &windowsIRR) {
    sort(windowsIRR.begin(), windowsIRR.end(), [](const WindowIRR &w1, const WindowIRR &w2) { return w1.windowName_ < w2.windowName_; });
}

void CalIRR::CalOneCompanyIRR::sort_by_algo_IRR(vector<WindowIRR> &windowsIRR) {
    sort(windowsIRR.begin(), windowsIRR.end(), [](const WindowIRR &w1, const WindowIRR &w2) { return w1.algoIRR_ > w2.algoIRR_; });
}

class MergeIRRFile {  //這邊會將mixed IRR file全部放在一起輸出成一個csv方便比較，可能用不到，直接打開csv手動複製就好
   public:
    Info info_;

    MergeIRRFile() {
        if (info_.techIndexs_.size() > 1) {
            vector<vector<vector<string>>> files;
            for (auto &techIndex : info_.techIndexs_) {
                files.push_back(read_data(info_.rootFolder_ + "/" + info_.allTech_[techIndex] + "_test_IRR.csv"));
            }
            vector<vector<string>> newFile = read_data(info_.rootFolder_ + "/" + info_.techType_ + "_test_IRR.csv");
            for (auto &file : files) {
                for (size_t row = 0; row < file.size(); row++) {
                    for (size_t col = 0; col < file[row].size(); col++) {
                        newFile[row].push_back(file[row][col]);
                    }
                }
            }
            ofstream n(info_.rootFolder_ + "/" + "merge_" + info_.techType_ + "_IRR.csv");
            for (auto &row : newFile) {
                for (auto &col : row) {
                    n << col << ",";
                }
                n << "\n";
            }
            n.close();
        }
    }
};

class SortIRRFileBy {  //選擇特定的IRR file，將整份文件以視窗名稱排序，或是以IRR排序(用第幾個col調整)
   public:
    Info info_;
    string sortBy_;
    int colToSort = 1;

    SortIRRFileBy(string sortBy = "IRR") : sortBy_(sortBy) {
        string inpuFileName = "SMA_test_IRR.csv";
        vector<vector<string>> inputFile = read_data(info_.rootFolder_ + "/" + inpuFileName);
        vector<vector<vector<string>>::iterator> equalSignIter;
        for (auto iter = inputFile.begin(); iter != inputFile.end(); iter++) {
            if ((*iter).front().front() == '=') {
                equalSignIter.push_back(iter);
            }
        }
        equalSignIter.push_back(inputFile.end());
        if (sortBy_ == "name") {
            for (size_t eachEqualIndex = 0; eachEqualIndex < equalSignIter.size() - 1; eachEqualIndex++) {
                sort(equalSignIter[eachEqualIndex] + 1, equalSignIter[eachEqualIndex + 1], [](const vector<string> &s1, const vector<string> &s2) {
                    return s1.front() < s2.front();
                });
            }
        }
        else if (sortBy_ == "IRR") {
            for (size_t eachEqualIndex = 0; eachEqualIndex < equalSignIter.size() - 1; eachEqualIndex++) {
                sort(equalSignIter[eachEqualIndex] + 1, equalSignIter[eachEqualIndex + 1], [&](const vector<string> &s1, const vector<string> &s2) {
                    return stod(s1[colToSort]) > stod(s2[colToSort]);
                });
            }
        }
        ofstream sortedFile(info_.rootFolder_ + "/" + cut_string(inpuFileName, '.')[0] + "_sorted_by_" + sortBy_ + ".csv");
        for (auto &row : inputFile) {
            for (auto &col : row) {
                sortedFile << col << ",";
            }
            sortedFile << "\n";
        }
        sortedFile.close();
    }
};

static vector<path> set_company_price_paths(const Info &info) {
    vector<path> companiesPricePath = get_path(info.pricePath_);
    if (info.setCompany_ != "all") {
        vector<string> setCcompany = cut_string(info.setCompany_);
        auto find_Index = [&](string &traget) {
            return distance(companiesPricePath.begin(), find_if(companiesPricePath.begin(), companiesPricePath.end(), [&](path &pricePath) { return pricePath.stem() == traget; }));
        };
        size_t startIndex = find_Index(setCcompany.front());
        size_t endIndex = find_Index(setCcompany.back()) + 1;
        vector<path> tmp(companiesPricePath.begin() + startIndex, companiesPricePath.begin() + endIndex);
        companiesPricePath = tmp;
    }
    return companiesPricePath;
}

int main(int argc, const char *argv[]) {
    time_point begin = steady_clock::now();
    vector<path> companyPricePaths = set_company_price_paths(_info);
    try {
        for (auto companyPricePath : companyPricePaths) {
            CompanyInfo company(companyPricePath, _info);
            switch (company.info_.mode_) {
                case 0: {
                    Train train(company);
                    break;
                }
                case 1: {
                    Test test(company, company.info_.setWindow_, false, true);
                    break;
                }
                case 2: {
                    Tradition trainTradition(company, company.info_.setWindow_);
                    break;
                }
                case 3: {
                    Test testTradition(company, company.info_.setWindow_, true, true);
                    break;
                }
                case 10: {
                    //                    Test(company, company.info_.setWindow_, false, true, vector<int>{0});
                    //                    Tradition tradition(company);
                    //                    Train train(company, "2011-12-01", "2011-12-30");
                    //                    Particle(&company, true, vector<int>{5, 20, 5, 20}).instant_trade("2020-01-02", "2021-06-30");
                    //                    Particle(&company, true, vector<int>{70, 44, 85, 8}).instant_trade("2011-12-01", "2011-12-30");
                    //                    Particle(&company, true, vector<int>{5, 10, 5, 10}).instant_trade("2020-01-02", "2020-05-29", true);
                    //                    Particle(&company, true, vector<int>{14, 30, 70}).instant_trade("2012-01-03", "2020-12-31", true);
                    //                    Test test(company, company.info_.setWindow_, false, true, vector<int>{0});
                    //                    CalIRR calIRR(companyPricePaths);
                    //                    MergeIRRFile mergeFile;
                    //                    SortIRRFileBy IRR("IRR");
                    //                    Train t(company, "2011-12-01", "2011-12-30");
                    break;
                }
            }
        }
    } catch (exception &e) {
        cout << "exception: " << e.what() << endl;
    }
    time_point end = steady_clock::now();
    cout << "time: " << duration_cast<milliseconds>(end - begin).count() / 1000.0 << " s" << endl;
    return 0;
}
