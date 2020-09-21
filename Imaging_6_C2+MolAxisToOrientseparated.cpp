// Imaging_C2+separateListWithOrientation.cpp : molAxis形式のリストデータ(C2+)を、配向角ごとに分ける
//

//アップデートログ
//2020.7.6:作成
//2020.7.9:VS Codeに移植
//2020.7.22:Imaging_C2+separateListWithOrientation.cpp -> Imaging_6_C2+MolAxisToOrientseparated.cppに改名
//2020.7.22:出力フォルダ名を"7_Orientseparated"に変更

#include "./library.hpp"

//分けるthetaの範囲
vector<double> t_min = { 0. , 30., 60. };
vector<double> t_max = { 30., 60., 90. };

//thetaの列
const int COL_THETA = 4;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "7_Orientseparated";

int main()
{
	cout << "****************************************************************" << endl;
	cout << "****      Imaging_6_C2+MolAxisToOrientseparated.cpp         ****" << endl;
	cout << "****    -molAxis形式のリストデータを、配向角ごとに分ける-   ****" << endl;
	cout << "****       para:0-30deg, mid:30-60deg, perp:60-90deg        ****" << endl;
	cout << "****                  ver.2020.07.22 written by R. Murase   ****" << endl;
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

		//区切るthetaごとにループ
		for (int i = 0; i < t_min.size(); i++)
		{
			//
			stringstream ss_min, ss_max;
			ss_min << fixed << setprecision(1) << t_min[i];
			ss_max << fixed << setprecision(1) << t_max[i];

			//出力ファイル
			vector<vector<double>> outputListData;
			for (int row = 0; row < inputListData.size(); row++)
			{
				if (t_min[i] <= inputListData[row][COL_THETA] && inputListData[row][COL_THETA] < t_max[i])
				{
					outputListData.push_back(inputListData[row]);
				}
			}
			string outputFilePath = outputDataFolder + "\\" + parsePath(inputFilePath)[1]+ "_"+ ss_min.str() + "-" + ss_max.str() + ".txt";
			writeListFile<double>(outputFilePath, outputListData);
			cout << parsePath(inputFilePath)[1] + "_" + ss_min.str() + "-" + ss_max.str() + "---done" << endl;
		}
	}
	cout << "全ての処理が終了しました。" << endl;
}
