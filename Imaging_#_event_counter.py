# Imaging_#_event_counter.py: Imagingリストデータのイベント数を集計する
# 具体的には、linear structureの時の"7_Orientseparated"フォルダ内のイベントを配向毎に数えて足し合わせる

import os

def main():
    print('**** Both_#_event_counter.py  ****')
    #working directory(wd)
    working_directory = input('Input the working directory\n --> ')
    outputdata = ''
    para_tot = 0
    perp_tot = 0
    mid_tot = 0

    #loop for filename in the wd
    for filename in os.listdir(working_directory):
        #avoid to open directory or event.txt (which you are going to make) file
        if '.txt' not in filename or 'event_counts' in filename:
            continue
        with open(os.path.join(working_directory, filename), 'r') as f:
            inputdat = f.read()
        
        #get line number by counting the number of '\n'
        event_counts = inputdat.count('\n')
        #compose output data
        outputdata += filename + '\t' + str(event_counts) + '\n'
        if '0.0-30.0' in filename:
            para_tot += event_counts
        elif '60.0-90.0' in filename:
            perp_tot += event_counts
        elif '30.0-60.0' in filename:
            mid_tot += event_counts
    #add to output text
    outputdata += 'para_tot\t' + str(para_tot) + '\n'
    outputdata += 'perp_tot\t' + str(perp_tot) + '\n'
    outputdata += 'mid_tot\t' + str(mid_tot) + '\n'
    outputdata += 'tot\t' + str(para_tot + perp_tot + mid_tot) + '\n'

    #write to the outputfile
    data_cond = working_directory.split('\\')[4]
    print(data_cond)
    outputpath = os.path.join(working_directory, data_cond + '_event_counts.txt')
    with open(outputpath, 'w') as f:
        f.write(outputdata)
    print(f'done --- outputfile: {outputpath}')

if __name__ == '__main__':
    main()