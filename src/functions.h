#ifndef functions_h
#define functions_h

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace filesystem;

class Ini_st {
    public:
    unordered_map<string* , unordered_map<string *, string *>> keyValue;
};

[[gnu::always_inline]] 
inline void read_ini (Ini_st **ini_st, const path filePath) 
{
    *ini_st = new Ini_st;
    ifstream infile(filePath);
    vector<vector<string>> data;
    if (!infile) {
        cout << filePath << " not found" << endl;
        exit(1);
    }
    //    cout << "reading " << filePath.filename() << endl;
    string row;
    string cell;
    vector<string> oneRow;
    while (getline(infile, row)) {
        stringstream lineStream(row);
        while (getline(lineStream, cell, '=')) {
            if (cell != "\r") {
                if (cell.find('\r') != std::string::npos) {
                    cell.pop_back();
                }
                oneRow.emplace_back(cell);
            }
        }
        if (oneRow.front().front() == '[') 
        {
            
        }
        data.emplace_back(oneRow);
        row.clear();
        cell.clear();
        oneRow.clear();
    }
    infile.close();
}

[[gnu::always_inline]]
inline vector<vector<string>> read_data(path filePath)
{
    ifstream infile(filePath);
    vector<vector<string>> data;
    if (!infile)
    {
        cout << filePath << " not found" << endl;
        exit(1);
    }
    //    cout << "reading " << filePath.filename() << endl;
    string row;
    string cell;
    vector<string> oneRow;
    while (getline(infile, row))
    {
        stringstream lineStream(row);
        while (getline(lineStream, cell, ','))
        {
            if (cell != "\r")
            {
                if (cell.find('\r') != std::string::npos)
                {
                    cell.pop_back();
                }
                oneRow.emplace_back(cell);
            }
        }
        data.emplace_back(oneRow);
        row.clear();
        cell.clear();
        oneRow.clear();
    }
    infile.close();
    return data;
}

[[gnu::always_inline]]
inline vector<path> get_path(path targetPath)
{
    if (!exists(targetPath))
    {
        cout << targetPath << " does not exist" << endl;
        exit(1);
    }
    vector<path> filePath;
    copy(directory_iterator(targetPath), directory_iterator(), back_inserter(filePath));
    sort(filePath.begin(), filePath.end());
    for (auto i = filePath.begin(); i != filePath.end(); i++)
    {
        if (i->filename() == ".DS_Store")
        {
            filePath.erase(i);
        }
    }
    return filePath;
}

[[gnu::always_inline]]
inline bool is_double(const string &s)
{
    string::const_iterator it = s.begin();
    int dotsCnt = 0;
    while (it != s.end() && (isdigit(*it) || *it == '.'))
    {
        if (*it == '.')
        {
            ++dotsCnt;
            if (dotsCnt > 1 || dotsCnt == s.size())
            {
                break;
            }
        }
        ++it;
    }
    return !s.empty() && it == s.end();
}

[[gnu::always_inline]]
inline string set_precision(const double inputDouble, const int n = 10)
{
    stringstream ss;
    ss << fixed << setprecision(n) << inputDouble;
    return ss.str();
}

[[gnu::always_inline]]
static inline vector<int> find_train_and_test_len(string window, char &delimiter)
{
    for (int i = 0; i < window.length(); i++)
    {
        if (isalpha(window[i]))
        {
            delimiter = window[i];
            break;
        }
    }
    string segment;
    vector<string> segmentList;
    stringstream toCut(window);
    while (getline(toCut, segment, delimiter))
    {
        segmentList.emplace_back(segment);
    }
    vector<int> TrainTest;
    for (auto i : segmentList)
    {
        if (i.length() == 0)
        {
            TrainTest.emplace_back(0);
        }
        else
        {
            TrainTest.emplace_back(stoi(i));
        }
    }
    return TrainTest;
}

[[gnu::always_inline]]
inline vector<string> cut_string(string input, char delimiter = ' ')
{
    string segment;
    vector<string> segmentList;
    stringstream toCut(input);
    while (getline(toCut, segment, delimiter))
    {
        segmentList.emplace_back(segment);
    }
    return segmentList;
}

[[gnu::always_inline]]
inline void check_startRowSize_endRowSize(int startRowSize, int endRowSize)
{
    if (startRowSize > endRowSize)
    {
        cout << "startRowSize > endRowSize" << endl;
        exit(1);
    }
    else if (startRowSize < endRowSize)
    {
        cout << "startRowSize < endRowSize" << endl;
        exit(1);
    }
}

