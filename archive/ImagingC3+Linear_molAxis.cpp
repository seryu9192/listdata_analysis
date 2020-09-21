// ImagingC3+Linear_molAxis.cpp :TriangleAnalysisによってlinearと判断されたリストデータに対して、分子軸の計算を行う
//

//アップデートログ
//2019.10.23:作成
//2020.7.9:VS Codeに移植
//2020.7.28:library.hppを導入

#include "./library.hpp"

constexpr auto m = 12;     //炭素の質量数(amu)
constexpr auto e2 = 14.4;//素電荷の二乗(eV・Å)
constexpr auto PI = 3.1415; //円周率
constexpr auto L = 1025.;//Distance between detector-target(mm)
constexpr auto mmPerPx = 0.08;//pixel -> mm
double E0;  //一原子当たりの初期運動エネルギー(MeV)
double r0;	//初期炭素原子核間距離(Å) r0 : 


vector<vector<double>> inputListData;
vector<vector<double>> outputListData;
string inputDataName;

void calcMolAxis();
double calcDist(pair<double, double>, pair<double, double>);
bool readParameter(string);

int main()
{
	cout << "*****************************************************************************" << endl;
	cout << "****                          -分子軸計算プログラム-                     ****" << endl;
	cout << "****         C3+ List data からlinear structureと判断したイベントの      ****" << endl;
	cout << "****       分子軸とビーム軸のなす角を計算  その形式のリストデータに変換  ****" << endl;
	cout << "**** ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                  ver.2020.7.28 written by R. Murase ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//dataname.txtを読み込み
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

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder);

	//出力データパスの生成
	string outputDataFolder, outputFileName;
	cout << "出力データのフォルダ名を入力してください (\"@\"入力で同じフォルダ内の\"linear_molAxis\"に保存)\n--->";
	outputDataFolder = upperFolder(inputDataFolder) + "\\linear_molAxis";

	//入力パラメータパスの生成
	string parameterFilePath = outputDataFolder + "\\" + inputDataName + "param.txt";
	if (!checkFileExistence(parameterFilePath))
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

	//フォルダ内のすべてのファイルに対して処理
	for(auto inputfilename : fNames)
	{
		//入力イメージングデータの読み込み
		string inputDataPath = inputDataFolder + "\\" + inputfilename;
		if (!checkFileExistence(inputDataPath))
			continue;
		//inputDataの読み込み
		cout << inputDataPath + "----データを読み込んでいます...";
		inputListData = readListFile<double>(inputDataPath);
		cout << "----読み込み完了 --> 処理を開始します...";

		//変換処理開始
		calcMolAxis();
		cout << "処理終了 --> ファイルに書き込んでいます...";

		//出力ファイルにoutputListDataを書き込み
		string outputFilePath = outputDataFolder + "\\" + inputfilename;
		writeListFile<double>(outputFilePath, outputListData);
		cout << "に書き込みました" << endl;

		//データクリア
		inputListData.clear();
		outputListData.clear();
	}

	//フォルダ内処理終了
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void calcMolAxis()
{
	//inputListDataをはじめから3行(1イベント)ずつ見ていく
	for (int i = 0; 3 * i < inputListData.size(); i++)
	{
		double fileNum = inputListData[3 * i][0];
		double frameNum = inputListData[3 * i][1];
		vector<double> x;
		vector<double> y;
		vector<double> q;
		double q_prod = 1;

		//3点全ての組み合わせを試すために3つめの要素に0つめの要素をコピー
		for (int j = 0; j < 4; j++)
		{
			int k = (j != 3) ? j : 0;
			x.push_back(inputListData[3 * i + k][2]);
			y.push_back(inputListData[3 * i + k][3]);
			q.push_back(inputListData[3 * i + k][5]);
			if (j < 3)q_prod *= q[j];
		}
		//両端の輝点間の距離と輝点のインデックス
		double dist_ma = 0;
		double ind_0, ind_1;
		for (int j = 0; j < 3; j++)
		{
			auto v_0 = make_pair(x[j], y[j]);
			auto v_1 = make_pair(x[j + 1], y[j + 1]);
			auto d = calcDist(v_0, v_1);
			//距離が最大の2点＝両端の2点を見つけたら、値を更新
			if (dist_ma < d)
			{
				dist_ma = d;
				ind_0 = j;
				ind_1 = (j + 1 != 3) ? j + 1 : 0;
			}
		}
		//両端の点の電荷で全ての積を割る
		double q_p1 = q_prod / q[ind_0];
		double q_p2 = q_prod / q[ind_1];
		//クーロン力により得られる運動量の計算
		double p_c = sqrt(e2 / r0) * (sqrt(q_p1) + sqrt(q_p2));
		//初期運動量
		double p_0 = sqrt(E0 * 1e6);

		cout << "p_0/p_c=" << p_0 / p_c << endl;
		
		//検出器上の距離(pixel -> rad)に変換
		double divAngle = dist_ma * mmPerPx / L;
		//arcsinが取れない角度が来たときは分子軸を計算しない
		if ((p_0/p_c * divAngle) > 1) continue;
		//分子軸の角度を計算
		double molAxis = asin((p_0 / p_c * divAngle)) * 180. / PI;
		
		//3点の重心の計算
		double x_c = (x[0] + x[1] + x[2]) / 3.;
		double y_c = (y[0] + y[1] + y[2]) / 3.;

		vector<double> outputLine;
		outputLine.push_back(fileNum);
		outputLine.push_back(frameNum);
		outputLine.push_back(x_c);
		outputLine.push_back(y_c);
		outputLine.push_back(molAxis);
		outputLine.push_back(q_prod);
		outputLine.push_back(q_p1);
		outputLine.push_back(q_p2);

		outputListData.push_back(outputLine);
	}
};

double calcDist(pair<double,double> p, pair<double, double> q)
{
	double x_0 = p.first;
	double y_0 = p.second;
	double x_1 = q.first;
	double y_1 = q.second;
	double r = sqrt((x_0 - x_1) * (x_0 - x_1) + (y_0 - y_1) * (y_0 - y_1));
	return r;
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
		double temp;
		ss >> temp;

		if (name == "E0")
		{
			E0 = temp;
		}
;		if (name == "r0")
		{
			r0 = temp;
		}
	}
	return isValid;
}
