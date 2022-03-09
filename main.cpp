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
    string setCompany_ = "AAPL";
    string setWindow_ = "M2M";

    double delta_ = 0.003;
    int expNum_ = 50;
    int genNum_ = 1000;
    int particleNum_ = 10;
    double totalCapitalLV_ = 10000000;

    int testDeltaLoop_ = 1;
    double testDeltaGap_ = 0.00001;
    double multiplyUp_ = -1;
    double multiplyDown_ = -1;

    int techIndex_ = 3;
    vector<string> allTech_ = {"SMA", "WMA", "EMA", "RSI"};
    int algoIndex_ = 2;
    vector<string> allAlgo_ = {"QTS", "GQTS", "GNQTS", "KNQTS"};

    string techType_ = allTech_[techIndex_];
    string algoType_ = allAlgo_[algoIndex_];

    path pricePath_ = "price";
    string testStartYear_ = "2012-01";
    string testEndYear_ = "2021-01";
    double testLength_ = stod(testEndYear_) - stod(testStartYear_);

    vector<string> slidingWindows_ = {"A2A", "YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    vector<string> slidingWindowsEx_ = {"A2A", "36M36", "36M24", "36M18", "36M12", "36M6", "36M3", "36M1", "24M24", "24M18", "24M12", "24M6", "24M3", "24M1", "18M18", "18M12", "18M6", "18M3", "18M1", "12M12", "12M6", "12M3", "12M1", "6M6", "6M3", "6M1", "3M3", "3M1", "1M1", "6M", "3M", "1M", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    int windowNumber_ = int(slidingWindows_.size());
} const _info;

class CompanyInfo {
   public:
    const Info &info_;
    string companyName_;

    map<string, string> allResultOutputPath_;
    map<string, string> allTechOuputPath_;
    map<string, string> allTrainFilePath_;
    map<string, string> allTestFilePath_;
    map<string, string> allTrainTraditionFilePath_;
    map<string, string> allTestTraditionFilePath_;
    map<string, string> allTestHoldFilePath_;

    int totalDays_;
    vector<string> date_;
    vector<double> price_;
    int testStartRow_ = -1;
    int testEndRow_ = -1;
    vector<vector<double>> techTable_;
    int tableStartRow_ = -1;

    void set_paths();
    void store_date_price(path priceFilePath);
    void create_folder();
    void find_table_start_row();
    void store_tech_to_vector();
    void output_Tech();
    void set_techFile_title(ofstream &out, int techPerid);

    CompanyInfo(const Info &info, path pricePath);
};

CompanyInfo::CompanyInfo(const Info &info, path pricePath) : info_(info), companyName_(pricePath.stem().string()) {
    set_paths();
    store_date_price(pricePath);
    create_folder();
    find_table_start_row();
}

