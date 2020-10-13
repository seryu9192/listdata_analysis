# automate_exe_C3.py: 一連のデータ変換を自動で行うスクリプト for 3量体(Imaging, TOF)

import os, subprocess, re

dataname = 'yyyymmdd'
invalid_list = []

FOLDER_IMG = 'imaging'
FOLDER_TOF = 'tof'

#入出力フォルダ：start, endファイル番号の自動検出に使うため（実際の出力フォルダの指定とフォルダ作成はC++側のプログラムで行われる）
img_inp_folders = ['1_COP', '2_charge', '3_C3', '4_Combseparated', \
                   '5_TriangleNotation', '4_Combseparated', '6_Linear']

img_out_folders = ['2_charge', '3_C3', '4_Combseparated', '5_TriangleNotation',\
                   '5_TriangleNotation\\5-1_Linear', '6_Linear', '7_Linear_molAxis']

img_exe_path = ['./Imaging_1_COPToCharge.exe', './Imaging_2_chargeToCn.exe', './Imaging_3_CnToCombseparated.exe', './Imaging_4_C3+CombsepToTriangleNotation.exe', \
                './Imaging_5_C3+TriangleNotationToTriLin.exe', './Imaging_5-1_C3+CombsepToTriLin.exe', './Imaging_6_C3+LinearTomolAxis.exe']

tof_folder_names = ['1_decoded', '2_offsetScale', '3_stop', '4_coinc']
tof_exe_path = ['./MCS6A_1_decodedToOffsetScale.exe', './MCS6A_2_offsetScaleToStop.exe', './MCS6A_3_stopToCoinc.exe']

combine_exe_path = './Both_combineListData.exe'

def read_invalid(ival_path):
    ival_list = []
    #invalid.txtの読み込み解析
    re_data = re.compile(r'invalid = (.*)')
    try:
        with open(ival_path) as f:
            s = f.read()
            mo = re_data.search(s)
            if mo != None:
                ival_list = list(map(int, mo.groups()[0].split(',')))
        print('invalid list :', ival_list)
    except FileNotFoundError:
        print('invalidファイルは見つかりませんでした')
    return ival_list

def read_dataname(dname_path):
    dname = ''
    #dataname.txtの読み込み解析
    re_data = re.compile(r'dataname = (\d{8})')
    with open(dname_path) as f:
        s = f.read()
        mo = re_data.search(s)
        dname = mo.groups()[0]
    return dname

