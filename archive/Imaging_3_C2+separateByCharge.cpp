// ImagingSeparateByCharge.cpp : 電荷ペア毎にlistファイルを分ける

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.9:ファイルを出力するときに、イベント番号が1,000,000を超えるとデフォルトのdouble型では桁落ちするので、setprecision関数で精度を7桁に設定した
//2020.01.16:中性粒子を抽出できるように、chargeProd配列に0を追加

#include "./library.hpp"

vector<vector<double>> inputListData;
vector<vector<double>> outputListData;

string inputDataName;

//電荷の積
vector<int> chargeProd = { 0,1,2,3,4,6,8,9,12,16,18,24,27,36,54,81 };

//電荷の積ごとにデータを分ける関数
void separateCharge(int);

int main()
{
	cout << "*******************************************************" << endl;
	cout << "****     -リストデータを電荷毎に分解プログラム-    ****" << endl;
	cout << "****           全体のC2pairリストデータを          ****" << endl;
	cout << "****       電荷積ごとに別のファイルに保存する      ****" << endl;
	cout << "****         ver.2020.01.16 written by R. Murase   ****" << endl;
	cout << "*******************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);
	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;

	}
	else if(readDataName(datanamePath) == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}
	

	string outputDataFolder;
	cout << "出力するリストデータのフォルダを入力してください (\"@\"入力で同じフォルダ内の\"4_separated\"に保存)\n--->";
	cin >> outputDataFolder;
	if (outputDataFolder == "@")
	{
		outputDataFolder = upperFolder(inputDataFolder) + "\\4_separated";
	}

	//入力データパスの生成
	inputDataPath = inputDataFolder + "\\" + inputDataName + ".txt";
	ifstream ifs(inputDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//listDataを読み込み
	string tmp;
	while (getline(ifs, tmp))
	{
		stringstream ss(tmp);
		string tmp2;
		vector<double> inputLine;
		while (ss >> tmp2)
		{
			inputLine.push_back(stod(tmp2));
		}
		inputListData.push_back(inputLine);
	}

	cout << inputDataName + ".txt" + "----データの読み込み完了" << endl;

	for (int i = 0; i < chargeProd.size(); i++)
	{
		//電荷積ごとにファイルを分けて保存
		int q = chargeProd[i];
		separateCharge(q);
		cout <<"q_prod = "<< q << "----データの処理終了" << endl;

		//電荷積の組合わせがない場合はoutputListを作成しない
		if (outputListData.size()==0)
		{
			continue;
		}

		//出力データパスの生成
		string qStr = to_string(q);
		string outputFilePath = outputDataFolder + "\\" + inputDataName +"_qprod="+ qStr +".txt";

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		int columnOutput = outputListData[0].size();

		//doubele型でイベント番号を扱う時、イベント数が1,000,000を超えるとデフォルト(6桁)では桁落ちするので、書き込み精度を7桁にする
		ofs << setprecision(7);

		for (int i = 0; i < outputListData.size(); i++)
		{
			for (int j = 0; j < columnOutput; j++)
			{
				ofs << outputListData[i][j];
				if (j != columnOutput - 1) ofs << "\t";//デリミタ
			}
			ofs << endl;
		}
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//次の出力ファイルのために、outputリストデータに使用したメモリをクリア
		outputListData.clear();
	}

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//電荷の積ごとにデータを分けてoutputListDataにpush_back
void separateCharge(int q)
{
	int columnCharge = inputListData[0].size();
	for (int i = 0; i < inputListData.size(); i++)
	{
		int qprod = inputListData[i][columnCharge - 2];
		if (qprod == q)
		{
			outputListData.push_back(inputListData[i]);
		}
	}
}

