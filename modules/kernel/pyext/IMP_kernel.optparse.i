%pythoncode %{

import optparse
class OptionParser(optparse.OptionParser):
    """IMP-specific subclass of optparse.OptionParser.
       This adds extra IMP-specific copyright and help information to the
       normal output from optparse.OptionParser, and should be used by
       IMP Python applications to give consistent behavior.

       @param imp_module An IMP module Python object. If given, this is used
                         to set the version (to that of the module) if that
                         is not explicitly set.
    """
    def __init__(self, imp_module=None, version=None, *args, **keys):
        if version is None:
            if imp_module is not None:
                version = "%prog " + imp_module.get_module_version()
            else:
                raise ValueError("Must supply either version or "
                                 "imp_module arguments")
        optparse.OptionParser.__init__(self, version=version, *args, **keys)

    def format_epilog(self, formatter):
        # Add IMP copyright and help info to epilog
        return optparse.OptionParser.format_epilog(self, formatter) + """
This program is part of IMP, the Integrative Modeling Platform,
which is Copyright 2007-2013 IMP Inventors.
For additional information about IMP, see http://salilab.org/imp/
"""
%}
