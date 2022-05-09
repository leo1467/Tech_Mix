#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
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

    vector<int> techIndexs_ = {3};
    bool mixedTech_;
    int techIndex_;
    vector<string> allTech_ = {"SMA", "WMA", "EMA", "RSI"};
    string techType_;
    int mixedTechNum_;

    int algoIndex_ = 2;
    vector<string> allAlgo_ = {"QTS", "GQTS", "GNQTS", "KNQTS"};
    string algoType_;

    double delta_ = 0.00016;
    int expNum_ = 50;
    int genNum_ = 10000;
    int particleNum_ = 10;
    double totalCapitalLV_ = 10000000;

    int companyThreadNum_ = 0;  //若有很多公司要跑，可以視情況增加thread數量，一間一間公司跑設0
    int windowThreadNum_ = 0;  //若只跑一間公司，可以視情況增加thread數量，一個一個視窗跑設0，若有開公司thread，這個要設為0，避免產生太多thread

    bool debug_ = false;

    int testDeltaLoop_ = 0;
    double testDeltaGap_ = 0.00001;
    double multiplyUp_ = 1.01;
    double multiplyDown_ = 0.99;
    int compareMode_ = 0;

    string testStartYear_ = "2012-01";
    string testEndYear_ = "2022-01";
    double testLength_;

    string priceFolder_ = "price_2021/"; //通常不會換price，只有在更新股價檔的時後才改
    
    string expFolder_ = "exp_result/";
    string rootFolder_ = "result_";

    vector<string> slidingWindows_ = {"A2A", "YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    map<string, tuple<string, char, int, int>> slidingWindowPairs_;

    int windowNumber_;

    void set_techIndex_and_techType() {
        sort(techIndexs_.begin(), techIndexs_.end());
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
        mixedTechNum_ = (int)techIndexs_.size();
    }

    void slidingWindowToEx() {
        map<char, int> componentLength{{'Y', 12}, {'H', 6}, {'Q', 3}, {'M', 1}};
        tuple<string, char, int, int> tmp;
        vector<int> trainTest;
        char delimiter;
        string slidingWindowEx;
        for (auto windowName : slidingWindows_) {
            if (isalpha(windowName.front()) && windowName != "A2A") {  //做10個傳統滑動視窗
                vector<string> trainTestPair = cut_string(windowName, '2');
                if (trainTestPair.size() == 2) {
                    for_each(trainTestPair.begin(), trainTestPair.end(), [&](string windowComp) {
                        int totalComponentLength = 0;
                        for_each(windowComp.begin(), windowComp.end(), [&](char component) {  //計算訓練期及測試期長度
                            totalComponentLength += componentLength.at(component);
                        });
                        trainTest.push_back(totalComponentLength);
                        slidingWindowEx += to_string(totalComponentLength) + "M";
                    });
                    slidingWindowEx.pop_back();
                    tmp = {slidingWindowEx, 'M', trainTest[0], trainTest[1]};
                }
                else if (trainTestPair.size() == 1) {  //做*滑動視窗
                    slidingWindowEx += to_string(componentLength.at(trainTestPair[0][0])) + "M";
                    tmp = {slidingWindowEx, 'S', componentLength.at(trainTestPair[0][0]), componentLength.at(trainTestPair[0][0])};
                }
            }
            else {  //做A2A及其餘自訂滑動視窗
                trainTest = find_train_and_test_len(windowName, delimiter);
                tmp = {windowName, delimiter, trainTest[0], trainTest[1]};
            }
            slidingWindowPairs_.insert({windowName, tmp});
            trainTest.clear();
            slidingWindowEx.clear();
        }
    }

    void set_folder() {
        priceFolder_ = current_path().string() + "/" + priceFolder_;
        expFolder_ = current_path().parent_path().string() + "/" + expFolder_;
        create_directories(expFolder_);
        current_path(expFolder_);
        rootFolder_ = rootFolder_ + to_string(stoi(testEndYear_) - 1) + "/";
    }

    Info() {
        set_techIndex_and_techType();
        slidingWindows_ = set_certain_range_of_vec(setWindow_, slidingWindows_);
        windowNumber_ = (int)slidingWindows_.size();
        slidingWindowToEx();
        set_folder();
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
    Info *info_;
    string companyName_;
    Path paths_;

    int totalDays_;
    vector<string> date_;
    vector<double> price_;
    int testStartRow_ = -1;
    int testEndRow_ = -1;
    int testDays_;
    double oneYearDays_;
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

CompanyInfo::CompanyInfo(path pricePath, Info &info) : companyName_(pricePath.stem().string()), info_(&info) {
    set_paths(paths_);
    store_date_price(pricePath);
    // create_folder(paths_);
    find_table_start_row();
    cout << companyName_ << endl;
}

void CompanyInfo::set_paths(Path &paths) {
    for (auto tech : info_->allTech_) {
        paths.techOuputPaths_.push_back(info_->rootFolder_ + "/tech/" + tech + "/" + companyName_ + "/");

        paths.resultOutputPaths_.push_back(info_->rootFolder_ + "result_" + tech + "/");

        string companyRootFolder = info_->rootFolder_ + "result_" + tech + "/" + companyName_ + "/";
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
    if (info_->debug_) {
        paths.trainFilePaths_[info_->techIndex_].clear();
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
        if (j == 0 && date_[i - 1].substr(0, 7) == info_->testStartYear_) {
            testStartRow_ = i - 1;
            j++;
        }
        else if (j == 1 && date_[i - 1].substr(0, 7) == info_->testEndYear_) {
            testEndRow_ = i - 2;
            j++;
        }
    }
    testDays_ = testEndRow_ - testStartRow_ + 1;
    oneYearDays_ = testDays_ / info_->testLength_;
}

void CompanyInfo::create_folder(Path &paths) {
    if (!info_->mixedTech_) {
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
    for (auto i : info_->slidingWindows_) {
        create_directories(paths.trainFilePaths_[info_->techIndex_] + i);
        create_directories(paths.testFilePaths_[info_->techIndex_] + i);
        create_directories(paths.trainTraditionFilePaths_[info_->techIndex_] + i);
        create_directories(paths.testTraditionFilePaths_[info_->techIndex_] + i);
    }
}

void CompanyInfo::find_table_start_row() {
    int longestTrainMonth = -1;
    for (auto windowComponent : info_->slidingWindowPairs_) {
        int trainMonth;
        if (get<1>(windowComponent.second) == 'M') {
            trainMonth = get<2>(windowComponent.second);
        }
        else {
            trainMonth = 12;
        }
        if (get<1>(windowComponent.second) == 'M' && trainMonth > longestTrainMonth) {
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
    cout << "calculating " << companyName_ << " " << info_->techType_ << endl;
    vector<double> tmp;
    techTable_.push_back(tmp);
    (this->*cal_tech_[info_->techIndex_])(tmp);  // or (*this.*cal_tech_[info_->techIndex_])(tmp);
    cout << "done calculating" << endl;
}

void CompanyInfo::cal_SMA(vector<double> &tmp) {
    for (int period = 1; period < 257; period++) {
        for (int dateRow = period - 1; dateRow < totalDays_; dateRow++) {
            double MARangePriceSum = 0;
            for (int i = dateRow, j = period; j > 0; i--, j--) {
                MARangePriceSum += price_[i];
            }
            tmp.push_back(MARangePriceSum / period);
        }
        techTable_.push_back(tmp);
        tmp.clear();
    }
}

void CompanyInfo::cal_WMA(vector<double> &tmp) {
    for (int period = 1; period < 257; period++) {
        for (int dateRow = period - 1; dateRow < totalDays_; dateRow++) {
            double weightedPriceSum = 0;
            int totalWeight = 0;
            for (int weight = period, tmpDateRow = dateRow; weight > 0; weight--, tmpDateRow--) {
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
    for (int period = 1; period < 257; period++) {
        double alpha = 2.0 / (double(period) + 1.0);
        double EMA = 0;
        for (int dateRow = 0; dateRow < totalDays_; dateRow++) {
            if (dateRow == 0) {
                EMA = price_[dateRow];
            }
            else {
                EMA = price_[dateRow] * alpha + (1.0 - alpha) * tmp[dateRow - 1];
            }
            tmp.push_back(EMA);
        }
        techTable_.push_back(tmp);
        tmp.clear();
    }
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
    if (info_->mixedTech_) {
        cout << "mixed tech no need to ouput tech" << endl;
        exit(1);
    }
    create_directories(paths_.techOuputPaths_[info_->techIndex_]);
    store_tech_to_vector();
    cout << "saving " << info_->techType_ << " file" << endl;
    for (int techPeriod = 1; techPeriod < 257; techPeriod++) {
        if (techPeriod % 10 == 0) {
            cout << ".";
        }
        ofstream out;
        set_techFile_title(out, techPeriod);
        int techSize = (int)techTable_[techPeriod].size();
        int dateRow = 0;
        switch (info_->techIndex_) {
            case 0:
            case 1:
                dateRow = techPeriod - 1;
                break;
            case 2: {
                dateRow = 0;
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
    techTable_.clear();
}

void CompanyInfo::set_techFile_title(ofstream &out, int techPerid) {
    if (techPerid < 10) {
        out.open(paths_.techOuputPaths_[info_->techIndex_] + companyName_ + "_" + info_->techType_ + "_00" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 10 && techPerid < 100) {
        out.open(paths_.techOuputPaths_[info_->techIndex_] + companyName_ + "_" + info_->techType_ + "_0" + to_string(techPerid) + ".csv");
    }
    else if (techPerid >= 100) {
        out.open(paths_.techOuputPaths_[info_->techIndex_] + companyName_ + "_" + info_->techType_ + "_" + to_string(techPerid) + ".csv");
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

TechTable::TechTable(CompanyInfo *company, int techIndex) : company_(company), techIndex_(techIndex), techType_(company->info_->allTech_[techIndex]), days_(company->totalDays_ - company->tableStartRow_) {
    create_directories(company_->paths_.techOuputPaths_[techIndex_]);
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
    out.open(company_->companyName_ + "_" + company_->info_->techType_ + "_table.csv");
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
    tuple<string, char, int, int> windowComponent_;
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

TestWindow::TestWindow(CompanyInfo &company, string window) : company_(company), windowName_(window), windowComponent_(company.info_->slidingWindowPairs_.at(window)), tableStartRow_(company.tableStartRow_) {
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
    trainLength_ = get<2>(windowComponent_);
    testLength_ = get<3>(windowComponent_);
    switch (get<1>(windowComponent_)) {
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
        if (bigWeekDay < smallWeekDay || is_over_7_days(company_.date_[dateRow - 1], company_.date_[dateRow])) {
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
        if (bigWeekDay < smallWeekDay || is_over_7_days(company_.date_[dateRow], company_.date_[dateRow + 1])) {
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
    cout << "test window: " << windowName_ << "=" << get<0>(windowComponent_) << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company_.date_[*it + tableStartRow_] << "~" << company_.date_[*(++it) + tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class TrainWindow : public TestWindow {
public:
    vector<int> interval_;
    int trainDays_;

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
    for (auto intervalIter = interval_.begin(); intervalIter != interval_.end(); intervalIter += 2) {
        trainDays_ += *(intervalIter + 1) - *intervalIter + 1;
    }
}

void TrainWindow::find_train_interval() {
    switch (get<1>(windowComponent_)) {
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
    switch (get<1>(windowComponent_)) {
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
            if (bigWeekDay < smallWeekDay || is_over_7_days(company_.date_[dateRow - 1], company_.date_[dateRow])) {
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
    cout << "train window: " << windowName_ << "=" << get<0>(windowComponent_) << endl;
    for (auto it = interval_.begin(); it != interval_.end(); it++) {
        cout << company_.date_[*it + tableStartRow_] << "~" << company_.date_[*(++it) + tableStartRow_] << endl;
    }
    cout << "==========" << endl;
}

class MA {
public:
    static const inline vector<int> eachVariableBitsNum_ = {8, 8, 8, 8};
    static const inline vector<vector<int>> traditionStrategy_ = {{5, 20, 5, 20}, {5, 60, 5, 60}, {10, 20, 10, 20}, {10, 60, 10, 60}, {20, 120, 20, 120}, {20, 240, 20, 240}, {60, 120, 60, 120}, {60, 240, 60, 240}};

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
    CompanyInfo *company_ = nullptr;

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
    string tradeRecord_;
    int gen_ = 0;
    int exp_ = 0;
    int bestCnt_ = 0;
    bool isRecordOn_ = false;

    double actualDelta_ = -1;

    string trainOrTestData_;

    int hold1OrHold2 = 1;

    typedef vector<bool (*)(vector<TechTable> *, vector<int> &, int)> buy_sell;
    buy_sell buy{&MA::buy_condition0, &MA::buy_condition0, &MA::buy_condition0, &RSI::buy_condition0};
    buy_sell sell{&MA::sell_condition0, &MA::sell_condition0, &MA::sell_condition0, &RSI::sell_condition0};

    vector<vector<int>> allTechEachVariableBitsNum_{MA::eachVariableBitsNum_, MA::eachVariableBitsNum_, MA::eachVariableBitsNum_, RSI::eachVariableBitsNum_};

    void init(CompanyInfo *company, int techIndex, bool isRecordOn = false, vector<int> variables = {});
    void instant_trade(string startDate, string endDate, bool hold = false);
    void push_holdData_column_Name(bool hold, string &holdData, string *&holdDataPtr);
    string set_title_variables();
    void set_instant_trade_file(ofstream &out, const string &startDate, const string &endDate);
    void output_trade_record(ofstream &out);
    void set_instant_trade_holdData(bool hold, const string &holdData, const string &startDate, const string &endDate);
    void ini_buyNum_sellNum();
    void trade(int startRow, int endRow, bool lastRecord = false, string *holdDataPtr = nullptr);
    bool set_buy_sell_condition(bool &condition, int stockHold, int i, int endRow, bool isBuy);
    void push_holdData_date_price(string *holdDataPtr, int i);
    void push_tradeData_column_name();
    void push_tradeData_buy(int stockHold, int i);
    void push_holdData_buy(string *holdDataPtr, int i);
    void push_extra_techData(int i, string *holdDataPtr);
    void push_tradeData_sell(int stockHold, int i);
    void push_holdData_sell(int endRow, string *holdDataPtr, int i);
    void push_holdData_holding(string *holdDataPtr, int i);
    void push_holdData_not_holding(string *holdDataPtr, int i);
    void push_tradeData_last(bool lastRecord);
    void check_buyNum_sellNum();
    void reset(double RoR = 0);
    void measure(BetaMatrix &betaMatrix);
    void convert_bi_dec();
    void print(ostream &out);
    void record_train_test_data(int startRow, int endRow, string *holdDataPtr = nullptr, vector<vector<string>> *trainFile = nullptr);

    Particle(CompanyInfo *company, int techIndex_, bool isRecordOn = false, vector<int> variables = {});
    Particle(){};
};

Particle::Particle(CompanyInfo *company, int techIndex, bool isRecordOn, vector<int> variables) {
    init(company, techIndex, isRecordOn, variables);
}

void Particle::init(CompanyInfo *company, int techIndex, bool isRecordOn, vector<int> variables) {
    company_ = company;
    techIndex_ = techIndex;
    techType_ = company_->info_->techType_;
    remain_ = company->info_->totalCapitalLV_;
    isRecordOn_ = isRecordOn;
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
    int startRow = find_index_of_string_in_vec((*tables_)[0].date_, startDate);
    int endRow = find_index_of_string_in_vec((*tables_)[0].date_, endDate);
    string holdData;
    string *holdDataPtr = nullptr;
    push_holdData_column_Name(hold, holdData, holdDataPtr);
    trade(startRow, endRow, true, holdDataPtr);
    ofstream out;
    set_instant_trade_file(out, startDate, endDate);
    output_trade_record(out);
    out.close();
    set_instant_trade_holdData(hold, holdData, startDate, endDate);
}

void Particle::push_holdData_column_Name(bool hold, string &holdData, string *&holdDataPtr) {
    if (hold) {
        holdData.clear();
        holdData += "Date,Price,hold 1,hold 2,buy,sell date,sell " + techType_ + ",";
        if (!company_->info_->mixedTech_) {
            switch (techIndex_) {
                case 0:
                case 1:
                case 2: {
                    holdData += "buy 1,buy 2,sell 1,sell 2,\n";
                    break;
                }
                case 3: {
                    holdData += "RSI,overSold,overbought,\n";
                    break;
                }
            }
        }
        else {
            holdData += "\n";
        }
        holdDataPtr = &holdData;
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

void Particle::output_trade_record(ofstream &out) {
    out << tradeRecord_;
}

void Particle::set_instant_trade_holdData(bool hold, const string &holdData, const string &startDate, const string &endDate) {
    if (hold) {
        string titleVariables = set_title_variables();
        ofstream holdFile(company_->companyName_ + "_" + company_->info_->techType_ + titleVariables + "_hold_" + startDate + "_" + endDate + ".csv");
        holdFile << holdData;
        holdFile.close();
    }
}

void Particle::ini_buyNum_sellNum() {
    buyNum_ = 0;
    sellNum_ = 0;
}

void Particle::trade(int startRow, int endRow, bool lastRecord, string *holdDataPtr) {
    int stockHold = 0;
    push_tradeData_column_name();
    ini_buyNum_sellNum();
    bool buyCondition = false;
    bool sellCondition = false;
    bool techNotAllZeros = all_of(decimal_.begin(), decimal_.end(), [](int i) { return i; });
    for (int i = startRow; i <= endRow; i++) {
        push_holdData_date_price(holdDataPtr, i);
        if (set_buy_sell_condition(buyCondition, stockHold, i, endRow, true) && techNotAllZeros) {
            stockHold = floor(remain_ / (*tables_)[0].price_[i]);
            remain_ = remain_ - stockHold * (*tables_)[0].price_[i];
            buyNum_++;
            push_tradeData_buy(stockHold, i);
            push_holdData_buy(holdDataPtr, i);
        }
        else if (set_buy_sell_condition(sellCondition, stockHold, i, endRow, false) && techNotAllZeros) {
            remain_ = remain_ + (double)stockHold * (*tables_)[0].price_[i];
            stockHold = 0;
            sellNum_++;
            push_tradeData_sell(stockHold, i);
            push_holdData_sell(endRow, holdDataPtr, i);
            if (hold1OrHold2 == 1)
                hold1OrHold2 = 2;
            else
                hold1OrHold2 = 1;
        }
        else if (holdDataPtr != nullptr && stockHold != 0) {
            push_holdData_holding(holdDataPtr, i);
        }
        else if (holdDataPtr != nullptr && stockHold == 0) {
            push_holdData_not_holding(holdDataPtr, i);
        }
    }
    check_buyNum_sellNum();
    RoR_ = (remain_ - company_->info_->totalCapitalLV_) / company_->info_->totalCapitalLV_ * 100.0;
    push_tradeData_last(lastRecord);
}

// bool RSIhold = true;
// bool MAhold = false;
// int buy1 = 10;
// int buy2 = 20;
// int sell1 = 10;
// int sell2 = 20;
bool Particle::set_buy_sell_condition(bool &condition, int stockHold, int i, int endRow, bool isBuy) {
    if (isBuy) {
        condition = !stockHold && (*buy[techIndex_])(tables_, decimal_, i) && i != endRow;
        return condition;
    }
    condition = stockHold && ((*sell[techIndex_])(tables_, decimal_, i) || i == endRow);
    return condition;

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
    //         RSIbuy = !RSIhold && (*buy[info_->techIndex_])(tables_, decimal_, i) && i != endRow;
    //         if (RSIbuy)
    //             RSIhold = true;
    //     }
    //     if (!MAhold) {
    //         MAbuy = !MAhold && ((*tables_)[1].techTable_[i][buy1] >= (*tables_)[1].techTable_[i][buy2]) && i != endRow;
    //         if (MAbuy)
    //             MAhold = true;
    //     }

    //     RSIsell = RSIhold && ((*sell[info_->techIndex_])(tables_, decimal_, i) || i == endRow);
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

void Particle::push_holdData_date_price(string *holdDataPtr, int i) {
    if (holdDataPtr != nullptr) {
        (*holdDataPtr) += (*tables_)[0].date_[i];
        (*holdDataPtr) += ",";
        (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
        (*holdDataPtr) += ",";
    }
}

void Particle::push_tradeData_column_name() {
    if (isRecordOn_) {
        tradeRecord_.clear();
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_ += ",date,price,preday 1,preday 2,today 1,today 2,stockHold,remain,capital lv\n";
                break;
            }
            case 3: {
                tradeRecord_ += ",date,price,RSI,stockHold,remain,capital lv\n";
                break;
            }
        }
    }
}

void Particle::push_tradeData_buy(int stockHold, int i) {
    if (isRecordOn_) {
        tradeRecord_ += "buy,";
        tradeRecord_ += (*tables_)[0].date_[i] + ",";
        tradeRecord_ += set_precision((*tables_)[0].price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i - 1][decimal_[0]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i - 1][decimal_[1]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[1]]) + ",";
                break;
            }
            case 3: {
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        tradeRecord_ += to_string(stockHold) + ",";
        tradeRecord_ += set_precision(remain_) + ",";
        tradeRecord_ += set_precision(remain_ + stockHold * (*tables_)[0].price_[i]) + "\n";
    }
}

void Particle::push_extra_techData(int i, string *holdDataPtr) {
    switch (techIndex_) {
        case 0:
        case 1:
        case 2: {
            for (auto variable : decimal_) {
                (*holdDataPtr) += set_precision((*tables_)[0].techTable_[i][variable]);
                (*holdDataPtr) += ",";
            }
            break;
        }
        case 3: {
            (*holdDataPtr) += set_precision((*tables_)[0].techTable_[i][decimal_[0]]);
            (*holdDataPtr) += ",";
            (*holdDataPtr) += to_string(decimal_[1]);
            (*holdDataPtr) += ",";
            (*holdDataPtr) += to_string(decimal_[2]);
            (*holdDataPtr) += ",";
            // (*holdDataPtr) += set_precision((*tables_)[1].techTable_[i][buy1]);
            // (*holdDataPtr) += ",";
            // (*holdDataPtr) += set_precision((*tables_)[1].techTable_[i][buy2]);
            // (*holdDataPtr) += ",";
            // (*holdDataPtr) += set_precision((*tables_)[1].techTable_[i][sell1]);
            // (*holdDataPtr) += ",";
            // (*holdDataPtr) += set_precision((*tables_)[1].techTable_[i][sell2]);
            break;
        }
    }
}

void Particle::push_holdData_buy(string *holdDataPtr, int i) {
    if (holdDataPtr != nullptr) {
        if (hold1OrHold2 == 2) 
            (*holdDataPtr) += ",";
        (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
        if (hold1OrHold2 == 1) 
            (*holdDataPtr) += ",";
        (*holdDataPtr) += ",";
        (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
        (*holdDataPtr) += ",,,";
        push_extra_techData(i, holdDataPtr);
        (*holdDataPtr) += "\n";
    }
}

void Particle::push_tradeData_sell(int stockHold, int i) {
    if (isRecordOn_) {
        tradeRecord_ += "sell,";
        tradeRecord_ += (*tables_)[0].date_[i] + ",";
        tradeRecord_ += set_precision((*tables_)[0].price_[i]) + ",";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i - 1][decimal_[2]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i - 1][decimal_[3]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[2]]) + ",";
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[3]]) + ",";
                break;
            }
            case 3: {
                tradeRecord_ += set_precision((*tables_)[0].techTable_[i][decimal_[0]]) + ",";
                break;
            }
        }
        tradeRecord_ += to_string(stockHold) + ",";
        tradeRecord_ += set_precision(remain_) + ",";
        tradeRecord_ += set_precision(remain_ + stockHold * (*tables_)[0].price_[i]) + "\n\n";
    }
}

void Particle::push_holdData_sell(int endRow, string *holdDataPtr, int i) {
    if (holdDataPtr != nullptr) {
        if (hold1OrHold2 == 2) 
            (*holdDataPtr) += ",";
        (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
        if (hold1OrHold2 == 1)
            (*holdDataPtr) += ",";
        if (i == endRow) {
            (*holdDataPtr) += ",,";
            (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
            (*holdDataPtr) += ",,";
        }
        else {
            (*holdDataPtr) += ",,,";
            (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
            (*holdDataPtr) += ",";
        }
        push_extra_techData(i, holdDataPtr);
        (*holdDataPtr) += "\n";
    }
}

void Particle::push_holdData_holding(string *holdDataPtr, int i) {
    if (hold1OrHold2 == 2)
        (*holdDataPtr) += ",";
    (*holdDataPtr) += set_precision((*tables_)[0].price_[i]);
    if (hold1OrHold2 == 1)
        (*holdDataPtr) += ",";
    (*holdDataPtr) += ",,,,";
    push_extra_techData(i, holdDataPtr);
    (*holdDataPtr) += "\n";
}

void Particle::push_holdData_not_holding(string *holdDataPtr, int i) {
    (*holdDataPtr) += ",,,,,";
    push_extra_techData(i, holdDataPtr);
    (*holdDataPtr) += "\n";
}

void Particle::check_buyNum_sellNum() {
    if (buyNum_ != sellNum_) {
        cout << "particle.buyNum_ = " << buyNum_ << ", particle.sellNum_ = " << sellNum_ << endl;
        cout << tradeRecord_ << endl;
        exit(1);
    }
}

void Particle::push_tradeData_last(bool lastRecord) {
    if (isRecordOn_ && lastRecord) {
        tradeRecord_ += "buyNum," + to_string(buyNum_);
        tradeRecord_ += ",sellNum," + to_string(sellNum_) + "\n";
        tradeRecord_ += "remain," + set_precision(remain_) + "\n";
        tradeRecord_ += "return rate," + set_precision(RoR_) + "%\n";
    }
}

void Particle::reset(double RoR) {
    fill(binary_.begin(), binary_.end(), 0);
    fill(decimal_.begin(), decimal_.end(), 0);
    buyNum_ = 0;
    sellNum_ = 0;
    remain_ = company_->info_->totalCapitalLV_;
    RoR_ = RoR;
    tradeRecord_.clear();
    trainOrTestData_.clear();
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

void Particle::record_train_test_data(int startRow, int endRow, string *holdDataPtr, vector<vector<string>> *trainFile) {
    isRecordOn_ = true;
    remain_ = company_->info_->totalCapitalLV_;
    trade(startRow, endRow, false, holdDataPtr);
    // if (all_of(decimal_.begin(), decimal_.end(), [](int i) { return !i; })) {
    //     reset();
    // }
    trainOrTestData_.clear();
    if (holdDataPtr == nullptr) {
        if (trainFile != nullptr) {
            for (size_t row = 0; row < 6; row++) {
                for (size_t col = 0; col < (*trainFile)[row].size(); col++) {
                    trainOrTestData_ += (*trainFile)[row][col] + ",";
                }
                trainOrTestData_ += "\n";
            }
            trainOrTestData_ += "\n";
        }
        else {
            trainOrTestData_ += "tech type," + techType_ + "\n";
            trainOrTestData_ += "algo," + company_->info_->algoType_ + "\n";
            trainOrTestData_ += "delta," + set_precision(company_->info_->delta_) + "\n";
            trainOrTestData_ += "exp," + to_string(company_->info_->expNum_) + "\n";
            trainOrTestData_ += "gen," + to_string(company_->info_->genNum_) + "\n";
            trainOrTestData_ += "p number," + to_string(company_->info_->particleNum_) + "\n\n";
        }
        trainOrTestData_ += "initial capital," + set_precision(company_->info_->totalCapitalLV_) + "\n";
        trainOrTestData_ += "final capital," + set_precision(remain_) + "\n";
        trainOrTestData_ += "final return," + set_precision(remain_ - company_->info_->totalCapitalLV_) + "\n";
        trainOrTestData_ += "return rate," + set_precision(RoR_) + "%\n\n";
        switch (techIndex_) {
            case 0:
            case 1:
            case 2: {
                trainOrTestData_ += "buy1," + to_string(decimal_[0]) + "\n";
                trainOrTestData_ += "buy2," + to_string(decimal_[1]) + "\n";
                trainOrTestData_ += "sell1," + to_string(decimal_[2]) + "\n";
                trainOrTestData_ += "sell2," + to_string(decimal_[3]) + "\n\n";
                break;
            }
            case 3: {
                trainOrTestData_ += "period," + to_string(decimal_[0]) + "\n";
                trainOrTestData_ += "overSold," + to_string(decimal_[1]) + "\n";
                trainOrTestData_ += "overBought," + to_string(decimal_[2]) + "\n\n";
                break;
            }
            default: {
                cout << "print_train_test_data exception" << endl;
                exit(1);
            }
        }
        trainOrTestData_ += "best exp," + to_string(exp_) + "\n";
        trainOrTestData_ += "best gen," + to_string(gen_) + "\n";
        trainOrTestData_ += "best cnt," + to_string(bestCnt_) + "\n\n";
        trainOrTestData_ += "trade," + to_string(sellNum_) + "\n";
        trainOrTestData_ += tradeRecord_;
    }
}

class TrainAPeriod {
public:
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

    string trainData_;

    void create_particles();
    void create_betaMatrix();
    ofstream set_debug_file(int expCnt);
    void start_exp(int expCnt);
    void initialize_KNQTS();
    void output_debug_exp(int expCnt);
    void evolve_particles(int i);
    void start_gen(int expCnt, int genCnt);
    void output_debug_gen(int genCnt);
    void output_debug_particle(int i);
    void store_exp_gen(int expCnt, int genCnt);
    void update_local();
    void update_global();
    void run_algo();
    void QTS();
    void GQTS();
    void GNQTS();
    void KNQTScompare();
    void KNQTSmultiply();
    void output_debug_beta();
    void update_best(int renewBest);

    TrainAPeriod(CompanyInfo &company, vector<TechTable> &tables, int startRow, int endRow, string windowName = "", bool record = false);
};

TrainAPeriod::TrainAPeriod(CompanyInfo &company, vector<TechTable> &tables, int startRow, int endRow, string windowName, bool record) : company_(company), tables_(tables), startRow_(startRow), endRow_(endRow), actualDelta_(company.info_->delta_), debug_(company.info_->debug_), record_(record) {
    // cout << tables_[0].date_[startRow_] << "~" << tables_[0].date_[endRow] << endl;
    create_particles();
    create_betaMatrix();
    globalP_[0].reset();
    for (int expCnt = 0; expCnt < company_.info_->expNum_; expCnt++) {
        debugOut_ = set_debug_file(expCnt);
        start_exp(expCnt);
        debugOut_.close();
    }
    globalP_[0].record_train_test_data(startRow_, endRow_);
    trainData_ = globalP_[0].trainOrTestData_;
    // cout << globalP_[0].RoR_ << "%" << endl;
    // cout << "==========" << endl;
}

void TrainAPeriod::create_particles() {
    Particle p(&company_, company_.info_->techIndex_, debug_);
    p.tables_ = &tables_;
    p.actualDelta_ = actualDelta_;
    for (int i = 0; i < company_.info_->particleNum_; i++) {
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

ofstream TrainAPeriod::set_debug_file(int expCnt) {
    ofstream out;
    if (debug_) {
        string delta = remove_zeros_at_end(actualDelta_);
        string title;
        title += "debug_";
        title += company_.companyName_ + "_";
        title += company_.info_->techType_ + "_";
        title += company_.info_->algoType_ + "_";
        title += delta + "_";
        title += get_date(tables_[0].date_, startRow_, endRow_) + "_";
        title += "exp_" + to_string(expCnt) + ".csv";
        out.open(title);
    }
    return out;
}

void TrainAPeriod::start_exp(int expCnt) {
    output_debug_exp(expCnt);
    globalP_[1].reset();
    betaMatrix_.reset();
    initialize_KNQTS();
    for (int genCnt = 0; genCnt < company_.info_->genNum_; genCnt++) {
        start_gen(expCnt, genCnt);
    }
    update_best(0);
}

void TrainAPeriod::initialize_KNQTS() {
    actualDelta_ = company_.info_->delta_;
    compareNew_ = 0;
    compareOld_ = 0;
}

void TrainAPeriod::output_debug_exp(int expCnt) {
    if (debug_)
        debugOut_ << "exp:" << expCnt << ",==========,==========" << endl;
}

void TrainAPeriod::start_gen(int expCnt, int genCnt) {
    output_debug_gen(genCnt);
    globalP_[3].reset();
    globalP_[4].reset(company_.info_->totalCapitalLV_);
    for (int i = 0; i < company_.info_->particleNum_; i++) {
        evolve_particles(i);
    }
    store_exp_gen(expCnt, genCnt);
    update_local();
    update_global();
    run_algo();
    output_debug_beta();
}

void TrainAPeriod::output_debug_gen(int genCnt) {
    if (debug_)
        debugOut_ << "gen:" << genCnt << ",=====" << endl;
}

void TrainAPeriod::evolve_particles(int i) {
    particles_[i].reset();
    particles_[i].measure(betaMatrix_);
    particles_[i].convert_bi_dec();
    particles_[i].trade(startRow_, endRow_);
    output_debug_particle(i);
}

void TrainAPeriod::output_debug_particle(int i) {
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
    switch (company_.info_->algoIndex_) {
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
    switch (company_.info_->compareMode_) {
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
        actualDelta_ *= company_.info_->multiplyUp_;
    }
    else {
        actualDelta_ *= company_.info_->multiplyDown_;
    }
    compareOld_ = compareNew_;
    compareNew_ = 0;
}

void TrainAPeriod::output_debug_beta() {
    if (debug_) {
        switch (company_.info_->algoIndex_) {
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

class Semaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count_;

public:
    inline void notify() {
        unique_lock<mutex> lock(mtx);
        count_++;
        cv.notify_one();
    }

    inline void wait() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&count_ = this->count_]() { return count_ > 0; });
        count_--;
    }

    Semaphore(int count = 1) : count_(count) {
        if (count_ == 0)
            count_ = 1;
    }
};

class MixedTechChooseTrainFile {
private:
    CompanyInfo *company_;
    TrainWindow *window_;

public:
    vector<path> goodTrainFile;

    MixedTechChooseTrainFile(CompanyInfo *company, TrainWindow *window, vector<string> &techTrainFilePath) : company_(company), window_(window), goodTrainFile(window->intervalSize_ / 2) {
        cout << "copying " << window->windowName_ << " train files" << endl;
        vector<string> trainFilePaths;
        for (auto &techIndex : company_->info_->techIndexs_) {
            trainFilePaths.push_back(techTrainFilePath[techIndex]);
        }
        vector<vector<path>> diffTechTrainFilePath;
        for (auto trainPath : trainFilePaths) {
            diffTechTrainFilePath.push_back(get_path(trainPath + window->windowName_));
        }
        vector<vector<vector<string>>> aPeriodTrainFiles(company_->info_->mixedTechNum_);
        for (int colIndex = 0; colIndex < window_->intervalSize_ / 2; colIndex++) {
            for (int rowIndex = 0; rowIndex < company_->info_->mixedTechNum_; rowIndex++) {
                aPeriodTrainFiles[rowIndex] = read_data(diffTechTrainFilePath[rowIndex][colIndex]);
            }
            size_t bestRoRIndex = 0;
            vector<double> everyRoR;
            for (auto &trainFile : aPeriodTrainFiles) {
                everyRoR.push_back(stod(trainFile[10][1]));
            }
            bestRoRIndex = distance(everyRoR.begin(), max_element(everyRoR.begin(), everyRoR.end()));
            goodTrainFile[colIndex] = diffTechTrainFilePath[bestRoRIndex][colIndex];
        }
    }
};

class Train {
private:
    CompanyInfo *company_;
    vector<TechTable> *tables_;

    Semaphore sem_;

    void train_a_window(string windowName);
    void print_date_train_file(string &trainFileData, string startDate, string endDate);
    void train_a_company();
    void output_train_file(vector<int>::iterator &intervalIter, string &ouputPath, string &trainData);

public:
    Train(CompanyInfo company, vector<TechTable> &tables, Semaphore &sem);
    Train(CompanyInfo &company, string startDate, string endDate);
    Train(CompanyInfo &company);
};

Train::Train(CompanyInfo company, vector<TechTable> &tables, Semaphore &sem) : tables_(&tables) {
    Info info = *company.info_;
    company.info_ = &info;
    company_ = &company;
    sem.wait();
    train_a_company();
    sem.notify();
}

Train::Train(CompanyInfo &company, string startDate, string endDate) : company_(&company), sem_(company.info_->windowThreadNum_) {
    vector<TechTable> tables{TechTable(&company, company.info_->techIndex_)};
    tables_ = &tables;
    int startRow = find_index_of_string_in_vec((*tables_)[0].date_, startDate);
    int endRow = find_index_of_string_in_vec((*tables_)[0].date_, endDate);
    srand(343);
    company_->paths_.trainFilePaths_[company_->info_->techIndex_].clear();
    TrainAPeriod trainAPeriod(*company_, *tables_, startRow, endRow);
    print_date_train_file(trainAPeriod.trainData_, startDate, endDate);
}

void Train::print_date_train_file(string &trainData, string startDate, string endDate) {
    string title = company_->info_->techType_ + "_";
    title += company_->companyName_ + "_";
    title += company_->info_->algoType_ + "_";
    title += remove_zeros_at_end(company_->info_->delta_) + "_";
    title += startDate + "_";
    title += endDate + ".csv";
    ofstream out(title);
    out << trainData;
    out.close();
}

Train::Train(CompanyInfo &company) : company_(&company), sem_(company.info_->windowThreadNum_) {
    vector<TechTable> tables;
    if (!company_->info_->mixedTech_) {
        tables = {TechTable(&company, company.info_->techIndex_)};
        tables_ = &tables;
    }
    train_a_company();
}

void Train::train_a_company() {
    vector<thread> windowThreads;
    for (auto windowName : company_->info_->slidingWindows_) {
        create_directories(company_->paths_.trainFilePaths_[company_->info_->techIndex_] + windowName);
        windowThreads.push_back(thread(&Train::train_a_window, this, windowName));
        this_thread::sleep_for(0.5s);
    }
    for (auto &thread : windowThreads) {
        thread.join();
    }
}

void Train::train_a_window(string windowName) {
    sem_.wait();
    TrainWindow window(*company_, windowName);
    if (window.interval_[0] >= 0) {
        string outputPath = [&company_ = this->company_, windowName]() {
            if (company_->info_->testDeltaLoop_) {
                string dir = windowName + "_" + to_string(company_->info_->delta_) + "/";
                create_directories(dir);
                return dir;
            }
            return company_->paths_.trainFilePaths_[company_->info_->techIndex_] + windowName + "/";
        }();
        if (company_->info_->mixedTech_) {
            MixedTechChooseTrainFile mixedTechChooseTrainFile(company_, &window, company_->paths_.trainFilePaths_);
            for (auto from : mixedTechChooseTrainFile.goodTrainFile) {
                filesystem::copy(from, outputPath, copy_options::overwrite_existing);
            }
        }
        else {
            window.print_train();
            srand(343);
            for (auto intervalIter = window.interval_.begin(); intervalIter != window.interval_.end(); intervalIter += 2) {
                TrainAPeriod trainAPeriod(*company_, *tables_, *intervalIter, *(intervalIter + 1), window.windowName_);
                cout << company_->companyName_ << "_" << windowName << "_";
                cout << (*tables_)[0].date_[*intervalIter] << "~" << (*tables_)[0].date_[*(intervalIter + 1)] << "_";
                cout << trainAPeriod.globalP_[0].RoR_ << "%" << endl;
                output_train_file(intervalIter, outputPath, trainAPeriod.trainData_);
            }
        }
    }
    else {
        cout << window.windowName_ << " train window is too old, skip this window" << endl;
    }
    sem_.notify();
}

void Train::output_train_file(vector<int>::iterator &intervalIter, string &ouputPath, string &trainData) {
    ofstream out(ouputPath + get_date((*tables_)[0].date_, *intervalIter, *(intervalIter + 1)) + ".csv");
    out << trainData;
    out.close();
}

class TrainLoop {
private:
    CompanyInfo &company_;
    vector<TechTable> tables_;

    Semaphore sem_;

public:
    TrainLoop(CompanyInfo &company);
};

TrainLoop::TrainLoop(CompanyInfo &company) : company_(company), tables_({TechTable(&company, company.info_->techIndex_)}), sem_(company.info_->companyThreadNum_) {
    vector<thread> loopThread;
    company_.paths_.trainFilePaths_[company_.info_->techIndex_].clear();
    for (int loop = 0; loop < company_.info_->testDeltaLoop_; loop++) {
        loopThread.push_back(thread([&]() { Train(company_, tables_, sem_); }));
        this_thread::sleep_for(1s);
        company_.info_->delta_ -= company_.info_->testDeltaGap_;
    }
    for (auto &t : loopThread) {
        t.join();
    }
}

class Test {
protected:
    class Path {
    public:
        string trainFilePaths_;
        string testFileOutputPath_;
    };
    CompanyInfo &company_;
    Path paths_;
    Particle p_;
    vector<TechTable> tables_;
    bool tradition_ = false;

    void add_tables(vector<int> additionTable = {});
    void set_particle();
    void set_train_test_file_path();
    void test_a_window(TrainWindow &window);
    void check_exception(vector<path> &eachTrainFilePath, TestWindow &window);
    void set_variables(vector<vector<string>> &thisTrainFile);
    void output_test_file(TestWindow &window, int startRow, int endRow);

public:
    Test(CompanyInfo &company, bool tradition, vector<int> additionTable = {});
    Test(CompanyInfo &company) : company_(company){};
};

Test::Test(CompanyInfo &company, bool tradition, vector<int> additionTable) : company_(company), tradition_(tradition) {
    add_tables(additionTable);
    set_particle();
    set_train_test_file_path();
    for (auto windowName : company_.info_->slidingWindows_) {
        if (windowName != "A2A") {
            if (tradition_) {
                create_directories(company_.paths_.testTraditionFilePaths_[company_.info_->techIndex_] + windowName);
            }
            else {
                create_directories(company_.paths_.testFilePaths_[company_.info_->techIndex_] + windowName);
            }
            TrainWindow window(company_, windowName);
            cout << window.windowName_ << endl;
            test_a_window(window);
        }
    }
}

void Test::add_tables(vector<int> additionTable) {
    if (!company_.info_->mixedTech_) {
        tables_.push_back(TechTable(&company_, company_.info_->techIndex_));
    }
    else {
        for (auto &techIndex : company_.info_->techIndexs_) {
            tables_.push_back(TechTable(&company_, techIndex));
        }
    }
    if (additionTable.size() > 0) {
        for (int i = 0; i < additionTable.size(); i++) {
            tables_.push_back(TechTable(&company_, additionTable[i]));
        }
    }
}

void Test::set_particle() {
    if (!company_.info_->mixedTech_) {
        p_.init(&company_, company_.info_->techIndex_);
    }
    else {
        p_.init(&company_, 0);
    }
    p_.tables_ = &tables_;
}

void Test::set_train_test_file_path() {
    if (tradition_) {
        paths_.trainFilePaths_ = company_.paths_.trainTraditionFilePaths_[company_.info_->techIndex_];
        paths_.testFileOutputPath_ = company_.paths_.testTraditionFilePaths_[company_.info_->techIndex_];
    }
    else {
        paths_.trainFilePaths_ = company_.paths_.trainFilePaths_[company_.info_->techIndex_];
        paths_.testFileOutputPath_ = company_.paths_.testFilePaths_[company_.info_->techIndex_];
    }
    cout << "test " << company_.companyName_ << endl;
}

void Test::test_a_window(TrainWindow &window) {
    vector<path> trainFilePaths = get_path(paths_.trainFilePaths_ + window.windowName_);
    if (window.interval_[0] >= 0) {
        vector<vector<string>> thisTrainFiles;
        check_exception(trainFilePaths, window);
        auto [intervalIter, filePathIndex] = tuple{window.TestWindow::interval_.begin(), 0};
        for (; intervalIter != window.TestWindow::interval_.end(); intervalIter += 2, filePathIndex++) {
            thisTrainFiles = read_data(trainFilePaths[filePathIndex]);
            p_.reset();
            set_variables(thisTrainFiles);
            p_.record_train_test_data(*intervalIter, *(intervalIter + 1), nullptr, &thisTrainFiles);
            output_test_file(window, *intervalIter, *(intervalIter + 1));
        }
    }
    else {
        cout << "no " << window.windowName_ << " train window in " << company_.companyName_;
        cout << ", skip this window" << endl;
    }
}

void Test::check_exception(vector<path> &trainFilePaths, TestWindow &window) {
    if (trainFilePaths.size() != window.intervalSize_ / 2) {
        cout << company_.companyName_ << " " << window.windowName_ << " test interval number is not equal to train fle number" << endl;
        exit(1);
    }
}

void Test::set_variables(vector<vector<string>> &thisTrainFile) {
    if (company_.info_->mixedTech_) {
        string techUse = thisTrainFile[0][1];
        int techUseIndex = find_index_of_string_in_vec(company_.info_->allTech_, techUse);
        p_.init(&company_, techUseIndex);
    }
    for (int i = 0; i < p_.variableNum_; i++) {
        p_.decimal_[i] = stoi(thisTrainFile[i + 12][1]);
    }
}

void Test::output_test_file(TestWindow &window, int startRow, int endRow) {
    ofstream out;
    if (!tradition_) {
        out.open(company_.paths_.testFilePaths_[company_.info_->techIndex_] + window.windowName_ + "/" + get_date(tables_[0].date_, startRow, endRow) + ".csv");
    }
    else {
        out.open(company_.paths_.testTraditionFilePaths_[company_.info_->techIndex_] + window.windowName_ + "/" + get_date(tables_[0].date_, startRow, endRow) + ".csv");
    }
    out << p_.trainOrTestData_;
    out.close();
}

class BH {
public:
    double BHRoR;
    BH(CompanyInfo &company, string startDate, string endDate) {
        int startRow = find_index_of_string_in_vec(company.date_, startDate);
        int endRow = find_index_of_string_in_vec(company.date_, endDate);
        int stockHold = company.info_->totalCapitalLV_ / company.price_[startRow];
        double remain = company.info_->totalCapitalLV_ - stockHold * company.price_[startRow];
        remain += stockHold * company.price_[endRow];
        BHRoR = ((remain - company.info_->totalCapitalLV_) / company.info_->totalCapitalLV_);
    }
};

class Tradition {
private:
    CompanyInfo &company_;
    vector<TechTable> tables_;
    vector<Particle> particles_;
    vector<vector<int>> traditionStrategy_;
    int traditionStrategyNum_ = -1;
    ofstream out_;

    vector<vector<vector<int>>> allTraditionStrategy_{MA::traditionStrategy_, MA::traditionStrategy_, MA::traditionStrategy_, RSI::traditionStrategy_};

    void create_particles();
    void set_strategy();
    void train_a_tradition_window(TrainWindow &window);
    void set_variables(int index);

public:
    Tradition(CompanyInfo &company);
};

Tradition::Tradition(CompanyInfo &company) : company_(company) {
    if (!company_.info_->mixedTech_) {
        vector<TechTable> tables{TechTable(&company, company.info_->techIndex_)};
        tables_ = tables;
        set_strategy();
        create_particles();
    }
    cout << "train " << company_.companyName_ << " tradition" << endl;
    for (auto windowName : company_.info_->slidingWindows_) {
        TrainWindow window(company_, windowName);
        if (window.interval_[0] >= 0) {
            create_directories(company_.paths_.trainTraditionFilePaths_[company_.info_->techIndex_] + windowName);
            cout << window.windowName_ << endl;
            train_a_tradition_window(window);
        }
        else {
            cout << window.windowName_ << " train window is too old, skip this window" << endl;
        }
    }
}

void Tradition::set_strategy() {
    traditionStrategy_ = allTraditionStrategy_[company_.info_->techIndex_];
    traditionStrategyNum_ = (int)traditionStrategy_.size();
}

void Tradition::create_particles() {
    Particle p(&company_, company_.info_->techIndex_);
    p.tables_ = &tables_;
    for (int i = 0; i < traditionStrategyNum_; i++) {
        particles_.push_back(p);
    }
}

void Tradition::train_a_tradition_window(TrainWindow &window) {
    if (company_.info_->mixedTech_) {
        MixedTechChooseTrainFile mixedTechChooseTrainFile(&company_, &window, company_.paths_.trainTraditionFilePaths_);
        for (auto from : mixedTechChooseTrainFile.goodTrainFile) {
            filesystem::copy(from, company_.paths_.trainFilePaths_[company_.info_->techIndex_] + window.windowName_ + "/", copy_options::overwrite_existing);
        }
    }
    else {
        string outputPath = company_.paths_.trainTraditionFilePaths_[company_.info_->techIndex_] + window.windowName_;
        for (auto intervalIter = window.interval_.begin(); intervalIter != window.interval_.end(); intervalIter += 2) {
            for (int i = 0; i < traditionStrategyNum_; i++) {
                particles_[i].reset();
                set_variables(i);
                particles_[i].trade(*intervalIter, *(intervalIter + 1));
            }
            stable_sort(particles_.begin(), particles_.end(), [](const Particle &a, const Particle &b) { return a.RoR_ > b.RoR_; });
            particles_[0].record_train_test_data(*intervalIter, *(intervalIter + 1));
            out_.open(outputPath + "/" + get_date(tables_[0].date_, *intervalIter, *(intervalIter + 1)) + ".csv");
            out_ << particles_[0].trainOrTestData_;
            out_.close();
        }
    }
}

void Tradition::set_variables(int index) {
    for (int i = 0; i < particles_[i].variableNum_; i++) {
        particles_[index].decimal_[i] = traditionStrategy_[index][i];
    }
}

class HoldFile : public Test {
private:
    CompanyInfo *company_;
    bool isTrain_;
    bool isTradition_;
    string targetWindowPaths_;
    string holdFileOuputPath_;

    string holdData_;
    string *holdDataPtr_ = nullptr;

    ofstream out_;

public:
    void cal_hold(vector<path> &filePaths, vector<vector<string>> &thisTargetFile, TrainWindow &window) {
        cout << "output " << window.windowName_ << " hold" << endl;
        vector<int> interval;
        if (isTrain_)
            interval = window.interval_;
        else
            interval = window.TestWindow::interval_;
        auto [filePathIter, intervalIter] = tuple{filePaths.begin(), interval.begin()};
        for (int periodIndex = 0; periodIndex < window.intervalSize_ / 2; periodIndex++, filePathIter++, intervalIter += 2) {
            thisTargetFile = read_data(*filePathIter);
            p_.reset();
            set_variables(thisTargetFile);
            p_.record_train_test_data(*intervalIter, *(intervalIter + 1), holdDataPtr_, &thisTargetFile);
        }
        out_.open(holdFileOuputPath_ + company_->companyName_ + "_" + window.windowName_ + "_hold.csv");
        out_ << holdData_;
        out_.close();
    }

    void set_paths_create_Folder() {
        auto set_path = [](string targetWindowPaths, string holdFileOuputPath) {
            create_directories(holdFileOuputPath);
            return tuple{targetWindowPaths, holdFileOuputPath};
        };
        if (isTrain_ && !isTradition_) {  //train
            tie(targetWindowPaths_, holdFileOuputPath_) = set_path(company_->paths_.trainFilePaths_[company_->info_->techIndex_], company_->paths_.trainHoldFilePaths_[company_->info_->techIndex_]);
        }
        else if (!isTrain_ && !isTradition_) {  //test
            tie(targetWindowPaths_, holdFileOuputPath_) = set_path(company_->paths_.testFilePaths_[company_->info_->techIndex_], company_->paths_.testHoldFilePaths_[company_->info_->techIndex_]);
        }
        else if (isTrain_ && isTradition_) {  //train tradition
            tie(targetWindowPaths_, holdFileOuputPath_) = set_path(company_->paths_.trainTraditionFilePaths_[company_->info_->techIndex_], company_->paths_.trainTraditionHoldFilePaths_[company_->info_->techIndex_]);
        }
        else if (!isTrain_ && isTradition_) {  //test tradition
            tie(targetWindowPaths_, holdFileOuputPath_) = set_path(company_->paths_.testTraditionFilePaths_[company_->info_->techIndex_], company_->paths_.testTraditionHoldFilePaths_[company_->info_->techIndex_]);
        }
    }
    
    HoldFile(CompanyInfo *company, bool isTrain, bool isTradition) : Test(*company), company_(company), isTrain_(isTrain), isTradition_(isTradition) {
        set_paths_create_Folder();
        add_tables();
        set_particle();
        vector<vector<string>> thisTargetFile;
        for (auto windowName : company_->info_->slidingWindows_) {
            if (windowName == "A2A" && isTrain_ == false)
                continue;
            vector<path> filePaths = get_path(targetWindowPaths_ + windowName);
            p_.push_holdData_column_Name(true, holdData_, holdDataPtr_);
            TrainWindow window(*company_, windowName);
            if (window.interval_[0] > 0) {
                cal_hold(filePaths, thisTargetFile, window);
            }
        }
    }
};

class CalIRR {
public:
    class WindowIRR {
    public:
        string windowName_;
        double algoIRR_;
        double traditionIRR_;
        double BHIRR_;
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

    class CompanyAllRoRData {
    public:
        string algoRoRoutData_;
        string traditionRoRoutData_;
    };

    class CalOneCompanyIRR {
    public:
        CompanyInfo &company_;
        vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR_;
        vector<Rank> &allCompanyWindowRank_;
        CompanyWindowIRRContainer thisCompanyWindowIRR_;
        CompanyAllRoRData allRoRData_;
        WindowIRR tmpWinodwIRR_;

        vector<size_t> eachVariableNum_{MA::eachVariableBitsNum_.size(), MA::eachVariableBitsNum_.size(), MA::eachVariableBitsNum_.size(), RSI::eachVariableBitsNum_.size()};

        int trainOrTestIndex_;
        vector<string> trainOrTestPaths_;

        void set_filePath();
        void cal_window_IRRs(string windowName);
        double cal_one_IRR(string &RoRoutData, TrainWindow &window, string &stratgyFilePath, bool tradition);
        string compute_and_record_window_RoR(vector<path> &strategyPaths, const vector<path>::iterator &filePathIter, double &totalRoR, bool tradition, vector<int>::iterator &intervalIter);
        WindowIRR cal_BH_IRR();
        void rank_algo_and_tradition_window(vector<WindowIRR> &windowsIRR);
        void sort_by_tradition_IRR(vector<WindowIRR> &windowsIRR);
        void rank_window(vector<WindowIRR> &windowsIRR);
        void sort_by_window_name(vector<WindowIRR> &windowsIRR);
        void sort_by_algo_IRR(vector<WindowIRR> &windowsIRR);

        CalOneCompanyIRR(CompanyInfo &company, vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR, vector<Rank> &allCompanyWindowRank, int trainOrTestIndex);
    };

    Info info_;
    vector<path> &companyPricePaths_;
    vector<Rank> allCompanyWindowRank_;
    vector<CompanyWindowIRRContainer> allCompanyWindowsIRR_;

    int trainOrTestIndex_;
    vector<string> trainOrTestVec_ = {"train", "test"};
    string trainOrTest_;

    void output_all_IRR();
    void output_IRR(ofstream &IRRout);
    void output_all_window_rank();
    vector<string> remove_A2A_and_sort();

    CalIRR(vector<path> &companyPricePaths, string trainOrTest);
};

CalIRR::CalIRR(vector<path> &companyPricePaths, string trainOrTest) : companyPricePaths_(companyPricePaths), trainOrTest_(trainOrTest) {
    trainOrTestIndex_ = find_index_of_string_in_vec(trainOrTestVec_, trainOrTest_);
    for (auto &companyPricePath : companyPricePaths_) {
        CompanyInfo company(companyPricePath, info_);
        CalOneCompanyIRR calOneCompanyIRR(company, allCompanyWindowsIRR_, allCompanyWindowRank_, trainOrTestIndex_);
        auto output_company_all_window_IRR = [](ofstream &out, const string &outputData) {
            out << outputData;
            out.close();
        };
        ofstream out(company.paths_.companyRootPaths_[company.info_->techIndex_] + company.companyName_ + "_" + trainOrTest_ + "RoR.csv");
        output_company_all_window_IRR(out, calOneCompanyIRR.allRoRData_.algoRoRoutData_);
        out.open(company.paths_.companyRootPaths_[company.info_->techIndex_] + company.companyName_ + "_" + trainOrTest_ + "TraditionRoR.csv");
        output_company_all_window_IRR(out, calOneCompanyIRR.allRoRData_.traditionRoRoutData_);
    }
    output_all_IRR();
    output_all_window_rank();
}

void CalIRR::output_all_IRR() {  //輸出以視窗名稱排序以及IRR排序
    ofstream IRRout(info_.rootFolder_ + trainOrTest_ + "_IRR_IRR_sorted_" + info_.techType_ + ".csv");
    output_IRR(IRRout);
    for_each(allCompanyWindowsIRR_.begin(), allCompanyWindowsIRR_.end(), [](CompanyWindowIRRContainer &eachCompanyContainer) {
        sort(eachCompanyContainer.windowsIRR_.begin(), eachCompanyContainer.windowsIRR_.end(), [](const WindowIRR &w1, const WindowIRR &w2) {
            return w1.windowName_ < w2.windowName_;
        });
    });
    IRRout.open(info_.rootFolder_ + trainOrTest_ + "_IRR_name_sorted_" + info_.techType_ + ".csv");
    output_IRR(IRRout);
    if (info_.mixedTech_) {
        IRRout.open(info_.rootFolder_ + info_.techType_ + "_tech_choosed.csv");
        vector<string> algoOrTradition{"algo", "tradition"};
        for (auto &company : allCompanyWindowsIRR_) {
            IRRout << "=====" << company.companyName_ << "=====,";
            for (int loop = 0; loop < 2; loop++) {
                for_each(info_.techIndexs_.begin(), info_.techIndexs_.end(), [&](const int &techIndex) {
                    IRRout << algoOrTradition[loop] << " choose " << info_.allTech_[techIndex] << ",";
                });
            }
            IRRout << "\n";
            for (auto &eachIRR : company.windowsIRR_) {
                IRRout << eachIRR.windowName_ << ",";
                for_each(eachIRR.techChooseTimes_.begin(), eachIRR.techChooseTimes_.end(), [&](const vector<int> &i) {
                    for_each(info_.techIndexs_.begin(), info_.techIndexs_.end(), [&](const int &j) {
                        IRRout << i[j] << ",";
                    });
                });
                IRRout << "\n";
            }
        }
        IRRout.close();
    }
}

void CalIRR::output_IRR(ofstream &IRRout) {
    for (auto &company : allCompanyWindowsIRR_) {
        IRRout << "=====" << company.companyName_ << "=====";
        IRRout << "," << info_.techType_ << " algo," << info_.techType_ << " Tradition,";
        if (trainOrTest_ == "train")
            IRRout << "B&H";
        IRRout << "\n";
        for (auto &eachIRR : company.windowsIRR_) {
            IRRout << eachIRR.windowName_ << ",";
            IRRout << set_precision(eachIRR.algoIRR_) << ",";
            IRRout << set_precision(eachIRR.traditionIRR_) << ",";
            if (trainOrTest_ == "train")
                IRRout << set_precision(eachIRR.BHIRR_) << ",";
            IRRout << "\n";
        }
    }
    IRRout.close();
}

void CalIRR::output_all_window_rank() {
    vector<string> windowSort = remove_A2A_and_sort();
    ofstream rankOut(info_.rootFolder_ + trainOrTest_ + "_windowRank_" + info_.techType_ + ".csv");
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
    if (A2Aiter != windowSort.end())
        windowSort.erase(A2Aiter);
    windowSort.push_back("B&H");
    sort(windowSort.begin(), windowSort.end());
    return windowSort;
}

CalIRR::CalOneCompanyIRR::CalOneCompanyIRR(CompanyInfo &company, vector<CompanyWindowIRRContainer> &allCompanyWindowsIRR, vector<Rank> &allCompanyWindowRank, int trainOrTestIndex) : company_(company), allCompanyWindowsIRR_(allCompanyWindowsIRR), allCompanyWindowRank_(allCompanyWindowRank), trainOrTestIndex_(trainOrTestIndex) {
    set_filePath();
    thisCompanyWindowIRR_.companyName_ = company_.companyName_;
    if (company_.info_->mixedTech_) {
        tmpWinodwIRR_.resize_vec(company_.info_->allTech_.size());
    }
    for (auto &windowName : company_.info_->slidingWindows_) {
        if (windowName != "A2A") {
            cout << windowName << endl;
            cal_window_IRRs(windowName);
        }
    }
    thisCompanyWindowIRR_.windowsIRR_.push_back(cal_BH_IRR());
    rank_algo_and_tradition_window(thisCompanyWindowIRR_.windowsIRR_);
    allCompanyWindowsIRR_.push_back(thisCompanyWindowIRR_);
}

void CalIRR::CalOneCompanyIRR::set_filePath() {
    if (trainOrTestIndex_ == 0) {
        trainOrTestPaths_ = {company_.paths_.trainFilePaths_[company_.info_->techIndex_], company_.paths_.trainTraditionFilePaths_[company_.info_->techIndex_]};
    }
    else {
        trainOrTestPaths_ = {company_.paths_.testFilePaths_[company_.info_->techIndex_], company_.paths_.testTraditionFilePaths_[company_.info_->techIndex_]};
    }
}

void CalIRR::CalOneCompanyIRR::cal_window_IRRs(string windowName) {
    TrainWindow window(company_, windowName);
    tmpWinodwIRR_.algoIRR_ = cal_one_IRR(allRoRData_.algoRoRoutData_, window, trainOrTestPaths_.front(), false);
    tmpWinodwIRR_.traditionIRR_ = cal_one_IRR(allRoRData_.traditionRoRoutData_, window, trainOrTestPaths_.back(), true);  //若還沒有傳統的test這行可以註解
    if (trainOrTestIndex_ == 0) {
        string trainStartDate = company_.date_[window.interval_.front() + company_.tableStartRow_];
        string trainEndDate = company_.date_[window.interval_.back() + company_.tableStartRow_];
        BH bh(company_, trainStartDate, trainEndDate);
        tmpWinodwIRR_.BHIRR_ = pow(bh.BHRoR + 1.0, 1.0 / (double)(window.interval_.back() - window.interval_.front()) * company_.oneYearDays_) - 1.0;
    }
    tmpWinodwIRR_.windowName_ = windowName;
    thisCompanyWindowIRR_.windowsIRR_.push_back(tmpWinodwIRR_);
    tmpWinodwIRR_.fill_zero();
}

double CalIRR::CalOneCompanyIRR::cal_one_IRR(string &RoRoutData, TrainWindow &window, string &stratgyFilePath, bool tradition) {
    RoRoutData += window.windowName_ + ",";
    double totalRoR = 0;
    vector<path> strategyPaths = get_path(stratgyFilePath + window.windowName_);
    auto [filePathIter, intervalIter] = tuple{strategyPaths.begin(), window.interval_.begin()};
    for (; filePathIter != strategyPaths.end(); filePathIter++, intervalIter += 2) {
        RoRoutData += compute_and_record_window_RoR(strategyPaths, filePathIter, totalRoR, tradition, intervalIter);
    }
    double windowIRR = 0;
    if (trainOrTestIndex_ == 0) {
        totalRoR = totalRoR / (window.intervalSize_ / 2);
        windowIRR = pow(totalRoR--, company_.oneYearDays_) - 1.0;
    }
    else {
        windowIRR = pow(totalRoR--, 1.0 / company_.info_->testLength_) - 1.0;
    }
    RoRoutData += ",,,,,,," + window.windowName_ + "," + set_precision(totalRoR) + "," + set_precision(windowIRR) + "\n\n";
    return windowIRR;
}

string CalIRR::CalOneCompanyIRR::compute_and_record_window_RoR(vector<path> &strategyPaths, const vector<path>::iterator &filePathIter, double &totalRoR, bool tradition, vector<int>::iterator &intervalIter) {
    vector<vector<string>> file = read_data(*filePathIter);
    double RoR = stod(file[10][1]);
    string push;
    auto cal_IRR = [](double periodDays, double RoR) {
        return pow(RoR / 100.0 + 1.0, 1.0 / periodDays);
    };
    if (filePathIter == strategyPaths.begin()) {
        if (trainOrTestIndex_ == 0)
            totalRoR = cal_IRR(*(intervalIter + 1) - *(intervalIter) + 1.0, RoR);
        else
            totalRoR = RoR / 100.0 + 1.0;
        push += filePathIter->stem().string() + ",";
    }
    else {
        if (trainOrTestIndex_ == 0)
            totalRoR += cal_IRR(*(intervalIter + 1) - *(intervalIter) + 1.0, RoR);
        else
            totalRoR = totalRoR * (RoR / 100.0 + 1.0);
        push += "," + filePathIter->stem().string() + ",";
    }
    int techIndex = -1;
    if (!company_.info_->mixedTech_) {
        techIndex = company_.info_->techIndex_;
    }
    else {
        techIndex = find_index_of_string_in_vec(company_.info_->allTech_, file[0][1]);
    }
    for (int i = 0; i < eachVariableNum_[techIndex]; i++) {
        push += file[i + 12][1] + ",";
    }
    if (techIndex == 3) {
        push += ",";
    }
    push += file[10][1] + "\n";
    if (company_.info_->mixedTech_) {
        if (!tradition) {
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
    tmpWinodwIRR_.windowName_ = "B&H";
    tmpWinodwIRR_.algoIRR_ = pow(bh.BHRoR + 1.0, 1.0 / company_.info_->testLength_) - 1.0;
    tmpWinodwIRR_.traditionIRR_ = tmpWinodwIRR_.algoIRR_;
    tmpWinodwIRR_.BHIRR_ = tmpWinodwIRR_.algoIRR_;
    return tmpWinodwIRR_;
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
    sort_by_algo_IRR(windowsIRR);
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
                files.push_back(read_data(info_.rootFolder_ + info_.allTech_[techIndex] + "_test_IRR.csv"));
            }
            vector<vector<string>> newFile = read_data(info_.rootFolder_ + info_.techType_ + "_test_IRR.csv");
            for (auto &file : files) {
                for (size_t row = 0; row < file.size(); row++) {
                    for (size_t col = 0; col < file[row].size(); col++) {
                        newFile[row].push_back(file[row][col]);
                    }
                }
            }
            ofstream n(info_.rootFolder_ + "merge_" + info_.techType_ + "_IRR.csv");
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
    typedef vector<vector<vector<string>>::iterator> equalIterType;
    Info *info_;
    string sortBy_;
    int colToSort_;

    void sortByName(vector<vector<string>> &inputFile, equalIterType &equalSignIter) {
        for (size_t eachEqualIndex = 0; eachEqualIndex < equalSignIter.size() - 1; eachEqualIndex++) {
            sort(equalSignIter[eachEqualIndex] + 1, equalSignIter[eachEqualIndex + 1], [](const vector<string> &s1, const vector<string> &s2) {
                return s1[0] < s2[0];
            });
        }
    }

    void sortByIRR(vector<vector<string>> &inputFile, equalIterType &equalSignIter) {
        for (size_t eachEqualIndex = 0; eachEqualIndex < equalSignIter.size() - 1; eachEqualIndex++) {
            sort(equalSignIter[eachEqualIndex] + 1, equalSignIter[eachEqualIndex + 1], [&](const vector<string> &s1, const vector<string> &s2) {
                return stod(s1[colToSort_]) > stod(s2[colToSort_]);
            });
        }
    }

    equalIterType findEqualSign(vector<vector<string>> &inputFile) {
        equalIterType equalSignIter;
        for (auto iter = inputFile.begin(); iter != inputFile.end(); iter++) {
            if ((*iter).front().front() == '=') {
                equalSignIter.push_back(iter);
            }
        }
        equalSignIter.push_back(inputFile.end());
        return equalSignIter;
    }

    SortIRRFileBy(Info *info, string inpuFileName, int colToSort) : info_(info), colToSort_(colToSort) {
        sortBy_ = [](int colToSort) {
            if (colToSort == 0)
                return "name";
            return "IRR";
        }(colToSort_);
        inpuFileName += ".csv";
        vector<vector<string>> inputFile = read_data(info_->rootFolder_ + inpuFileName);
        equalIterType equalSignIter = findEqualSign(inputFile);
        if (colToSort_ == 0)
            sortByName(inputFile, equalSignIter);
        else
            sortByIRR(inputFile, equalSignIter);

        ofstream sortedFile(info_->rootFolder_ + cut_string(inpuFileName, '.')[0] + "_sorted_by_" + sortBy_ + ".csv");
        for (auto &row : inputFile) {
            for (auto &col : row) {
                sortedFile << col << ",";
            }
            sortedFile << "\n";
        }
        sortedFile.close();
    }

    SortIRRFileBy(vector<vector<string>> &inputFile, int colToSort) : colToSort_(colToSort) {
        equalIterType equalSignIter = findEqualSign(inputFile);
        sortByIRR(inputFile, equalSignIter);
    }
};

class FindBestHold {
public:
    Info *info_;
    string trainOrTest_;
    string techUse_;

    void start_copy(string companyRootPath, string fromFolder, string toFolder, string trainOrTest, string company, string window) {
        string from = companyRootPath + trainOrTest + fromFolder + company + "_" + window + ".csv";
        string to = companyRootPath + trainOrTest + toFolder;
        filesystem::copy(from, to, copy_options::overwrite_existing);
    }

    void copyBestHold(vector<vector<string>> &companyBestPeriod) {
        for (auto &companyBest : companyBestPeriod) {
            string companyRootPath = info_->rootFolder_ + "result_" + techUse_ + "/" + companyBest[0] + "/";
            start_copy(companyRootPath, "Hold/", "BestHold/", trainOrTest_, companyBest[0], companyBest[1]);
            start_copy(companyRootPath, "TraditionHold/", "TraditionBestHold/", trainOrTest_, companyBest[0], companyBest[2]);
        }
    }

    vector<vector<string>> findBestWindow(vector<vector<string>> &IRRFile) {
        vector<vector<string>> companyBestWindow;
        for (auto rowIter = IRRFile.begin(); rowIter != IRRFile.end(); rowIter++) {
            if ((*rowIter)[0].front() == '=') {
                vector<string> toPush;
                toPush.push_back((*rowIter)[0]);
                toPush[0].erase(remove(toPush[0].begin(), toPush[0].end(), '='), toPush[0].end());
                for (int addRow = 1; addRow < 5; addRow++) {
                    if ((*(rowIter + addRow))[0] != "B&H") {
                        toPush.push_back((*(rowIter + addRow))[0]);
                        break;
                    }
                }
                companyBestWindow.push_back(toPush);
            }
        }
        return companyBestWindow;
    }

    FindBestHold(Info *info, string IRRFileName) : info_(info), trainOrTest_(cut_string(IRRFileName, '_')[0]) {
        IRRFileName += ".csv";
        techUse_ = [&]() {
            string sorted = "sorted";
            size_t found = IRRFileName.find(sorted) + sorted.length();
            return cut_string(IRRFileName.substr(found, IRRFileName.length() - found), '.')[0];
        }();
        vector<vector<string>> IRRFile = read_data(info_->rootFolder_ + IRRFileName);
        vector<vector<string>> companyBestWindow1 = findBestWindow(IRRFile);
        SortIRRFileBy sortIRRFileBy(IRRFile, 2);
        vector<vector<string>> companyBestWindow2 = findBestWindow(IRRFile);
        auto [bestIter1, bestIter2] = tuple{companyBestWindow1.begin(), companyBestWindow2.begin()};
        for (; bestIter1 != companyBestWindow1.end(); bestIter1++, bestIter2++) {
            (*bestIter1).push_back((*bestIter2)[1]);
        }
        for (auto i : companyBestWindow1) {
            for (auto j : i)
                cout << j << ",";
            cout << endl;
        }
        copyBestHold(companyBestWindow1);
    }
};

class RunMode {
private:
    Info &info_;
    vector<path> &companyPricePaths_;

    Semaphore sem_;

    void run_mode(CompanyInfo &company) {
        sem_.wait();
        switch (company.info_->mode_) {
            case 0: {
                cout << "start train" << endl;
                Train train(company);
                break;
            }
            case 1: {
                cout << "start test" << endl;
                Test test(company, false);
                break;
            }
            case 2: {
                cout << "start train tradition" << endl;
                Tradition trainTradition(company);
                break;
            }
            case 3: {
                cout << "start test tradition" << endl;
                Test testTradition(company, true);
                break;
            }
            case 10: {
                // Test(company, company.info_->setWindow_, false, true, vector<int>{0});
                // Tradition tradition(company);
                // Train train(company, "2011-12-01", "2011-12-30");
                // Particle(&company, true, vector<int>{5, 20, 5, 20}).instant_trade("2020-01-02", "2021-06-30");
                // Particle(&company, true, vector<int>{70, 44, 85, 8}).instant_trade("2011-12-01", "2011-12-30");
                // Particle(&company, true, vector<int>{5, 10, 5, 10}).instant_trade("2020-01-02", "2020-05-29", true);
                // Particle(&company, true, vector<int>{14, 30, 70}).instant_trade("2012-01-03", "2020-12-31", true);
                // Test test(company, company.info_->setWindow_, false, true, vector<int>{0});
                // Particle(&company, company.info_->techIndex_, true, vector<int>{10, 12, 173, 162}).instant_trade("2012-09-04", "2012-09-28", true);
                // TrainLoop loop(company);
                // company.output_Tech();
                // HoldFile holdFile(&company, true, false);
                break;
            }
        }
        sem_.notify();
    }

public:
    RunMode(Info &info, vector<path> &companyPricePaths) : info_(info), companyPricePaths_(companyPricePaths), sem_(info.companyThreadNum_) {
        vector<thread> companyThread;
        vector<CompanyInfo> companiesInfo;
        for (auto companyPricePath : companyPricePaths) {
            companiesInfo.push_back(CompanyInfo(companyPricePath, _info));
        }
        for (auto &company : companiesInfo) {
            companyThread.push_back(thread(&RunMode::run_mode, this, ref(company)));
            this_thread::sleep_for(0.5s);
        }
        for (auto &thread : companyThread) {
            thread.join();
        }
    }
};

static vector<path> set_company_price_paths(const Info &info) {
    vector<path> companiesPricePath = get_path(info.priceFolder_);
    if (info.setCompany_ != "all") {
        vector<string> setCcompany = cut_string(info.setCompany_);
        auto find_Index = [&](string &traget) {
            return distance(companiesPricePath.begin(), find_if(companiesPricePath.begin(), companiesPricePath.end(), [&](path &pricePath) { return pricePath.stem() == traget; }));
        };
        size_t startIndex = find_Index(setCcompany.front());
        size_t endIndex = find_Index(setCcompany.back()) + 1;
        companiesPricePath = vector<path>(companiesPricePath.begin() + startIndex, companiesPricePath.begin() + endIndex);
    }
    return companiesPricePath;
}

int main(int argc, const char *argv[]) {
    time_point begin = steady_clock::now();
    vector<path> companyPricePaths = set_company_price_paths(_info);
    try {
        if (_info.mode_ <= 10) {
            if (_info.mode_ == 0) {
                cout << "this is train, will probably contaminate the existing train files" << endl;
                cout << "enter y to continue, enter any other key to abort" << endl;
                char check;
                cin >> check;
                if (check != 'y')
                    exit(0);
            }
            RunMode runMode(_info, companyPricePaths);
        }
        else {
            // CalIRR calIRR(companyPricePaths, "train");
            // MergeIRRFile mergeFile;
            // SortIRRFileBy IRR(&_info, "train_IRR_name_sorted_SMA_2", 1);
            // FindBestHold findBestHold(&_info, "test_IRR_IRR_sorted_SMA");
        }
    } catch (exception &e) {
        cout << "exception: " << e.what() << endl;
    }
    time_point end = steady_clock::now();
    cout << "time: " << duration_cast<milliseconds>(end - begin).count() / 1000.0 << " s" << endl;
    return 0;
}
