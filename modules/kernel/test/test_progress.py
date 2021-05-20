import IMP
import IMP.test
import sys


class Tests(IMP.test.TestCase):

    def test_progress(self):
        """Test progress bar"""
        IMP.set_log_level(IMP.PROGRESS)
        IMP.set_progress_display("Test progress", 110)
        for i in range(0, 110, 5):
            IMP.add_to_progress_display(5)

        IMP.set_progress_display("Test progress round 2", 130)
        for i in range(0, 130, 5):
            IMP.add_to_progress_display(5)


if __name__ == '__main__':
    IMP.test.main()
