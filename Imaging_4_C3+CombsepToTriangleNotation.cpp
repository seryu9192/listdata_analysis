// Imaging_4_C3+CombsepToTriangleNotation.cpp
//：C3+のイメージングリストデータにおける3つのC分解片の点
//　からなる三角形の面積、辺の長さ、内角、対称座標を計算しリストデータにする

//列 0:

//アップデートログ
//2018.10.2:プログラム作成
//2019.9.26:発掘。変数名等を2量体のデータ解析に用いたプログラムと統一
//2019.9.28:3点のみ抽出する機能を別のプログラム"ImagingC3+extract3Points"と分離
//2019.9.28:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.9.28:filename.txtから自動でファイル名を読み込ませるようにした
//2019.9.28:charge stateの組み合わせによってファイルを分けて保存するようにした
//2019.9.29:outputListDataのcol=13にSR=sqrt(S2*S2+S3*S3)を追加
//2019.9.29:suffixの形式を(q1,q2,q3)からq1*q2*q3に変更
//2019.10.15:SRの範囲ごとにinputListDataを分ける機能(sepaListData)を追加
//2019.10.15:SRの範囲ごとにoutputListDataを分けて保存する機能を追加
//2020.7.9:VS Codeに移植
//2020.7.27:C3+TriangleAnalysis.cpp -> Imaging_4_C3+CombsepToTriangleNotation.cppに改名
//2020.7.27:"library.hpp"を導入
//2020.7.27:SRの範囲ごとに分ける機能を廃止（分離）
//2020.9.18:外から別のプログラムで動かせるように、コマンドライン引数から入力フォルダパスを受け取れるようにする
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする

#include "./library.hpp"

vector<vector<int>> inputListData;
vector<vector<double>> outputListData;//TNデータを出力

//outputfolderName
const string OUTPUT_FOLDER_NAME = "5_TriangleNotation";

//inputdata列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_BRIGHT = 4;
const int COL_CHARGE = 5;

string inputDataName;

void calcTriangle(int, int, vector<int>, vector<int>);//三角形の形状を計算してoutputListDataにpushback
double calcMean(vector<int>);
double calcDist(int, int);

int main(int argc, char* argv[])
{
	cout << "*************************************************************************" << endl;
	cout << "**              C3+ イメージングリストデータ解析プログラム             **" << endl;
	cout << "**    C分解片3つの輝点位置の座標->3点のなす三角形のデータ(TN)に変換    **" << endl;
	cout << "**     データ形式：fileNum/frameNum/xc/yc/S/a/b/c/A/B/C/S2/S3/SR       **" << endl;
	cout << "**                             ver.2020.9.20 written by R. Murase      **" << endl;
	cout << "*************************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
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
	}		//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder);
	
	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}

	//dataname.txtを探す
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
		cout << "dataname.txtファイルを読み込みました\n";
		inputDataName = readDataName(datanamePath);
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}

	//outputFolderPath
	cout << "同じフォルダ内の" + OUTPUT_FOLDER_NAME + "に保存（なければ自動作成）)\n";
	string outputFolderPath = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	_mkdir(outputFolderPath.c_str());

	//ファイルを1つずつ処理
	for(auto inputfilename : fNames)
	{
		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputfilename;
		if (!checkFileExistence(inputDataPath))
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}
		//inputListDataを読み込み
		cout << inputfilename + "----データを読み込んでいます...";
		inputListData = readListFile<int>(inputDataPath, COL_FILE, start, end+1);
		cout << "読み込み完了 -> 処理を開始します...";

		//1行ずつ処理
		int k = 0;
		int fileNum = inputListData[0][COL_FILE];
		int frameNum = inputListData[0][COL_FRAME];
		while (k < inputListData.size())
		{
			//3点分の部分配列
			vector<int> x;
			vector<int> y;
			vector<int> q;
			fileNum = inputListData[k][COL_FILE];
			frameNum = inputListData[k][COL_FRAME];
			x.push_back(inputListData[k][COL_X]);
			y.push_back(inputListData[k][COL_Y]);
			q.push_back(inputListData[k][COL_CHARGE]);

			//file,frame番号が変わるところまで(3行)で部分配列を抜き出す
			while ((k + 1 < inputListData.size()) && (inputListData[k][COL_FILE] == inputListData[k + 1][COL_FILE]) 
					&& (inputListData[k][COL_FRAME] == inputListData[k + 1][COL_FRAME]))
			{
				x.push_back(inputListData[k + 1][2]);//col=2
				y.push_back(inputListData[k + 1][3]);//col=3
				q.push_back(inputListData[k + 1][5]);//col=5
				k++;
			}
			//データ変換処理
			calcTriangle(fileNum, frameNum, x, y);
			k++;
		}
		//出力ファイルの書き込み
		string outputFilePath = outputFolderPath + "\\" + inputfilename;
		cout << "処理終了 --> 書き込んでいます...";
		
		//start == 1の時は新規、それ以外はappend
		bool append = true;
		if (start == 1)append = false;
		writeListFile<double>(outputFilePath, outputListData, '\t', append);

		cout << "書き込みました" << endl;
		//出力ファイル用メモリの初期化
		inputListData.clear();
		outputListData.clear();
	}
	cout << "finished" << endl;
	return 0;
}

