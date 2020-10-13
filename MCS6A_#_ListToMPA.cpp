// MCS6A_ListToMPA.cpp : リストファイルからTOFヒストグラムを作成。
//

//アップデートログ
//2019.10.12:リストデータから自動的にchNum(TAC rangeを設定するようにした)
//2019.10.12:コマンドライン引数から、ドラッグ&ドロップでリストデータのパスを受け渡せるようにした
//2020.7.9:VS Codeに移植
//2020.8.7:library.hppを導入(readListFile, writeListFile, parsePath)

#include "./library.hpp"

//実験時のMCSのbin幅とchannel範囲の決定
constexpr auto binWidthExp = 8;
ui mcsChExp = 8000;

//リストデータからTOFスペクトルを再構成するときのchannelとbin幅の決定
constexpr auto binWidth = 8;
ui chNum = binWidthExp * mcsChExp / binWidth;

//列
int COL_CHN = 1;
int COL_TOF = 2;

vector<vector<ui>> inputListData;
vector<ui> integrateTOF(ui);

int main(int argc, char* argv[])
{
	cout << "*************************************************" << endl;
	cout << "****       MCS6A listデータ(stop)から        ****" << endl;
	cout << "****   TOFスペクトルを構成するプログラム     ****" << endl;
	cout << "****   ドラッグ&ドロップでもパス入力可能     ****" << endl;
	cout << "****       ※ただしファイルは一つまで        ****" << endl;
	cout << "****       ver.2020.8.7 written by R. Murase ****" << endl;
	cout << "*************************************************" << endl << endl;

	//入力データの指定
	string inputDataPath;
	if (argc == 1) 
	{
		cout << "変換するリストデータのフルパスを入力してください\n--->";
		cin >> inputDataPath;
	}
	else
	{
		inputDataPath = argv[1];
		cout << "解析するリストデータのファイル名：\n" << inputDataPath << endl;
	}


	//フルパスの分解
	string folderpath = parsePath(inputDataPath)[0];
	string filename = parsePath(inputDataPath)[1];

	if (!checkFileExistence(inputDataPath))
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
	cout << inputDataPath + "----データを読み込んでいます...";
	inputListData = readListFile<ui> (inputDataPath);
	cout << "データの読み込み完了、処理を開始します...";

	//読み込んだリストデータの列数で判断
	COL_CHN = inputListData[0].size() - 2;
	COL_TOF = inputListData[0].size() - 1;
	//chNumの自動設定(リストデータからchの最大値を探す)
	for (int i = 0; i < inputListData.size(); i++)
	{
		chNum = max(chNum, inputListData[i][COL_TOF] / 8);
	}
	//TOFのチャンネルごとのyieldを数える
	auto tofIntegrated = integrateTOF(chNum);
	cout << tofIntegrated.size() << endl;
	cout << "処理終了 ---> 書き込んでいます...";

	//出力データパスの生成
	string outputFilePath = folderpath + "\\" + filename + "bin=" + to_string(binWidth) + "integrated.txt";
	//出力ファイルにoutputListDataを書き込み
	writeListFile<ui> (outputFilePath, tofIntegrated);
	cout << "書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します" << endl;
	return 0;
}

// 1行ずつchを読み取ってtofIntegratedに積算(引数はtof range)
vector<ui> integrateTOF(ui maxCh)
{
	vector<ui> res(maxCh, 0);
	for (int i = 0; i < inputListData.size(); i++)
	{
		//stop信号(1)のみ積算
		if (inputListData[i][COL_CHN] == 1)
		{
			ui ch = inputListData[i][COL_TOF] / binWidth;//binWidthごとに1チャンネルにまとめる
			if (ch>=chNum)
			{
				continue;	
			}
			res[ch]++;
		}
	}
	return res;
}