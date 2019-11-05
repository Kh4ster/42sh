from argparse import ArgumentParser
from pathlib import Path
from difflib import unified_diff
from termcolor import colored
import yaml
import os
import subprocess as sp

def run_shell(args, stdin):
    return sp.run(args, capture_output=True, text=True, input=stdin)

def diff(ref, student):
    ref = ref.splitlines(keepends=True)
    student = student.splitlines(keepends=True)

    return ''.join(unified_diff(ref, student, fromfile="ref", tofile="42sh"))

def test(binary, test_case):
    ref = run_shell(["bash", "--posix"], test_case["stdin"])
    student = run_shell([binary], test_case["stdin"])

    for check in test_case.get("checks", ["stdout", "stderr", "returncode"]):
        if check == "stdout":
            assert ref.stdout == student.stdout, \
                f"stdout differs:\n{diff(ref.stdout, student.stdout)}"
        elif check == "stderr":
            assert ref.stderr == student.stderr, \
                f"stderr differs:\n{diff(ref.stderr, student.stderr)}"
        elif check == "returncode":
            assert ref.returncode == student.returncode, \
                f"Exited with {student.returncode}, expected {ref.returncode}"
        elif check == "has_stderr":
            assert student.stderr != "", \
                "The code should print an error message on stderr"

def launch_test(binary, test_case):
    try:
        test(binary, test_case)
    except AssertionError as err:
        print(f"[{colored('KO', 'red')}]", test_case)
        print(err)
    else:
        print(f"[{colored('OK', 'green')}]", test_case)

def launch_tests(binary, categories, args):
    """ Launch all tests found
    find all tests in directories (or using categories given by user)
    launch tests on binary taking into account arguments given
    """
    tests_files=[]
    if not categories: # categories was not defined by the user
        # Find all test files
        tests_files = Path(os.path.dirname(__file__)).rglob('tests.yml')
    else:
        # find test files corresponding to categories
        for category in categories:
            tests_files.append(Path(category).rglob('tests.yml'))

    print(tests_files)

    # launch all tests of each file
    for tests_file in tests_files:
        with open(tests_file, "r") as tf:
            print(f"{tests_file}\n")
            for test_case in yaml.safe_load(tf):
                launch_test(binary, test_case)

if __name__ == "__main__":

    # Set all arguments that can be used by the script
    parser = ArgumentParser(description="42sh Functional Testsuite")
    parser.add_argument("bin", metavar="BIN")
    parser.add_argument("-l", "--list", action="store_true",
            dest="list", default=False,
            help="Display the list of test categories")
    parser.add_argument("-c", "--category", action="append", metavar="CATEGORY",
            dest="categories", default=[],
            help="Execute the test suite on the categories passed in argument \
            only.")
    parser.add_argument("-s", "--sanity", action="store_true",
            dest="sanity", default=False,
            help="Execute the test suite with sanity checks enabled")
    parser.add_argument("-t", "--timeout", action="store",
            dest="time", default=0,
            help="Set time as a general timeout time (in seconds).")
    args = parser.parse_args()

    binary = Path(args.bin).absolute()

    # if list is activated, just show name of folders (categories)

    # if sanity is activated, add option to binary

    launch_tests(binary, args.categories, args)