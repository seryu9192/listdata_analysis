// Imaging_@_averagePointsPerFrame.cpp : フレームあたりの輝点の数を数える

//アップデートログ
//2020.8.8:作成

#include "./library.hpp"

vector<vector<ui>> inputListData;
vector<vector<int>> outputListData;

string inputDataName;

const int COL_FRAME = 0;
const int framesPerBin = 10000;
void countPointsPerFrame();

int main(int argc, char* argv[])
{
	string inputDataFolder,inputDataPath;
	
    cout << "解析するリストデータのパスを入力してください\n--->";
    cin >> inputDataPath;
    inputDataName = parsePath(inputDataPath)[1] + parsePath(inputDataPath)[2];

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n同じフォルダ内に保存\n";
	outputFolderName = parsePath(inputDataPath)[0];

    //出力データパスの生成
    string outputFilePath = outputFolderName +  parsePath(inputDataPath)[1] + "_framesNum" + parsePath(inputDataPath)[2];

    //listDataの読み込み
    cout << inputDataPath << "----データを読み込んでいます...";
    inputListData = readListFile<ui> (inputDataPath);
    cout << "読み込み完了、処理を開始します...";

    countPointsPerFrame();

    cout << "処理終了" << endl;

    //出力ファイルにoutputListDataを書き込み
    cout << outputFilePath << "に書き込んでいます...";
    writeListFile<int> (outputFilePath, outputListData);		
    cout << "書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void countPointsPerFrame()
{
    int bin = 0;
    int cnt = 0;
    for (int i = 0; i < inputListData.size(); i++)
    {
        if(inputListData[i][COL_FRAME] > (bin+1) * framesPerBin)
        {
            vector<int> outputLine = {bin * framesPerBin, cnt};
            outputListData.push_back(outputLine);
            bin++;
            cnt = 0;
        }
        cnt++;
    }
}