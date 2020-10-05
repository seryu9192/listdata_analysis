//Imaging_search_event.cpp: 目的のイメージングイベントを探すための使い捨てプログラム

#include "./library.hpp"

int main()
{
    cout << "************************************************" << endl;
	cout << "**          Imaging_search_event              **" << endl;
	cout << "**        ver.2020.09.25 written by R. Murase **" << endl;
	cout << "************************************************" << endl << endl;
    
    string inputDataPath = "E:\\Cluster_sputtering\\MT2020\\MT2020#4\\900keVC4_pos\\imaging\\7_Linear_molAxis\\20200919_q=1122.txt";
    auto inputListData = readListFile<double> (inputDataPath);
    for (int i = 0; i < inputListData.size(); i++)
    {
        double t =  inputListData[i][4];
        if(t < 10)
        {
            cout << setprecision(10) << inputListData[i][0] << " " << inputListData[i][1] << " " << inputListData[i][4] << endl;
        }
    }
    return 0;
}