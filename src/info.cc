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
#include "functions.h"

using namespace std;
using namespace filesystem; // C++17以上才有的library


void Info::set_techIndex_and_techType()
{
	if (techIndexs_.size() == 1)
	{
		mixedTech_ = false;
	}
	else
	{
		techIndex_ = (int)allTech_.size();
		mixedTech_ = true;
	}
	mixedTechNum_ = (int)techIndexs_.size();

	if (!(mixedTech_ && mixType_ == 2))
	{
		sort(techIndexs_.begin(), techIndexs_.end());
	}

	for (auto &techIndex : techIndexs_)
	{
		techType_ += allTech_[techIndex] + "_";
	}
	techType_.pop_back();

	if (techIndexs_.size() > 1 && mixType_ > 0)
	{
		techType_ += "_" + to_string(mixType_);
	}

	algoType_ = allAlgo_[algoIndex_];
	testLength_ = stod(testEndYear_) - stod(testStartYear_);

	if (techIndexs_.size() == 1)
	{
		techIndex_ = techIndexs_[0];
	}
	else
	{
		techIndex_ = (int)allTech_.size();
		// =====改tech名稱=====
		if (techType_ == "SMA_RSI_2")
		{
			techType_ = "HI-SR";
		}
		else if (techType_ == "RSI_SMA_2")
		{
			techType_ = "HI-RS";
		}
		else if (techType_ == "SMA_RSI_3")
		{
			techType_ = "HI-all";
		}
		// ===================
		allTech_.emplace_back(techType_);
		if (mixType_ == 3) // 如果mixType_ 3，需要把mix的名稱放進去，company生路徑
		{
			for (int i = 0; i < 2; i++)
			{
				string mixeType2 = allTech_[techIndexs_[0]] + "_" + allTech_[techIndexs_[1]] + "_2";
				// =====改tech名稱=====
				if (mixeType2 == "SMA_RSI_2")
				{
					mixeType2 = "HI-SR";
				}
				else if (mixeType2 == "RSI_SMA_2")
				{
					mixeType2 = "HI-RS";
				}
				// ===================
				allTech_.emplace_back(mixeType2);
				reverse(techIndexs_.begin(), techIndexs_.end());
			}
		}
	}
}

void Info::slidingWindowToEx()
{
	map<char, int> componentLength{{'Y', 12}, {'H', 6}, {'Q', 3}, {'M', 1}};
	tuple<string, char, int, int> tmp;
	vector<int> trainTest;
	char delimiter;
	string slidingWindowEx;
	for (auto windowName : slidingWindows_)
	{
		// 做10個傳統滑動視窗
		if (isalpha(windowName.front()) && windowName != "A2A")
		{
			vector<string> trainTestPair = cut_string(windowName, '2');
			if (trainTestPair.size() == 2)
			{
				for_each(trainTestPair.begin(), trainTestPair.end(), [&](string windowComp)
				{
					int totalComponentLength = 0;
					// 計算訓練期及測試期長度
					for_each(windowComp.begin(), windowComp.end(), [&](char component)
					{
						totalComponentLength += componentLength.at(component);
					});
					trainTest.emplace_back(totalComponentLength);
					slidingWindowEx += to_string(totalComponentLength) + "M";
				});
				slidingWindowEx.pop_back();
				tmp = {slidingWindowEx, 'M', trainTest[0], trainTest[1]};
			}
			else if (trainTestPair.size() == 1) // 做*滑動視窗
			{
				slidingWindowEx += to_string(componentLength.at(trainTestPair[0][0])) + "M";
				tmp = {slidingWindowEx, 'S', componentLength.at(trainTestPair[0][0]), componentLength.at(trainTestPair[0][0])};
			}
		}
		else
		{ // 做A2A及其餘自訂滑動視窗
			trainTest = find_train_and_test_len(windowName, delimiter);
			tmp = {windowName, delimiter, trainTest[0], trainTest[1]};
		}
		slidingWindowPairs_.insert({windowName, tmp});
		trainTest.clear();
		slidingWindowEx.clear();
	}
}

void Info::set_folder()
{
	priceFolder_ = current_path().string() + "/" + priceFolder_;
	expFolder_ = current_path().parent_path().string() + "/" + expFolder_;
	// techFolder_ = expFolder_ + techFolder_;
	techFolder_ = current_path().string() + "/" + techFolder_;
	create_directories(expFolder_);
	current_path(expFolder_);
	rootFolder_ = rootFolder_ + testStartYear_ + "_" + testEndYear_ + "/";
}

Info::Info()
{
	set_techIndex_and_techType();
	slidingWindows_ = set_certain_range_of_vec(setWindow_, slidingWindows_);
	windowNumber_ = (int)slidingWindows_.size();
	slidingWindowToEx();
	set_folder();
}


