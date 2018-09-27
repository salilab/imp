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


def _remove_id(obj):
    """Remove any unique ID from obj"""
    if hasattr(obj, '_id'):
        del obj._id

def _assign_id(obj, seen_objs, obj_by_id):
    """Assign a unique ID to obj, and track all ids in obj_by_id."""
    if obj not in seen_objs:
        if not hasattr(obj, '_id'):
            obj_by_id.append(obj)
            obj._id = len(obj_by_id)
        seen_objs[obj] = obj._id
    else:
        obj._id = seen_objs[obj]

def _get_relative_path(reference, path):
    """Return `path` interpreted relative to `reference`"""
    if os.path.isabs(path):
        return path
    else:
        return os.path.join(os.path.dirname(reference), path)
