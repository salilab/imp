##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Michael Habeck and Wolfgang Rieping
##        
##          Copyright (C) Michael Habeck and Wolfgang Rieping
## 
##          All rights reserved.
##
## NO WARRANTY. This library is provided 'as is' without warranty of any
## kind, expressed or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##

class logfile(file):

    default_path = '/tmp/isd.log'

    def __init__(self, name = None):
        if name is None: name = self.default_path
        file.__init__(self, name, 'w', 1)