//三角形の形状を計算してoutputListDataにpush_back
void calcTriangle(int file, int frame, vector<int> X, vector<int> Y)
{
	if (X.size() != 3 || Y.size() != 3)
	{
		cerr << "calcTriangle error!" << endl;
		return;
	}
	//3点の重心を求める
	double xc = calcMean(X);
	double yc = calcMean(Y);
	
	//3点のなす三角形の面積を求める
	vector<int> X0 = { X[1] - X[0],X[2] - X[0],X[2] - X[1] };
	vector<int> Y0 = { Y[1] - Y[0],Y[2] - Y[0],Y[2] - Y[1] };
	double S = abs(X0[0] * Y0[1] - X0[1] * Y0[0]) / 2;
	
	//3点のなす三角形の各辺を求める（昇順にソート）
	vector<double> sides = { calcDist(X0[0],Y0[0]),calcDist(X0[1],Y0[1]) ,calcDist(X0[2],Y0[2]) };
	sort(sides.begin(), sides.end());
	
	//余弦定理で内角を求める（昇順にソート）
	double a = sides[0], b = sides[1], c = sides[2];
	vector<double> intAng = { acos((a*a + b*b-c*c)/(2*a*b)) *180/PI,acos((b*b + c * c - a * a) / (2 * b*c)) *180/PI 
		,acos((c*c + a * a - b * b) / (2 * c*a))*180/PI };
	sort(intAng.begin(), intAng.end());
	double A = intAng[0], B = intAng[1], C = intAng[2];

	//対称座標S2,S3を計算する
	vector<double> d(3);
	for (int i = 0; i < d.size(); i++)
	{
		d[i] = calcDist(X[i] - xc, Y[i] - yc);
	}
	double D01 = d[0] / d[1];
	double D12 = d[1] / d[2];
	double D20 = d[2] / d[0];
	
	double S2 = (D01 - D12) / sqrt(2);
	double S3 = (2 * D20 - D01 - D12) / sqrt(6);
	double SR = sqrt(S2*S2 + S3*S3);

	//三角形一つの情報をベクトルにまとめてoutputListData配列に追加
	vector<double> outputLine = { (double)file,(double)frame,xc,yc,S,a,b,c,A,B,C,S2,S3,SR };
	outputListData.push_back(outputLine);
	return;
}

double calcMean(vector<int> v)
{
	int size = v.size();
	double sum = 0;
	for (int i = 0; i < size; i++)
	{
		sum += v[i];
	}
	return sum / size;
}

double calcDist(int x, int y)
{
	double dist = sqrt(double(x*x + y*y));
	return dist;
}