[[gnu::always_inline]]
inline vector<int> save_startRow_EndRow(vector<int> &startRow, vector<int> &endRow)
{
    vector<int> interval;
    for (int i = 0; i < startRow.size(); i++)
    {
        interval.emplace_back(startRow[i]);
        interval.emplace_back(endRow[i]);
    }
    return interval;
}

[[gnu::always_inline]]
inline int cal_weekday(string date)
{
    int y = stoi(date.substr(0, 4));
    int m = stoi(date.substr(5, 2)) - 1;
    int d = stoi(date.substr(8, 2));
    tm time_in = {0, 0, 0, d, m, y - 1900};
    time_t time_temp = mktime(&time_in);
    const tm *time_out = localtime(&time_temp);
    return time_out->tm_wday;
}

[[gnu::always_inline]]
inline bool is_over_7_days(string date1, string date2)
{
    tm d1 = {0, 0, 0, stoi(date1.substr(8, 2)), stoi(date1.substr(5, 2)) - 1, stoi(date1.substr(0, 4)) - 1990};
    tm d2 = {0, 0, 0, stoi(date2.substr(8, 2)), stoi(date2.substr(5, 2)) - 1, stoi(date2.substr(0, 4)) - 1990};

    time_t t1 = mktime(&d1);
    time_t t2 = mktime(&d2);
    double daysApart = abs(difftime(t1, t2) / (60.0 * 60.0 * 24.0));

    return daysApart >= 7;
}

// bool is_week_changed(vector<string> &date, int bigWeekDay, int smallWeekDay, int big_i, int small_i) {
//     int bigDate = stoi(date[big_i].substr(8, 2));
//     int smallDate = stoi(date[small_i].substr(8, 2));
//     return (bigWeekDay <= smallWeekDay ||
//             bigDate - smallDate >= 7 ||
//             ((bigDate < smallDate) && (bigDate + 30 - smallDate) >= 7));
// }

[[gnu::always_inline]]
inline int find_index_of_string_in_vec(const vector<string> &stringVector, const string targetString)
{
    auto iter = find(stringVector.begin(), stringVector.end(), targetString);
    if (iter == stringVector.end())
    {
        cout << "cant find " << targetString << endl;
        exit(1);
    }
    return (int)distance(stringVector.begin(), iter);
}

[[gnu::always_inline]]
inline vector<string> set_certain_range_of_vec(const string &inputString, vector<string> &targetVector)
{
    if (inputString.find(",") != string::npos)
    {
        return cut_string(inputString, ',');
    }
    string inputForSetTarget = [&]()
    {
        if (inputString == "all")
        {
            return targetVector.front() + " to " + targetVector.back();
        }
        return inputString;
    }();
    vector<string> setTarget = cut_string(inputForSetTarget);
    if (auto iter = find_if(setTarget.begin(), setTarget.end(), [](string i) { return i == "to"; }); iter != setTarget.end())
    {
        setTarget.erase(iter);
    }
    auto find_target = [&](string &targetString)
    { 
        return find_if(targetVector.begin(), targetVector.end(), [&](string &string) { return string == targetString; }); 
    };
    auto firstWindowIter = find_target(setTarget.front());
    auto lastWindowIter = find_target(setTarget.back()) + 1;
    if (firstWindowIter == targetVector.end() || lastWindowIter - 1 == targetVector.end())
    {
        cout << "cant find inpuString in targetVector" << endl;
        exit(1);
    }
    return vector<string>(firstWindowIter, lastWindowIter);
}

[[gnu::always_inline]]
inline string remove_zeros_at_end(double n)
{
    string stringWOzeroAtEnd = set_precision(n);
    stringWOzeroAtEnd.erase(stringWOzeroAtEnd.find_last_not_of('0') + 1, std::string::npos);
    return stringWOzeroAtEnd;
}

[[gnu::always_inline]]
inline string get_date(vector<string> &date, int startRow, int endRow)
{
    return date[startRow] + "_" + date[endRow];
}

[[gnu::always_inline]]
inline vector<int> change_vec_string_to_int(vector<string> aStrategyStrings)
{
    if (aStrategyStrings.size() != 0)
    {
        vector<int> aStrategyInt;
        for (auto s : aStrategyStrings)
        {
            aStrategyInt.emplace_back(stoi(s));
        }
        return aStrategyInt;
    }
    return vector<int>{};
}
#endif /* functions_h */
