// MCS6A_ListToMPA_all_in_folder.cpp : リストファイルからTOFヒストグラムを作成。(フォルダ内のすべてのデータについて処理）
//

//アップデートログ
//2020.12.30:MCS6A_ListToMPA_all_in_folder.cppから作成。全ての結果を足し合わせて一つのヒストグラムにする

#include "./library.hpp"

//実験時のMCSのbin幅とchannel範囲の決定
constexpr auto BIN_WIDTH_EXP = 8;
constexpr auto MCS_CH_EXP = 8000;

//リストデータからTOFスペクトルを再構成するときのchannelとbin幅の決定
constexpr auto BIN_WIDTH = 8;
constexpr auto CHNUM = BIN_WIDTH_EXP * MCS_CH_EXP / BIN_WIDTH;

void integrateTOF(vector<ui>&, vector<vector<int>>);

int main()
{
	cout << "**********************************************************************" << endl;
	cout << "***           MCS6A_ListToMPA_all_in_folder_summation.cpp          ***" << endl;
	cout << "***                    MCS6A listデータ(stop)から                  ***" << endl;
	cout << "***                 TOFスペクトルを構成するプログラム              ***" << endl;
	cout << "***      (フォルダ内のすべてのリストデータを処理し足し合わせ)      ***" << endl;
	cout << "***                            ver.2020.12.30 written by R. Murase ***" << endl;
	cout << "**********************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder);

	//出力フォルダの指定
	cout << "同じフォルダ内の\"histogram\"フォルダに保存（なければ新たに作成）)\n";
	string outputFolderName = inputDataFolder + "\\histogram";

	//フォルダの作成
	_mkdir(outputFolderName.c_str());

	//ヒストグラムを初期化(output用)
	vector<ui> outputHistogram(CHNUM, 0);

	//フォルダの中のファイルについてループ
	for (auto inputDataName : fNames)
	{
		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName;
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + "----ファイルを開けませんでした" << endl;
			continue;
		}
		//リストデータ読み取り
		auto inputListData = readListFile<int>(inputDataPath);
		cout << inputDataName + "----データの読み込み完了...";

		//ヒストグラムを作成
		integrateTOF(outputHistogram, inputListData);
		cout << "処理終了" << endl;
	}
	
	//出力データパスの生成
	string outputFilePath = outputFolderName + "\\histogram_sum.txt";

	//出力ファイルにoutputListDataを書き込み
	ofstream ofs(outputFilePath);

	//出力ファイルへの書き込み処理
	writeListFile<ui>(outputFilePath, outputHistogram);

	cout << outputFilePath << "に書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します" << endl;
	return 0;
}

// 1行ずつchを読み取ってヒストグラムを構成
void integrateTOF(vector<ui>& tofIntegrated, vector<vector<int>> listData)
{
	int columnInput = listData[0].size();
	int columnChannel = columnInput - 2; // start(6)/stop(1)を識別する値の列の列番号
	int columnTof = columnInput - 1; // 飛行時間(チャンネル）の列番号
	for (int i = 0; i < listData.size(); i++)
	{
		//stop信号(1)のみ積算
		if (listData[i][columnChannel] == 1)
		{
			ui ch = listData[i][columnTof] / BIN_WIDTH;//binWidthごとに1チャンネルにまとめる
			if (ch >= CHNUM)//chNumを超えるデータ（バグ）は読み飛ばす
			{
				continue;
			}
			tofIntegrated[ch]++;
		}
	}
	return;
}
