#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

    //#include "CompanyInfo.h"
    //#include "TechTable.h"
#include "functions.h"

using namespace std;
using namespace chrono;
using namespace filesystem;

int _mode = 10;
string _setCompany = "AAPL";
string _setWindow = "1W1";
int _techIndex = 0;
vector<string> _allTech = {"SMA", "WMA", "EMA", "RSI"};
int _algoIndex = 2;
vector<string> _allAlgo = {"QTS", "GQTS", "GNQTS", "KNQTS"};

const path _pricePath = "price";
string _testStartYear = "2012-01";
string _testEndYear = "2021-01";

#define PARTICAL_AMOUNT 10
#define TOTAL_CP_LV 10000000.0

vector<string> _slidingWindows = {"A2A", "YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

vector<string> _slidingWindowsEx = {"A2A", "36M36", "36M24", "36M18", "36M12", "36M6", "36M3", "36M1", "24M24", "24M18", "24M12", "24M6", "24M3", "24M1", "18M18", "18M12", "18M6", "18M3", "18M1", "12M12", "12M6", "12M3", "12M1", "6M6", "6M3", "6M1", "3M3", "3M1", "1M1", "6M", "3M", "1M", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

class CompanyInfo {
public:
    string companyName_;
    vector<string> allTech_;
    int techIndex_ = -1;
    string techType_;
    
    map<string, string> allResultOutputPath_;
    map<string, string> allTechOuputPath_;
    map<string, string> allTrainFilePath_;
    map<string, string> allTestFilePath_;
    map<string, string> allTrainTraditionFilePath_;
    map<string, string> allTestTraditionFilePath_;
    
    vector<string> slidingWindows_;
    vector<string> slidingWindowsEx_;
    int windowNumber_ = -1;
    string testStartYear_;
    string testEndYear_;
    double testLength_;
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
        allResultOutputPath_[tech] = tech + "_result";
        allTechOuputPath_[tech] = tech + "/" + companyName_;
        allTrainFilePath_[tech] = tech + "_result" + "/" + companyName_ + "/train/";
        allTestFilePath_[tech] = tech + "_result" + "/" + companyName_ + "/test/";
        allTrainTraditionFilePath_[tech] = tech + "_result" + "/" + companyName_ + "/trainTradition/";
        allTestTraditionFilePath_[tech] = tech + "_result" + "/" + companyName_ + "/testTradition/";
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
        create_directories(allTrainFilePath_[techType_] + i);
        create_directories(allTestFilePath_[techType_] + i);
        create_directories(allTrainTraditionFilePath_[techType_] + i);
        create_directories(allTestTraditionFilePath_[techType_] + i);
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
        out.open(allTechOuputPath_[techType_] + "/" + companyName_ + "_" + techType_ + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100) {
        out.open(allTechOuputPath_[techType_] + "/" + companyName_ + "_" + techType_ + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100) {
        out.open(allTechOuputPath_[techType_] + "/" + companyName_ + "_" + techType_ + "_" + to_string(techPerid) + ".csv");
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
    
        //    TechTable(CompanyInfo &company, int techIndex);
};

    //TechTable::TechTable(CompanyInfo &company, int techIndex) : companyName_(company.companyName_), techIndex_(techIndex), techType_(company.allTech_[techIndex]) {
    //    create_techTable(company);
    //}

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
    techFilePath = get_path(company.allTechOuputPath_[techType_]);
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
    vector<int> binary_;
    vector<int> decimal_;
    
    static bool buy_condition0(TechTable &table0, int stockHold, int i, int endRow, int buy1, int buy2) {
        double MAbuy1PreDay = table0.techTable_[i - 1][buy1];
        double MAbuy2PreDay = table0.techTable_[i - 1][buy2];
        double MAbuy1Today = table0.techTable_[i][buy1];
        double MAbuy2Today = table0.techTable_[i][buy2];
        return stockHold == 0 && MAbuy1PreDay <= MAbuy2PreDay && MAbuy1Today > MAbuy2Today && i != endRow;
    }
    
    static bool sell_condition0(TechTable &table0, int stockHold, int i, int endRow, int sell1, int sell2) {
        double MAsell1PreDay = table0.techTable_[i - 1][sell1];
        double MAsell2PreDay = table0.techTable_[i - 1][sell2];
        double MAsell1Today = table0.techTable_[i][sell1];
        double MAsell2Today = table0.techTable_[i][sell2];
        return stockHold != 0 && ((MAsell1PreDay >= MAsell2PreDay && MAsell1Today < MAsell2Today) || i == endRow);
    }
    
    MA() {
        int bitNum = 0;
        for (auto i : bitsSize_) {
            bitNum += i;
        }
        binary_.resize(bitNum);
        decimal_.resize(bitsSize_.size());
    }
};

class RSI {
public:
    const vector<int> bitsSize_ = {8, 7, 7};
    vector<int> binary_;
    vector<int> decimal_;
    
    static bool buy_condition0(TechTable &table0, int stockHold, int i, int endRow, int RSIPeriod, int overSold) {
        double RSI = table0.techTable_[i][RSIPeriod];
        return stockHold == 0 && RSI <= overSold && i != endRow;
    }
    
    static bool sell_condition0(TechTable &table0, int stockHold, int i, int endRow, int RSIPeriod, int overBought) {
        double RSI = table0.techTable_[i][RSIPeriod];
        return stockHold != 0 && ((RSI >= overBought) || i == endRow);
    }
    
    RSI() {
        int bitNum = 0;
        for (auto i : bitsSize_) {
            bitNum += i;
        }
        binary_.resize(bitNum);
        decimal_.resize(bitsSize_.size());
    }
};

class Particle {
public:
    double totalCapitalLV_ = -1;
    int techIndex_ = -1;
    vector<int> binary_;
    vector<int> decimal_;
    double remain_ = 0;
    double RoR_ = 0;
    int buyNum_ = 0;
    int sellNum_ = 0;
    bool isRecordOn_ = false;
    vector<string> tradeRecord_;
    int gen_ = 0;
    int exp_ = 0;
    int bestCnt = 0;
    
    void ini_particle(int techIndex, double totalCapitalLV, bool on, vector<int> variables);
    void instant_trade(CompanyInfo &company, string startDate, string endDate);
    void set_instant_trade_file(CompanyInfo &company, ofstream &out, const string &endDate, const string &startDate);
    void print_trade_record(ofstream &out);
    void ini_buyNum_sellNum();
    void trade(TechTable &table, int startRow, int endRow, bool lastRecord = false);
    void set_buy_sell_condition(TechTable &table, bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow);
    void push_title();
    void push_buy_info(TechTable &table, int stockHold, int i);
    void push_sell_info(TechTable &table, int stockHold, int i);
    void push_last_info(bool lastRecord);
    void check_buyNum_sellNum();
    
    Particle(int techIndex = -1, double totalCapitalLV = -1, bool on = false, vector<int> variables = {});
};

Particle::Particle(int techIndex, double totalCapitalLV, bool on, vector<int> variables) : totalCapitalLV_(totalCapitalLV), techIndex_(techIndex), remain_(totalCapitalLV), isRecordOn_(on) {
    if (techIndex != -1) {
        ini_particle(techIndex, totalCapitalLV, on, variables);
    }
}

void Particle::ini_particle(int techIndex, double totalCapitalLV, bool on, vector<int> variables) {
    totalCapitalLV_ = totalCapitalLV;
    techIndex_ = techIndex;
    remain_ = totalCapitalLV;
    isRecordOn_ = on;
    switch (techIndex) {
        case 0:
        case 1:
        case 2: {
            binary_ = MA().binary_;
            decimal_ = MA().decimal_;
            for (int i = 0; i < variables.size(); i++) {
                decimal_[i] = variables[i];
            }
            break;
        }
        case 3: {
            binary_ = RSI().binary_;
            decimal_ = RSI().decimal_;
            for (int i = 0; i < variables.size(); i++) {
                decimal_[i] = variables[i];
            }
            break;
        }
        default: {
            cout << "no techIndex_ " << techIndex_ << ", choose a techIndex_" << endl;
            exit(1);
        }
    }
}

void Particle::instant_trade(CompanyInfo &company, string startDate, string endDate) {
    TechTable table;
    table.ini_techTable(company, techIndex_);
    int startRow = -1, endRow = -1;
    for (int dateRow = 0; dateRow < table.days_; dateRow++) {
        if (startDate == table.date_[dateRow]) {
            startRow = dateRow;
            break;
        }
    }
    for (int dateRow = startRow; dateRow < table.days_; dateRow++) {
        if (endDate == table.date_[dateRow]) {
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
    trade(table, startRow, endRow, true);
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
            out << "company,startDate,endDate,RSIPeriod,overBought,overSold" << endl;
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

void Particle::trade(TechTable &table, int startRow, int endRow, bool lastRecord) {
    int stockHold = 0;
    push_title();
    ini_buyNum_sellNum();
    bool buyCondition = false;
    bool sellCondition = false;
    for (int i = startRow; i <= endRow; i++) {
        set_buy_sell_condition(table, buyCondition, sellCondition, stockHold, i, endRow);
        if (buyCondition) {
            stockHold = floor(remain_ / table.price_[i]);
            remain_ = remain_ - stockHold * table.price_[i];
            buyNum_++;
            push_buy_info(table, stockHold, i);
        }
        else if (sellCondition) {
            remain_ = remain_ + (double)stockHold * table.price_[i];
            stockHold = 0;
            sellNum_++;
            push_sell_info(table, stockHold, i);
        }
    }
    check_buyNum_sellNum();
    RoR_ = (remain_ - totalCapitalLV_) / totalCapitalLV_ * 100.0;
    push_last_info(lastRecord);
}

void Particle::set_buy_sell_condition(TechTable &table, bool &buyCondition, bool &sellCondition, int stockHold, int i, int endRow) {
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            buyCondition = MA::buy_condition0(table, stockHold, i, endRow, decimal_[0], decimal_[1]) && remain_ >= table.price_[i];
            sellCondition = MA::sell_condition0(table, stockHold, i, endRow, decimal_[2], decimal_[3]);
            break;
        }
        case 3: {
            buyCondition = RSI::buy_condition0(table, stockHold, i, endRow, decimal_[0], decimal_[1]) && remain_ >= table.price_[i];
            sellCondition = RSI::sell_condition0(table, stockHold, i, endRow, decimal_[0], decimal_[2]);
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

void Particle::push_buy_info(TechTable &table, int stockHold, int i) {
    if (isRecordOn_) {
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_.push_back("buy," + table.date_[i] + "," + set_precision(table.price_[i]) + "," + set_precision(table.techTable_[i - 1][decimal_[0]]) + "," + set_precision(table.techTable_[i - 1][decimal_[1]]) + "," + set_precision(table.techTable_[i][decimal_[0]]) + "," + set_precision(table.techTable_[i][decimal_[1]]) + "," + to_string(stockHold) + "," + set_precision(remain_) + "," + set_precision(remain_ + stockHold * table.price_[i]) + "\n");
                break;
            }
            case 3: {
                tradeRecord_.push_back("buy," + table.date_[i] + "," + set_precision(table.price_[i]) + "," + set_precision(table.techTable_[i][decimal_[0]]) + "," + to_string(stockHold) + "," + set_precision(remain_) + "," + set_precision(remain_ + stockHold * table.price_[i]) + "\n");
                break;
            }
        }
    }
}

void Particle::push_sell_info(TechTable &table, int stockHold, int i) {
    if (isRecordOn_) {
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_.push_back("sell," + table.date_[i] + "," + set_precision(table.price_[i]) + "," + set_precision(table.techTable_[i - 1][decimal_[2]]) + "," + set_precision(table.techTable_[i - 1][decimal_[3]]) + "," + set_precision(table.techTable_[i][decimal_[2]]) + "," + set_precision(table.techTable_[i][decimal_[3]]) + "," + to_string(stockHold) + "," + set_precision(remain_) + "," + set_precision(remain_ + stockHold * table.price_[i]) + "\n\n");
                break;
            }
            case 3: {
                tradeRecord_.push_back("sell," + table.date_[i] + "," + set_precision(table.price_[i]) + "," + set_precision(table.techTable_[i][decimal_[0]]) + "," + to_string(stockHold) + "," + set_precision(remain_) + "," + set_precision(remain_ + stockHold * table.price_[i]) + "\n\n");
                break;
            }
        }
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

class BetaMatrix {
public:
    vector<double> betaMatrix_;
    
    void ini();
    void print(ofstream &out, bool debug);
        //    BetaMatrix();
};

class Train {
private:
    const double delta_ = 0.0012;
    const int expNumber_ = 50;
    const int generationNumber_ = 10000;
    const int particleAmount_ = 10;
    const double totalCapitalLV_ = 10000000;
    
public:
    CompanyInfo &company_;
    
    BetaMatrix betaMatrix_;
    vector<Particle> particles_;
    map<string, Particle> globalParticles_;
    
    int actualStartRow_ = -1;
    int actualEndRow_ = -1;
    
    TechTable table0_;
    
    void set_variables_condition(CompanyInfo &company, string &targetWindow, string &startDate, string &endDate, bool &debug) {
        if (targetWindow.length() == startDate.length()) {
            if (endDate == "debug") {
                debug = true;
            }
            endDate = startDate;
            startDate = targetWindow;
            targetWindow = "A2A";
            company.allTrainFilePath_[company.techType_] = "";
        }
        else if (startDate == "debug") {
            startDate = "";
            debug = true;
        }
    }
    
    Train(CompanyInfo &company, string targetWindow = "all", string startDate = "", string endDate = "", bool debug = false, bool record = false) : company_(company) {
        set_variables_condition(company, targetWindow, startDate, endDate, debug);
        table0_.ini_techTable(company, company.techIndex_);
        if (startDate != "") {
            for (int i = 0; i < table0_.days_; i++) {
                if (startDate == table0_.date_[i]) {
                    actualStartRow_ = i;
                    break;
                }
            }
            for (int i = actualStartRow_; i < table0_.days_; i++) {
                if (endDate == table0_.date_[i]) {
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
        
        for (int i = 0; i < particleAmount_; i++) {
            particles_.push_back(Particle(company.techIndex_, totalCapitalLV_));
        }
        for (int i = 0; i < 4; i++) {
            globalParticles_.insert({"localBest", Particle(company.techIndex_, totalCapitalLV_)});
            globalParticles_.insert({"localWorst", Particle(company.techIndex_, totalCapitalLV_)});
            globalParticles_.insert({"globalBest", Particle(company.techIndex_, totalCapitalLV_)});
            globalParticles_.insert({"best", Particle(company.techIndex_, totalCapitalLV_)});
        }
    }
};

int main(int argc, const char *argv[]) {
    time_point begin = steady_clock::now();
    vector<path> companyPricePath = get_path(_pricePath);
    string setCompany = _setCompany;
    string setWindow = _setWindow;
    int setMode = _mode;
    int techIndex = _techIndex;
    vector<string> allTech = _allTech;
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
        CompanyInfo company(targetCompanyPricePath, allTech, techIndex, _slidingWindows, _slidingWindowsEx, _testStartYear, _testEndYear);
        cout << company.companyName_ << endl;
            //        Train train(company, "2012-01-03", "2012-12-31", "debug");
            //        Particle(company.techIndex_, TOTAL_CP_LV, true, vector<int>{5, 20, 5, 20}).instant_trade(company, "2020-01-02", "2021-06-30");
            //        Particle(3, TOTAL_CP_LV, true, vector<int>{1, 93, 20}).instant_trade(company, "2011-12-27", "2012-01-03");
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
