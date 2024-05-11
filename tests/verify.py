from colorama import Fore, Back, Style
import subprocess, sys, os, shutil, tempfile, time


def create_temporary_copy(path):
    temp_dir = tempfile.gettempdir()
    temp_path = os.path.join(temp_dir, 'tmp_copy')
    shutil.copy2(path, temp_path)
    return temp_path


input_data = 'test_input_data'
picosat = 'picosat'
my_solver = '../cmake-build-debug/DPLL'


def create_tmp_cnf(my_model):
    target = open('tmp.cnf', 'w')
    src = open(cnf.path, 'r')
    line = src.readline()
    header = line.split(' ')
    header[3] = str(int(header[3].replace('\n', '')) + len(my_model)) + '\n'
    new_header = ' '.join(header)
    target.write(new_header)
    for i in my_model:
        target.write(i + ' 0\n')
    target.writelines(src.readlines())
    target.close()
    src.close()


for cnf in os.scandir(input_data):
    if ('Gilgamesh.cnf' not in cnf.path):
        start = time.time()
        my_sat_res = subprocess.run([my_solver, cnf.path], stdout=subprocess.PIPE)
        end = time.time()

        my_output = str(my_sat_res.stdout).split('\\n')
        print(my_output)
        my_unsatisfiable = 'UNSATISFIABLE' in my_output[0]
        pico_unsatisfiable = my_unsatisfiable
        my_model = list()
        pico_model = list()
        equal = bool

        if not my_unsatisfiable:
            my_model = my_output[1].split(' ')[1:-1]
            create_tmp_cnf(my_model)
            pico_res = subprocess.run([picosat, 'tmp.cnf'], stdout=subprocess.PIPE)
            os.remove('tmp.cnf')
            pico_output = str(pico_res).split('\\n')
            pico_unsatisfiable = 'UNSATISFIABLE' in pico_output[0]
            if not pico_unsatisfiable:
                pico_model = pico_output[1].split(' ')[1:-1]
                equal = pico_model == my_model
            else:
                equal = False
        else:
            pico_res = subprocess.run([picosat, cnf.path], stdout=subprocess.PIPE)
            pico_unsatisfiable = 'UNSATISFIABLE' in str(pico_res.stdout).split('\\n')[0]
            equal = my_unsatisfiable == pico_unsatisfiable

        if equal:
            print(f"{cnf.path} is {Fore.GREEN}ok{Style.RESET_ALL}")
        else:
            print(f"{cnf.path} is {Fore.RED}wrong{Style.RESET_ALL}")
            print(my_model)
            print(pico_model)
            print('\n')
            # os.exit(-1)
        print(' elapsed time: {:.2f}'.format(end - start))
