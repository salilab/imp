"""Utility classes"""

import string
import os


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
