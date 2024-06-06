Design principles
*****************

Lightweight
===========

The classes in this package are designed to be lightweight, taking up as
little memory as possible. For example, individual atoms are *not* stored
in Python classes, and are only requested when needed. This is because the
library is designed to work with an existing modeling package, which likely
already stores data on the system in its own files or data structures, such
that duplicating this information would be very inefficient.

Mutable
=======

All classes are designed to be *mutable*; that is, their contents can be
changed after creation. For example, protein chains can be added to or removed
from an existing :class:`ihm.Assembly` object, or the amino acid sequence
of an :class:`ihm.Entity` can be extended. This because some of the modeling
packages which use these classes build up their own data model in a similar
way.

Types rather than enums
=======================

Where the underlying IHM mmCIF dictionary uses an enumeration, generally this
corresponds to separate sibling classes in this package. For example, two
datasets which differ only in their ``data_type``
`in the dictionary <https://mmcif.wwpdb.org/dictionaries/mmcif_ihm.dic/Items/_ihm_dataset_list.data_type.html>`_
(such as a electron microscopy density map and small angle scattering data)
are represented with two classes in this package:
:class:`ihm.dataset.EMDensityDataset` and :class:`ihm.dataset.SASDataset`.
This cleanly enforces the allowed types in the most Pythonic manner.

Hierarchy of classes
====================

The underlying IHM mmCIF dictionary is essentially structured as a set of
rows in database tables, with IDs acting as keys or pointers into other tables.
This is naturally represented in Python as a hierarchy of classes, with
members pointing to other objects as appropriate. IDs are not used to look
up other objects, and are only used internally to populate the tables.
For example, to group multiple models together, the dictionary assigns all of
the models the same `model_group id <https://mmcif.wwpdb.org/dictionaries/mmcif_ihm_ext.dic/Categories/ihm_model_group.html>`_
while in the Python package the :class:`ihm.model.Model` objects are placed
into a :class:`ihm.model.ModelGroup` object, which acts like a simple Python
list.

The table-based representation of the dictionary does allow for objects to
exist that are not referenced by other objects, unlike the Python-based
hierarchy. Such 'orphan' objects can be referenced from orphan lists in
the top-level :class:`ihm.System` if necessary.

Equal versus identical objects
==============================

Since the Python objects are mutable, can be constructed iteratively by a
modeling package, and live in a hierarchy, it can sometimes turn out that two
Python objects while not identical (they point to different locations in
memory) are equal (their contents are the same). For example, the two
:class:`ihm.Assembly` objects, one of proteins A, B, and C, and the other of
A, C, and B, are not identical (they are different objects) but are equal
(the order of the proteins does not matter). The library will attempt to
detect such objects and consolidate them on output, describing both of them
in the mmCIF file with the same ID, to avoid meaningless duplication of rows
in the output tables. This removes some of the burden from the author of the
modeling package, which may not care about such a distinction.

mmCIF backend
=============

The classes in this package roughly correspond to
`categories <https://mmcif.wwpdb.org/dictionaries/mmcif_ihm.dic/Categories/index.html>`_
in the underlying IHM mmCIF dictionary. This allows for simple output of
mmCIF formatted files, but also allows for the potential future support for
other file formats that support the dictionary or a subset of it, such
as `MMTF <https://mmtf.rcsb.org/>`_.
