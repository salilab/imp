HEAD
====
 - Authors of the mmCIF file itself (`_audit_author` category) can now be
   set by manipulating :attr:`ihm.System.authors`. (If this list is empty on
   output, the set of all citation authors is used instead, as before.)
 - Any grants that supported the modeling can now be listed in
   :attr:`ihm.System.grants`.

0.6 - 2019-03-22
================
 - :class:`Entity` now takes an optional :class:`ihm.source.Source` object to
   describe the method by which the sample for the entity was produced.
   :class:`ihm.metadata.PDBParser` will also extract this information
   from input PDB files.
 - :func:`ihm.reader.read` and :func:`ihm.dumper.write` now support reading
   or writing additional user-defined mmCIF categories.

0.5 - 2019-01-17
================
 - :class:`ihm.restraint.CrossLinkRestraint` now takes an
   :class:`ihm.ChemDescriptor` object rather than the name of the cross-linker
   used. This allows the use of novel cross-linkers (beyond those currently
   listed in a fixed enumeration in the IHM dictionary).
   :class:`ihm.ChemDescriptor` allows for the chemical structure of the
   cross-linker to be uniquely specified, as a SMILES or INCHI string.
   The :mod:`ihm.cross_linkers` module provides chemical descriptors for
   some commonly-used cross-linkers.
 - Pseudo sites are now supported. :class:`ihm.restraint.PseudoSiteFeature`
   allows points or spheres with arbitrary coordinates to be designated as
   features, which can then be used in
   :class:`ihm.restraint.DerivedDistanceRestraint`.

0.4 - 2018-12-17
================
 - Certain restraints can now be grouped using the
   :class:`ihm.restraint.RestraintGroup` class. Due to limitations of the
   underlying dictionary, this only works for some restraint types (currently
   only :class:`ihm.restraint.DerivedDistanceRestraint`) and all restraints
   in the group must be of the same type.
 - Bugfix: the the model's representation (see :mod:`ihm.representation`)
   need not be a strict subset of the model's :class:`ihm.Assembly`. However,
   any :class:`ihm.model.Atom` or :class:`ihm.model.Sphere` objects must be
   covered by both the representation and the model's :class:`ihm.Assembly`.
 - Bugfix: the reader no longer fails to read files that contain
   _entity.formula_weight.

0.3 - 2018-11-21
================

 - The library now includes basic support for nonpolymers and water molecules.
   In addition to the previous support for polymers (amino or nucleic acid
   chains), :class:`ihm.Entity` objects can now comprise ligands, water
   molecules, and user-defined chemical components.
 - The library can now read mmCIF dictionaries and validate mmCIF or BinaryCIF
   files against them. See :mod:`ihm.dictionary`.
 - Any :class:`ihm.model.Atom` or :class:`ihm.model.Sphere` objects are now
   checked against the model's representation (see :mod:`ihm.representation`);
   for example, an :class:`ihm.model.Atom` must correspond to an
   :class:`ihm.representation.AtomicSegment`. The representation in turn must
   be a subset of the model's :class:`ihm.Assembly`.
 - More examples are now provided, of creating and using non-standard residue
   types (chemical components); representing nonpolymers; and using the C
   mmCIF parser in other C programs.

0.2 - 2018-09-06
================

 - This release should fix installation of the package using pip:
   `pip install ihm` should now work correctly.

0.1 - 2018-09-06
================

 - First stable release. This provides largely complete support for the current
   version of the wwPDB IHM mmCIF extension dictionary, and will read and
   write mmCIF and BinaryCIF files that are compliant with the PDBx and
   IHM dictionaries.

