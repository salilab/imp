def _handle_seq_indx(seqtype, indx):
    if isinstance(indx, int):
        if indx < 0:
            indx += len(seqtype)
        if indx < 0 or indx >= len(seqtype):
            raise IndexError("list index out of range")
        else:
            return indx
    elif isinstance(indx, slice):
        return range(*indx.indices(len(seqtype)))
    else:
        raise TypeError("expecting an integer index")


class VarListIterator:
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


class VarList:
    """A list-like object that wraps IMP C++ object accessor methods"""
    def __init__(self, getdimfunc, getfunc, erasefunc, appendfunc, extendfunc,
                 clearfunc, indexfunc):
        self.__getdimfunc = getdimfunc
        self.__getfunc = getfunc
        self.__erasefunc = erasefunc
        self.__appendfunc = appendfunc
        self.__extendfunc = extendfunc
        self.__clearfunc = clearfunc
        self.__indexfunc = indexfunc

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

    def __eq__(self, other):
        # e.g. lists compare equal to other lists, but not to tuples
        if not isinstance(other, (list, VarList)):
            return False
        if len(other) != len(self):
            return False
        return all(a == b for a, b in zip(self, other))

    def __getitem__(self, indx):
        ret = _handle_seq_indx(self, indx)
        if isinstance(ret, int):
            return self.__getfunc(ret)
        else:
            return [self[i] for i in ret]

    def __delitem__(self, indx):
        ret = _handle_seq_indx(self, indx)
        if isinstance(ret, int):
            return self.__erasefunc(ret)
        else:
            for i in sorted(ret, reverse=True):
                del self[i]

    def index(self, obj, start=0, stop=2**30):
        # 2**30 *should* be large enough to be larger than any list we use,
        # but not large enough to have to worry about overflow
        # (Python uses 2**63)
        return self.__indexfunc(obj, start, stop)

    def remove(self, obj):
        del self[self.index(obj)]

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