def main():
    while True:
        print('**** Automate_exe_C3  ****')
        #working directory
        working_directory = input('Input the working directory(press ctrl+c to quit)\n --> ')

        dataname_path = os.path.join(working_directory, 'dataname.txt')
        invalid_path = os.path.join(working_directory, 'invalid.txt')
        #dataname.txtの読み込み解析
        dataname = read_dataname(dataname_path)

        #invalid.txtの読み込み解析
        invalid_list = read_invalid(invalid_path)

        print('dataname : {}'.format(dataname))

        #imaging program
        #フォルダごとに異なるプログラムを使う
        for i in range(len(img_exe_path)):
            inputfolder = os.path.join(working_directory, FOLDER_IMG, img_inp_folders[i])
            outputfolder = os.path.join(working_directory, FOLDER_IMG, img_out_folders[i])
            files_in_input = os.listdir(inputfolder)

            #連番データが更新されているかを確認する -> [start, end)区間について処理
            #startを決める:startの決め方は、連番か結合かで分かれる
            start = 1
            try:
                files_in_output = os.listdir(outputfolder)
                #exe1-2:出力が連番ファイルなので、ファイル番号の更新分だけ探す
                if i < 2:
                    while dataname + '{0:03d}.txt'.format(start) in files_in_output or start in invalid_list:
                        start += 1
                #exe3-:出力が結合ファイルなので、出力ファイルの最後の行の0列目からファイル番号を読み取って、比較
                else:
                    try:
                        with open(os.path.join(outputfolder , files_in_output[1])) as f:
                            lines = f.read().split('\n')
                            if lines[-1] == '':
                                del lines[-1]
                            #最後の行の0列目、ファイル番号を読み取る
                            line = lines[-1].split()
                            start = int(line[0]) + 1
                    except IndexError: #出力ファイルにデータがない場合は1からスタート
                        pass
                    
            except FileNotFoundError: #まだ出力ファイルがない場合は1からスタート
                pass

            #endを決める:
            end = 1
            #入力が連番ファイルなので、入力ファイルの最後のfile番号+1をendにする
            if i < 3:
                while dataname + '{0:03d}.txt'.format(end) in files_in_input or end in invalid_list:
                    end += 1
            #入力が結合ファイルなので、入力ファイルの最後行から読み取ったfile番号+1をendにする
            else:
                files_in_input = os.listdir(inputfolder)
                with open(os.path.join(inputfolder,files_in_input[1])) as f:
                    lines = f.read().split('\n')
                    if lines[-1] == '':
                        del lines[-1]
                    #入力ファイルの0列目、ファイル番号を読み取る
                    last_line = lines[-1].split()
                    end = int(last_line[0]) + 1

            #読み取ったstart, endを表示
            print('Imaging_exe #', i+1, end=' --> ')
            print('start:', start, end=' ')
            print('end:', end,)

            #連番データが更新されていたらプログラムを実行
            if start != end:
                print('{} : ファイル#{} - {}が更新されました'.format(inputfolder, start, end - 1))
                exe_path = img_exe_path[i]
                print('{}を開きます'.format(exe_path))
                if i < 3:
                    code = subprocess.run([exe_path, inputfolder, str(start), str(end-1)]) 
                    if code.returncode == 0:
                        print(exe_path + ': 正常に終了しました')
                    else :
                        print(exe_path + ': エラー停止しました')
                else:
                    code = subprocess.run([exe_path, inputfolder]) 
                    if code.returncode == 0:
                        print(exe_path + ': 正常に終了しました')
                    else :
                        print(exe_path + ': エラー停止しました')

        #TOFプログラム
        for i in range(len(tof_exe_path)):

            inputfolder = os.path.join(working_directory, FOLDER_TOF ,tof_folder_names[i])
            outputfolder = os.path.join(working_directory, FOLDER_TOF ,tof_folder_names[i+1])

            #連番データが更新されているかを確認する -> [start, end)区間について処理
            #startを決める:
            start = 1
            try:
                files_in_output = os.listdir(outputfolder)
                #連番ファイルなので、ファイル番号の更新分だけ探す(invalid_listは抜かす)
                while dataname + '{0:03d}.txt'.format(start) in files_in_output or start in invalid_list:
                    start += 1
            except FileNotFoundError: #まだ出力ファイルがない場合は1からスタート
                pass

            #endを決める:
            end = 1
            #入力が連番ファイルなので、入力ファイルの最後のfile番号+1をendにする
            files_in_input = os.listdir(inputfolder)
            while dataname + '{0:03d}.txt'.format(end) in files_in_input or end in invalid_list:
                end += 1
            
            #読み取ったstart, endを表示
            print('TOF_exe #', i+1, end=' --> ')
            print('start:', start, end=' ')
            print('end:', end,)
            if start != end:
                print('{} : ファイル#{} - #{}が更新されました'.format(inputfolder, start, end - 1))
                exe_path = tof_exe_path[i]
                print('{}を開きます'.format(exe_path))

                #MCS6A_3_stopToCoinc.exe以外は、引数にinputfolder, start, endを渡せばいい
                if i < 2:
                    code = subprocess.run([exe_path, inputfolder, str(start), str(end-1)]) 
                else:
                    #MCS6A_3_stopToCoinc.exeではイメージングフォルダ名を引数から読むので、引数に'3_C3'を指定
                    code = subprocess.run([exe_path, inputfolder, str(start), str(end-1), '3_C3']) 

                if code.returncode == 0:
                    print(exe_path + ': 正常に終了しました')
                else :
                    print(exe_path + ': エラー停止しました')

        #tof dataをcombine(Both_combineListData.exeを呼び出し)
        inputfolder = os.path.join(working_directory, FOLDER_TOF ,tof_folder_names[-1])
        outputfolder = inputfolder

        #連番データが更新されているかを確認する -> [start, end)区間について処理
        #startを決める:
        start = 1
        try:
            combined_path = os.path.join(inputfolder, dataname + '.txt')
            with open(combined_path) as f:
                lines = f.read().split('\n')
                if lines[-1] == '':
                    del lines[-1]
                #出力ファイルの0列目、ファイル番号を読み取る
                last_line = lines[-1].split()
                start = int(last_line[0]) + 1
        except FileNotFoundError: #まだ出力ファイルがない場合は1からスタート
            pass

        #endを決める:
        end = 1
        #入力が連番ファイルなので、入力ファイルの最後のfile番号+1をendにする
        files_in_input = os.listdir(inputfolder)
        while dataname + '{0:03d}.txt'.format(end) in files_in_input or end in invalid_list:
            end += 1
        
        #読み取ったstart, endを表示
        print('TOF_exe #', 4, end=' --> ')
        print('start:', start, end=' ')
        print('end:', end)
        if start != end:
            print('{} : ファイル#{} - #{}が更新されました'.format(inputfolder, start, end - 1))
            exe_path = combine_exe_path
            print('{}を開きます'.format(exe_path))
            code = subprocess.run([exe_path, inputfolder, str(start), str(end-1)]) 
            if code.returncode == 0:
                print(exe_path + ': 正常に終了しました')
            else :
                print(exe_path + ': エラー停止しました')


        #TODO : coinc program


if __name__ == '__main__':
    main()
