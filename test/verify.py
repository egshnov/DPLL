import os
import subprocess
import time
import sys

from colorama import Fore, Style

input_data = 'test_input_data'  # 're# s'
# input_data = 'res'
# input_data = 'another'
picosat = 'picosat'
my_solver = '../build/DPLL'
kksat = './kksat'


def create_tmp_cnf(path, model):
    target = open('tmp.cnf', 'w')
    src = open(path, 'r')
    # обновляем header файла с кнф
    line = src.readline()
    header = line.split(' ')
    header[3] = str(int(header[3].replace('\n', '')) + len(model)) + '\n'
    new_header = ' '.join(header)
    target.write(new_header)

    # записываем новые дизъюнкты
    for i in model:
        target.write(i + ' 0\n')

    # записываем все отсальное
    target.writelines(src.readlines())
    target.close()
    src.close()


for cnf in os.scandir(input_data):
    #if "random.cnf" in cnf.path:
    if 'Gilgamesh.cnf' not in cnf.path:
        # запускаем наш солвер
        start = time.time()
        my_sat_res = subprocess.run([my_solver, cnf.path], stdout=subprocess.PIPE)
        end = time.time()
        my_output = str(my_sat_res.stdout).split('\\n')

        # создаем файл с копией кнф
        my_model = my_output[1].split(' ')[1:-1]
        create_tmp_cnf(path=cnf.path, model=my_model)

        # запускаем picosat на копии
        pico_res = subprocess.run([picosat, 'tmp.cnf'], stdout=subprocess.PIPE)
        # os.remove('tmp.cnf')

        my_output = " ".join(my_output).replace('v', '').replace('  ', ' ')
        pico_output = " ".join(str(pico_res.stdout).replace('v', '').split('\\n')).replace("  ", " ")

        sat = "UNSATISFIABLE" not in pico_output[0]
        if my_output == pico_output:
            print(f"{cnf.path.split('/')[-1]} is {Fore.GREEN}ok{Style.RESET_ALL}", "sat" if sat else "unsat",
                  'elapsed time: {:.2f}'.format(end - start))

        else:
            print(f"{cnf.path.split('/')[-1]} is {Fore.RED}wrong{Style.RESET_ALL}",
                  'elapsed time: {:.2f}'.format(end - start))
            print(pico_output)
            print()
            print(my_output)
            sys.exit(-1)
