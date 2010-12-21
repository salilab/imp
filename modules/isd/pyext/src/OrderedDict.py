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


class OrderedDict(dict):

    def __init__(self, order = None):

        dict.__init__(self)

        if order is None:
            order = []
        
        self.__order = order

    def keys(self):

        if self.__order is not None:
            return self.__order
        else:
            return dict.keys(self)

    def values(self):
        return map(lambda k, s = self: s[k], self.keys())

    def items(self):
        return map(lambda k, s = self: (k, s[k]), self.keys())

    def get_first(self):
        return dict.__getitem__(self, self.__order[0])

    def pop_first(self):

        key = self.__order[0]

        return self.pop(key)

    def get_first_item(self):

        key = self.__order[0]
        value = dict.__getitem__(self, key)

        return key, value

    def pop_first_item(self):

        key = self.__order[0]

        return key, self.pop(key)

    def pop(self, key):

        value = dict.__getitem__(self, key)

        del self[key]

        return value

    def __setitem__(self, key, value):
        
        if key not in self.__order:
            self.__order.append(key) 

        dict.__setitem__(self, key, value)

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self.__order.remove(key)

