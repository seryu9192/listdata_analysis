// ImagingC2+DivAngleToMolAxis.cpp : 電荷毎にある核間距離を仮定してdivAngleをmolAxisに変換する
//

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.9:ファイルを出力するときに、イベント番号が1,000,000を超えるとデフォルトのdouble型では桁落ちするので、setprecision関数で精度を7桁に設定した
//2020.4.2:"library.hpp"を導入
//2020.4.2:OUTPUT_FOLDER_NAMEを導入。自動で出力フォルダを用意するようにした
//2020.7.9:VS Codeに移植
//2020.7.16:parameterをinputDataFolderから読み込むように変更
//2020.7.16:"divToMol"の返り値をvoidから、divAngle -> theta変換に使ったdouble r0になるようにした
//2020.7.18:出力フォルダを固定化(6_molAXis)
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする

#include "./library.hpp"

double E0;  //一原子当たりの初期運動エネルギー(MeV)
vector<double> r0s(17, 1);	//初期核間距離(Å) r0(qprod):

//リストデータを扱う配列
vector<vector<double>> inputListData;
vector<vector<double>> outputListData;

//データ名
string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "6_molAxis";

//リストデータの列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_DIVANGLE = 4;
const int COL_CHARGEPROD = 5;
const int COL_BRIGHTDIFF = 6;

//データの変換に使う関数
double divToMol();
bool readParameter(string);

int main(int argc, char* argv[])
{
	cout << "*****************************************************************************" << endl;
	cout << "****                          -分子軸計算プログラム-                     ****" << endl;
	cout << "****               divAngleから分子軸とビーム軸のなす角を計算            ****" << endl;
	cout << "****                      その形式のリストデータに変換                   ****" << endl;
	cout << "**** ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                 ver.2020.09.20 written by R. Murase ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	int start, end;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder;

		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 4)
	{
		inputDataFolder = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}	

	//dataname.txtを探す
	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);
	inputDataName = readDataName(datanamePath);
	if (inputDataName != "")
	{
		cout << "dataname.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}
	else
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}

	//出力データパスの生成
	string outputFileName;
	cout << "同じフォルダ内の" + OUTPUT_FOLDER_NAME +  "に保存（なければ自動作成）)\n";
	string outputDataFolder = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	_mkdir(outputDataFolder.c_str());

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "param.txt";
	if (!checkFileExistence(parameterFilePath))
	{
		cerr << "パラメータファイルが見つかりませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//フォルダ内のファイルを列挙
	auto fileNames = getFileNameInDir(inputDataFolder);
	cout << "--inputfile list--" << endl;
	for(auto fileName : fileNames)
		if (fileName.find("param") == string::npos)
			cout << fileName << endl;
	cout << endl;

	//フォルダ内の全ファイルを処理
	for(auto inputfilename : fileNames)
	{
		//param fileは読み飛ばし
		if (inputfilename.find("param") != string::npos)
			continue;
		
		string inputDataPath = inputDataFolder + "\\" + inputfilename;
		//listDataの読み込み
		cout << inputfilename + "----データを読み込んでいます...";
		inputListData = readListFile<double>(inputDataPath, COL_FILE, start, end+1);
		cout << "データの読み込み完了 -> 処理を開始します...";

		//変換処理開始
		double r0 = divToMol();
		cout << "処理終了" << endl;

		//outputListData書き込み
		string outputFilePath = outputDataFolder + "\\" + stripExtension(inputfilename) + "_r0=" + doubletoString(r0, 2) + ".txt";
		cout << outputFilePath << "に書き込んでいます...";
		//start == 1の時は新規、それ以外はappend
		bool append = true;
		if (start == 1)append = false;
		writeListFile<double>(outputFilePath, outputListData, '\t', append);
		cout << "書き込みました" << endl;

		//メモリクリア
		inputListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

bool readParameter(string pfilePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = true;

	string filepath = pfilePath;
	ifstream ifs(filepath);

	//パラメータファイルを1行ずつ読み取る
	string line;
	while (getline(ifs, line))
	{
		//0文字目が'#'の行は読み飛ばす
		if (line[0] == '#')
			continue;
		//'='がない行も読み飛ばす
		if (line.find('=') == string::npos)
			continue;

		//stringstreamインスタンスに読み取った文字列を入れる
		stringstream ss(line);

		//読み取った文字列のうち、スペースの前までの部分をname文字列に入れる
		string name;
		ss >> name;

		//'='に出会うまでの文字列を無視
		ss.ignore(line.size(), '=');

		//'='以降のパラメータをtempに格納
		double tmp;
		ss >> tmp;

		if (name == "E0")
		{
			//E0のimport
			E0 = tmp;
		}
		for (int i = 1; i <= 16; i++)
		{
			//r0のimport
			string paraName = "r0_" + to_string(i);
			if (name == paraName)
			{
				r0s[i] = tmp;
			}
		}
	}
	return isValid;
}

double divToMol()
{
	double res = 0;
	//inputListDataをはじめから1行ずつ見ていく
	for (int i = 0; i < inputListData.size(); i++)
	{
		//各列が表す量
		int fileNum = inputListData[i][COL_FILE];
		int frameNum = inputListData[i][COL_FRAME];
		int x = inputListData[i][COL_X];
		int y = inputListData[i][COL_Y];
		double divAngle = inputListData[i][COL_DIVANGLE];
		double q_prod = inputListData[i][COL_CHARGEPROD];
		int bright_diff = inputListData[i][COL_BRIGHTDIFF];

		//クーロンエネルギーの計算
		double Ec = q_prod * e2 / r0s[q_prod];

		//初期運動量とCoulomb explosionによる運動量変化の比を計算した諸々の量(p0/pc = sqrt(E0/2/Ec))
		double a = sqrt(E0 * pow(10, 6) / (double)2 / Ec);

		//arcsinが取れない角度が来たときは分子軸を計算しない
		if ((a * divAngle / 1000.) > 1) continue;
		//分子軸の角度を計算
		double molAxis = asin(a * divAngle / 1000.) * 180. / PI;

		//outputLineを構成
		vector<double> outputLine;
		outputLine.push_back(fileNum);
		outputLine.push_back(frameNum);
		outputLine.push_back(x);
		outputLine.push_back(y);
		outputLine.push_back(molAxis);
		outputLine.push_back(q_prod);
		outputLine.push_back(bright_diff);

		outputListData.push_back(outputLine);

		res = r0s[q_prod];
	}
	return res;
};