void CompanyInfo::set_paths() {
    for (auto tech : info_.allTech_) {
        allResultOutputPath_.insert({tech, tech + "_result"});
        allTechOuputPath_.insert({tech, tech + "/" + companyName_});
        allTrainFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/train/"});
        allTestFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/test/"});
        allTrainTraditionFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/trainTradition/"});
        allTestTraditionFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/testTradition/"});
        allTestHoldFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/testHoldPerid/"});
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

void CompanyInfo::create_folder() {
    create_directories(info_.techType_ + "/" + companyName_);
    create_directories(allTestHoldFilePath_.at(info_.techType_));
    for (auto i : info_.slidingWindows_) {
        create_directories(allTrainFilePath_.at(info_.techType_) + i);
        create_directories(allTestFilePath_.at(info_.techType_) + i);
        create_directories(allTrainTraditionFilePath_.at(info_.techType_) + i);
        create_directories(allTestTraditionFilePath_.at(info_.techType_) + i);
    }
}

void CompanyInfo::find_table_start_row() {
    char delimiter;
    int longestTrainMonth = -1;
    for (int i = 0; i < info_.windowNumber_; i++) {
        vector<string> trainTest = find_train_and_test_len(info_.slidingWindowsEx_[i], delimiter);
        string trainMonth;
        if (trainTest.size() == 1) {
            trainMonth = "12";
        }
        else {
            trainMonth = trainTest[0];
        }
        if (delimiter == 'M' && stoi(trainMonth) > longestTrainMonth) {
            longestTrainMonth = stoi(trainMonth);
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
    switch (info_.techIndex_) {
        case 0: {
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
            break;
        }
        case 1: {
            break;
        }
        case 2: {
            break;
        }
        case 3: {
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
            break;
        }
        default: {
            cout << "store_tech_to_vector exception" << endl;
            exit(1);
        }
    }
    cout << "done calculating" << endl;
}

void CompanyInfo::output_Tech() {
    store_tech_to_vector();
    cout << "saving " << info_.techType_ << " file" << endl;
    switch (info_.techIndex_) {
        case 0: {
            for (int SMAPeriod = 1; SMAPeriod < 257; SMAPeriod++) {
                if (SMAPeriod % 10 == 0) {
                    cout << ".";
                }
                ofstream out;
                set_techFile_title(out, SMAPeriod);
                int techSize = (int)techTable_[SMAPeriod].size();
                for (int i = 0, dateRow = SMAPeriod - 1; i < techSize; i++, dateRow++) {
                    out << date_[dateRow] << "," << set_precision(techTable_[SMAPeriod][i]) << endl;
                }
                out.close();
            }
            cout << endl;
            break;
        }
        case 1: {
            break;
        }
        case 2: {
            break;
        }
        case 3: {
            for (int RSIPerid = 1; RSIPerid < 257; RSIPerid++) {
                if (RSIPerid % 10 == 0) {
                    cout << ".";
                }
                ofstream out;
                set_techFile_title(out, RSIPerid);
                int techSize = (int)techTable_[RSIPerid].size();
                for (int i = 0, dateRow = RSIPerid; i < techSize; i++, dateRow++) {
                    out << date_[dateRow] << "," << set_precision(techTable_[RSIPerid][i]) << endl;
                }
                out.close();
            }
            cout << endl;
            break;
        }
        default: {
            cout << "store_tech_to_vector exception" << endl;
            exit(1);
        }
    }
}

void CompanyInfo::set_techFile_title(ofstream &out, int techPerid) {
    if (techPerid < 10) {
        out.open(allTechOuputPath_.at(info_.techType_) + "/" + companyName_ + "_" + info_.techType_ + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100) {
        out.open(allTechOuputPath_.at(info_.techType_) + "/" + companyName_ + "_" + info_.techType_ + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100) {
        out.open(allTechOuputPath_.at(info_.techType_) + "/" + companyName_ + "_" + info_.techType_ + "_" + to_string(techPerid) + ".csv");
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

    void create_techTable(CompanyInfo *company);
    void output_techTable();

    TechTable(CompanyInfo *company, int techIndex);
};

TechTable::TechTable(CompanyInfo *company, int techIndex) : company_(company), techIndex_(techIndex), techType_(company->info_.allTech_[techIndex]) {
    create_techTable(company_);
}

void TechTable::create_techTable(CompanyInfo *company) {
    days_ = company->totalDays_ - company->tableStartRow_;
    date_.resize(days_);
    price_.resize(days_);
    for (int i = company->tableStartRow_, j = 0; i < company->totalDays_; i++, j++) {
        date_[j] = company->date_[i];
        price_[j] = company->price_[i];
    }
    techTable_.resize(days_);
    for (int i = 0; i < days_; i++) {
        techTable_[i].resize(257);
    }
    vector<path> techFilePath = get_path(company->allTechOuputPath_.at(techType_));
    int techFilePathSize = (int)techFilePath.size();
    if (techFilePathSize == 0) {
        cout << "no MA file" << endl;
        exit(1);
    }
    cout << "reading " << techType_ << " files";
    for (int i = 0; i < techFilePathSize; i++) {
        if (i % 16 == 0) {
            cout << ".";
        }
        vector<vector<string>> techFile = read_data(techFilePath[i]);
        int techFileSize = (int)techFile.size();
        if (i == 0 && techFile[techFileSize - 1][0] != date_[days_ - 1]) {
            cout << "last date of price file and techFile are different, need to generate new techFile" << endl;
            exit(1);
        }
        if (techFileSize - days_ < 0) {
            cout << endl;
            cout << company->companyName_ << " tech file not old enougth" << endl;
            exit(1);
        }
        for (int j = 0, k = techFileSize - days_; k < techFileSize; j++, k++) {
            techTable_[j][i + 1] = stod(techFile[k][1]);
        }
    }
    cout << endl;
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

TestWindow::TestWindow(CompanyInfo &company, string window) : company_(company), windowName_(window), windowNameEx_(company.info_.slidingWindowsEx_[distance(company.info_.slidingWindows_.begin(), find(company.info_.slidingWindows_.begin(), company.info_.slidingWindows_.end(), windowName_))]), tableStartRow_(company.tableStartRow_) {
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
    vector<string> trainTestType = find_train_and_test_len(windowNameEx_, windowType_);
    if (trainTestType.size() == 0) {
        cout << "testType.size() cant be 0" << endl;
        exit(1);
    }
    if (trainTestType.size() == 1) {
        trainLength_ = stoi(trainTestType[0]);
        testLength_ = trainLength_;
        windowType_ = 'S';
    }
    else {
        trainLength_ = stoi(trainTestType[0]);
        testLength_ = stoi(trainTestType[1]);
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
    switch (TestWindow::windowType_) {
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
    const vector<int> eachVariableBitsNum_ = {8, 8, 8, 8};
    const vector<vector<int>> traditionStrategy_ = {{5, 10, 5, 10}, {5, 20, 5, 20}, {5, 60, 5, 60}, {10, 20, 10, 20}, {10, 60, 10, 60}, {20, 60, 20, 60}, {120, 240, 120, 240}};

    static bool buy_condition0(vector<TechTable> *tables, int stockHold, int i, int endRow, int buy1, int buy2) {
        double MAbuy1PreDay = (*tables)[0].techTable_[i - 1][buy1];
        double MAbuy2PreDay = (*tables)[0].techTable_[i - 1][buy2];
        double MAbuy1Today = (*tables)[0].techTable_[i][buy1];
        double MAbuy2Today = (*tables)[0].techTable_[i][buy2];
        return stockHold == 0 && MAbuy1PreDay <= MAbuy2PreDay && MAbuy1Today > MAbuy2Today && i != endRow;
    }

    static bool sell_condition0(vector<TechTable> *tables, int stockHold, int i, int endRow, int sell1, int sell2) {
        double MAsell1PreDay = (*tables)[0].techTable_[i - 1][sell1];
        double MAsell2PreDay = (*tables)[0].techTable_[i - 1][sell2];
        double MAsell1Today = (*tables)[0].techTable_[i][sell1];
        double MAsell2Today = (*tables)[0].techTable_[i][sell2];
        return stockHold != 0 && ((MAsell1PreDay >= MAsell2PreDay && MAsell1Today < MAsell2Today) || i == endRow);
    }
};

class RSI {
   public:
    const vector<int> eachVariableBitsNum_ = {8, 7, 7};
    const vector<vector<int>> traditionStrategy_ = {{5, 20, 80}, {5, 30, 70}, {6, 20, 80}, {6, 30, 70}, {14, 20, 80}, {14, 30, 70}};
    static bool buy_condition0(vector<TechTable> *tables, int stockHold, int i, int endRow, int RSIPeriod, int overSold) {
        double RSI = (*tables)[0].techTable_[i][RSIPeriod];
        return stockHold == 0 && RSI <= overSold && i != endRow;
    }

    static bool sell_condition0(vector<TechTable> *tables, int stockHold, int i, int endRow, int RSIPeriod, int overBought) {
        double RSI = (*tables)[0].techTable_[i][RSIPeriod];
        return stockHold != 0 && ((RSI >= overBought) || i == endRow);
    }
};

class Particle {
   private:
    CompanyInfo *company_;

   public:
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

    void instant_trade(string startDate, string endDate, bool hold = false);
    void find_instant_trade_startRow_endRow(const string &startDate, const string &endDate, int &startRow, int &endRow);
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
    void push_tradeInfo_sell(int stockHold, int i);
    void push_holdInfo_sell(int endRow, vector<string> *holdInfoPtr, int i);
    void push_holdInfo_holding(vector<string> *holdInfoPtr, int i);
    void push_hodInfo_not_holding(vector<string> *holdInfoPtr, int i);
    void push_tradeInfo_last(bool lastRecord);
    void check_buyNum_sellNum();
    void reset(double RoR = 0);
    void measure(vector<double> &betaMatrix);
    void convert_bi_dec();
    void print(ostream &out);
    void print_train_test_data(string windowName, string outputPath, int actualStartRow, int actualEndRow, vector<string> *holdInfoPtr = nullptr);
    string set_output_filePath(string windowName, string &outputPath, int actualEndRow, int actualStartRow);

    Particle(CompanyInfo *company, bool isRecordOn = false, vector<int> variables = {});
};

Particle::Particle(CompanyInfo *company, bool isRecordOn, vector<int> variables) : company_(company), remain_(company->info_.totalCapitalLV_), isRecordOn_(isRecordOn) {
    switch (company_->info_.techIndex_) {
        case 0:
        case 1:
        case 2: {
            eachVariableBitsNum_ = MA().eachVariableBitsNum_;
            break;
        }
        case 3: {
            eachVariableBitsNum_ = RSI().eachVariableBitsNum_;
            break;
        }
        default: {
            cout << "no techIndex_ " << company_->info_.techIndex_ << ", choose a techIndex_" << endl;
            exit(1);
        }
    }
    bitsNum_ = accumulate(eachVariableBitsNum_.begin(), eachVariableBitsNum_.end(), 0);
    binary_.resize(bitsNum_);
    variableNum_ = (int)eachVariableBitsNum_.size();
    decimal_.resize(variableNum_);
    for (int i = 0; i < variables.size(); i++) {
        decimal_[i] = variables[i];
    }
}

void Particle::instant_trade(string startDate, string endDate, bool hold) {
    vector<TechTable> tmp = {TechTable(company_, company_->info_.techIndex_)};
    tables_ = &tmp;
    int startRow = -1, endRow = -1;
    find_instant_trade_startRow_endRow(startDate, endDate, startRow, endRow);
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

void Particle::find_instant_trade_startRow_endRow(const string &startDate, const string &endDate, int &startRow, int &endRow) {
    startRow = (int)distance((*tables_)[0].date_.begin(), find((*tables_)[0].date_.begin(), (*tables_)[0].date_.end(), startDate));
    if (startRow == (*tables_)[0].date_.size()) {
        cout << "instant trade startDate is not found" << endl;
        exit(1);
    }
    endRow = (int)distance((*tables_)[0].date_.begin(), find((*tables_)[0].date_.begin(), (*tables_)[0].date_.end(), endDate));
    if (endRow == (*tables_)[0].date_.size()) {
        cout << "instant trade endDate is not found" << endl;
        exit(1);
    }
}

void Particle::push_holdInfo_column_Name(bool hold, vector<string> &holdInfo, vector<string> *&holdInfoPtr) {
    if (hold) {
        holdInfo.clear();
        holdInfo.push_back("Date,Price,Hold,buy,sell date,sell " + company_->info_.techType_ + ",");
        switch (company_->info_.techIndex_) {
            case 0:
            case 1:
            case 2: {
                holdInfo.push_back("buy 1,buy 2,sell 1,sell 2\n");
                break;
            }
            case 3: {
                holdInfo.push_back("overSold,overbought\n");
            }
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
    out.open(company_->companyName_ + "_" + company_->info_.techType_ + titleVariables + "_instantTrade_" + startDate + "_" + endDate + ".csv");
    switch (company_->info_.techIndex_) {
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
            push_hodInfo_not_holding(holdInfoPtr, i);
        }
    }
    check_buyNum_sellNum();
    RoR_ = (remain_ - company_->info_.totalCapitalLV_) / company_->info_.totalCapitalLV_ * 100.0;
    push_tradeInfo_last(lastRecord);
}

void Particle::set_buy_sell_condition(bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow) {
    switch (company_->info_.techIndex_) {
        case 0:
        case 1:
        case 2: {
            buyCondition = MA::buy_condition0(tables_, stockHold, i, endRow, decimal_[0], decimal_[1]) && remain_ >= (*tables_)[0].price_[i];
            sellCondition = MA::sell_condition0(tables_, stockHold, i, endRow, decimal_[2], decimal_[3]);
            break;
        }
        case 3: {
            buyCondition = RSI::buy_condition0(tables_, stockHold, i, endRow, decimal_[0], decimal_[1]) && remain_ >= (*tables_)[0].price_[i];
            sellCondition = RSI::sell_condition0(tables_, stockHold, i, endRow, decimal_[0], decimal_[2]);
            // buyCondition = buyCondition && !((*tables_)[1].techTable_[i][5] <= (*tables_)[1].techTable_[i][10]);
            break;
        }
        default: {
            break;
        }
    }
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
        switch (company_->info_.techIndex_) {
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
        switch (company_->info_.techIndex_) {
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

void Particle::push_holdInfo_buy(vector<string> *holdInfoPtr, int i) {
    if (holdInfoPtr != nullptr) {
        string push;
        push += set_precision((*tables_)[0].price_[i]);
        push += ",";
        push += set_precision((*tables_)[0].price_[i]);
        push += ",,,";
        for (auto variable : decimal_) {
            push += set_precision((*tables_)[0].techTable_[i][variable]);
            push += ",";
        }
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
        switch (company_->info_.techIndex_) {
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
        for (auto variable : decimal_) {
            push += set_precision((*tables_)[0].techTable_[i][variable]);
            push += ",";
        }
        push += "\n";
        (*holdInfoPtr).push_back(push);
    }
}

void Particle::push_holdInfo_holding(vector<string> *holdInfoPtr, int i) {
    string push;
    push += set_precision((*tables_)[0].price_[i]);
    push += ",,,,";
    for (auto variable : decimal_) {
        push += set_precision((*tables_)[0].techTable_[i][variable]);
        push += ",";
    }
    push += "\n";
    (*holdInfoPtr).push_back(push);
}

void Particle::push_hodInfo_not_holding(vector<string> *holdInfoPtr, int i) {
    string push;
    push += ",,,,";
    for (auto variable : decimal_) {
        push += set_precision((*tables_)[0].techTable_[i][variable]);
        push += ",";
    }
    push += "\n";
    (*holdInfoPtr).push_back(push);
}

void Particle::check_buyNum_sellNum() {
    if (buyNum_ != sellNum_) {
        cout << "particle.buyNum_ = " << buyNum_ << ", particle.sellNum_ = " << sellNum_ << endl;
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

void Particle::measure(vector<double> &betaMatrix) {
    double r;
    int bitSize = (int)betaMatrix.size();
    for (int i = 0; i < bitSize; i++) {
        r = (double)rand() / (double)RAND_MAX;
        if (r < betaMatrix[i]) {
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
    if (company_->info_.techIndex_ == 3) {
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

void Particle::print_train_test_data(string windowName, string outputPath, int actualStartRow, int actualEndRow, vector<string> *holdInfoPtr) {
    string filePath = set_output_filePath(windowName, outputPath, actualEndRow, actualStartRow);
    isRecordOn_ = true;
    remain_ = company_->info_.totalCapitalLV_;
    trade(actualStartRow, actualEndRow, false, holdInfoPtr);
    if (holdInfoPtr == nullptr || company_->info_.mode_ == 1 || company_->info_.mode_ == 10) {
        ofstream out;
        out.open(filePath);
        out << "algo," << company_->info_.algoType_ << endl;
        out << "delta," << set_precision(company_->info_.delta_) << endl;
        out << "exp," << company_->info_.expNum_ << endl;
        out << "gen," << company_->info_.genNum_ << endl;
        out << "p number," << company_->info_.particleNum_ << endl;
        out << endl;
        out << "initial capital," << set_precision(company_->info_.totalCapitalLV_) << endl;
        out << "final capital," << set_precision(remain_) << endl;
        out << "final return," << set_precision(remain_ - company_->info_.totalCapitalLV_) << endl;
        out << endl;
        switch (company_->info_.techIndex_) {
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
        out << "trade," << sellNum_ << endl;
        out << "return rate," << set_precision(RoR_) << "%" << endl;
        out << endl;
        out << "best exp," << exp_ << endl;
        out << "best gen," << gen_ << endl;
        out << "best cnt," << bestCnt_ << endl;
        out << endl;
        print_trade_record(out);
        out.close();
    }
}

string Particle::set_output_filePath(string windowName, string &outputPath, int actualEndRow, int actualStartRow) {
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
        outputPath = company_->info_.techType_ + "_";
        outputPath += company_->companyName_ + "_";
        outputPath += company_->info_.algoType_ + "_";
        outputPath += delta + "_";
    }
    outputPath += (*tables_)[0].date_[actualStartRow] + "_";
    outputPath += (*tables_)[0].date_[actualEndRow] + ".csv";
    return outputPath;
}

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

class Train {
   private:
    CompanyInfo &company_;
    vector<TechTable> tables_;

    vector<Particle> particles_;
    vector<Particle> globalP_;  //0:best,1:globalBest,2:globalWorst,3:localBest,4:localWorst
    BetaMatrix betaMatrix_;

    int actualStartRow_ = -1;
    int actualEndRow_ = -1;

    double actualDelta_ = -1;
    int compareNew_ = -1;
    int compareOld_ = -1;

    void start_train(string targetWindow, string startDate, string endDate, bool debug);
    void set_variables_and_condition(string &targetWindow, string &startDate, string &endDate, bool &debug);
    void find_new_row(string &startDate, string &endDate);
    void create_particles(bool debug);
    void create_betaMatrix();
    TrainWindow set_window(string &targetWindow, string &startDate, int &windowIndex);
    void set_row_and_break_condition(TrainWindow &window, string &startDate, int &windowIndex, int &intervalIndex);
    ofstream set_debug_file(bool debug);
    void start_exp(ofstream &out, int expCnt, bool debug);
    void print_debug_exp(ofstream &out, int expCnt, bool debug);
    void start_gen(ofstream &out, int expCnt, int genCnt, bool debug);
    void print_debug_gen(ofstream &out, int genCnt, bool debug);
    void print_debug_particle(ofstream &out, int i, bool debug);
    void store_exp_gen(int expCnt, int genCnt);
    void update_local();
    void update_global();
    void run_algo();
    void QTS();
    void GQTS();
    void GNQTS();
    void print_debug_beta(ofstream &out, bool debug);
    void update_best(int renewBest);
    void clear_STL();

   public:
    Train(CompanyInfo &company, string targetWindow = "all", string startDate = "", string endDate = "", bool debug = false, bool record = false);
};

Train::Train(CompanyInfo &company, string targetWindow, string startDate, string endDate, bool debug, bool record) : company_(company), tables_{TechTable(&company, company.info_.techIndex_)}, actualDelta_(company.info_.delta_) {
    if (company.info_.testDeltaLoop_ == 0) {
        start_train(targetWindow, startDate, endDate, debug);
    }
    else {
        for (int loop = 0; loop < company.info_.testDeltaLoop_; loop++) {
            start_train(targetWindow, startDate, endDate, debug);
            actualDelta_ = company_.info_.delta_;
            actualDelta_ -= company.info_.testDeltaGap_ * (loop + 1);
        }
    }
}

void Train::start_train(string targetWindow, string startDate, string endDate, bool debug) {
    set_variables_and_condition(targetWindow, startDate, endDate, debug);
    find_new_row(startDate, endDate);
    create_particles(debug);
    create_betaMatrix();
    for (int windowIndex = 0; windowIndex < company_.info_.windowNumber_; windowIndex++) {
        TrainWindow window = set_window(targetWindow, startDate, windowIndex);
        srand(343);
        for (int intervalIndex = 0; intervalIndex < window.intervalSize_; intervalIndex += 2) {
            set_row_and_break_condition(window, startDate, windowIndex, intervalIndex);
            globalP_[0].reset();
            ofstream out = set_debug_file(debug);
            for (int expCnt = 0; expCnt < company_.info_.expNum_; expCnt++) {
                start_exp(out, expCnt, debug);
            }
            out.close();
            globalP_[0].print_train_test_data(window.windowName_, company_.allTrainFilePath_.at(company_.info_.techType_) + window.windowName_, actualStartRow_, actualEndRow_);
            cout << globalP_[0].RoR_ << "%" << endl;
        }
        cout << "==========" << endl;
    }
    clear_STL();
}

void Train::set_variables_and_condition(string &targetWindow, string &startDate, string &endDate, bool &debug) {
    if (startDate == "debug" || endDate == "debug") {
        debug = true;
        company_.allTrainFilePath_.at(company_.info_.techType_) = "";
    }
    if (targetWindow.length() == startDate.length()) {
        endDate = startDate;
        startDate = targetWindow;
        targetWindow = "A2A";
        company_.allTrainFilePath_.at(company_.info_.techType_) = "";
    }
    else {
        startDate = "";
    }
}

void Train::find_new_row(string &startDate, string &endDate) {
    if (startDate != "") {
        for (int i = 0; i < tables_[0].days_; i++) {
            if (startDate == tables_[0].date_[i]) {
                actualStartRow_ = i;
                break;
            }
        }
        for (int i = actualStartRow_; i < tables_[0].days_; i++) {
            if (endDate == tables_[0].date_[i]) {
                actualEndRow_ = i;
                break;
            }
        }
        if (actualStartRow_ == -1) {
            cout << "input trainStartDate is not found" << endl;
            exit(1);
        }
        if (actualEndRow_ == -1) {
            cout << "input trainEndDate is not found" << endl;
            exit(1);
        }
    }
}

void Train::create_particles(bool debug) {
    Particle p(&company_, debug);
    p.tables_ = &tables_;
    p.actualDelta_ = actualDelta_;
    for (int i = 0; i < company_.info_.particleNum_; i++) {
        particles_.push_back(p);
    }
    for (int i = 0; i < 5; i++) {
        globalP_.push_back(p);
    }
}

void Train::create_betaMatrix() {
    betaMatrix_.variableNum_ = particles_[0].variableNum_;
    betaMatrix_.eachVariableBitsNum_ = particles_[0].eachVariableBitsNum_;
    betaMatrix_.matrix_.resize(particles_[0].bitsNum_);
    betaMatrix_.bitsNum_ = particles_[0].bitsNum_;
}

TrainWindow Train::set_window(string &targetWindow, string &startDate, int &windowIndex) {
    string accuallWindow = company_.info_.slidingWindows_[windowIndex];
    if (targetWindow != "all") {
        accuallWindow = targetWindow;
        windowIndex = company_.info_.windowNumber_;
    }
    TrainWindow window(company_, accuallWindow);
    if (startDate == "") {
        window.print_train();
    }
    if (company_.allTrainFilePath_.at(company_.info_.techType_) == "") {
        window.windowName_ = "";
    }
    return window;
}

void Train::set_row_and_break_condition(TrainWindow &window, string &startDate, int &windowIndex, int &intervalIndex) {
    if (startDate != "") {
        windowIndex = company_.info_.windowNumber_;
        intervalIndex = window.intervalSize_;
    }
    else {
        actualStartRow_ = window.interval_[intervalIndex];
        actualEndRow_ = window.interval_[intervalIndex + 1];
    }
    cout << tables_[0].date_[actualStartRow_] << "~" << tables_[0].date_[actualEndRow_] << endl;
}

ofstream Train::set_debug_file(bool debug) {
    ofstream out;
    if (debug) {
        string delta = set_precision(actualDelta_);
        delta.erase(delta.find_last_not_of('0') + 1, std::string::npos);
        string title;
        title += "debug_";
        title += company_.companyName_ + "_";
        title += company_.info_.techType_ + "_";
        title += company_.info_.algoType_ + "_";
        title += delta + "_";
        title += tables_[0].date_[actualStartRow_] + "_";
        title += tables_[0].date_[actualEndRow_] + ".csv";
        out.open(title);
    }
    return out;
}

void Train::start_exp(ofstream &out, int expCnt, bool debug) {
    print_debug_exp(out, expCnt, debug);
    globalP_[1].reset();
    betaMatrix_.reset();
    for (int genCnt = 0; genCnt < company_.info_.genNum_; genCnt++) {
        start_gen(out, expCnt, genCnt, debug);
    }
    update_best(0);
}

void Train::print_debug_exp(ofstream &out, int expCnt, bool debug) {
    if (debug)
        out << "exp:" << expCnt << ",==========,==========" << endl;
}

void Train::start_gen(ofstream &out, int expCnt, int genCnt, bool debug) {
    print_debug_gen(out, genCnt, debug);
    globalP_[3].reset();
    globalP_[4].reset(company_.info_.totalCapitalLV_);
    for (int i = 0; i < company_.info_.particleNum_; i++) {
        particles_[i].reset();
        particles_[i].measure(betaMatrix_.matrix_);
        particles_[i].convert_bi_dec();
        particles_[i].trade(actualStartRow_, actualEndRow_);
        print_debug_particle(out, i, debug);
    }
    store_exp_gen(expCnt, genCnt);
    update_local();
    update_global();
    run_algo();
    print_debug_beta(out, debug);
}

void Train::print_debug_gen(ofstream &out, int genCnt, bool debug) {
    if (debug)
        out << "gen:" << genCnt << ",=====" << endl;
}

void Train::print_debug_particle(ofstream &out, int i, bool debug) {
    if (debug)
        particles_[i].print(out);
}

void Train::store_exp_gen(int expCnt, int genCnt) {
    for_each(particles_.begin(), particles_.end(), [genCnt, expCnt](auto &p) {
        p.exp_ = expCnt;
        p.gen_ = genCnt;
    });
}

void Train::update_local() {
    for (auto p : particles_) {
        if (p.RoR_ > globalP_[3].RoR_) {
            globalP_[3] = p;
        }
        if (p.RoR_ < globalP_[4].RoR_) {
            globalP_[4] = p;
        }
    }
}

void Train::update_global() {
    if (globalP_[3].RoR_ > globalP_[1].RoR_) {
        globalP_[1] = globalP_[3];
    }
}

void Train::run_algo() {
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
                //GNQTS();
            }
            //compare_and_multiply();
            break;
        }
        default: {
            cout << "wrong algo" << endl;
            exit(1);
        }
    }
}

void Train::QTS() {
    for (int i = 0; i < betaMatrix_.bitsNum_; i++) {
        if (globalP_[3].binary_[i] == 1) {
            betaMatrix_.matrix_[i] += actualDelta_;
        }
        if (globalP_[4].binary_[i] == 1) {
            betaMatrix_.matrix_[i] -= actualDelta_;
        }
    }
}

void Train::GQTS() {
    for (int i = 0; i < betaMatrix_.bitsNum_; i++) {
        if (globalP_[1].binary_[i] == 1) {
            betaMatrix_.matrix_[i] += actualDelta_;
        }
        if (globalP_[4].binary_[i] == 1) {
            betaMatrix_.matrix_[i] -= actualDelta_;
        }
    }
}

void Train::GNQTS() {
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

void Train::print_debug_beta(ofstream &out, bool debug) {
    if (debug) {
        switch (company_.info_.algoIndex_) {
            case 0: {
                out << "local best" << endl;
                globalP_[3].print(out);
                out << "local worst" << endl;
                globalP_[4].print(out);
                break;
            }
            case 1:
            case 2: {
                out << "global best" << endl;
                globalP_[1].print(out);
                out << "local worst" << endl;
                globalP_[4].print(out);
                break;
            }
            case 3: {
                out << "global best" << endl;
                globalP_[1].print(out);
                out << "local best" << endl;
                globalP_[3].print(out);
                out << "local worst" << endl;
                globalP_[4].print(out);
                out << actualDelta_ << endl;
                break;
            }
        }
        betaMatrix_.print(out);
    }
}

void Train::update_best(int renewBest) {
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

void Train::clear_STL() {
    particles_.clear();
    globalP_.clear();
    betaMatrix_.matrix_.clear();
}

class Test {
   private:
    CompanyInfo &company_;
    Particle p_;
    vector<TechTable> tables_;
    bool tradition_ = false;
    bool hold_ = false;
    vector<string> holdInfo_;
    vector<string> *holdInfoPtr_ = nullptr;

    void add_tables(vector<int> addtionTable);
    void set_test_file_path(string &trainFilePath, string &testFileOutputPath);
    void start_test(string &actualWindow, string &targetWindow, const string &testFileOutputPath, const string &trainFilePath, int &windowIndex);
    TestWindow set_window(string &targetWindow, string &actualWindow, int &windowIndex);
    void check_exception(vector<path> &eachTrainFilePath, TestWindow &window);
    void set_variables(vector<vector<std::string>> &thisTrainFile);
    void print_test_holdInfo(TestWindow &window);

   public:
    Test(CompanyInfo &company, string targetWindow = "all", bool tradition = false, bool hold = false, vector<int> addtionTable = {});
};

Test::Test(CompanyInfo &company, string targetWindow, bool tradition, bool hold, vector<int> addtionTable) : company_(company), p_(&company), tables_{TechTable(&company, company.info_.techIndex_)}, tradition_(tradition), hold_(hold) {
    add_tables(addtionTable);
    p_.tables_ = &tables_;
    string trainFilePath;
    string testFileOutputPath;
    set_test_file_path(trainFilePath, testFileOutputPath);
    for (int windowIndex = 0; windowIndex < company_.info_.windowNumber_; windowIndex++) {
        string actualWindow = company_.info_.slidingWindows_[windowIndex];
        if (actualWindow != "A2A") {
            start_test(actualWindow, targetWindow, testFileOutputPath, trainFilePath, windowIndex);
        }
    }
}

void Test::add_tables(vector<int> addtionTable) {
    for (int i = 0; i < addtionTable.size(); i++) {
        tables_.push_back(TechTable(&company_, addtionTable[i]));
    }
}

void Test::set_test_file_path(string &trainFilePath, string &testFileOutputPath) {
    if (tradition_) {
        trainFilePath = company_.allTrainTraditionFilePath_.at(company_.info_.techType_);
        testFileOutputPath = company_.allTestTraditionFilePath_.at(company_.info_.techType_);
    }
    else {
        trainFilePath = company_.allTrainFilePath_.at(company_.info_.techType_);
        testFileOutputPath = company_.allTestFilePath_.at(company_.info_.techType_);
    }
    cout << "test " << company_.companyName_ << endl;
}

void Test::start_test(string &actualWindow, string &targetWindow, const string &testFileOutputPath, const string &trainFilePath, int &windowIndex) {
    TestWindow window = set_window(targetWindow, actualWindow, windowIndex);
    vector<path> eachTrainFilePath = get_path(trainFilePath + window.windowName_);
    check_exception(eachTrainFilePath, window);
    for (int intervalIndex = 0, trainFileIndex = 0; intervalIndex < window.intervalSize_; intervalIndex += 2, trainFileIndex++) {
        vector<vector<string>> thisTrainFile = read_data(eachTrainFilePath[trainFileIndex]);
        p_.reset();
        set_variables(thisTrainFile);
        p_.push_holdInfo_column_Name(hold_, holdInfo_, holdInfoPtr_);
        p_.print_train_test_data(window.windowName_, testFileOutputPath + window.windowName_, window.interval_[intervalIndex], window.interval_[intervalIndex + 1], holdInfoPtr_);
        print_test_holdInfo(window);
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

void Test::check_exception(vector<path> &eachTrainFilePath, TestWindow &window) {
    if (eachTrainFilePath.size() != window.intervalSize_ / 2) {
        cout << window.windowName_ << " test interval number is not equal to train fle number" << endl;
        exit(1);
    }
}

void Test::print_test_holdInfo(TestWindow &window) {
    if (hold_) {
        ofstream holdFile;
        if (!tradition_) {
            holdFile.open(company_.allTestHoldFilePath_.at(company_.info_.techType_) + company_.companyName_ + "_" + window.windowName_ + ".csv");
        }
        else if (tradition_) {
            //set tradition hold file path
        }
        for (auto info : holdInfo_) {
            holdFile << info;
        }
        holdFile.close();
    }
}

void Test::set_variables(vector<vector<string>> &thisTrainFile) {
    p_.decimal_[0] = stoi(thisTrainFile[10][1]);
    p_.decimal_[1] = stoi(thisTrainFile[11][1]);
    p_.decimal_[2] = stoi(thisTrainFile[12][1]);
    switch (company_.info_.techIndex_) {
        case 0:
        case 1:
        case 2: {
            p_.decimal_[3] = stoi(thisTrainFile[13][1]);
            break;
        }
        case 3: {
            break;
        }
        default: {
        }
    }
}

class BH {
   public:
    double BHRoR;
    BH(string startDate, string endDate, CompanyInfo &company, double totalCPLv) {
        int startRow = -1;
        int endRow = -1;
        for (int i = 0; i < company.totalDays_; i++) {
            if (startDate == company.date_[i]) {
                startRow = i;
                break;
            }
        }
        for (int i = startRow; i < company.totalDays_; i++) {
            if (endDate == company.date_[i]) {
                endRow = i;
                break;
            }
        }
        if (startRow == -1 || endRow == -1) {
            cout << "cant find B&H startRow or endRow" << endl;
            exit(1);
        }
        int stockHold = totalCPLv / company.price_[startRow];
        double remain = totalCPLv - stockHold * company.price_[startRow];
        remain += stockHold * company.price_[endRow];
        BHRoR = ((remain - totalCPLv) / totalCPLv);
    }
};

class Tradition {
   private:
    CompanyInfo &company_;
    vector<TechTable> tables_;
    vector<Particle> particles_;
    vector<vector<int>> traditionStrategy_;
    int traditionStrategyNum_ = -1;

    void train_Tradition(string &targetWindow);
    void create_particles();
    void set_strategy();
    TrainWindow set_window(string &targetWindow, int &windowIndex);
    void set_variables(int index);

   public:
    Tradition(CompanyInfo &company, string targetWindow = "all");
};

Tradition::Tradition(CompanyInfo &company, string targetWindow) : company_(company), tables_{TechTable(&company, company.info_.techIndex_)} {
    train_Tradition(targetWindow);
}

void Tradition::train_Tradition(string &targetWindow) {
    set_strategy();
    create_particles();
    cout << "train " << company_.companyName_ << " tradition" << endl;
    for (int windowIndex = 0; windowIndex < company_.info_.windowNumber_; windowIndex++) {
        TrainWindow window = set_window(targetWindow, windowIndex);
        string outputPath = company_.allTrainTraditionFilePath_.at(company_.info_.techType_) + window.windowName_;
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
}

void Tradition::set_strategy() {
    switch (company_.info_.techIndex_) {
        case 0:
        case 1:
        case 2: {
            traditionStrategy_ = MA().traditionStrategy_;
            break;
        }
        case 3: {
            traditionStrategy_ = RSI().traditionStrategy_;
            break;
        }
    }
    traditionStrategyNum_ = (int)traditionStrategy_.size();
}

void Tradition::create_particles() {
    Particle p(&company_);
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

void Tradition::set_variables(int index) {
    for (int i = 0; i < particles_[i].variableNum_; i++) {
        particles_[index].decimal_[i] = traditionStrategy_[index][i];
    }
}

static CompanyInfo set_company(vector<path> &companyPricePath, vector<path>::iterator &companyPricePathIter) {
    path targetCompanyPricePath = *companyPricePathIter;
    if (_info.setCompany_ != "all") {
        for (auto i : companyPricePath) {
            if (i.stem() == _info.setCompany_) {
                targetCompanyPricePath = i;
                break;
            }
        }
        companyPricePathIter = companyPricePath.end() - 1;
    }
    CompanyInfo company(_info, targetCompanyPricePath);
    cout << company.companyName_ << endl;
    return company;
}

int main(int argc, const char *argv[]) {
    time_point begin = steady_clock::now();
    vector<path> companyPricePath = get_path(_info.pricePath_);
    try {
        for (auto companyPricePathIter = companyPricePath.begin(); companyPricePathIter != companyPricePath.end(); companyPricePathIter++) {
            CompanyInfo company = set_company(companyPricePath, companyPricePathIter);
            switch (company.info_.mode_) {
                case 0: {
                    Train train(company, company.info_.setWindow_);
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
                    Test testTradition(company, company.info_.setWindow_, true, false);
                    break;
                }
                case 10: {
                    //                Test(company, company.info_.setWindow_, false, true, vector<int>{0});
                    //                Tradition tradition(company);
                    //                Train train(company, "2011-12-01", "2011-12-30", "debug");
                    //                Particle(&company, true, vector<int>{5, 20, 5, 20}).instant_trade("2020-01-02", "2021-06-30");
                    //                Particle(&company, true, vector<int>{70, 44, 85, 8}).instant_trade("2011-12-01", "2011-12-30");
                    //                Particle(&company, true, vector<int>{5, 10, 5, 10}).instant_trade("2020-01-02", "2020-05-29", true);
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
