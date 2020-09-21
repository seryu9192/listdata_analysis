// C3+TriangleAnalysis.cpp
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

#include "./library.hpp"

vector<vector<int>> inputListData;
vector<vector<double>> outputListData;//S2-S3座標系の
vector<vector<int>> sepaListData;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "5_TriangleNotation";

//const vector<int> qProd = { 1,2,3,4,6,8,9,12,18,27 };
string inputDataName;
vector<double> SRmin;
vector<double> SRmax;
int x_c, y_c, R;

bool calcTriangle(int, int, vector<int>, vector<int>, double, double);//三角形の形状を計算してoutputListDataにpushback
double calcMean(vector<int>);
int calcProd(vector<int> v);
double calcDist(int, int);
bool readParameter(string);

int main()
{
	cout << "*************************************************************************" << endl;
	cout << "**              C3+ イメージングリストデータ解析プログラム             **" << endl;
	cout << "**    C分解片3つの輝点位置の座標->3点のなす三角形のデータ(TON)に変換   **" << endl;
	cout << "**     データ形式：fileNum/frameNum/xc/yc/S/a/b/c/A/B/C/S2/S3/SR       **" << endl;
	cout << "**                            ver.2020.7.27 written by R. Murase      **" << endl;
	cout << "*************************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

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


	//shapeファイルのパス
	cout << "同じフォルダ内の" + OUTPUT_FOLDER_NAME + "に保存（なければ自動作成）)\n";
	string outputFolderPath = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	_mkdir(outputFolderPath.c_str());

	//入力データパスの生成
	inputDataPath = inputDataFolder + "\\" + inputDataName + ".txt";
	ifstream ifs(inputDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//inputListDataを読み込み
	string tmp;
	while (getline(ifs, tmp))
	{
		stringstream ss(tmp);
		string tmp2;
		vector<int> inputLine;
		while (ss >> tmp2)
		{
			inputLine.push_back(stod(tmp2));
		}
		inputListData.push_back(inputLine);
	}
	cout << inputDataName + ".txt" + "----データの読み込み完了" << endl;

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "param.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//separated用のフォルダをまず作成
	string separatedDir=inputDataFolder+"\\separated";
	_mkdir(separatedDir.c_str());

	//SR=(S2^2+S3^2)^1/2の値によって分けてファイルに保存する
	for (int i = 0; i < SRmin.size(); i++)
	{
		//今のSRの範囲
		double SRmin_now = SRmin[i];
		double SRmax_now = SRmax[i];

		//フォルダの名前用suffix(sr_str)
		ostringstream ss_SRmin, ss_SRmax;
		ss_SRmin << fixed << setprecision(2) << SRmin_now;
		ss_SRmax << fixed << setprecision(2) << SRmax_now;
		string sr_str = "SR=" + ss_SRmin.str() + "-" + ss_SRmax.str();

		//SR毎にフォルダの作成
		string outDirName = outputDataFolder + "\\" + sr_str;
		string sepaDirName = inputDataFolder + "\\separated\\" + sr_str;
		if (_mkdir(outDirName.c_str()) == 0)
		{
			cout << sr_str + "_出力フォルダを作成しました。" << endl;
		}
		else
		{
			cout << sr_str + "_現在ある出力フォルダに書き込みます。" << endl;
		}
		if (_mkdir(sepaDirName.c_str()) == 0)
		{
			cout << sr_str + "_分割用フォルダを作成しました。" << endl;
		}
		else
		{
			cout << sr_str + "_現在ある分割用フォルダに書き込みます。" << endl;
		}

		//charge stateの組み合わせ毎に処理(ループの最後にtotalを処理)
		for (int j = 0; j < qProd.size() + 1; j++)
		{
			//着目する価数ペアを決定
			int q_now = 0;
			if (j < qProd.size()) q_now = qProd[j];

			int k = 0;
			int fileNum = inputListData[0][0];
			int frameNum = inputListData[0][1];
			while (k < inputListData.size())
			{
				//3点分の部分配列
				vector<int> x;//col=2
				vector<int> y;//col=3
				vector<int> q;//col=5
				vector<vector<int>> parSec;
				fileNum = inputListData[k][0];
				frameNum = inputListData[k][1];
				x.push_back(inputListData[k][2]);//col=2
				y.push_back(inputListData[k][3]);//col=3
				q.push_back(inputListData[k][5]);//col=5
				parSec.push_back(inputListData[k]);

				//file,frame番号が変わるところまで(3行)で部分配列を抜き出す
				while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k + 1][0]) && (inputListData[k][1] == inputListData[k + 1][1]))
				{
					x.push_back(inputListData[k + 1][2]);//col=2
					y.push_back(inputListData[k + 1][3]);//col=3
					q.push_back(inputListData[k + 1][5]);//col=5
					parSec.push_back(inputListData[k + 1]);
					k++;
				}

				//もし今のイベントのSRが範囲に入っていれば
				if ((j == qProd.size() || calcProd(q) == q_now) && calcTriangle(fileNum, frameNum, x, y, SRmin_now, SRmax_now))
				{
					//部分配列をsepaListDataに入れる
					for (int n = 0; n < parSec.size(); n++)
					{
						sepaListData.push_back(parSec[n]);
					}
				}
				k++;
			}

			//suffixの定義
			string suf;
			if (j < qProd.size())
			{
				suf = "qprod=" + to_string(q_now) +"_"+ sr_str;
			}
			else
			{
				suf = "total_" + sr_str;
			}

			//出力ファイルの書き込み
			string outputDataPath = outDirName + "\\" + inputDataName + "_" + suf + ".txt";
			ofstream ofs(outputDataPath);
			ofs << setprecision(7);
			if (!ofs)
			{
				cerr << "ファイルを保存できませんでした" << endl;
				system("pause");
				return -1;
			}
			for (int r = 0; r < outputListData.size(); r++)
			{
				for (int c = 0; c < outputListData[0].size(); c++)
				{
					ofs << outputListData[r][c];
					if (c < outputListData[0].size() - 1) ofs << "\t";
				}
				ofs << endl;
			}
			cout << outputDataPath << "に書き込みました" << endl;
			//出力ファイル用メモリの初期化
			outputListData.clear();

			//分割ファイルの書き込み
			string sepaDataPath = sepaDirName + "\\" + inputDataName + "_" + suf + ".txt";
			ofstream ofs2(sepaDataPath);
			ofs2 << setprecision(7);
			if (!ofs2)
			{
				cerr << "ファイルを保存できませんでした" << endl;
				system("pause");
				return -1;
			}
			for (int i = 0; i < sepaListData.size(); i++)
			{
				for (int j = 0; j < sepaListData[0].size(); j++)
				{
					ofs2 << sepaListData[i][j];
					if (j < sepaListData[0].size() - 1) ofs2 << "\t";
				}
				ofs2 << endl;
			}
			cout << sepaDataPath << "に書き込みました" << endl;
			//分割ファイル用メモリの初期化
			sepaListData.clear();
		}
	}
	return 0;
}

