"""Utility classes"""

import string
import os
import ihm


class _AsymIDs:
    """Map indices to multi-character asym (chain) IDs.
       We label the first 26 chains A-Z, then we move to two-letter
       chain IDs: AA through AZ, then BA through BZ, through to ZZ.
       This continues with longer chain IDs."""
    def __getitem__(self, ind):
        chars = string.ascii_uppercase
        lc = len(chars)
        ids = []
        while ind >= lc:
            ids.append(chars[ind % lc])
            ind = ind // lc - 1
        ids.append(chars[ind])
        return "".join(reversed(ids))


def _remove_id(obj, attr='_id'):
    """Remove any unique ID from obj"""
    if hasattr(obj, attr):
        delattr(obj, attr)


def _assign_id(obj, seen_objs, obj_by_id, attr='_id', seen_obj=None,
               by_id_obj=None):
    """Assign a unique ID to obj, and track all ids in obj_by_id."""
    if seen_obj is None:
        seen_obj = obj
    if by_id_obj is None:
        by_id_obj = obj
    if seen_obj not in seen_objs:
        if not hasattr(obj, attr):
            obj_by_id.append(by_id_obj)
            setattr(obj, attr, len(obj_by_id))
        seen_objs[seen_obj] = getattr(obj, attr)
    else:
        setattr(obj, attr, seen_objs[seen_obj])


def _get_relative_path(reference, path):
    """Return `path` interpreted relative to `reference`"""
    if os.path.isabs(path):
        return path
    else:
        return os.path.join(os.path.dirname(reference), path)


def _text_choice_property(attr, choices, doc=None):
    """Like `property` but requires that the value be one of the set choices"""
    schoices = frozenset(choices)

    def getfunc(obj):
        return getattr(obj, "_" + attr)

    def setfunc(obj, val):
        if val is not None and val is not ihm.unknown and val not in schoices:
            raise ValueError(
                "Invalid choice %s for %s; valid values are %s, "
                "None, ihm.unknown"
                % (repr(val), attr, ", ".join(repr(x) for x in choices)))
        setattr(obj, "_" + attr, val)

    return property(getfunc, setfunc, doc=doc)


def _check_residue_range(seq_id_range, entity):
    """Make sure that a residue range is not out of range of its Entity"""
    if not entity or not entity._range_check:
        return
    if seq_id_range[1] < seq_id_range[0]:
        raise ValueError("Range %d-%d is invalid; end is before start"
                         % seq_id_range)
    if (seq_id_range[1] > len(entity.sequence)
            or seq_id_range[0] < 1):
        raise IndexError("Range %d-%d out of range for %s (1-%d)"
                         % (seq_id_range[0], seq_id_range[1],
                            entity, len(entity.sequence)))


def _check_residue(r):
    """Make sure that a residue is not out of range of its Entity"""
    if not r.entity or not r.entity._range_check:
        return
    if r.seq_id > len(r.entity.sequence) or r.seq_id < 1:
        raise IndexError("Residue %d out of range for %s (1-%d)"
                         % (r.seq_id, r.entity, len(r.entity.sequence)))


def _check_transform(t):
    if t.rot_matrix in (None, ihm.unknown):
        raise ValueError("Transformation %s is missing rotation" % t)
    if t.tr_vector in (None, ihm.unknown):
        raise ValueError("Transformation %s is missing translation" % t)


def _invert_ranges(ranges, end, start=1):
    """Given a sorted list of non-overlapping ranges, yield a new list which
       contains every range in the range start-end which was not in the
       original list.  For example, if end=4,
       [(2, 3)] -> [(1, 1), (4, 4)]"""
    for r in ranges:
        if r[0] > start:
            yield (start, r[0] - 1)
        start = r[1] + 1
    if end >= start:
        yield (start, end)


def _pred_ranges(ranges, end):
    """Given a sorted list of non-overlapping ranges, yield a new list which
       covers the range 1-end. Each element in the new list contains a new
       third bool member which is True iff the element was in the original
       list. For example, if end=4,
       [(2, 3)] -> [(1, 1, False), (2, 3, True), (4, 4, False)]"""
    start = 1
    for r in ranges:
        if r[0] > start:
            yield (start, r[0] - 1, False)
        yield (r[0], r[1], True)
        start = r[1] + 1
    if end >= start:
        yield (start, end, False)


def _combine_ranges(ranges):
    """Sort the input ranges and remove any overlaps; yield the result.
       For example, [(8, 10), (1, 2), (3, 4)] -> [(1, 4), (8, 10)]"""
    ranges = sorted(ranges)
    if not ranges:
        return
    current = ranges[0]
    for r in ranges[1:]:
        if current[1] + 1 >= r[0]:
            current = (current[0], max(r[1], current[1]))
        else:
            yield current
            current = r
    yield current


def _make_range_from_list(rr):
    """Yield a list of ranges given a sorted list of values.
       For example, [1, 2, 5, 6] -> [[1, 2], [5, 6]]"""
    if not rr:
        return
    current = [rr[0], rr[0]]
    for r in rr[1:]:
        if current[1] + 1 == r:
            current[1] = r
        else:
            yield current
            current = [r, r]
    yield current


def _get_codes(codestr):
    """Convert a one-letter-code string into a sequence of individual
       codes"""
    if codestr is None or codestr is ihm.unknown:
        return
    i = 0
    while i < len(codestr):
        # Strip out linebreaks
        if codestr[i] == '\n':
            pass
        elif codestr[i] == '(':
            end = codestr.index(')', i)
            yield codestr[i + 1:end]
            i = end
        else:
            yield codestr[i]
        i += 1
