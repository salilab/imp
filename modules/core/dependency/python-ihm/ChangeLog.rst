2.8 - 2025-12-12
================
  - The new :meth:`ihm.System.get_representative_model` method returns a single
    :class:`ihm.model.ModelRepresentative` that best represents the entire
    system (#174).
  - Bugfix: spurious segments in the ``ihm_entity_poly_segment`` table
    are no longer created for :class:`ihm.restraint.ResidueFeature` (#175).

2.7 - 2025-07-07
================
  - Representative models for :class:`ihm.model.ModelGroup` can now be
    denoted using the :class:`ihm.model.ModelRepresentative` class (#173).
  - Bugfix: fix reference counts of Python bool objects.

2.6 - 2025-06-09
================
  - The new :class:`ihm.License` and :class:`ihm.Disclaimer` classes
    allow describing how the data in the file can be used, and map to
    the ``pdbx_data_usage`` mmCIF table (#171).
  - Two :class:`ihm.Assembly` classes are now considered equal if they
    contain the same set of constituents, even if those constituents are
    in differing orders or are split differently into domains (#170).
  - The new :class:`ihm.format.ChangeFuncValueFilter` class allows modifying
    mmCIF values by passing them through an arbitrary Python function.
  - Checks for non-canonical atom names no longer use Ligand Expo to obtain
    CCD info, as it is being retired (#169).
  - :class:`ihm.metadata.CIFParser` and :class:`ihm.metadata.BinaryCIFParser`
    now return the same ``entity_source`` mapping that
    :class:`ihm.metadata.PDBParser` does (#168).

2.5 - 2025-04-25
================
  - Assemblies are now checked at file-output time to ensure that each
    asym referenced in an assembly is represented by atoms and/or spheres
    in at least one model (#165).
  - ``make_mmcif`` has a new ``--check_atom_names`` option which, if set,
    will check for any non-canonical atom names (#166).
  - ``pip install`` should no longer fail to install if the C extension
    module cannot be built; it will fall back to use the slower pure Python
    implementation.

2.4 - 2025-03-25
================
  - Add support for fits of model groups or ensembles to cross-links
    using the new :class:`ihm.restraint.CrossLinkGroupFit` class.

2.3 - 2025-03-13
================
  - The C-accelerated BinaryCIF parser now supports interval quantization
    encoding and a wider range of data types.
  - ``make_mmcif`` now has a new ``--histidines`` option to convert any
    non-standard histidine names to HIS (#164).
  - Bugfix: don't change name or description of the complete assembly
    in ``make_mmcif``.
  - Bugfix: if :class:`ihm.reference.SeqDif` is used to annotate an
    insertion or deletion, don't erroneously claim the alignment is incorrect.

2.2 - 2025-02-13
================
  - The C-accelerated BinaryCIF parser now supports a wider range of data types.

2.1 - 2025-02-12
================
  - Minor fixes to the C-accelerated BinaryCIF parser.

2.0 - 2025-02-11
================
  - python-ihm no longer supports Python 2; the oldest supported version
    is now Python 3.6 (#161).
  - BinaryCIF files are now read in using a C-accelerated parser, which is
    faster and more memory efficient than the older Python parser (#160).
  - The tokenizers for mmCIF and BinaryCIF now return data of the correct
    type (int, float, bool, or string); previously, all values were returned
    as strings. :class:`ihm.reader.Handler` subclasses now request data of
    the correct type using Python type annotations. The API of the
    C-accelerated parsers has changed accordingly (#162).
  - The new :class:`ihm.metadata.BinaryCIFParser` class can extract metadata
    such as database IDs and template information from BinaryCIF files, in
    a similar fashion to the existing :class:`ihm.metadata.CIFParser`.
  - Information about a deposited structure, such as the deposition date,
    can now be read from :attr:`System.database_status` (#163).
  - The new :class:`ihm.format.ReplaceCategoryFilter` class can be used to
    completely replace or remove an mmCIF category when using
    :class:`ihm.format.CifTokenReader`.

1.8 - 2024-11-26
================
  - Support added for datasets containing EPR, X-ray diffraction, footprinting
    or predicted contacts using the :class:`ihm.dataset.EPRDataset`,
    :class:`ihm.dataset.XRayDiffractionDataset`,
    :class:`ihm.dataset.HydroxylRadicalFootprintingDataset`,
    :class:`ihm.dataset.DNAFootprintingDataset` and
    :class:`ihm.dataset.PredictedContactsDataset` classes (#157).
  - Revision information (in the ``pdbx_audit_revision_*`` mmCIF tables)
    can now be read or written using the new :class:`ihm.Revision`
    and :class:`ihm.RevisionDetails` classes (#156).
  - The new :class:`ihm.location.BMRbigLocation` class allows for
    referencing datasets stored in the BMRbig database.
  - All references to the old PDB-Dev database are now updated to PDB-IHM.

1.7 - 2024-10-22
================
  - Sanity checks when writing out a file can now be disabled if desired,
    using the new ``check`` argument to :func:`ihm.dumper.write` (#153).
  - Data that have been split over multiple mmCIF or BinaryCIF files can now
    be combined into a single :class:`ihm.System` object using the new
    ``add_to_system`` argument to :func:`ihm.reader.read`.
  - Input files that assign multiple :class:`ihm.location.Location` objects
    to a single :class:`ihm.dataset.Dataset` can now be read (#151).
  - Bugfix: multiple :class:`ihm.restraint.EM3DRestraint` and
    :class:`ihm.restraint.SASRestraint` objects can now be created for a
    single dataset, as long as they act on different assemblies, as allowed
    by the dictionary.
  - Bugfix: allow for non-standard residues in the ``struct_ref`` table (#154).

1.6 - 2024-09-27
================
  - The new class :class:`ihm.model.NotModeledResidueRange` allows for
    the annotation of residue ranges that were explicitly not modeled.
    These are written to the ``_ihm_residues_not_modeled`` mmCIF table,
    and any residue marked as not-modeled in all models will also be
    excluded from the ``pdbx_poly_seq_scheme`` table.
  - The ``make_mmcif`` utility script will now automatically add any
    missing :class:`ihm.model.NotModeledResidueRange` objects for
    not-modeled residue ranges (#150).
  - Bugfix: the residue range checks introduced in version 1.5 broke the
    API used by python-modelcif. They have been reimplemented using the
    original API.
  - Bugfix: an unknown (?) value for ``pdbx_poly_seq_scheme.auth_seq_num``
    is now preserved, not silently removed, when reading an mmCIF file.

1.5 - 2024-09-06
================
  - Trying to create a :class:`ihm.Residue`, :class:`ihm.EntityRange`, or
    :class:`ihm.AsymUnitRange` that references out-of-range residues (i.e.
    ``seq_id`` less than 1 or beyond the length of the :class:`ihm.Entity`
    sequence) will now raise an error.
  - Bugfix: :class:`ihm.reference.Reference` objects are no longer given
    erroneous duplicate IDs on output (#149).

1.4 - 2024-08-30
================
  - :class:`ihm.metadata.CIFParser` now extracts metadata from mmCIF starting
    models from Model Archive or compliant with the ModelCIF dictionary.
  - :meth:`ihm.Citation.from_pubmed_id` now takes an ``is_primary`` argument,
    to allow denoting the publication as the most pertinent for the modeling.
  - Duplicate references, pseudo sites, and features are now pruned on
    output (#148).
  - :class:`ihm.restraint.ResidueFeature` now reports an error if it is
    given zero residue ranges (#147).
  - Bugfix: allow for :class:`ihm.startmodel.Template` ``seq_id_range``
    or ``template_seq_id_range`` to be empty.

1.3 - 2024-07-16
================
  - The new class :class:`ihm.location.ProteomeXchangeLocation` can be used
    for datasets stored in the ProteomeXchange database.
  - Support is added for changes in the IHMCIF dictionary, specifically
    the renaming of "CX-MS data" to "Crosslinking-MS data" and the
    ``_ihm_ordered_ensemble`` category to ``_ihm_ordered_model``. python-ihm
    will output the new names, but for backwards compatibility will read both
    old and new names.
  - :class:`ihm.protocol.Protocol` can now be given additional text to
    describe the protocol.
  - :class:`ihm.model.Atom` now takes an ``alt_id`` argument to support
    alternate conformations (#146).
  - Support added for NumPy 2.0.

1.2 - 2024-06-12
================
  - :class:`ihm.format.CifTokenReader` allows for reading an mmCIF file
    and breaking it into tokens. This can be used for various housekeeping
    tasks directly on an mmCIF file, such as changing chain IDs or renaming
    categories or data items, while preserving most other formatting such
    as comments and whitespace (#141).
  - :class:`ihm.restraint.HDXRestraint` adds support for restraints
    derived from Hydrogen-Deuterium Exchange experiments (#143).
  - The ``make_mmcif`` utility script now preserves more "orphan" data from
    the input file that is not referenced by other tables (#144).

1.1 - 2024-05-09
================
  - :class:`ihm.System` now allows for one or more official database IDs to
    be associated with the system using the new :class:`ihm.Database` class.
    This maps to the mmCIF ``_database_2`` category (#135).
  - :class:`ihm.location.FileLocation` now allows for an optional file format
    to be specified (#139).
  - The ``util/make-mmcif.py`` script is now included in the installed package,
    so can be run if desired with ``python3 -m ihm.util.make_mmcif`` (#134).
  - Bugfix: allow for file sizes in input mmCIF files to be floating point
    values (#138).
  - Bugfix: the 'Other' content type is now handled correctly when reading
    information about external files from an mmCIF file (#139).

1.0 - 2024-02-13
================
  - Support for multi-state schemes (such as kinetic rates and relaxation
    times for conversions between states) was added;
    see :mod:`ihm.multi_state_scheme`.
  - Residue numbering in non-polymer, water, and branched entities should
    now be better handled, no longer requiring the various scheme tables
    to precede ``atom_site``. If you subclass :class:`ihm.model.Model`, atoms
    may need to be renumbered; see :meth:`ihm.model.Model.add_atom` (#130).
  - Original author-provided numbering can now be provided for waters,
    using the ``orig_auth_seq_id_map`` argument to :class:`ihm.WaterAsymUnit`.
  - The make-mmcif.py utility script now has basic functionality for
    combining multiple input files into one, relabeling chain IDs if necessary.
  - An :class:`ihm.Entity` containing just a single sugar is now classified
    as a nonpolymer, not branched.

0.43 - 2023-12-08
=================
  - Branched and polymeric empty entities are now distinguished
    based on entity.type in the input file (previously, any empty
    entity would always be reported as a polymer) (#129).
  - Warn rather than reporting an error if the system contains
    one or more empty entities (#128).
  - If an input file contains multiple duplicated datasets, preserve
    them as is rather than consolidating into a single dataset (#127).
  - Allow for multiple branched entities to have the same composition
    (they could have different connectivity) (#126).

0.42 - 2023-11-30
=================
  - The :class:`ihm.metadata.CIFParser` class now parses Modeller-specific
    CIF categories to add information about software and templates for
    Modeller-generated mmCIF starting models.
  - Basic support for original author-provided residue numbering is now
    provided in the :class:`ihm.AsymUnit` class with a new
    ``orig_auth_seq_id_map`` argument. This information is read from and
    written to the various mmCIF tables such as ``pdbx_poly_seq_scheme``
    (#124).

0.41 - 2023-10-02
=================
  - More complete support for oligosaccharides, in particular correct
    numbering for atoms in `atom_site`, and the addition of some
    data items to the output which are required for full
    dictionary compliance.

0.40 - 2023-09-25
=================
  - Basic support for oligosaccharides is now provided. New classes are
    provided to describe saccharide chemical components
    (:class:`ihm.SaccharideChemComp` and subclasses). Unlike polymers and
    non-polymers, oligosaccharides can be branched, and a new
    :class:`ihm.BranchLink` class allows the linkage between individual
    components to be described.
  - A summary report of the system can now be produced by calling
    :meth:`ihm.System.report`. This can help to reveal errors or
    inconsistencies, and will warn about missing data that may not be
    technically required for a compliant mmCIF file, but is usually
    expected to be present.
  - :class:`ihm.metadata.MRCParser` now uses the new EMDB API to extract
    version information and details for electron density map datasets.
  - RPM packages are now available for recent versions of Fedora and
    RedHat Enterprise Linux.

0.39 - 2023-08-04
=================
  - :class:`ihm.location.DatabaseLocation` no longer accepts a ``db_name``
    parameter. Derived classes (such as :class:`ihm.location.PDBLocation`)
    should be used instead; the base class should only be used for "other"
    databases that are not described in the IHM dictionary (#116).
  - Bugfix: AlphaFold models in PDB format are no longer categorized by
    :class:`ihm.metadata.PDBParser` as being deposited in the PDB database
    with an empty accession code.

0.38 - 2023-05-26
=================
  - Convenience classes are added to describe datasets stored in
    the Model Archive, iProX, and AlphaFoldDB repositories
    (:class:`ihm.location.ModelArchiveLocation`,
    :class:`ihm.location.IProXLocation`, and
    :class:`ihm.location.AlphaFoldDBLocation` respectively).
  - The new class :class:`ihm.metadata.CIFParser` can be used to extract
    metadata from starting models in mmCIF format. It is currently in
    development and only supports model metadata from PDB or Model Archive
    at this time.
  - Line wrapping of output mmCIF files can now be turned if desired using
    :func:`ihm.dumper.set_line_wrap` (by default files are wrapped to 80
    characters if possible).
  - The make-mmcif.py utility script now allows for the name of the output
    mmCIF file to be overridden (#115).


0.37 - 2023-02-03
=================
  - Convenience classes are added to describe ensemble FRET datasets
    (:class:`ihm.dataset.EnsembleFRETDataset`) and datasets stored in
    the jPOSTrepo repository (:class:`ihm.location.JPOSTLocation`).
  - Related depositions can now be grouped using the :class:`ihm.Collection`
    class (#108).
  - The :class:`ihm.model.Ensemble` class has a new ``superimposed`` attribute
    to indicate whether the grouped models are structurally aligned.

0.36 - 2023-01-25
=================
  - When reading a file that references external files, preserve any
    information on the size of those files (#104).
  - When reading a file containing models not in a model group, preserve
    any information on the number of models deposited (#105).
  - Bugfix: :func:`ihm.dictionary.read` now correctly handles dictionaries
    that define a category after data items in that category (#107).

0.35 - 2022-09-16
=================
  - Author names now use PDB style ("Lastname, A.B.") by default rather
    than PubMed style ("Lastname AB") (#95).
  - Asyms containing multiple water molecules should now be correctly
    handled (previously every water molecule in the output ``atom_site``
    table  was given the same ``auth_seq_id``). Use the new
    :class:`ihm.WaterAsymUnit` to create an asym containing waters (#98).
  - Masses for all elements are now included, so that
    ``_entity.formula_weight`` can be correctly populated for ligands (#99).
  - Bugfix: :class:`ihm.analysis.Analysis` objects are now read correctly
    from input files when two objects share the same ID but are part
    of different protocols (#101).

0.34 - 2022-08-03
=================
  - Strings that start with STAR reserved words such as ``stop_`` are now
    quoted to help some readers such as the GEMMI library
    (ihmwg/python-modelcif#25).
  - If an input file defines a chemical descriptor with an empty name
    but also defines ``linker_type``, use that to fill in the name (#91).
  - :class:`ihm.ChemComp` now allows for chemical components to be defined
    in a chemical component dictionary (CCD) outside of the wwPDB CCD. This
    is not used in python-ihm itself but can be used in python-modelcif.
  - Bugfix: if a read mmCIF file defines a complete assembly, do not overwrite
    its name and description on output (#92).
  - Bugfix: only allow clustering methods/features that are supported by
    the underlying IHM dictionary for :class:`ihm.model.Ensemble` (#94).
  - Bugfix: categories such as ``_struct`` that are not typically looped
    now support multi-line strings (ihmwg/python-modelcif#27).

0.33 - 2022-06-27
=================
  - Improve reading of mmCIF files with incomplete data (#86, #87) or with
    categories in an unexpected order (#85).
  - Bugfix: fix sanity check for multiple atoms with the same atom_id and
    seq_id to handle bulk water (where such duplicates are OK) (#88).

0.32 - 2022-05-31
=================
  - :class:`ihm.protocol.Step` now takes an ``ensemble`` flag, to indicate
    whether the modeling involved an ensemble, and which defaults to True if
    the system contains at least one :class:`ihm.model.Ensemble` (#83).
  - When reading an incomplete mmCIF file, such as that generated by some
    versions of PyMOL, python-ihm will now fill in missing entity-related
    information by guessing the sequence from the atom_site table (#67).
  - Bugfix: :class:`ihm.flr.RefMeasurementGroup` objects are now read
    from mmCIF files correctly.

0.31 - 2022-04-14
=================
  - The :class:`ihm.dumper.IgnoreVariant` class can now be used to exclude
    selected categories from the mmCIF/BinaryCIF output.
  - The _pdbx_nonpoly_scheme CIF table should now fully comply with the
    PDBx dictionary.
  - Atoms are now checked at file-output time to ensure that a given model
    chain does not contain multiple atoms with the same atom_id and
    seq_id (#81).

0.30 - 2022-04-05
=================
  - Add support for a long description of the system (like an abstract)
    using struct.pdbx_model_details (#80).
  - Bugfix: correctly read mmCIF files with missing entity.type.

0.29 - 2022-04-01
=================
  - Output mmCIF files containing non-polymers should now validate against
    the PDBx dictionary (#76).
  - Bugfix: non-polymers that are erroneously marked as polymers in
    the input mmCIF can now be read in without causing a Python
    exception (#78).
  - Bugfix: strings starting with an underscore (e.g. chain names) are now
    quoted in mmCIF output to conform to the CIF syntax (#75).

0.28 - 2022-03-21
=================
  - :class:`ihm.Citation` now takes a ``is_primary`` argument, which can
    be used to denote the most pertinent publication for the modeling.
  - Improved support for non-standard residues, and for standard amino acids
    used as nonpolymers.

0.27 - 2022-01-27
=================
  - Minor documentation improvements.
  - Add support for the _struct.pdbx_structure_determination_methodology
    mmCIF data item.

0.26 - 2022-01-12
=================
  - :func:`ihm.dumper.write` and :func:`ihm.reader.read` both now take
    a ``variant`` argument which can be used to control the set of tables
    that are read/written. This can be used by other libraries (such as
    python-ma) to support other mmCIF extensions.

0.25 - 2021-12-03
=================
  - :func:`ihm.dictionary.Dictionary.validate` will now report errors for
    any keywords or categories in the file that are not present in the
    dictionary.
  - :class:`ihm.LPeptideAlphabet` now supports the ASX and GLX ambiguous
    residue types.

0.24 - 2021-12-01
=================
  - :class:`ihm.AsymUnit` now supports insertion codes in its
    ``auth_seq_id_map``. The target of this mapping can either be an
    author-provided residue number (as previously) or a 2-element tuple
    containing this number and an insertion code.
  - :class:`ihm.AsymUnit` now allows the PDB or author-provided strand/chain ID
    to be different from the regular ID.
  - Bugfix: if two :class:`ihm.dictionary.Dictionary` objects both contain
    information about a given category, adding the two dictionaries together
    now combines the category information, rather than just using that from
    one dictionary.
  - Bugfix: :class:`ihm.dictionary.Dictionary` should now be able to validate
    BinaryCIF files containing integer or float values (#66).

0.23 - 2021-11-01
=================
  - Bugfix: _struct_ref.pdbx_seq_one_letter_code is now treated as the subset
    of the reference (e.g. UniProt) sequence that overlaps with our Entities,
    not the entire sequence (#64).

0.22 - 2021-10-22
=================
  - The :class:`ihm.Software` class now allows a citation for the software
    to be provided.
  - A new :mod:`ihm.citations` module contains citations for some packages
    that are commonly used in integrative modeling.

0.21 - 2021-07-14
=================
  - BinaryCIF files now use UTF8 msgpack strings for all text, rather than
    raw bytes. This should make python-ihm's BinaryCIF files interoperable
    with those used by, e.g., CoordinateServer.
  - Output mmCIF files now include author-provided numbering (auth_seq_id)
    for atoms in the atom_site table. This should help packages that don't
    read the pdbx_poly_seq_scheme table to show the desired residue
    numbering (#61).

0.20 - 2021-05-06
=================
  - Support for Python 2.6 has been dropped. The library needs Python 2.7
    or Python 3.
  - Bugfix: correctly read in multiline reference sequence one-letter codes.
  - Bugfix: the reader is now more tolerant of omitted or unknown values
    (. or ?) in input mmCIF files.

0.19 - 2021-04-16
=================
  - A convenience class is added to describe datasets stored in the
    ProXL database (:class:`ihm.location.ProXLLocation`).

0.18 - 2020-11-06
=================
  - Update to match latest FLR dictionary.
  - Add a simple utility (util/make-mmcif.py) to make a minimal compliant
    IHM mmCIF file, given an mmCIF file (potentially just coordinates) as input.
  - Bugfix: the full residue range spanned by a starting model is now reported,
    rather than just the subset that is mapped to one or more templates (#55).
  - Bugfix: handle TrEMBL UniProt sequences (#57).

0.17 - 2020-07-10
=================
  - Convenience classes are added to describe hydrogen/deuterium exchange
    data (:class:`ihm.dataset.HDXDataset`) and datasets stored in the
    PDB-Dev database (:class:`ihm.location.PDBDevLocation`).
  - Multiple :class:`ihm.restraint.CrossLinkPseudoSite` objects can now
    be assigned to a given :class:`ihm.restraint.CrossLink`.
  - Bugfix: the :class:`ihm.dataset.Dataset` base class now has a type
    of "Other" rather than "unspecified" to conform with the latest
    IHM dictionary.

0.16 - 2020-05-29
=================
  - :func:`ihm.reader.read` no longer discards models read from non-IHM mmCIF
    files; they are instead placed in their own :class:`ihm.model.ModelGroup`.
  - Bugfix: both the pure Python and C-accelerated mmCIF readers are now more
    robust, able to handle files in binary mode (e.g. from opening a URL)
    and in Unicode (mmCIF files are supposed to be ASCII but python-ihm should
    handle any encoding Python supports).

0.15 - 2020-04-14
=================
  - :class:`ihm.dataset.Dataset` objects that derive from another dataset
    can now record any transformation involved; see
    :class:`ihm.dataset.TransformedDataset`.
  - :class:`ihm.metadata.PDBParser` now extracts basic metadata from
    PDB files generated by SWISS-MODEL.
  - An :class:`ihm.Entity` can now be linked to one or more reference databases
    (e.g. UniProt). See the classes in the :mod:`ihm.reference` module.

0.14 - 2020-02-26
=================
 - A cross-link can now use pseudo sites to represent one or both ends of the
   link. The new :class:`ihm.restraint.CrossLinkPseudoSite` object is used
   when the end of the cross-link is not represented in the model but its
   position is known (e.g. it may have been approximated given the position
   of nearby residues).
 - :class:`ihm.restraint.PseudoSiteFeature` now references an underlying
   :class:`ihm.restraint.PseudoSite`, allowing a single pseudo site to be
   shared between a feature and a cross-link if desired.
 - :class:`ihm.model.Ensemble` now supports describing subsamples from which
   the ensemble was constructed; see :class:`ihm.model.Subsample`.
 - Bugfix: :meth:`ihm.Citation.from_pubmed_id` now works correctly when the
   journal volume or page range are empty, or the page "range" is just a
   single page.

0.13 - 2019-11-14
=================
 - :func:`ihm.reader.read` has a new optional ``reject_old_file`` argument.
   If set, it will raise an exception if asked to read a file that conforms
   to too old a version of the IHM extension dictionary.
 - Definitions for the DHSO and BMSO cross-linkers are now provided in the
   :mod:`ihm.cross_linkers` module.

0.12 - 2019-10-16
=================
 - :class:`ihm.restraint.ResidueFeature` objects can now act on one or
   more :class:`Residue` objects, which act equivalently to
   1-residue ranges (:class:`AsymUnitRange` or :class:`EntityRange`).
 - The new :class:`ihm.dataset.GeneticInteractionsDataset` class and the
   ``mic_value`` argument to :class:`ihm.restraint.DerivedDistanceRestraint`
   can be used to represent restraints from genetic interactions, such as
   point-mutant epistatic miniarray profile (pE-MAP) data.

0.11 - 2019-09-05
=================
 - :class:`ihm.Assembly` objects can now only contain :class:`AsymUnit`
   and :class:`AsymUnitRange` objects (not :class:`Entity` or
   :class:`EntityRange`).
 - Bugfix: ensembles that don't reference a :class:`ihm.model.ModelGroup`
   no longer cause the reader to create bogus empty model groups.

0.10 - 2019-07-09
=================
 - Features (:class:`ihm.restraint.AtomFeature`,
   :class:`ihm.restraint.ResidueFeature`, and
   :class:`ihm.restraint.NonPolyFeature`), which previously could select part
   or all of an :class:`ihm.AsymUnit`, can now also select parts of an
   :class:`Entity`. A restraint acting on an entity-feature is assumed
   to apply to all instances of that entity.

0.9 - 2019-05-31
================
 - Add support for the latest version of the IHM dictionary.

0.8 - 2019-05-28
================
 - :func:`ihm.reader.read` can now be asked to warn if it encounters
   categories or keywords in the mmCIF or BinaryCIF file that it doesn't
   know about (and will ignore).
 - Predicted contacts (:class:`ihm.restraint.PredictedContactRestraint`)
   are now supported.
 - :func:`ihm.reader.read` will now read starting model coordinates and
   sequence difference information into the
   :class:`ihm.startmodel.StartingModel` class. Applications that don't require
   coordinates can instruct the reader to ignore them with the new
   `read_starting_model_coord` flag.
 - The new :mod:`ihm.flr` module allows for information from
   Fluorescence / FRET experiments to be stored. This follows the definitions
   in the `FLR dictionary <https://github.com/ihmwg/FLR-dictionary/>`_.

0.7 - 2019-04-24
================
 - Authors of the mmCIF file itself (`_audit_author` category) can now be
   set by manipulating :attr:`ihm.System.authors`. (If this list is empty on
   output, the set of all citation authors is used instead, as before.)
 - Any grants that supported the modeling can now be listed in
   :attr:`ihm.System.grants`.
 - A copy of `SWIG <https://www.swig.org/>`_ is no longer needed to install
   releases of python-ihm via `pip` as pre-generated SWIG outputs are
   included in the PyPI package. SWIG is still needed to build directly
   from source code though.

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
 - Bugfix: the model's representation (see :mod:`ihm.representation`)
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

