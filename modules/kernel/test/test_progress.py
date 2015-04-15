import IMP
import sys

IMP.setup_from_argv(sys.argv, "Test progress bar")
IMP.set_log_level(IMP.PROGRESS)
IMP.set_progress_display("Test progress", 110)
for i in range(0, 110, 5):
    IMP.add_to_progress_display(5)

IMP.set_progress_display("Test progress round 2", 130)
for i in range(0, 130, 5):
    IMP.add_to_progress_display(5)
