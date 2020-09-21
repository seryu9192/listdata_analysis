// MCS6A_ListToMPA_all_in_folder.cpp : リストファイルからTOFヒストグラムを作成。(フォルダ内のすべてのデータについて処理）
//

//アップデートログ
//2019.8.23:ファイル名をfilename.txtから読み込むようにした
//2019.8.24:連番処理 -> フォルダ内のデータを一括処理に変更
//2020.2.25:#_spectraフォルダを自動生成するようにした
//2020.7.7:新ソリューション作り直し+"MCS6A_ListToMPAseries.cpp"を引き継ぎ
//2020.7.8:"library.hpp"を導入。データ(inputListDataなど)をグローバル変数に置くのを廃止。出力フォルダのデフォルトを入力フォルダの下に設定。integrateTOFを引数付きにした
//2020.7.9:VS Codeに移植

#include "./library.hpp"

//実験時のMCSのbin幅とchannel範囲の決定
constexpr auto BIN_WIDTH_EXP = 8;
constexpr auto MCS_CH_EXP = 8000;

//リストデータからTOFスペクトルを再構成するときのchannelとbin幅の決定
constexpr auto BIN_WIDTH = 8;
constexpr auto CHNUM = BIN_WIDTH_EXP * MCS_CH_EXP / BIN_WIDTH;

vector<ui> integrateTOF(vector<vector<int>>);

int main()
{
	cout << "*************************************************************" << endl;
	cout << "***            MCS6A_ListToMPA_all_in_folder.cpp          ***" << endl;
	cout << "***                 MCS6A listデータ(stop)から            ***" << endl;
	cout << "***            TOFスペクトルを構成するプログラム          ***" << endl;
	cout << "***        (フォルダ内のすべてのリストデータを処理)       ***" << endl;
	cout << "***                    ver.2020.7.8 written by R. Murase ***" << endl;
	cout << "*************************************************************" << endl << endl;

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
		auto outputData = integrateTOF(inputListData);
		cout << "処理終了...";

		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);

		//出力ファイルへの書き込み処理
		writeListFile<ui>(outputFilePath, outputData);

		cout << outputFilePath << "に書き込みました" << endl;

	}
	cout << "全ての処理が完了しました。プログラムを終了します" << endl;
	return 0;
}

// 1行ずつchを読み取ってヒストグラムを構成
vector<ui> integrateTOF(vector<vector<int>> listData)
{
	vector<ui> tofIntegrated(CHNUM, 0);
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
	return tofIntegrated;
}
