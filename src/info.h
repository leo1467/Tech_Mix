#ifndef INFO_H
#define INFO_H

#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace chrono;
using namespace filesystem; // C++17以上才有的library

class Info
{ // 放各種參數，要改參數大部分都在這邊
public:
    int mode_ = 11; // 0: 訓練期, 1: 測試期, 2: 傳統訓練期, 3: 傳統測試期, 4: 暴力法, 10: 其他自選功能(line 3398), 11: 主要用來輸出公司中每個視窗的ARR，還有一些自選功能(line 3472)

    string setCompany_ = "AAPL"; // "all": 跑全部公司, "AAPL,V,WBA": 跑這幾間公司, "AAPL to JPM": 跑這兩個公司(包含)之間的所有公司
    string setWindow_ = "1W1";   // "all": 跑全部視窗, "M2M,10D10,1W1": 跑這幾個視窗, "Y2Y to M2M": 跑這兩個視窗(包含)之間的所有視窗

    vector<int> techIndexs_ = {3, 3}; // 0: SMA, 1: WMA, 2: EMA, 3: RSI, if mixType_ 2, 先後順序決定買賣指標(買, 賣)
    int mixType_ = 2;                 // 0: 單純選好的指數, 1: 指數裡選好的買好的賣, 2: 用GN跑不同指標買賣, 3: 從2跑的選出報酬率高的，實驗只會用到2跟3
    bool mixedTech_;
    int techIndex_;
    vector<int> instantTrade = {}; // instant_trade用的指標的參數，配合techIndexs_使用，如SMA會是{5, 20, 5, 20}，RSI{14, 30, 70}, HI-SR{5, 20, 14, 30, 70}, HI-RS{14, 30, 70, 5, 20}

    vector<string> allTech_ = {"SMA", "WMA", "EMA", "RSI"};
    string techType_;
    int mixedTechNum_;

    int algoIndex_ = 3; // 根據下面演算法的index選擇要跑什麼
    vector<string> allAlgo_ = {"QTS", "GQTS", "GNQTS", "KNQTS"};
    string algoType_;

    double delta_ = 0.00016;      // 旋轉角度
    int expNum_ = 50;             // 實驗次數，SMA: 50, RSI: 3
    int genNum_ = 10000;          // 迭代次數
    int particleNum_ = 10;        // 粒子數量
    double iniFundLV_ = 10000000; // 初始資金

    int companyThreadNum_ = 0; // 公司執行緒數量，若有很多公司要跑，可以視情況增加thread數量，一間一間公司跑設0
    int windowThreadNum_ = 12; // 滑動視窗執行緒數量，若只跑一間公司，可以視情況增加thread數量，一個一個視窗跑設0，若有開公司thread，這個要設為0，避免產生太多thread

    bool debug_ = false; // 若要debug則改成true，會印出每個粒子的資訊

    int testDeltaLoop_ = 0;         // 用來測試delta的迴圈數量，確認哪一個delta比較好
    double testDeltaGap_ = 0.00001; // 根據上面的迴圈數量，每個迴圈都會減少這麼多的delta

    double multiplyUp_ = 1.01;   // KNQTS用
    double multiplyDown_ = 0.99; // KNQTS用
    int compareMode_ = 0;        // KNQTS用, 0: 比較漢明距, 1: 比較10進位距離

    string testStartYear_ = "2013-01"; // 測試期開始年月
    string testEndYear_ = "2023-01";   // 測試期結束的下一個月
    double testLength_;

    string priceFolder_ = "price/"; // 股價的folder名稱，通常不會換，只有在更新股價檔的時後才改，股價資料夾要放在同一個路徑

    string expFolder_ = "exp_result/"; // 所有的實驗的output都在這裡
    string rootFolder_ = "result_";    // 訓練期及測試期的相關資料在這
    string techFolder_ = "tech/";      // 存計算出來的技術指標

    vector<string> slidingWindows_ = {"YYY2YYY", "YYY2YY", "YYY2YH", "YYY2Y", "YYY2H", "YYY2Q", "YYY2M", "YY2YY", "YY2YH", "YY2Y", "YY2H", "YY2Q", "YY2M", "YH2YH", "YH2Y", "YH2H", "YH2Q", "YH2M", "Y2Y", "Y2H", "Y2Q", "Y2M", "H2H", "H2Q", "H2M", "Q2Q", "Q2M", "M2M", "H#", "Q#", "M#", "20D20", "20D15", "20D10", "20D5", "15D15", "15D10", "15D5", "10D10", "10D5", "5D4", "5D3", "5D2", "4D4", "4D3", "4D2", "3D3", "3D2", "2D2", "4W4", "4W3", "4W2", "4W1", "3W3", "3W2", "3W1", "2W2", "2W1", "1W1"};

    map<string, tuple<string, char, int, int>> slidingWindowPairs_;

    int windowNumber_;

    void set_techIndex_and_techType();

    void slidingWindowToEx();

    void set_folder();

    Info();
};


#endif
