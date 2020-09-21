#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;
typedef unsigned int ui;
typedef unsigned long long ull;

vector<vector<int>> inputListData;
vector<vector<int>> intermediateListData;//BG�����������X�g���ꎞ�I�ɓ���
vector<vector<int>> outputListData;

string inputDataName;

void elimBG();
void extractOne();
string upperPath(string);
bool readFilename(string);


//ImagingC+rejectPileupAnalysis
//�o�b�N�O���E���h(q=-1)������
void elimBG()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui frameNum = inputListData[i][0];
		int x = inputListData[i][1];
		int y = inputListData[i][2];
		ui brightness = inputListData[i][3];
		int q = inputListData[i][4];

		if (q != -1)
		{
			//intermediateListData1�s(intermediateLine)���\��
			vector<int> intermediateLine;
			intermediateLine.push_back(frameNum);
			intermediateLine.push_back(x);
			intermediateLine.push_back(y);
			intermediateLine.push_back(brightness);
			intermediateLine.push_back(q);

			//intermediateListData�ɒǉ�
			intermediateListData.push_back(intermediateLine);
		}
	}
}

//�P�_��1�̂��݂̂̂�outputListData�ɕۑ�
void extractOne()
{
	int frameNum = intermediateListData[0][0];//�����Ă���t���[���̃C���f�b�N�X
	int k = 0;//�����Ă���s�̃C���f�b�N�X

	//���X�g�f�[�^�̍Ō�܂�1�����Ă���
	while (k < intermediateListData.size())
	{
		vector<vector<int>> tmp;// 1�t���[�����̕����z��
		frameNum = intermediateListData[k][0];
		intermediateListData[k].push_back(0);//�񐔒����̂���0��}��
		tmp.push_back(intermediateListData[k]);

		//frame�ԍ����ς��Ƃ���܂łŕ����z��𔲂��o��(tmp)
		while ((k + 1 < intermediateListData.size()) && (intermediateListData[k][0] == intermediateListData[k + 1][0]))
		{
			tmp.push_back(intermediateListData[k + 1]);
			k++;
		}

		//�P�_�̐���1�̃t���[���ɂ��Ă̂݉�͂��s��
		int pointNum = tmp.size();
		if (pointNum == 1)
		{
			//outputListData��push_back
			outputListData.push_back(tmp[0]);
		}
		k++;
	}
}

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string upperPath = fullPath.substr(0, path_i);//�Ō��'\'�͊܂܂Ȃ�
	return upperPath;
}

bool readFilename(string fnamePath)
{
	//�p�����[�^���L�����ǂ����̃t���O
	bool isValid = false;

	string filepath = fnamePath;
	ifstream ifs(filepath);

	//�p�����[�^�t�@�C����1�s���ǂݎ��
	string line;
	while (getline(ifs, line))
	{
		//0�����ڂ�'#'�̍s�͓ǂݔ�΂�
		if (line[0] == '#')
			continue;
		//'='���Ȃ��s���ǂݔ�΂�
		if (line.find('=') == string::npos)
			continue;

		//stringstream�C���X�^���X�ɓǂݎ���������������
		stringstream ss(line);

		//�ǂݎ����������̂����A�X�y�[�X�̑O�܂ł̕�����name������ɓ����
		string name;
		ss >> name;

		//'='�ɏo��܂ł̕�����𖳎�
		ss.ignore(line.size(), '=');

		//'='�ȍ~�̃p�����[�^��temp�Ɋi�[
		string temp;
		ss >> temp;

		if (name == "filename")
		{
			isValid = true;
			inputDataName = temp;
		}
	}
	return isValid;
}