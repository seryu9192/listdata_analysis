// Imaging_1_COPToCharge.cpp : Standard形式のイメージングリストデータから、Cの価数に応じて電荷を示す列を追加し、輝点の位置を電場で振り分ける前に戻す

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2020.2.12:出力フォルダを自動生成するようにした
//2020.7.9:VS Codeに移植
//2020.7.10:ImagingStandardToCharge -> Imaging_1_COPToChargeに改名
//2020.7.16:"library.hpp"を使うようにする
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.20:ファイルが見つからなかった時の動作を強制終了(-1)からcontinueに変更

#include "./library.hpp"

//電場で振り分けた後のC分解片の各価数のスポットの中心座標と半径 : 積算イメージング像とクーロン爆発による広がりの計算から決定
//C0
int x_C0;
int y_C0;
int r_C0;
//C1+
int x_C1;
int y_C1;
int r_C1;
//C2+
int x_C2;
int y_C2;
int r_C2;
//C3+
int x_C3;
int y_C3;
int r_C3;
//C4+
int x_C4;
int y_C4;
int r_C4;

vector<vector<ui>> inputListData;
vector<vector<int>> intermediateListData;
vector<vector<int>> outputListData;

string inputDataName;

//素電荷あたりのdeflect距離
int x_def;
int y_def;

bool readParameter(string);
void addChargeColumn();
void deDeflect();

int main(int argc, char* argv[])
{
	cout << "*****************************************************************************" << endl;
	cout << "****                 -Imaging C分解片電荷解析プログラム-                 ****" << endl;
	cout << "****      Cの価数に応じて電荷を示す列を追加し、 電場で振り分ける前に戻す ****" << endl;
	cout << "****  *Parameter file は入力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                               ver.2020.07.20 written by R. Murase   ****" << endl;
	cout << "*****************************************************************************" << endl << endl;


	string inputDataFolder,inputDataPath;
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

	//datanameを読み取り
	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);

	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	if (readDataName(datanamePath) == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else
	{
		inputDataName = readDataName(datanamePath);
		cout << "dataname.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "param.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if(!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n同じフォルダ内の\"2_charge\"に保存(ない場合は自動生成)\n";
	outputFolderName = upperFolder(inputDataFolder) + "\\2_charge";
	_mkdir(outputFolderName.c_str());

	//素電荷あたりのdeflect距離
	x_def = x_C2 - x_C1;//x方向のdeflect (TOFの引き出し電場による)
	y_def = y_C2 - y_C1;//y方向のdeflect (平行平板ディフレクタによる)

	//連番処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + suf << "---ファイルを開けませんでした" << endl;
			continue;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf + ".txt";

		//listDataの読み込み
		cout << inputDataName + suf << "----データを読み込んでいます...";
		inputListData = readListFile<ui> (inputDataPath);
		cout << "読み込み完了、処理を開始します...";

		//電荷の列を追加
		addChargeColumn();

		//電荷に応じて座標を振り分ける前に戻す
		deDeflect();

		cout << "処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		cout << outputFilePath << "に書き込んでいます...";
		writeListFile<int> (outputFilePath, outputListData);		
		cout << "書き込みました" << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		intermediateListData.clear();
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

		int temp;
		vector<int> param;
		while (ss >> temp)
		{
			param.push_back(temp);
			//','に出会うまでの文字列を無視
			ss.ignore(line.size(), ',');
		}

		if (name == "C0")
		{
			x_C0 = param[0];
			y_C0 = param[1];
			r_C0 = param[2];
		}
		else if (name == "C1")
		{
			x_C1 = param[0];
			y_C1 = param[1];
			r_C1 = param[2];
		}
		else if (name == "C2")
		{
			x_C2 = param[0];
			y_C2 = param[1];
			r_C2 = param[2];
		}
		else if (name == "C3")
		{
			x_C3 = param[0];
			y_C3 = param[1];
			r_C3 = param[2];
		}
		else if (name == "C4")
		{
			x_C4 = param[0];
			y_C4 = param[1];
			r_C4 = param[2];
		}
		else
		{
			isValid = false;
		}
	}
	return isValid;
}

void addChargeColumn()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui triggerNum = inputListData[i][0];
		int x = inputListData[i][1];
		int y = inputListData[i][2];
		ui brightness = inputListData[i][3];
		int q = -1;

		if ((x - x_C0)*(x - x_C0)+ (y - y_C0)*(y - y_C0) <= r_C0 * r_C0)
		{
			q = 0;
		}
		else if ((x - x_C1)*(x - x_C1) + (y - y_C1)*(y - y_C1) <= r_C1 * r_C1)
		{
			q = 1;
		}
		else if ((x - x_C2)*(x - x_C2) + (y - y_C2)*(y - y_C2) <= r_C2 * r_C2)
		{
			q = 2;
		}
		else if ((x - x_C3)*(x - x_C3) + (y - y_C3)*(y - y_C3) <= r_C3 * r_C3)
		{
			q = 3;
		}
		else if ((x - x_C4)*(x - x_C4) + (y - y_C4)*(y - y_C4) <= r_C4 * r_C4)
		{
			q = 4;
		}
		else
		{
			q = -1;
		}

		//intermediateData1行を構成
		vector<int> intermediateLine;
		intermediateLine.push_back(triggerNum);
		intermediateLine.push_back(x);
		intermediateLine.push_back(y);
		intermediateLine.push_back(brightness);
		intermediateLine.push_back(q);

		//intermediateListDataに追加
		intermediateListData.push_back(intermediateLine);
	}
}

void deDeflect()
{
	for (int i = 0; i < intermediateListData.size(); i++)
	{
		ui triggerNum= intermediateListData[i][0];
		int x = intermediateListData[i][1];
		int y = intermediateListData[i][2];
		ui brightness = intermediateListData[i][3];
		int q = intermediateListData[i][4];

		if (q != -1)
		{
			x -= q * x_def;
			y -= q * y_def;
		}
		//outputData1行を構成
		vector<int> outputLine;
		outputLine.push_back(triggerNum);
		outputLine.push_back(x);
		outputLine.push_back(y);
		outputLine.push_back(brightness);
		outputLine.push_back(q);

		//outputListDataに追加
		outputListData.push_back(outputLine);
	}
}
