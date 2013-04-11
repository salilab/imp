%pythoncode %{

import optparse
class OptionParser(optparse.OptionParser):
    """IMP-specific subclass of optparse.OptionParser.
       Please use the flags support in IMP.base. See IMP::base::add_string_flag()
       and IMP::base::setup_from_argv() for example.
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
