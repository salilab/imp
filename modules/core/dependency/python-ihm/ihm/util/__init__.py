"""Utility classes"""

import string
import os
import ihm


class _AsymIDs(object):
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


def _assign_id(obj, seen_objs, obj_by_id, attr='_id', seen_obj=None):
    """Assign a unique ID to obj, and track all ids in obj_by_id."""
    if seen_obj is None:
        seen_obj = obj
    if seen_obj not in seen_objs:
        if not hasattr(obj, attr):
            obj_by_id.append(obj)
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


def _check_residue_range(rng):
    """Make sure that a residue range is not out of range of its Entity"""
    if rng.seq_id_range[1] < rng.seq_id_range[0]:
        raise ValueError("Range %d-%d is invalid; end is before start"
                         % rng.seq_id_range)
    if (rng.seq_id_range[1] > len(rng.entity.sequence)
            or rng.seq_id_range[0] < 1):
        raise IndexError("Range %d-%d out of range for %s (1-%d)"
                         % (rng.seq_id_range[0], rng.seq_id_range[1],
                            rng.entity, len(rng.entity.sequence)))
