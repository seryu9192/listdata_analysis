# Imaging_#_event_counter_compose_C2_charge_pairs_table.py: Imagingリストデータのイベント数を集計する
# 具体的には、linear structureの時の"7_Orientseparated"フォルダ内のイベントを配向毎に数えて足し合わせる
# さらに、配向ごとの電荷分布をグラフ化する際に便利なテーブル形式にして出力する

import os

def main():
    print('**** Both_#_event_counter.py  ****')
    #working directory(wd)
    working_directory = input('Input the working directory\n --> ')
    outputdata = ''
    para_tot = 0
    perp_tot = 0
    mid_tot = 0

    # keep count for random (sum for each orientation)
    random_cnt = {}

    #loop for filename in the wd
    for filename in os.listdir(working_directory):
        #avoid to open directory or event.txt (which you are going to make) file
        if '.txt' not in filename or 'event_counts' in filename or 'charge_pairs_table' in filename:
            continue
        with open(os.path.join(working_directory, filename), 'r') as f:
            inputdat = f.read()
        
        q = filename[11:13]
        random_cnt.setdefault(q, 0)
        #get line number by counting the number of '\n'
        event_counts = inputdat.count('\n')
        #compose output data
        if '0.0-30.0' in filename:
            para_tot += event_counts
            tmin = '0'
        elif '60.0-90.0' in filename:
            perp_tot += event_counts
            tmin = '60'
        elif '30.0-60.0' in filename:
            mid_tot += event_counts
            tmin = '30'
        outputdata += f'2\t1800\t{q}\t{tmin}\t{event_counts}\n'
        random_cnt[q] += event_counts

    for k, v in random_cnt.items():
        outputdata += f'2\t1800\t{k}\t-1\t{v}\n'


    #write to the outputfile
    data_cond = working_directory.split('\\')[4]
    print(data_cond)
    outputpath = os.path.join(working_directory, data_cond + '_charge_pairs_table.txt')
    with open(outputpath, 'w') as f:
        f.write(outputdata)
    print(f'done --- outputfile: {outputpath}')

if __name__ == '__main__':
    main()

