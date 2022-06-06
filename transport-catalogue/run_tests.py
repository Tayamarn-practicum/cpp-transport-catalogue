import difflib
import json
import subprocess
import xml.etree.ElementTree as ET

class XmlTree():

    @staticmethod
    def convert_string_to_tree(xmlString):

        return ET.fromstring(xmlString)

    def xml_compare(self, x1, x2, excludes=[]):
        """
        Compares two xml etrees
        :param x1: the first tree
        :param x2: the second tree
        :param excludes: list of string of attributes to exclude from comparison
        :return:
            True if both files match
        """

        if x1.tag != x2.tag:
            print('Tags do not match: %s and %s' % (x1.tag, x2.tag))
            return False
        for name, value in x1.attrib.items():
            if not name in excludes:
                if x2.attrib.get(name) != value:
                    print('Attributes do not match: %s=%r, %s=%r'
                                 % (name, value, name, x2.attrib.get(name)))
                    return False
        for name in x2.attrib.keys():
            if not name in excludes:
                if name not in x1.attrib:
                    print('x2 has an attribute x1 is missing: %s'
                                 % name)
                    return False
        if not self.text_compare(x1.text, x2.text):
            print('text: %r != %r' % (x1.text, x2.text))
            return False
        if not self.text_compare(x1.tail, x2.tail):
            print('tail: %r != %r' % (x1.tail, x2.tail))
            return False
        cl1 = list(x1)
        cl2 = list(x2)
        if len(cl1) != len(cl2):
            print('children length differs, %i != %i'
                         % (len(cl1), len(cl2)))
            return False
        i = 0
        for c1, c2 in zip(cl1, cl2):
            i += 1
            if not c1.tag in excludes:
                if not self.xml_compare(c1, c2, excludes):
                    print('children %i do not match: %s'
                                 % (i, c1.tag))
                    return False
        return True

    def text_compare(self, t1, t2):
        """
        Compare two text strings
        :param t1: text one
        :param t2: text two
        :return:
            True if a match
        """
        if not t1 and not t2:
            return True
        if t1 == '*' or t2 == '*':
            return True
        return (t1 or '').strip() == (t2 or '').strip()


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

    with open('test_input.json', 'r') as fin:
        with open('test_output.svg', 'w') as fout:
            s = subprocess.run('main.exe', stdin=fin, stdout=fout)
    print('Run STDERR:')
    print(s.stderr)
    print('------------')
    print()

    # print('DIFF:')
    # with open('test_out_example.svg') as file_1:
    #     file_1_text = file_1.readlines()

    # with open('test_output.svg') as file_2:
    #     file_2_text = file_2.readlines()

    # # Find and print the diff:
    # for line in difflib.unified_diff(
    #         file_1_text, file_2_text, fromfile='test_out_example3.svg',
    #         tofile='test_output.svg', lineterm=''):
    #     print(line)

    print('DIFF:')
    with open('test_out_example.svg') as file_1:
        file_1_text = file_1.read()

    with open('test_output.svg') as file_2:
        file_2_text = file_2.read()

    tree1 = XmlTree.convert_string_to_tree(file_1_text)
    tree2 = XmlTree.convert_string_to_tree(file_2_text)

    comparator = XmlTree()

    if comparator.xml_compare(tree1, tree2, []):
        print("XMLs match")
    else:
        print("XMLs don't match")


    # with open('test_out_example.json', 'r') as f:
    #     j1 = json.load(f)
    # with open('test_output.json', 'r') as f:
    #     j2 = json.load(f)
    # if (j1 == j2):
    #     print('OK!')
    # else:
    #     print('Something is wrong')


if __name__ == '__main__':
    main()
