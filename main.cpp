#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
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
    
    double delta_ = 0.0012;
    int expNumber_ = 1;
    int genNumber_ = 1;
    int particleNumber_ = 1;
    double totalCapitalLV_ = 10000000;
    
    int testDeltaLoop_ = 5;
    double testDeltaGap_ = 0.00001;
    double multiplyUp_ = -1;
    double multiplyDown_ = -1;
    
    int techIndex_ = 0;
    vector<string> allTech_ = {"SMA", "WMA", "EMA", "RSI"};
    int algoIndex_ = 2;
    vector<string> allAlgo_ = {"QTS", "GQTS", "GNQTS", "KNQTS"};
    
    path pricePath_ = "price";
    string testStartYear_ = "2012-01";
    string testEndYear_ = "2021-01";
    
    vector<string> slidingWindows_ = {"A2A", "YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};
    
    vector<string> slidingWindowsEx_ = {"A2A", "36M36", "36M24", "36M18", "36M12", "36M6", "36M3", "36M1", "24M24", "24M18", "24M12", "24M6", "24M3", "24M1", "18M18", "18M12", "18M6", "18M3", "18M1", "12M12", "12M6", "12M3", "12M1", "6M6", "6M3", "6M1", "3M3", "3M1", "1M1", "6M", "3M", "1M", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};
}const _info;


class CompanyInfo {
public:
    string companyName_;
    vector<string> allTech_;
    int techIndex_ = -1;
    string techType_;
    vector<string> slidingWindows_;
    vector<string> slidingWindowsEx_;
    int windowNumber_ = -1;
    string testStartYear_;
    string testEndYear_;
    double testLength_;
    
    map<string, string> allResultOutputPath_;
    map<string, string> allTechOuputPath_;
    map<string, string> allTrainFilePath_;
    map<string, string> allTestFilePath_;
    map<string, string> allTrainTraditionFilePath_;
    map<string, string> allTestTraditionFilePath_;
    
    int totalDays_;
    vector<string> date_;
    vector<double> price_;
    int testStartRow_ = -1;
    int testEndRow_ = -1;
    vector<vector<double>> techTable_;
    int tableStartRow_ = -1;
    
    void store_date_price(path priceFilePath);
    void create_folder();
    void find_table_start_row();
    void store_tech_to_vector();
    void output_Tech();
    void set_techFile_title(ofstream &out, int techPerid);
    
    CompanyInfo(path pricePath, vector<string> allTech, int techIndex, vector<string> slidingWindows, vector<string> slidingWindowEx, string testStartYear, string testEndYear);
};

CompanyInfo::CompanyInfo(path pricePath, vector<string> allTech, int techIndex, vector<string> slidingWindows, vector<string> slidingWindowEx, string testStartYear, string testEndYear) : companyName_(pricePath.stem().string()), allTech_(allTech), techIndex_(techIndex), techType_(allTech[techIndex]), slidingWindows_(slidingWindows), slidingWindowsEx_(slidingWindowEx), windowNumber_(int(slidingWindows.size())), testStartYear_(testStartYear), testEndYear_(testEndYear), testLength_(stod(testEndYear) - stod(testStartYear)) {
    for (auto tech : allTech_) {
        allResultOutputPath_.insert({tech, tech + "_result"});
        allTechOuputPath_.insert({tech, tech + "/" + companyName_});
        allTrainFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/train/"});
        allTestFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/test/"});
        allTrainTraditionFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/trainTradition/"});
        allTestTraditionFilePath_.insert({tech, tech + "_result" + "/" + companyName_ + "/testTradition/"});
    }
    store_date_price(pricePath);
    create_folder();
    find_table_start_row();
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
        if (j == 0 && date_[i - 1].substr(0, 7) == testStartYear_) {
            testStartRow_ = i - 1;
            j++;
        }
        else if (j == 1 && date_[i - 1].substr(0, 7) == testEndYear_) {
            testEndRow_ = i - 2;
            j++;
        }
    }
}

void CompanyInfo::create_folder() {
    create_directories(techType_ + "/" + companyName_);
    for (auto i : slidingWindows_) {
        create_directories(allTrainFilePath_.at(techType_) + i);
        create_directories(allTestFilePath_.at(techType_) + i);
        create_directories(allTrainTraditionFilePath_.at(techType_) + i);
        create_directories(allTestTraditionFilePath_.at(techType_) + i);
    }
}

