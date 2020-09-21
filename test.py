dat = [["Hanako", 19, "F"],
       ["Taro", 4, "M"],
       ["Pochi", 21, "M"],
       ["Tama", 6, "F"],
       ["John", 12, "M"],
       ["Petra", 23, "F"]]

def add(dat):
    dat[1] += 1
    return dat

def main():
    dat_male = list(filter(lambda a : a[2] == "M", dat))
    dat_kid = list(filter(lambda a : a[1] <= 20, dat))
    print(dat_male)
    print(dat_kid)
    dat_one_year_later = list(map(add, dat))
    print(dat_one_year_later)

