//Imaging_search_event.cpp: 目的のイメージングイベントを探すための使い捨てプログラム

#include "./library.hpp"

const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_MOLAXIS = 4;

const int X_C = 270;
const int Y_C = 1020;
const int R = 10;

int main()
{
    cout << "************************************************" << endl;
	cout << "**          Imaging_search_event              **" << endl;
	cout << "**        ver.2020.09.25 written by R. Murase **" << endl;
	cout << "************************************************" << endl << endl;
    
    string inputDataPath = "E:\\Cluster_sputtering\\MT2020\\MT2020#4\\900keVC2_pos\\imaging\\6_molAxis\\20200916_q=12_r0=1.27.txt";
    auto inputListData = readListFile<double> (inputDataPath);
    for (int i = 0; i < inputListData.size(); i++)
    {
        double x = inputListData[i][COL_X];
        double y = inputListData[i][COL_Y];
        double t = inputListData[i][COL_MOLAXIS];
        if(t > 87 && (x-X_C)*(x-X_C) + (y-Y_C)*(y-Y_C) < R*R)
        {
            cout << setprecision(10) << inputListData[i][COL_FILE] << " " << inputListData[i][COL_FRAME] << " " << inputListData[i][COL_MOLAXIS] << endl;
        }
    }
    return 0;
}