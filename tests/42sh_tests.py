from argparse import ArgumentParser
from pathlib import Path

def launch_tests(binary, args):
    test_list = args.list
    print(f"should launch tests on {binary}")

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

    # launch tests on binary taking into account arguments given
    launch_tests(binary, args)