void CompanyInfo::find_table_start_row() {
    char delimiter;
    int longestTrainMonth = -1;
    for (int i = 0; i < windowNumber_; i++) {
        vector<string> trainTest = find_train_and_test_len(slidingWindowsEx_[i], delimiter);
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
    cout << "calculating " << companyName_ << " " << techType_ << endl;
    vector<double> tmp;
    techTable_.push_back(tmp);
    switch (techIndex_) {
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
    cout << "saving " << techType_ << " file" << endl;
    switch (techIndex_) {
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
        out.open(allTechOuputPath_.at(techType_) + "/" + companyName_ + "_" + techType_ + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100) {
        out.open(allTechOuputPath_.at(techType_) + "/" + companyName_ + "_" + techType_ + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100) {
        out.open(allTechOuputPath_.at(techType_) + "/" + companyName_ + "_" + techType_ + "_" + to_string(techPerid) + ".csv");
    }
}

class TechTable {
public:
    string companyName_;
    int techIndex_ = -1;
    string techType_;
    
    int days_;
    vector<string> date_;
    vector<double> price_;
    vector<vector<double>> techTable_;
    
    void ini_techTable(CompanyInfo &company, int techIndex);
    void create_techTable(CompanyInfo &company);
    void output_techTable();
    
    TechTable(CompanyInfo &company, int techIndex);
};

TechTable::TechTable(CompanyInfo &company, int techIndex) : companyName_(company.companyName_), techIndex_(techIndex), techType_(company.allTech_[techIndex]) {
    create_techTable(company);
}

void TechTable::ini_techTable(CompanyInfo &company, int techIndex) {
    companyName_ = company.companyName_;
    techIndex_ = techIndex;
    techType_ = company.allTech_[techIndex];
    create_techTable(company);
}

void TechTable::create_techTable(CompanyInfo &company) {
    days_ = company.totalDays_ - company.tableStartRow_;
    date_.resize(days_);
    price_.resize(days_);
    for (int i = company.tableStartRow_, j = 0; i < company.totalDays_; i++, j++) {
        date_[j] = company.date_[i];
        price_[j] = company.price_[i];
    }
    techTable_.resize(days_);
    for (int i = 0; i < days_; i++) {
        techTable_[i].resize(257);
    }
    vector<path> techFilePath;
    techFilePath = get_path(company.allTechOuputPath_.at(techType_));
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
        vector<vector<string>> MAFile = read_data(techFilePath[i]);
        int techFileSize = (int)MAFile.size();
        if (techFileSize - days_ < 0) {
            cout << company.companyName_ << " MA file not old enougth" << endl;
            exit(1);
        }
        for (int j = 0, k = techFileSize - days_; k < techFileSize; j++, k++) {
            techTable_[j][i + 1] = stod(MAFile[k][1]);
        }
    }
    cout << endl;
}

void TechTable::output_techTable() {
    ofstream out;
    out.open(companyName_ + "_" + techType_ + "_table.csv");
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
public:
    string windowName_;
    string windowNameEx_;
    int tableStartRow_ = -1;
    int trainLength_ = -1;
    int testLength_ = -1;
    char windowType_;
    vector<int> interval_;
    
    void find_test_interval(CompanyInfo &company);
    void find_M_test(CompanyInfo &company);
    void find_W_test(CompanyInfo &company);
    void find_D_test(CompanyInfo &company);
    void print_test(CompanyInfo &company);
    
    TestWindow(CompanyInfo company, string window);
};

TestWindow::TestWindow(CompanyInfo company, string window) : windowName_(window), windowNameEx_(company.slidingWindowsEx_[distance(company.slidingWindows_.begin(), find(company.slidingWindows_.begin(), company.slidingWindows_.end(), windowName_))]), tableStartRow_(company.tableStartRow_) {
    if (windowName_ != "A2A") {
        find_test_interval(company);
        for (auto &i : interval_) {
            i -= tableStartRow_;
        }
    }
}

void TestWindow::find_test_interval(CompanyInfo &company) {
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
            find_M_test(company);
            break;
        }
        case 'W': {
            find_W_test(company);
            break;
        }
        case 'D': {
            find_D_test(company);
            break;
        }
        default: {
            cout << "test delimiter wrong" << endl;
            exit(1);
        }
    }
}

void TestWindow::find_M_test(CompanyInfo &company) {
    vector<int> startRow, endRow;
    for (int dateRow = company.testStartRow_, monthCnt = testLength_ - 1; dateRow <= company.testEndRow_; dateRow++) {
        if (company.date_[dateRow - 1].substr(5, 2) != company.date_[dateRow].substr(5, 2)) {
            monthCnt++;
            if (monthCnt == testLength_) {
                startRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    for (int dateRow = company.testStartRow_, monthCnt = 0; dateRow <= company.testEndRow_; dateRow++) {
        if (company.date_[dateRow].substr(5, 2) != company.date_[dateRow + 1].substr(5, 2)) {
            monthCnt++;
            if (monthCnt == testLength_ || dateRow == company.testEndRow_) {
                endRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::find_W_test(CompanyInfo &company) {
    vector<int> startRow, endRow;
    int smallWeekDay = -1;
    int bigWeekDay = -1;
    for (int dateRow = company.testStartRow_, monthCnt = testLength_ - 1; dateRow < company.testEndRow_; dateRow++) {
        smallWeekDay = cal_weekday(company.date_[dateRow - 1]);
        bigWeekDay = cal_weekday(company.date_[dateRow]);
        if (is_week_changed(company.date_, bigWeekDay, smallWeekDay, dateRow, dateRow - 1)) {
            monthCnt++;
            if (monthCnt == testLength_) {
                startRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    for (int dateRow = company.testStartRow_, monthCnt = 0; dateRow <= company.testEndRow_; dateRow++) {
        smallWeekDay = cal_weekday(company.date_[dateRow]);
        bigWeekDay = cal_weekday(company.date_[dateRow + 1]);
        if (is_week_changed(company.date_, bigWeekDay, smallWeekDay, dateRow + 1, dateRow)) {
            monthCnt++;
            if (monthCnt == testLength_ || dateRow == company.testEndRow_) {
                endRow.push_back(dateRow);
                monthCnt = 0;
            }
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::find_D_test(CompanyInfo &company) {
    vector<int> startRow, endRow;
    for (int dateRow = company.testStartRow_; dateRow <= company.testEndRow_; dateRow += testLength_) {
        startRow.push_back(dateRow);
    }
    for (int dateRow = company.testStartRow_ + testLength_ - 1; dateRow <= company.testEndRow_; dateRow += testLength_) {
        endRow.push_back(dateRow);
    }
    if (startRow.size() > endRow.size()) {
        endRow.push_back(company.testEndRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TestWindow::print_test(CompanyInfo &company) {
    cout << "test window: " << windowName_ << "=" << windowNameEx_ << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company.date_[*it + tableStartRow_] << "~" << company.date_[*(++it) + tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class TrainWindow : public TestWindow {
public:
    vector<int> interval_;
    
    void find_train_interval(CompanyInfo &company);
    void find_M_train(CompanyInfo &company);
    void find_regular_M_train(CompanyInfo &company, vector<int> &endRow, vector<int> &startRow);
    void find_star_train(CompanyInfo &company, vector<int> &endRow, vector<int> &startRow);
    void find_W_train(CompanyInfo &company);
    void find_D_train(CompanyInfo &company);
    void print_train(CompanyInfo &company);
    
    TrainWindow(CompanyInfo &company, string window);
};

TrainWindow::TrainWindow(CompanyInfo &company, string window) : TestWindow(company, window) {
    if (TestWindow::windowName_ != "A2A") {
        find_train_interval(company);
    }
    else {
        interval_.push_back(company.testStartRow_);
        interval_.push_back(company.testEndRow_);
    }
    for (auto &i : interval_) {
        i -= TestWindow::tableStartRow_;
    }
}

void TrainWindow::find_train_interval(CompanyInfo &company) {
    switch (TestWindow::windowType_) {
        case 'M':
        case 'S': {
            find_M_train(company);
            break;
        }
        case 'W': {
            find_W_train(company);
            break;
        }
        case 'D': {
            find_D_train(company);
            break;
        }
        default: {
            cout << "trainWindow exception" << endl;
            exit(1);
        }
    }
}

void TrainWindow::find_M_train(CompanyInfo &company) {
    vector<int> startRow, endRow;
    switch (TestWindow::windowType_) {
        case 'M': {
            find_regular_M_train(company, endRow, startRow);
            break;
        }
        case 'S': {
            find_star_train(company, endRow, startRow);
            break;
        }
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::find_regular_M_train(CompanyInfo &company, vector<int> &endRow, vector<int> &startRow) {
    int intervalSize = (int)TestWindow::interval_.size();
    for (int intervalIndex = 0; intervalIndex < intervalSize; intervalIndex += 2) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + TestWindow::tableStartRow_, monthCnt = 0; dateRow > 0; dateRow--) {
            if (company.date_[dateRow].substr(5, 2) != company.date_[dateRow - 1].substr(5, 2)) {
                monthCnt++;
                if (monthCnt == TestWindow::trainLength_) {
                    startRow.push_back(dateRow);
                    break;
                }
            }
        }
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + TestWindow::tableStartRow_);
    }
}

void TrainWindow::find_star_train(CompanyInfo &company, vector<int> &endRow, vector<int> &startRow) {
    int intervalSize = (int)TestWindow::interval_.size();
    for (int intervalIndex = 0; intervalIndex < intervalSize; intervalIndex++) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + TestWindow::tableStartRow_, monthCnt = 0; dateRow > 0; dateRow--) {
            if (company.date_[dateRow].substr(5, 2) != company.date_[dateRow - 1].substr(5, 2)) {
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

void TrainWindow::find_W_train(CompanyInfo &company) {
    vector<int> startRow, endRow;
    int smallWeekDay = -1;
    int bigWeekDay = -1;
    int intervalSize = (int)TestWindow::interval_.size();
    for (int intervalIndex = 0; intervalIndex < intervalSize; intervalIndex += 2) {
        for (int dateRow = TestWindow::interval_[intervalIndex] - 1 + company.tableStartRow_, weekCnt = 0; dateRow > 0; dateRow--) {
            smallWeekDay = cal_weekday(company.date_[dateRow - 1]);
            bigWeekDay = cal_weekday(company.date_[dateRow]);
            if (is_week_changed(company.date_, bigWeekDay, smallWeekDay, dateRow, dateRow - 1)) {
                weekCnt++;
                if (weekCnt == TestWindow::trainLength_) {
                    startRow.push_back(dateRow);
                    break;
                }
            }
        }
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + TestWindow::tableStartRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::find_D_train(CompanyInfo &company) {
    vector<int> startRow, endRow;
    int intervalSize = (int)TestWindow::interval_.size();
    for (int intervalIndex = 0; intervalIndex < intervalSize; intervalIndex += 2) {
        startRow.push_back(TestWindow::interval_[intervalIndex] - TestWindow::trainLength_ + TestWindow::tableStartRow_);
        endRow.push_back(TestWindow::interval_[intervalIndex] - 1 + TestWindow::tableStartRow_);
    }
    check_startRowSize_endRowSize((int)startRow.size(), (int)endRow.size());
    interval_ = save_startRow_EndRow(startRow, endRow);
}

void TrainWindow::print_train(CompanyInfo &company) {
    cout << "train window: " << TestWindow::windowName_ << "=" << TestWindow::windowNameEx_ << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company.date_[*it + TestWindow::tableStartRow_] << "~" << company.date_[*(++it) + TestWindow::tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class MA {
public:
    const vector<int> bitsSize_ = {8, 8, 8, 8};
    
    static bool buy_condition0(map<string, TechTable> *tables, int stockHold, int i, int endRow, string techType, int buy1, int buy2) {
        double MAbuy1PreDay = tables->at(techType).techTable_[i - 1][buy1];
        double MAbuy2PreDay = tables->at(techType).techTable_[i - 1][buy2];
        double MAbuy1Today = tables->at(techType).techTable_[i][buy1];
        double MAbuy2Today = tables->at(techType).techTable_[i][buy2];
        return stockHold == 0 && MAbuy1PreDay <= MAbuy2PreDay && MAbuy1Today > MAbuy2Today && i != endRow;
    }
    
    static bool sell_condition0(map<string, TechTable> *tables, int stockHold, int i, int endRow, string techType, int sell1, int sell2) {
        double MAsell1PreDay = tables->at(techType).techTable_[i - 1][sell1];
        double MAsell2PreDay = tables->at(techType).techTable_[i - 1][sell2];
        double MAsell1Today = tables->at(techType).techTable_[i][sell1];
        double MAsell2Today = tables->at(techType).techTable_[i][sell2];
        return stockHold != 0 && ((MAsell1PreDay >= MAsell2PreDay && MAsell1Today < MAsell2Today) || i == endRow);
    }
};

class RSI {
public:
    const vector<int> bitsSize_ = {8, 7, 7};
    
    static bool buy_condition0(map<string, TechTable> *tables, int stockHold, int i, int endRow, string techType, int RSIPeriod, int overSold) {
        double RSI = tables->at(techType).techTable_[i][RSIPeriod];
        return stockHold == 0 && RSI <= overSold && i != endRow;
    }
    
    static bool sell_condition0(map<string, TechTable> *tables, int stockHold, int i, int endRow, string techType, int RSIPeriod, int overBought) {
        double RSI = tables->at(techType).techTable_[i][RSIPeriod];
        return stockHold != 0 && ((RSI >= overBought) || i == endRow);
    }
};

class Particle {
public:
    int techIndex_ = -1;
    string techType_ = "";
    double totalCapitalLV_ = -1;
    bool isRecordOn_ = false;
    
    vector<int> bitsSize_;
    int bitsNum_ = 0;
    vector<int> binary_;
    int variableNum_ = 0;
    vector<int> decimal_;
    
    map<string, TechTable> *tables_ = nullptr;
    
    double remain_ = 0;
    double RoR_ = 0;
    int buyNum_ = 0;
    int sellNum_ = 0;
    vector<string> tradeRecord_;
    int gen_ = 0;
    int exp_ = 0;
    int bestCnt_ = 0;
    
    int testDeltaLoop_ = -1;
    double delta_ = -1;
    int algoIndex_ = -1;
    vector<string> allAlgo_;
    int expNumber_ = -1;
    int genNumber_ = -1;
    int particleNumber_ = -1;
    
    void ini_particle(int techIndex, string techType, double totalCapitalLV, bool on, vector<int> variables);
    void instant_trade(CompanyInfo &company, string startDate, string endDate);
    void set_instant_trade_file(CompanyInfo &company, ofstream &out, const string &endDate, const string &startDate);
    void print_trade_record(ofstream &out);
    void ini_buyNum_sellNum();
    void trade(int startRow, int endRow, bool lastRecord = false);
    void set_buy_sell_condition(bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow);
    void push_title();
    void push_buy_info(int stockHold, int i);
    void push_sell_info(int stockHold, int i);
    void push_last_info(bool lastRecord);
    void check_buyNum_sellNum();
    void reset(double RoR = 0);
    void measure(vector<double> &betaMatrix);
    void convert_bi_dec();
    void print(ofstream &out, bool debug);
    string set_output_filePath(CompanyInfo &company, string windowName, string &outputPath, int actualEndRow, int actualStartRow);
    void print_train_test_data(CompanyInfo &company, string windowName, string outputPath, int actualStartRow, int actualEndRow);
    
    Particle(int techIndex, string techType, double totalCapitalLV, bool on = false, vector<int> variables = {});
};

Particle::Particle(int techIndex, string techType, double totalCapitalLV, bool on, vector<int> variables) : techIndex_(techIndex), techType_(techType), totalCapitalLV_(totalCapitalLV), remain_(totalCapitalLV), isRecordOn_(on) {
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            bitsSize_ = MA().bitsSize_;
            break;
        }
        case 3: {
            bitsSize_ = RSI().bitsSize_;
            break;
        }
        default: {
            cout << "no techIndex_ " << techIndex_ << ", choose a techIndex_" << endl;
            exit(1);
        }
    }
    bitsNum_ = accumulate(bitsSize_.begin(), bitsSize_.end(), 0);
    binary_.resize(bitsNum_);
    variableNum_ = (int)bitsSize_.size();
    decimal_.resize(variableNum_);
    for (int i = 0; i < variables.size(); i++) {
        decimal_[i] = variables[i];
    }
        //    if (techIndex != -1) {
        //        ini_particle(techIndex, techType, totalCapitalLV, on, variables);
        //    }
}

void Particle::ini_particle(int techIndex, string techType, double totalCapitalLV, bool on, vector<int> variables) {
    totalCapitalLV_ = totalCapitalLV;
    techIndex_ = techIndex;
    techType_ = techType;
    remain_ = totalCapitalLV;
    isRecordOn_ = on;
    switch (techIndex) {
        case 0:
        case 1:
        case 2: {
            bitsSize_ = MA().bitsSize_;
            break;
        }
        case 3: {
            bitsSize_ = RSI().bitsSize_;
            break;
        }
        default: {
            cout << "no techIndex_ " << techIndex_ << ", choose a techIndex_" << endl;
            exit(1);
        }
    }
    binary_.resize(accumulate(bitsSize_.begin(), bitsSize_.end(), 0));
    decimal_.resize(bitsSize_.size());
    for (int i = 0; i < variables.size(); i++) {
        decimal_[i] = variables[i];
    }
}

void Particle::instant_trade(CompanyInfo &company, string startDate, string endDate) {
    map<string, TechTable> tmp{{techType_, TechTable(company, techIndex_)}};
    tables_ = &tmp;
    int startRow = -1, endRow = -1;
    for (int dateRow = 0; dateRow < tables_->at(techType_).days_; dateRow++) {
        if (startDate == tables_->at(techType_).date_[dateRow]) {
            startRow = dateRow;
            break;
        }
    }
    for (int dateRow = startRow; dateRow < tables_->at(techType_).days_; dateRow++) {
        if (endDate == tables_->at(techType_).date_[dateRow]) {
            endRow = dateRow;
            break;
        }
    }
    if (startRow == -1) {
        cout << "instant trade startDate is not found" << endl;
        exit(1);
    }
    if (endRow == -1) {
        cout << "instant trade endDate is not found" << endl;
        exit(1);
    }
    trade(startRow, endRow, true);
    ofstream out;
    set_instant_trade_file(company, out, endDate, startDate);
    print_trade_record(out);
    out.close();
}

void Particle::set_instant_trade_file(CompanyInfo &company, ofstream &out, const string &endDate, const string &startDate) {
    string titleVariables;
    string showVariablesInFile;
    for (auto i : decimal_) {
        titleVariables += "_";
        titleVariables += to_string(i);
        showVariablesInFile += ",";
        showVariablesInFile += to_string(i);
    }
    out.open(company.companyName_ + "_" + company.allTech_[techIndex_] + "_instantTrade_" + startDate + "_" + endDate + titleVariables + ".csv");
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            out << "company,startDate,endDate,buy1,buy2,sell1,sell2" << endl;
            break;
        }
        case 3: {
            out << "company,startDate,endDate,RSIPeriod,overSold,overBought" << endl;
            break;
        }
        default: {
            cout << "set_instant_trade_file exception" << endl;
            exit(1);
        }
    }
    out << company.companyName_ << "," << startDate << "," << endDate << showVariablesInFile << "\n\n";
}

void Particle::print_trade_record(ofstream &out) {
    for (auto record : tradeRecord_) {
        out << record;
    }
}

void Particle::ini_buyNum_sellNum() {
    if (buyNum_ != 0 || sellNum_ != 0) {
        buyNum_ = 0;
        sellNum_ = 0;
    }
}

void Particle::trade(int startRow, int endRow, bool lastRecord) {
    int stockHold = 0;
    push_title();
    ini_buyNum_sellNum();
    bool buyCondition = false;
    bool sellCondition = false;
    for (int i = startRow; i <= endRow; i++) {
        set_buy_sell_condition(buyCondition, sellCondition, stockHold, i, endRow);
        if (buyCondition) {
            stockHold = floor(remain_ / tables_->at(techType_).price_[i]);
            remain_ = remain_ - stockHold * tables_->at(techType_).price_[i];
            buyNum_++;
            push_buy_info(stockHold, i);
        }
        else if (sellCondition) {
            remain_ = remain_ + (double)stockHold * tables_->at(techType_).price_[i];
            stockHold = 0;
            sellNum_++;
            push_sell_info(stockHold, i);
        }
    }
    check_buyNum_sellNum();
    RoR_ = (remain_ - totalCapitalLV_) / totalCapitalLV_ * 100.0;
    push_last_info(lastRecord);
}

void Particle::set_buy_sell_condition(bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow) {
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            buyCondition = MA::buy_condition0(tables_, stockHold, i, endRow, techType_, decimal_[0], decimal_[1]) && remain_ >= tables_->at(techType_).price_[i];
            sellCondition = MA::sell_condition0(tables_, stockHold, i, endRow, techType_, decimal_[2], decimal_[3]);
            break;
        }
        case 3: {
            buyCondition = RSI::buy_condition0(tables_, stockHold, i, endRow, techType_, decimal_[0], decimal_[1]) && remain_ >= tables_->at(techType_).price_[i];
            sellCondition = RSI::sell_condition0(tables_, stockHold, i, endRow, techType_, decimal_[0], decimal_[2]);
            break;
        }
        default: {
            break;
        }
    }
}

void Particle::push_title() {
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

void Particle::push_buy_info(int stockHold, int i) {
    if (isRecordOn_) {
        string push;
        push += "buy,";
        push += tables_->at(techType_).date_[i] + ",";
        push += set_precision(tables_->at(techType_).price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                push += set_precision(tables_->at(techType_).techTable_[i - 1][decimal_[0]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i - 1][decimal_[1]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[0]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[1]]) + ",";
                break;
            }
            case 3: {
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        push += to_string(stockHold) + ",";
        push += set_precision(remain_) + ",";
        push += set_precision(remain_ + stockHold * tables_->at(techType_).price_[i]) + "\n";
        tradeRecord_.push_back(push);
    }
}

void Particle::push_sell_info(int stockHold, int i) {
    if (isRecordOn_) {
        string push;
        push += "sell,";
        push += tables_->at(techType_).date_[i] + ",";
        push += set_precision(tables_->at(techType_).price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                push += set_precision(tables_->at(techType_).techTable_[i - 1][decimal_[2]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i - 1][decimal_[3]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[2]]) + ",";
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[3]]) + ",";
                break;
            }
            case 3: {
                push += set_precision(tables_->at(techType_).techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        push += to_string(stockHold) + ",";
        push += set_precision(remain_) + ",";
        push += set_precision(remain_ + stockHold * tables_->at(techType_).price_[i]) + "\n\n";
        tradeRecord_.push_back(push);
    }
}

void Particle::check_buyNum_sellNum() {
    if (buyNum_ != sellNum_) {
        cout << "particle.buyNum_ = " << buyNum_ << ", particle.sellNum_ = " << sellNum_ << endl;
        exit(1);
    }
}

void Particle::push_last_info(bool lastRecord) {
    if (isRecordOn_ && lastRecord) {
        tradeRecord_.push_back("buyNum," + to_string(buyNum_) + ",sellNum," + to_string(sellNum_) + "\nremain," + set_precision(remain_) + "\nreturn rate," + set_precision(RoR_) + "%\n");
    }
}

void Particle::reset(double RoR) {
    fill(binary_.begin(), binary_.end(), 0);
    fill(decimal_.begin(), decimal_.end(), 0);
    buyNum_ = 0;
    sellNum_ = 0;
    remain_ = totalCapitalLV_;
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
        for (int fakeIndex = 0, variableBitPosition = bitIndex, power = bitsSize_[variableIndex] - 1; fakeIndex < bitsSize_[variableIndex]; fakeIndex++, variableBitPosition++, power--) {
            decimal_[variableIndex] += pow(2, power) * binary_[variableBitPosition];
        }
        decimal_[variableIndex]++;
        bitIndex += bitsSize_[variableIndex];
    }
}

void Particle::print(ofstream &out, bool debug) {
    for (int variableIndex = 0, bitIndex = 0; variableIndex < variableNum_; variableIndex++) {
        for (int fakeBitIndex = 0; fakeBitIndex < bitsSize_[variableIndex]; fakeBitIndex++, bitIndex++) {
            if (debug)
                out << binary_[bitIndex] << ",";
            else
                cout << binary_[bitIndex] << ",";
        }
        if (debug)
            out << ",";
        else
            cout << "|";
    }
    for (int variableIndex = 0; variableIndex < variableNum_; variableIndex++) {
        if (debug)
            out << decimal_[variableIndex] << ",";
        else
            cout << decimal_[variableIndex] << ",";
    }
    if (debug)
        out << set_precision(RoR_) << "%,";
    else
        cout << set_precision(RoR_) << "%,";
    if (debug)
        out << endl;
    else
        cout << endl;
}

string Particle::set_output_filePath(CompanyInfo &company, string windowName, string &outputPath, int actualEndRow, int actualStartRow) {
    if (outputPath != "") {
        if (testDeltaLoop_ > 0) {
            string folderName = windowName + "_" + to_string(delta_);
            create_directories(folderName);
            outputPath = folderName;
        }
        outputPath += "/";
    }
    else {
        string delta = set_precision(delta_);
        delta.erase(delta.find_last_not_of('0') + 1, std::string::npos);
        outputPath = company.techType_ + "_" + company.companyName_ + "_" + allAlgo_[algoIndex_] + "_" + delta + "_";
    }
    return outputPath + tables_->at(company.techType_).date_[actualStartRow] + "_" + tables_->at(company.techType_).date_[actualEndRow] + ".csv";
}

void Particle::print_train_test_data(CompanyInfo &company, string windowName, string outputPath, int actualStartRow, int actualEndRow) {
    string filePath = set_output_filePath(company, windowName, outputPath, actualEndRow, actualStartRow);
    isRecordOn_ = true;
    remain_ = totalCapitalLV_;
    trade(actualStartRow, actualEndRow);
    ofstream out;
    out.open(filePath);
    out << "algo," << allAlgo_[algoIndex_] << endl;
    out << "delta," << set_precision(delta_) << endl;
    out << "exp," << expNumber_ << endl;
    out << "gen," << genNumber_ << endl;
    out << "p number," << particleNumber_ << endl;
    out << endl;
    out << "initial capital," << set_precision(totalCapitalLV_) << endl;
    out << "final capital," << set_precision(remain_) << endl;
    out << "final return," << set_precision(remain_ - totalCapitalLV_) << endl;
    out << endl;
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            out << "buy1," << decimal_[0] << endl;
            out << "buy1," << decimal_[1] << endl;
            out << "buy1," << decimal_[2] << endl;
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
    out << "return rate," << set_precision(RoR_) << "%" << endl;
    out << endl;
    out << "best exp," << exp_ << endl;
    out << "best gen," << gen_ << endl;
    out << "best cnt," << bestCnt_ << endl;
    out << endl;
    print_trade_record(out);
    out.close();
    
}

class BetaMatrix {
public:
    int variableNum_;
    vector<int> bitsSize_;
    vector<double> matrix_;
    
    void reset();
    void print(ofstream &out, bool debug);
};

void BetaMatrix::reset() {
    fill(matrix_.begin(), matrix_.end(), 0.5);
}

void BetaMatrix::print(ofstream &out, bool debug) {
    if (debug)
        out << "beta matrix" << endl;
    else
        cout << "beta matrix" << endl;
    for (int variableIndex = 0, bitIndex = 0; variableIndex < variableNum_; variableIndex++) {
        for (int fakeBitIndex = 0; fakeBitIndex < bitsSize_[variableIndex]; fakeBitIndex++, bitIndex++) {
            if (debug)
                out << matrix_[bitIndex] << ",";
            else
                cout << matrix_[bitIndex] << ",";
        }
        if (debug)
            out << ",";
        else
            cout << "|";
    }
    if (debug)
        out << endl;
    else
        cout << endl;
    
}

class Train {
private:
    const Info &info_;
public:
    CompanyInfo &company_;
    map<string, TechTable> tables_;
    
    vector<Particle> particles_;
    map<string, Particle> globalParticles_;
    BetaMatrix betaMatrix_;
    
    int actualStartRow_ = -1;
    int actualEndRow_ = -1;
    
    double actualDelta_ = -1;
    int compareNew_ = -1;
    int compareOld_ = -1;
    
    void set_variables_and_condition(string &targetWindow, string &startDate, string &endDate, bool &debug) {
        if (startDate == "debug" || endDate == "debug") {
            debug = true;
            company_.allTrainFilePath_.at(company_.techType_) = "";
            if (targetWindow.length() == startDate.length()) {
                endDate = startDate;
                startDate = targetWindow;
                targetWindow = "A2A";
            }
            else {
                startDate = "";
            }
        }
    }
    
    void find_new_row(string &startDate, string &endDate) {
        if (startDate != "") {
            for (int i = 0; i < tables_.at(company_.techType_).days_; i++) {
                if (startDate == tables_.at(company_.techType_).date_[i]) {
                    actualStartRow_ = i;
                    break;
                }
            }
            for (int i = actualStartRow_; i < tables_.at(company_.techType_).days_; i++) {
                if (endDate == tables_.at(company_.techType_).date_[i]) {
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
    
    void create_particles(bool debug) {
        for (int i = 0; i < info_.particleNumber_; i++) {
            particles_.push_back(Particle(company_.techIndex_, company_.techType_, info_.totalCapitalLV_, debug));
            particles_[i].tables_ = &tables_;
            particles_[i].testDeltaLoop_ = info_.testDeltaLoop_;
            particles_[i].delta_ = actualDelta_;
            particles_[i].algoIndex_ = info_.algoIndex_;
            particles_[i].allAlgo_ = info_.allAlgo_;
            particles_[i].expNumber_ = info_.expNumber_;
            particles_[i].genNumber_ = info_.genNumber_;
            particles_[i].particleNumber_ = info_.particleNumber_;
        }
        globalParticles_.insert({"localBest", particles_[0]});
        globalParticles_.insert({"localWorst", particles_[0]});
        globalParticles_.insert({"globalBest", particles_[0]});
        globalParticles_.insert({"globalWorst", particles_[0]});
        globalParticles_.insert({"best", particles_[0]});
    }
    
    void create_betaMatrix() {
        betaMatrix_.variableNum_ = particles_[0].variableNum_;
        betaMatrix_.bitsSize_ = particles_[0].bitsSize_;
        betaMatrix_.matrix_.resize(particles_[0].bitsNum_);
    }
    
    TrainWindow set_window(string &targetWindow, string &startDate, int &windowIndex) {
        string accuallWindow = company_.slidingWindows_[windowIndex];
        if (targetWindow != "all") {
            accuallWindow = targetWindow;
            windowIndex = company_.windowNumber_;
        }
        TrainWindow window(company_, accuallWindow);
        if (startDate == "") {
            window.print_train(company_);
        }
        if (company_.allTrainFilePath_.at(company_.techType_) == "") {
            window.TestWindow::windowName_ = "";
        }
        return window;
    }
    
    void set_row_and_break_condition(TrainWindow &window, string &startDate, int &windowIndex, int &intervalIndex) {
        if (startDate != "") {
            windowIndex = company_.windowNumber_;
            intervalIndex = (int)window.interval_.size();
        }
        else {
            actualStartRow_ = window.interval_[intervalIndex];
            actualEndRow_ = window.interval_[intervalIndex + 1];
        }
        cout << tables_.at(company_.techType_).date_[actualStartRow_] << "~" << tables_.at(company_.techType_).date_[actualEndRow_] << endl;
    }
    
    ofstream set_debug_file(bool debug) {
        ofstream out;
        if (debug) {
            string delta = set_precision(actualDelta_);
            delta.erase(delta.find_last_not_of('0') + 1, std::string::npos);
            string title;
            title += "debug_";
            title += company_.companyName_ + "_";
            title += company_.techType_ + "_";
            title += info_.allAlgo_[info_.algoIndex_] + "_";
            title += delta + "_";
            title += tables_.at(company_.techType_).date_[actualStartRow_] + "_";
            title += tables_.at(company_.techType_).date_[actualEndRow_] + ".csv";
            out.open(title);
        }
        return out;
    }
    
    void print_debug_exp(ofstream &out, int expCnt, bool debug) {
        if (debug)
            out << "exp:" << expCnt << ",==========,==========" << endl;
    }
    
    void print_debug_gen(ofstream &out, int genCnt, bool debug) {
        if (debug)
            out << "gen:" << genCnt << ",=====" << endl;
    }
    
    void print_debug_particle(ofstream &out, int i, bool debug) {
        if (debug)
            particles_[i].print(out, debug);
    }
    
    void store_exp_gen(int expCnt, int genCnt) {
        for_each(particles_.begin(), particles_.end(), [genCnt, expCnt](auto &p) {
            p.exp_ = expCnt;
            p.gen_ = genCnt;
        });
    }
    
    void update_local() {
        for (auto p : particles_) {
            if (p.RoR_ > globalParticles_.at("localBest").RoR_) {
                globalParticles_.at("localBest") = p;
            }
            if (p.RoR_ < globalParticles_.at("localWorst").RoR_) {
                globalParticles_.at("localWorst") = p;
            }
        }
    }
    
    void update_global() {
        if (globalParticles_.at("localBest").RoR_ > globalParticles_.at("globalBest").RoR_) {
            globalParticles_.at("globalBest") = globalParticles_.at("localBest");
        }
    }
    
    void run_algo() {
        switch (info_.algoIndex_) {
            case 0: {
                if (globalParticles_.at("localBest").RoR_ > 0) {
                        //QTS();
                }
                break;
            }
            case 1: {
                if (globalParticles_.at("globalBest").RoR_ > 0) {
                        //GQTS();
                }
                break;
            }
            case 2: {
                if (globalParticles_.at("globalBest").RoR_ > 0) {
                        //GNQTS();
                }
                break;
            }
            case 3: {
                if (globalParticles_.at("globalBest").RoR_ > 0) {
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
    
    void print_debug_beta(ofstream &out, bool debug) {
        if (debug) {
            switch (info_.algoIndex_) {
                case 0: {
                    out << "local best" << endl;
                    globalParticles_.at("localBest").print(out, debug);
                    out << "local worst" << endl;
                    globalParticles_.at("localWorst").print(out, debug);
                    break;
                }
                case 1:
                case 2: {
                    out << "global best" << endl;
                    globalParticles_.at("globalBest").print(out, debug);
                    out << "local worst" << endl;
                    globalParticles_.at("localWorst").print(out, debug);
                    break;
                }
                case 3: {
                    out << "global best" << endl;
                    globalParticles_.at("globalBest").print(out, debug);
                    out << "local best" << endl;
                    globalParticles_.at("localBest").print(out, debug);
                    out << "local worst" << endl;
                    globalParticles_.at("localWorst").print(out, debug);
                    out << actualDelta_ << endl;
                    break;
                }
            }
            betaMatrix_.print(out, debug);
        }
    }
    
    void start_gen(ofstream &out, int expCnt, int genCnt, bool debug) {
        print_debug_gen(out, genCnt, debug);
        globalParticles_.at("localBest").reset();
        globalParticles_.at("localWorst").reset(info_.totalCapitalLV_);
        for (int i = 0; i < info_.particleNumber_; i++) {
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
    
    void start_exp(ofstream &out, int expCnt, bool debug) {
        print_debug_exp(out, expCnt, debug);
        globalParticles_.at("globalBest").reset();
        betaMatrix_.reset();
        for (int genCnt = 0; genCnt < info_.genNumber_; genCnt++) {
            start_gen(out, expCnt, genCnt, debug);
        }
        update_best(0);
    }
    
    void update_best(int renewBest) {
        if (globalParticles_.at("globalBest").RoR_ < globalParticles_.at("globalBest").RoR_) {
            globalParticles_.at("globalBest") = globalParticles_.at("globalBest");
        }
        switch (renewBest) {
            case 0: {
                if (globalParticles_.at("globalBest").binary_ == globalParticles_.at("best").binary_) {
                    globalParticles_.at("best").bestCnt_++;
                }
                break;
            }
            case 1: {
                if (globalParticles_.at("globalBest").RoR_ == globalParticles_.at("best").RoR_) {
                    globalParticles_.at("best").bestCnt_++;
                }
                break;
            }
            default: {
                cout << "" << endl;
                exit(1);
            }
        }
    }
    
    void clear_STL() {
        particles_.clear();
        globalParticles_.clear();
        betaMatrix_.matrix_.clear();
    }
    
    void start_train(string targetWindow, string startDate, string endDate, bool debug) {
        set_variables_and_condition(targetWindow, startDate, endDate, debug);
        find_new_row(startDate, endDate);
        create_particles(debug);
        create_betaMatrix();
        for (int windowIndex = 0; windowIndex < company_.windowNumber_; windowIndex++) {
            TrainWindow window = set_window(targetWindow, startDate, windowIndex);
            srand(343);
            for (int intervalIndex = 0; intervalIndex < window.interval_.size(); intervalIndex += 2) {
                set_row_and_break_condition(window, startDate, windowIndex, intervalIndex);
                globalParticles_.at("best").reset();
                ofstream out = set_debug_file(debug);
                for (int expCnt = 0; expCnt < info_.expNumber_; expCnt++) {
                    start_exp(out, expCnt, debug);
                }
                out.close();
                globalParticles_.at("best").print_train_test_data(company_, window.TestWindow::windowName_, company_.allTrainFilePath_.at(company_.techType_) + window.TestWindow::windowName_, actualStartRow_, actualEndRow_);
                cout << globalParticles_.at("best").RoR_ << "%" << endl;
            }
            cout << "==========" << endl;
        }
        clear_STL();
    }
    
    Train(CompanyInfo &company, const Info &info, string targetWindow = "all", string startDate = "", string endDate = "", bool debug = false, bool record = false) : company_(company), info_(info), tables_{pair<string, TechTable>(company.techType_, TechTable(company, company.techIndex_))}, actualDelta_(info.delta_) {
        if (info_.testDeltaLoop_ == 0) {
            start_train(targetWindow, startDate, endDate, debug);
        }
        else {
            for (int loop = 0; loop < info_.testDeltaLoop_; loop++) {
                start_train(targetWindow, startDate, endDate, debug);
                actualDelta_ = _info.delta_;
                actualDelta_ -= info_.testDeltaGap_ * (loop + 1);
            }
        }
    }
};

int main(int argc, const char *argv[]) {
    time_point begin = steady_clock::now();
    vector<path> companyPricePath = get_path(_info.pricePath_);
    string setCompany = _info.setCompany_;
    string setWindow = _info.setWindow_;
    int setMode = _info.mode_;
    int techIndex = _info.techIndex_;
    vector<string> allTech = _info.allTech_;
    for (int companyIndex = 0; companyIndex < companyPricePath.size(); companyIndex++) {
        path targetCompanyPricePath = companyPricePath[companyIndex];
        if (setCompany != "all") {
            for (auto i : companyPricePath) {
                if (i.stem() == setCompany) {
                    targetCompanyPricePath = i;
                    break;
                }
            }
            companyPricePath.clear();
            companyPricePath.push_back(targetCompanyPricePath);
        }
        CompanyInfo company(targetCompanyPricePath, allTech, techIndex, _info.slidingWindows_, _info.slidingWindowsEx_, _info.testStartYear_, _info.testEndYear_);
        cout << company.companyName_ << endl;
        Train train(company, _info, setWindow);
            //        Particle(company.techIndex_, company.techType_, TOTAL_CP_LV, true, vector<int>{5, 20, 5, 20}).instant_trade(company, "2020-01-02", "2021-06-30");
            //        Particle(3, _allTech[3], TOTAL_CP_LV, true, vector<int>{44, 70, 42}).instant_trade(company, "2011-12-23", "2011-12-30");
        Particle p(3, "RSI", _info.totalCapitalLV_);
        ofstream out;
        p.print(out, false);
        switch (setMode) {
                    //            case 0: {
                    //                company.train(setWindow);
                    //                break;
                    //            }
                    //            case 1: {
                    //                company.test(setWindow);
                    //                break;
                    //            }
                    //            case 2: {
                    //                Tradition tradition(company, setWindow);
                    //                break;
                    //            }
                    //            case 3: {
                    //                IRRout outputIRR(_testYearLength, companyPricePath, _slidingWindows, setMA, TOTAL_CP_LV, _outputPath);
                    //                break;
                    //            }
                    //            case 10: {
                    //                //                company.output_MA();
                    //                //                company.train("debug", "2020-01-02", "2021-06-30");
                    //                //                company.train("2012-01-03", "2012-12-31");
                    //                //                company.instant_trade("2020-01-02", "2021-06-30", 43, 236, 20, 95);
                    //                break;
                    //            }
        }
    }
    time_point end = steady_clock::now();
    cout << "time: " << duration_cast<milliseconds>(end - begin).count() / 1000.0 << " s" << endl;
    return 0;
}
