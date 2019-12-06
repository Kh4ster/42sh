from argparse import ArgumentParser
from argparse import RawDescriptionHelpFormatter
from pathlib import Path
from difflib import unified_diff
from termcolor import colored
import yaml
import os
import subprocess as sp
import xml.etree.ElementTree as ET

DEFAULT_TIMEOUT = 2
XML_DATA = ET.Element('synthesis')
EPILOG = """tests.yml description :
  The folder in which the tests.yml is found is considered as its category.
  In each tests.yml you can have many tests in the format :

\"\"\"tests/example/tests.yml
- name: ls simple       # Name of the test
  stdin: ls             # Stdin given to bash --posix and binary

- name: ls with args
  stdin: ls -la

- name: pipe missing right
  stdin: echo test |
  checks:               # it overwrite default checks on stdout, stderr, return
    - has_stderr        # check only if binary have an stderr
    - returncode        # check if binary have same returncode has bash --posix

- name: timeout expected tree root
  stdin: tree /
  timeout: 1            # Change default timeout for the test

- name: pass options c instead of stdin
  options:              # add options to binary and bash --posix
    - "-c"
    - "echo hello world"
\"\"\"
"""

DESCRIPTION = """42sh Functional Testsuite :
  It launches all tests.yml found in subdirectories"""

def run_shell(args, stdin, timeout):
    """ Run a process with given args and stdin. Return the captured output """

    args = ["timeout", "--signal=KILL", f"{timeout}"] + args
    res = sp.run(args, capture_output=True, text=True, input=stdin)
    if (res.returncode == -9):
        raise TimeoutError
    return res

def diff(ref, student):
    """ Return nice difference between two inputs strings """

    ref = ref.splitlines(keepends=True)
    student = student.splitlines(keepends=True)

    return ''.join(unified_diff(ref, student, fromfile="ref", tofile="42sh"))

def test(binary, test_case, timeout, args):
    """ Run bash and binary, check diff (return nothing or assertException) """

    try:
        ref = run_shell(["bash", "--posix"] + test_case.get("options", []),\
            test_case.get("stdin", ""), timeout)
        binary = [binary]
        if args.sanity:
            binary = ["valgrind", "-q", "--error-exitcode=42"] + binary
        student = run_shell(binary + test_case.get("options", []),\
            test_case.get("stdin", ""), timeout)
    except TimeoutError:
        raise AssertionError(f"Timeout after {timeout} seconds")

    for check in test_case.get("checks", ["stderr", "stdout", "returncode"]):
        if check == "stdout":
            ref_stdout = test_case.get("expected_stdout", ref.stdout)
            assert ref_stdout == student.stdout, \
                f"stdout differs:\n{diff(ref_stdout, student.stdout)}"
        elif check == "stderr":
            ref_stderr = test_case.get("expected_stderr", ref.stderr)
            assert ref_stderr == student.stderr, \
                f"stderr differs:\n{diff(ref_stderr, student.stderr)}"
        elif check == "returncode":
            ref_returncode = test_case.get("expected_returncode", ref.returncode)
            assert ref_returncode == student.returncode, \
                f"Exited with {student.returncode}, expected {ref_returncode}"
        elif check == "has_stderr":
            assert ref.stderr != "", \
                "Bash does not have an stderr (and you used has_stderr)"
            assert student.stderr != "", \
                "The code should print an error message on stderr"
        elif check == "has_stdout":
            assert ref.stdout != "", \
                "Bash does not have an stdout (and you used has_stdout)"
            assert student.stdout != "", \
                "The code should print a message on stdout"

def launch_test(binary, test_case, args, category_xml):
    """ Launch the test and print OK or KO (+diff) depending on the result

    return : 1 if test passed, 0 otherwise
    """

    # get timeout define in yaml or set DEFAULT_TIMEOUT
    timeout = test_case.get("timeout", DEFAULT_TIMEOUT)
    if args.timeout:
        timeout = args.timeout

    try:
        test(binary, test_case, timeout, args)
    except AssertionError as err:
        print(f"[{colored('KO', 'red')}]", test_case.get("name"))
        test_result_xml = ET.SubElement(category_xml, "failed")
        test_result_xml.set('name', test_case.get("name"))
        test_result_xml.text = f"{err}"
        if args.verbose:
            print(f"{err}")
        return 0

    print(f"[{colored('OK', 'green')}]", test_case.get("name"))
    test_result_xml = ET.SubElement(category_xml, "passed")
    test_result_xml.set('name', test_case.get("name"))
    test_result_xml.text = f"This test completed successfully."
    return 1

