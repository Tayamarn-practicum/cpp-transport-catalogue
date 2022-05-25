import difflib
import subprocess


def main():
    s = subprocess.run('clang++ *.cpp -o main.exe  -std=c++2a -Wfatal-errors')
    print('Compilation STDOUT:')
    print(s.stdout)
    print('------------')
    print()
    print('Compilation STDERR:')
    print(s.stderr)
    print('------------')
    print()

    with open('test_in.txt', 'r') as fin:
        with open('test_out.txt', 'w') as fout:
            s = subprocess.run('main.exe', stdin=fin, stdout=fout)
    print('Run STDERR:')
    print(s.stderr)
    print('------------')
    print()

    print('DIFF:')
    with open('test_out_example.txt') as file_1:
        file_1_text = file_1.readlines()

    with open('test_out.txt') as file_2:
        file_2_text = file_2.readlines()

    # Find and print the diff:
    for line in difflib.unified_diff(
            file_1_text, file_2_text, fromfile='test_out_example.txt',
            tofile='test_out.txt', lineterm=''):
        print(line)


if __name__ == '__main__':
    main()
