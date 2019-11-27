from __future__ import print_function
import time
import contextlib
import IMP.test
import IMP.pmi.tools

@contextlib.contextmanager
def mocked_object(parent, objname, replacement):
    """Temporarily replace parent.objname with replacement.
       Typically `parent` is a module or class object."""
    oldobj = getattr(parent, objname)
    setattr(parent, objname, replacement)
    yield
    setattr(parent, objname, oldobj)

def get_times(outkey, *args, **kwargs):
    """Get a sequence of times by calling Stopwatch repeatedly. Ensure that
       times are reliable by overriding time.clock() to always return whole
       seconds."""
    class MockClock(object):
        count = 0
        def __call__(self):
            self.count += 1
            return self.count
    with mocked_object(IMP.pmi.tools, 'process_time', MockClock()):
        s = IMP.pmi.tools.Stopwatch(*args, **kwargs)
        return [s.get_output()[outkey] for _ in range(4)]

class Tests(IMP.test.TestCase):
    def test_stopwatch_with_delta(self):
        """Test Stopwatch with delta"""
        s = IMP.pmi.tools.Stopwatch()
        self.assertEqual(s.label, "None")
        s.set_label("foo")
        self.assertEqual(s.label, "foo")
        output = s.get_output()
        self.assertEqual(list(output.keys()), ['Stopwatch_foo_delta_seconds'])
        self.assertIsInstance(list(output.values())[0], str)
        # Times should be deltas
        times = get_times('Stopwatch_None_delta_seconds')
        self.assertEqual(times, ['1'] * 4)

    def test_stopwatch_elapsed(self):
        """Test Stopwatch without delta"""
        s = IMP.pmi.tools.Stopwatch(isdelta=False)
        output = s.get_output()
        self.assertEqual(list(output.keys()),
                         ['Stopwatch_None_elapsed_seconds'])
        self.assertIsInstance(list(output.values())[0], str)
        # Times should be cumulative
        times = get_times('Stopwatch_None_elapsed_seconds', isdelta=False)
        self.assertEqual(times, ['1', '2', '3', '4'])

if __name__ == '__main__':
    IMP.test.main()
