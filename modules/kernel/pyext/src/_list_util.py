class VarListIterator(object):
    def __init__(self, varlist, getfunc):
        self.__varlist = varlist
        self.__count = len(varlist)
        self.__getfunc = getfunc
        self.__n = -1

    def __next__(self):
        self.__n += 1
        if self.__n >= self.__count:
            raise StopIteration
        return self.__getfunc(self.__n)


class VarList(object):
    """A list-like object that wraps IMP C++ object accessor methods"""
    def __init__(self, getdimfunc, getfunc, erasefunc, appendfunc, extendfunc,
                 clearfunc):
        self.__getdimfunc = getdimfunc
        self.__getfunc = getfunc
        self.__erasefunc = erasefunc
        self.__appendfunc = appendfunc
        self.__extendfunc = extendfunc
        self.__clearfunc = clearfunc

    def __iter__(self):
        return VarListIterator(self, self.__getfunc)

    def __len__(self):
        return self.__getdimfunc()

    def clear(self):
        self.__clearfunc()

    def append(self, obj):
        return self.__appendfunc(obj)

    def extend(self, obj):
        return self.__extendfunc(obj)

    def __str__(self):
        return str([val for val in self])

    def __repr__(self):
        return str(self)

    def __getitem__(self, indx):
        if indx < 0:
            indx += len(self)
        if indx < 0 or indx >= len(self):
            raise IndexError("list index out of range")
        return self.__getfunc(indx)

    def __delitem__(self, indx):
        if indx < 0:
            indx += len(self)
        if indx < 0 or indx >= len(self):
            raise IndexError("list assignment index out of range")
        return self.__erasefunc(indx)

    def pop(self, indx=-1):
        if len(self) == 0:
            raise IndexError("pop from empty list")
        if indx < 0:
            indx += len(self)
        if indx < 0 or indx >= len(self):
            raise IndexError("pop index out of range")
        x = self[indx]
        del self[indx]
        return x


def set_varlist(lst, obj):
    """Helper function to set members that are VarList objects"""
    lst.clear()
    lst.extend(obj)


def del_varlist(lst):
    """Helper function to delete members that are VarList objects"""
    lst.clear()