//三角形の形状を計算してoutputListDataにpush_back
bool calcTriangle(int file, int frame, vector<int> X, vector<int> Y,double SR_min, double SR_max)
{
	bool save = false;

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
	double SR = sqrt(S2*S2 + S3 * S3);

	//もし今のイベントのsRが範囲に入っていれば
	if (SR_min<=SR && SR<=SR_max)
	{
		//三角形一つの情報をベクトルにまとめてoutputListData配列に追加
		vector<double> outputLine = { (double)file,(double)frame,xc,yc,S,a,b,c,A,B,C,S2,S3,SR };
		outputListData.push_back(outputLine);
		save = true;
	}
	return save;
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
	double dist = sqrt(x*x + y*y);
	return dist;
}

int calcProd(vector<int> v)
{
	int prod = 1;
	for (int i = 0; i < v.size(); i++)
	{
		prod *= v[i];
	}
	return prod;
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

		if (name == "SRmin")
		{
			double temp;
			while (ss >> temp)
			{
				SRmin.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "SRmax")
		{
			double temp;
			while (ss >> temp)
			{
				SRmax.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "CM")
		{
			int temp;
			vector<int> CM;
			while (ss >> temp)
			{
				CM.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
			x_c = CM[0];
			y_c = CM[1];
			R = CM[2];
		}
		else
		{
			isValid = false;
		}
	}
	//sRminとsRmaxの数が合わないときはパラメータファイルが不正
	if (SRmin.size() != SRmax.size())
	{
		isValid = false;
	}
	return isValid;
}