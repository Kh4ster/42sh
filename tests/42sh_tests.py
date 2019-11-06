from argparse import ArgumentParser
from pathlib import Path
from difflib import unified_diff
from termcolor import colored
import yaml
import os
import subprocess as sp

DEFAULT_TIMEOUT = 2

def run_shell(args, stdin, timeout):
    """ Run the given args with stdin and return captured output """

    return sp.run(args, capture_output=True, text=True, input=stdin,
        timeout=timeout)

def diff(ref, student):
    """ Return differences between input strings """

    ref = ref.splitlines(keepends=True)
    student = student.splitlines(keepends=True)

    return ''.join(unified_diff(ref, student, fromfile="ref", tofile="42sh"))

def test(binary, test_case, timeout, args):
    """ run ref and student, check diff (return nothing or assertException) """

    try:
        ref = test_case.get("expected", "")
        if ref == "":
            ref = run_shell(["bash", "--posix"], test_case["stdin"], timeout)
        if args.sanity:
            student = run_shell(["valgrind", binary], test_case["stdin"],
                timeout)
        else:
            student = run_shell([binary], test_case["stdin"], timeout)
    except sp.TimeoutExpired:
        raise AssertionError(f"Timeout after {timeout} seconds")

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

def launch_test(binary, test_case, args):
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
        print(err)
        return 0

    print(f"[{colored('OK', 'green')}]", test_case.get("name"))
    return 1

def pretty_print_synthesis(passed, failed):
    print(f"[{colored('==', 'red' if failed else 'blue')}] Synthesis:", end='')
    color_tested = colored(f"{passed + failed:2d}", 'blue')
    print(f" Tested: {color_tested} | ", end='')
    color_passed = colored (f"{passed:2d}", 'green')
    print(f"Passing: {color_passed} | ", end='')
    color_failed = colored (f"{failed:2d}", 'red' if failed else 'blue')
    print(f"Failing: {color_failed}")


def launch_tests(binary, tests_files, args):
    """ Launch with binary all tests found in each test_file """

    total_passed, total_failed = 0, 0

    for tests_file in tests_files:
        with open(tests_file, "r") as tf:
            print("\n" + f" {tests_file} ".center(80, "-") + "\n")
            nb_passed, nb_failed = 0, 0
            for test_case in yaml.safe_load(tf):
                if launch_test(binary, test_case, args):
                    nb_passed += 1
                else:
                    nb_failed += 1
        total_passed += nb_passed
        total_failed += nb_failed
        pretty_print_synthesis(nb_passed, nb_failed)

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
    parser = ArgumentParser(description="42sh Functional Testsuite")
    parser.add_argument("bin", metavar="BIN")
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
    parser.add_argument("-t", "--timeout", type=int,
            dest="timeout", default=0,
            help="Set time as a general timeout time (in seconds).")
    args = parser.parse_args()

    binary = Path(args.bin).absolute()

    tests_files = get_tests_files(args.categories)
    if args.list: # if list is activated, just show name of categories
        print(f" tests categories ".center(80, "-"))
        list_categories(tests_files)
    else:
        launch_tests(binary, tests_files, args)