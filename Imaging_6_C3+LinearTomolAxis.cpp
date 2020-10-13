// Imaging_6_C3+LinearTomolAxis.cpp :TriangleAnalysisによってlinearと判断されたリストデータに対して、分子軸の計算を行う
//

//アップデートログ
//2020.7.28:ImagingC3+Linear_molAxis.cpp から作成
//2020.9.18:外から別のプログラムで動かせるように、コマンドライン引数から入力フォルダパスを受け取れるようにする

#include "./library.hpp"

double E0;  //一原子当たりの初期運動エネルギー(MeV)
double r0 = 1.27;	//初期炭素原子核間距離(Å) r0 : 

vector<vector<double>> inputListData;
vector<vector<double>> outputListData;
string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\7_Linear_molAxis";

//inputdata列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_BRIGHT = 4;
const int COL_CHARGE = 5;

void calcMolAxis();
bool readParameter(string);

int main(int argc, char* argv[])
{
	cout << "*****************************************************************************" << endl;
	cout << "****                          -分子軸計算プログラム-                     ****" << endl;
	cout << "****         C3+ List data からlinear structureと判断したイベントの      ****" << endl;
	cout << "****       分子軸とビーム軸のなす角を計算  その形式のリストデータに変換  ****" << endl;
	cout << "****  ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                 ver.2020.09.18 written by R. Murase ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダパスを入力してください\n--->";
		cin >> inputDataFolder;
	}
	//コマンドライン引数で読み取る
	else if(argc == 2)
	{
		inputDataFolder = argv[1];
		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}

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
	cout << " 同じフォルダ内の"+ OUTPUT_FOLDER_NAME +"に保存(なければ自動生成）" << endl;
	
	outputDataFolder = upperFolder(inputDataFolder) + OUTPUT_FOLDER_NAME;
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
	}
	else
	{
		cout << "既存のフォルダに出力ファイルを保存します。" << endl;
	}

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "param.txt";
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
		//param fileは読み飛ばし
		if (inputfilename.find("param") != string::npos)
			continue;
		//入力イメージングデータの読み込み
		string inputDataPath = inputDataFolder + "\\" + inputfilename;

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
		cout << "書き込みました" << endl;

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
		double fileNum = inputListData[3 * i][COL_FILE];
		double frameNum = inputListData[3 * i][COL_FRAME];
		vector<double> x;
		vector<double> y;
		vector<double> q;
		double q_prod = 1;

		//3点全ての組み合わせcylicに試すために3つめの要素に0つめの要素をコピー
		for (int j = 0; j < 4; j++)
		{
			int k = (j != 3) ? j : 0;
			x.push_back(inputListData[3 * i + k][COL_X]);
			y.push_back(inputListData[3 * i + k][COL_Y]);
			q.push_back(inputListData[3 * i + k][COL_CHARGE]);
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
			//距離が最も離れている二点＝両端の二点の時に、値の更新
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
		double p_c = sqrt(e2 / r0) * sqrt(q_p1 + q_p2 + q[ind_0]*q[ind_1]/2)*2;
		//初期運動量
		double p_0 = sqrt(2*E0 * 1e6);

		//クーロン力が0の時は数えない
		if (p_c == 0)
			continue;

		//検出器上の距離(pixel -> rad)に変換
		double divAngle = dist_ma * mmPerPx / L;
		//arcsinが取れない角度が来たときは分子軸を計算しない
		if ((p_0/p_c * divAngle) > 1) continue;
		//分子軸の角度を計算
		double molAxis = asin((p_0 / p_c * divAngle)) * 180. / PI;
		
		//3点の重心の計算
		double x_c = (x[0] + x[1] + x[2]) / 3.;
		double y_c = (y[0] + y[1] + y[2]) / 3.;

		vector<double> outputLine = {fileNum, frameNum, x_c, y_c, molAxis, q_prod, q_p1, q_p2};
		outputListData.push_back(outputLine);
	}
};

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

		//'='以降のパラメータをtmpに格納
		double tmp;
		ss >> tmp;

		if (name == "E0")
		{
			E0 = tmp;
		}
		if (name == "r0")
		{
			r0 = tmp;
		}
	}
	return isValid;
}
