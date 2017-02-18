"""Test cases for layout file validation

We are looking to have either a successful parse, or a LayoutException nothing else
"""

import StringIO
import csv
import unittest
from hypothesis import given, settings
from hypothesis.strategies import tuples, integers, text, lists, builds, one_of, just
import tools.app_layout

def lists_to_csv_fd(lists, process=str):
    output = StringIO.StringIO()
    writer = csv.writer(output)
    for name, size in lists:
        writer.writerow((name, process(size)))
    return StringIO.StringIO(output.getvalue())

ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-;&[]{+()}"

class TestLayoutParsing(unittest.TestCase):

    @settings(max_examples=500)
    @given(builds(lists_to_csv_fd, lists(tuples(text(ALPHABET), integers()))))
    def test_conforming_csv_int(self, csv_file):
        list(tools.app_layout.layout_to_regions(csv_file, 0, 0))

    @settings(max_examples=500)
    @given(lists(tuples(text(ALPHABET), integers())))
    def test_conforming_csv_hex(self, to_construct):
        csv_file = lists_to_csv_fd(to_construct, lambda x: "0x" + str(x))
        list(tools.app_layout.layout_to_regions(csv_file, 0, 0))

    @settings(max_examples=500)
    @given(text(ALPHABET + "0123456789\n\t, *"))
    def test_arbitrary_text(self, to_construct):
        csv_file = StringIO.StringIO(to_construct)
        try:
            list(tools.app_layout.layout_to_regions(csv_file, 0, 0))
        except tools.app_layout.LayoutException as e:
            pass

    @settings(max_examples=500)
    @given(builds(lists_to_csv_fd,
                  lists(tuples(text(ALPHABET, min_size=1),
                               one_of(just("*"), integers())),
                        min_size=2)))
    def test_conforming_with_globs(self, csv_file):
        try:
            list(tools.app_layout.layout_to_regions(csv_file, 0, 0))
        except tools.app_layout.LayoutException as e:
            pass


if __name__ == "__main__":
    unittest.main()
