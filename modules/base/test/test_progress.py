import IMP.base
import sys

IMP.base.setup_from_argv(sys.argv, "Test progress bar")
IMP.base.set_log_level(IMP.base.PROGRESS)
IMP.base.set_progress_display("Test progress", 110)
for i in range(0, 110, 5):
    IMP.base.add_to_progress_display(5)
