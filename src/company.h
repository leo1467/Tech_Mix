#ifndef COMPANY_H
#define COMPANY_H

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

#include "info.h"

using namespace std;
using namespace chrono;
using namespace filesystem; // C++17以上才有的library

class CompanyInfo
{ // 放各種跟公司有關的資料
public:
    class Path
    {
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

    typedef vector<void (CompanyInfo::*)(vector<double> &)> calTech;
    calTech cal_tech_{&CompanyInfo::cal_SMA, &CompanyInfo::cal_WMA, &CompanyInfo::cal_EMA, &CompanyInfo::cal_RSI};

    void set_paths(Path &paths);
    void store_date_price(path priceFilePath);
    void create_folder(Path &paths); // 用不到
    void find_table_start_row();
    void store_tech_to_vector(int techIndex);
    void cal_SMA(vector<double> &tmp);
    void cal_WMA(vector<double> &tmp);
    void cal_EMA(vector<double> &tmp);
    void cal_RSI(vector<double> &tmp);
    void output_tech_file(int techIndex);
    void set_techFile_title(ofstream &out, int techPerid, int techIndex);

    CompanyInfo(path pricePath, Info &info);
};


#endif
