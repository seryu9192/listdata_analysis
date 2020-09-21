// Imaging_@_cutoffByFrameNum.cpp : あるframe番号以上のイベントを切り捨てる

//アップデートログ
//2020.8.8:作成

#include "./library.hpp"

vector<vector<int>> inputListData;
vector<vector<int>> outputListData;

string inputDataName;

const int COL_FRAME = 0;
const int CUTOFF_NUM = 2349055;
void cutoffByFramenum(int);

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
    string outputFilePath = outputFolderName +  parsePath(inputDataPath)[1] + "_cutoff" + parsePath(inputDataPath)[2];

    //listDataの読み込み
    cout << inputDataPath << "----データを読み込んでいます...";
    inputListData = readListFile<int> (inputDataPath);
    cout << "読み込み完了、処理を開始します...";

    cutoffByFramenum(CUTOFF_NUM);

    cout << "処理終了" << endl;

    //出力ファイルにoutputListDataを書き込み
    cout << outputFilePath << "に書き込んでいます...";
    writeListFile<int> (outputFilePath, outputListData);		
    cout << "書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void cutoffByFramenum(int cutoff)
{
    for (int i = 0; i < inputListData.size(); i++)
    {
        int frame = inputListData[i][COL_FRAME];
        if(frame <= cutoff)
        {
            outputListData.push_back(inputListData[i]);
        }
    }
}