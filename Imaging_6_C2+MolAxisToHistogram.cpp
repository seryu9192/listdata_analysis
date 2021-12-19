// Imaging_6_C2+MolAxisToHistogram.cpp : molAxis形式のリストデータ(C2+)から、配向角に対するヒストグラムを作る
//

//アップデートログ
//2021.12.19: Imaging_6_C2+MolAxisToOrientseparatedから作成


#include "./library.hpp"

//thetaの最大値と刻み幅
const double dt = 5.;
const double t_max = 90.;
const int n_t = int(t_max/dt);

//thetaの列
const int COL_THETA = 4;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "8_Histogram";

int main()
{
	cout << "****************************************************************" << endl;
	cout << "****         Imaging_6_C2+MolAxisToHistogram.cpp            ****" << endl;
	cout << "****    -molAxis形式のリストデータからヒストグラムを作成  -   ****" << endl;
	cout << "****                  ver.2021.12.19 written by R. Murase   ****" << endl;
	cout << "****************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);
	
	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}
	
	string outputDataFolder = upperFolder(inputDataFolder_img) + "\\" + OUTPUT_FOLDER_NAME;
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
	}
	else
	{
		cout << "既存のフォルダに出力ファイルを保存します。" << endl;
	}

	//入力ファイルごとにループ
	for (auto inputFilename : fNames)
	{
		string inputFilePath = inputDataFolder_img + "\\" + inputFilename;
		//リストデータの読み込み
		auto inputListData = readListFile<double>(inputFilePath);

		//ヒストグラムデータ
		vector <int> outputHistgram(n_t, 0);

		//区切るthetaごとにループ
		for (int row = 0; row < inputListData.size(); row++)
		{
			// その行のthetaを判別
			auto theta = inputListData[row][COL_THETA];
			for (int i = 0; i < n_t; i++)
			{
				// その範囲にthetaが入っていたらそのbinにカウント+1して集計終了
				if (i*dt <= theta && theta < (i+1)*dt)
				{
					outputHistgram[i]++;
					break;
				}
			}
		}
		string outputFilePath = outputDataFolder + "\\" + inputFilename;
		writeListFile<int>(outputFilePath, outputHistgram);
		cout << parsePath(inputFilePath)[1] + "---done" << endl;
	}
	cout << "全ての処理が終了しました。" << endl;
}
