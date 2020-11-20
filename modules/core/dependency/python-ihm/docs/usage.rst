Usage
*****

Usage of the library for output consists of first creating a hierarchy of
Python objects that together describe the system, and then dumping that
hierarchy to an mmCIF file.

For a complete worked example, see the
`simple docking example <https://github.com/ihmwg/python-ihm/blob/master/examples/simple-docking.py>`_.

The top level of the hierarchy in IHM is the :class:`ihm.System`. All other
objects are referenced from a System object.

Datasets
========

Any data used anywhere in the modeling (including in validation) can be
referenced with an :class:`ihm.dataset.Dataset`. For example,
electron microscopy data is referenced with
:class:`ihm.dataset.EMDensityDataset` and small angle scattering data with
:class:`ihm.dataset.SASDataset`.

A dataset uses an
:class:`ihm.location.Location` object to describe where it is stored.
Typically this is an :class:`ihm.location.DatabaseLocation` for something
that's deposited in a experiment-specific database such as PDB, EMDB, PRIDE,
or EMPIAR, or :class:`ihm.location.InputFileLocation` for something that's
stored as a simple file, either on the local disk or at a location described
with a DOI such as `Zenodo <https://zenodo.org>`_ or a publication's
supplementary information. See the
`locations example <https://github.com/ihmwg/python-ihm/blob/master/examples/locations.py>`_
for more examples.

System architecture
===================

The architecture of the system is described with a number of classes:

 - :class:`ihm.Entity` describes each unique sequence.
 - :class:`ihm.AsymUnit` describes each asymmetric unit (chain) in the system.
   For example, a homodimer would consist of two asymmetric units, both
   pointing to the same entity, while a heterodimer contains two entities.
   It is also possible for an entity to exist with no asymmetric units pointing
   to it - this typically corresponds to something seen in an experiment (such
   as a cross-linking study) which was not modeled. Note that the IHM
   extension currently contains no support for symmetry, so two chains that
   are symmetrically related should each be represented as an "asymmetric"
   unit.
 - :class:`ihm.Assembly` groups asymmetric units and/or entities, or parts of
   them. Assemblies are used to describe which parts of the system correspond
   to each input source of data, or that were modeled.
 - :class:`ihm.representation.Representation` describes how each part of the
   system was represented in the modeling, for example
   :class:`as atoms <ihm.representation.AtomicSegment>` or
   :class:`as coarse-grained spheres <ihm.representation.FeatureSegment>`.

Restraints and sampling
=======================

Restraints, that score or otherwise fit the computational model against
the input data, can be created as :class:`ihm.restraint.Restraint` objects.
These generally take as input a :class:`~ihm.dataset.Dataset` pointing to
the input data, and an :class:`~ihm.Assembly` describing which part of the
model the data corresponds to. For example, there are restraints for
:class:`3D EM <ihm.restraint.EM3DRestraint>` and
:class:`small angle scattering <ihm.restraint.SASRestraint>`.

:class:`ihm.protocol.Protocol` objects describe how models were generated
from the input data. A protocol can consist of
:class:`multiple steps <ihm.protocol.Step>`, such as molecular dynamics or
Monte Carlo, followed by one or more analyses, such as clustering, filtering,
rescoring, or validation, described by :class:`ihm.analysis.Analysis` objects.
These objects generally take an :class:`~ihm.Assembly` to indicate what part
of the system was considered and a
:class:`group of datasets <ihm.dataset.DatasetGroup>` to show which data
guided the modeling or analysis.

Model coordinates
=================

:class:`ihm.model.Model` objects give the actual coordinates of the final
generated models. These point to the :class:`~ihm.Assembly` of what was
modeled, the :class:`~ihm.protocol.Protocol` describing how the modeling
was done, and the :class:`~ihm.representation.Representation` showing how
the model was represented.

Models can be grouped together for any purpose using the
:class:`ihm.model.ModelGroup` class. If a given group describes an ensemble
of models, the :class:`ihm.model.Ensemble` class allows for additional
information on the ensemble to be provided, such as
:class:`localization densities <ihm.model.LocalizationDensity>` of parts of
the system and precision. Due to size, generally only representative models
of an ensemble are deposited in mmCIF, but the :class:`~ihm.model.Ensemble`
class allows the full ensemble to be referred to, for example in a more
compact binary format (e.g. DCD) deposited at a given DOI. Groups of models
can also be shown as corresponding to different states of the system using
the :class:`ihm.model.State` class.

Metadata
========

Metadata can also be added to the system, such as

 - :class:`ihm.Citation`: publication(s) that describe this modeling or the
   methods used in it.
 - :class:`ihm.Software`: software packages used to process the experimental
   data, generate intermediate inputs, do the modeling itself, and/or
   process the output.
 - :class:`ihm.Grant`: funding support for the modeling.
 - :class:`ihm.reference.UniProtSequence`: information on a sequence used
   in modeling, in UniProt.

Residue numbering
=================

The library keeps track of several numbering schemes to reflect the reality
of the data used in modeling:

 - *Internal numbering*. Residues are always numbered sequentially starting at
   1 in an :class:`~ihm.Entity`. All references to residues or residue ranges in
   the library use this numbering.
 - *Author-provided numbering*. If a different numbering scheme is used by the
   authors, for example to correspond to the numbering of the original sequence
   that is modeled, this can be given as an author-provided numbering for
   one or more asymmetric units. See the ``auth_seq_id_map`` parameter to
   :class:`~ihm.AsymUnit`. (The mapping between author-provided and internal
   numbering is given in the ``pdbx_poly_seq_scheme`` table in the mmCIF file.)
 - *Starting model numbering*. If the initial state of the modeling is given
   by one or more PDB files, the numbering of residues in those files may not
   line up with the internal numbering. In this case an offset from starting
   model numbering to internal numbering can be provided - see the ``offset``
   parameter to :class:`~ihm.startmodel.StartingModel`.

Output
======

Once the hierarchy of classes is complete, it can be freely inspected or
modified. All the classes are simple lightweight Python objects, generally
with the relevant data available as member variables. For example, modeling
packages such as `IMP <https://integrativemodeling.org>`_ will typically
generate an IHM hierarchy from their own internal data models, but in many
cases some information relevant to IHM (such as
the :class:`associated publication <ihm.Citation>`) cannot be determined
automatically and can be filled in by adding more objects to the hierarchy.

The complete hierarchy can be written out to an mmCIF or BinaryCIF file using
the :func:`ihm.dumper.write` function.

Input
=====

Hierarchies of IHM classes can also be read from mmCIF or BinaryCIF files.
This is done using the :func:`ihm.reader.read` function, which returns a list of
:class:`ihm.System` objects.
