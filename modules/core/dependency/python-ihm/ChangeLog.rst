HEAD
====

 - Certain restraints can now be grouped using the
   :class:`ihm.restraint.RestraintGroup` class. Due to limitations of the
   underlying dictionary, this only works for some restraint types (currently
   only :class:`ihm.restraint.DerivedDistanceRestraint`) and all restraints
   in the group must be of the same type.

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

