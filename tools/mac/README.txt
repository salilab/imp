Double-click on the package (.pkg) file and follow the prompts to
install IMP on your Mac.

(Newer Macs may complain that the installer is from an unknown developer.
This is expected; simply right click on the installer, and select "Open"
from the menu, rather than double-clicking on it.)

Your Mac might complain that a newer version of IMP already exists, if you
previously installed a different release of IMP. To fix this, you first need to
remove the package receipt. On older Macs, remove "IMP 1.0 10v4.pkg" or similar
from /Library/Receipts (not Library/Receipts; you won't find it in there).
On newer Macs, remove org.salilab.imp.* from /var/db/receipts.

The installer will install IMP under the /usr/local/ directory (you won't see
an IMP application in your Applications folder). For example, the FoXS fitting
program is installed in /usr/local/bin/foxs, so can be run simply by typing
"foxs" in a Terminal window. Note that any existing IMP installation on your
machine will be overwritten.

The IMP Python libraries are automatically set up to run with the default
version of Python on your Mac. To run an IMP Python script called 'foo.py',
simply run 'python foo.py' from a Terminal window in the directory containing
the script. (If you have multiple versions of Python installed on your
system, you may need to force use of the default system version by typing
'/usr/bin/python' rather than just 'python'.)

To completely uninstall IMP, run
sudo imp_uninstall
from a Terminal window.