def pretty_print_synthesis(passed, failed):
    """ Print on stdout the synthesis depending on nb failed/passed tests """

    print(f"[{colored('==', 'red' if failed else 'blue')}] Synthesis:", end='')
    color_tested = colored(f"{passed + failed:2d}", 'blue')
    print(f" Tested: {color_tested} | ", end='')
    color_passed = colored (f"{passed:2d}", 'green')
    print(f"Passing: {color_passed} | ", end='')
    color_failed = colored (f"{failed:2d}", 'red' if failed else 'blue')
    print(f"Failing: {color_failed}")


def launch_tests(binary, tests_files, args):
    """ Launch with binary all tests found in each tests_file """

    total_passed, total_failed = 0, 0
    global XML_DATA

    for tests_file in tests_files:
        with open(tests_file, "r") as tf:
            print("\n" + f" {tests_file} ".center(80, "-") + "\n")
            category_xml = ET.SubElement(XML_DATA, "category")
            category_xml.set('name', f"{tests_file.parent}")
            nb_passed, nb_failed = 0, 0
            for test_case in yaml.safe_load(tf):
                if launch_test(binary, test_case, args, category_xml):
                    nb_passed += 1
                else:
                    nb_failed += 1
        total_passed += nb_passed
        total_failed += nb_failed
        pretty_print_synthesis(nb_passed, nb_failed)
        category_xml.set("passed", f"{nb_passed}")
        category_xml.set("failed", f"{nb_failed}")

    print("\n" + " GLOBAL SYNTHESIS ".center(80, "-"))
    pretty_print_synthesis(total_passed, total_failed)
    print("".center(80, "-") + "\n")

def get_tests_files(categories):
    """ Find all tests in directories (or using categories given by user) """

    tests_files=[]
    if not categories: # categories were not defined by the user
        # Find all test files
        for found_test in Path(os.path.dirname(__file__)).rglob('tests.yml'):
            tests_files.append(found_test)
    else:
        # find test files corresponding to categories
        for category in categories:
            for found_test in Path(category).rglob('tests.yml'):
                tests_files.append(found_test)
    return tests_files

def list_categories(tests_files):
    """ List all categories of tests """

    category_list = []
    for tests_file in tests_files:
        print(tests_file.parent)


if __name__ == "__main__":
    """ This script launch automatically the functional testssuite """

    # Set all arguments that can be used by the script
    parser = ArgumentParser(description= DESCRIPTION,
            formatter_class = RawDescriptionHelpFormatter,
            epilog = EPILOG)
    parser.add_argument("bin", metavar="BINARY")
    parser.add_argument("-l", "--list", action="store_true",
            dest="list", default=False,
            help="Display the list of test categories")
    parser.add_argument("-c", "--category", action="append",
            metavar="CATEGORY", dest="categories", default=[],
            help="Execute the test suite on the categories passed in argument \
            only.")
    parser.add_argument("-s", "--sanity", action="store_true",
            dest="sanity", default=False,
            help="Execute the test suite with sanity checks enabled")
    parser.add_argument("-t", "--timeout", type=float,
            dest="timeout", default=0,
            help="Set time as a general timeout time (in seconds).")
    parser.add_argument("-v", "--verbose", action="store_true",
            dest="verbose", default=False,
            help="Show all errors of tests")
    parser.add_argument("-o", "--output", action="store",
            dest="output_file_name", default="",
            help="Set the name of the output xml file containing errors")
    args = parser.parse_args()

    binary = Path(args.bin).absolute()

    tests_files = get_tests_files(args.categories)
    if args.list: # if list is activated, just show name of categories
        print(f" tests categories ".center(80, "-"))
        list_categories(tests_files)
    else:
        launch_tests(binary, tests_files, args)
        if Path(args.output_file_name).suffix == "xml":
            xml_data_str = f"{ET.tostring(XML_DATA)}"[2:-1]
            with open(args.output_file_name, "w") as output_file:
                output_file.write(xml_data_str)
