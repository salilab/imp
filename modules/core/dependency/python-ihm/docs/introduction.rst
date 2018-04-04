Introduction
************

This package provides a mechanism to describe an integrative modeling
application with a set of Python objects. This includes

 - the data used for the modeling, such as previous computional models
   from comparative or integrative modeling, and experimental datasets from
   X-ray crystallography, mass spectrometry, electron microscopy;
 - the protocol used to generate models, such as molecular dynamics, clustering,
   and rescoring;
 - the actual coordinates of output models, which may be multi-scale (including
   both atomic coordinates and more coarse-grained representations),
   multi-state (multiple conformations and/or compositions of the system needed
   to explain the input data), or ordered (such as different points in a
   chemical reaction);
 - grouping of multiple models into ensembles or clusters;
 - validation of models, for example by scoring against data not used in the
   modeling itself.

Once created, this set of Python objects can be written to an mmCIF file
that is compliant with the
`IHM extension <https://github.com/ihmwg/IHM-dictionary>`_
to the `PDBx/mmCIF dictionary <http://mmcif.wwpdb.org/>`_,
suitable for deposition in the
`PDB-Dev repository <https://pdb-dev.wwpdb.org/>`_. The files are best viewed
in a viewer that supports IHM mmCIF, such as
`UCSF ChimeraX <https://www.cgl.ucsf.edu/chimerax/>`_, although they may be
partially viewable in regular PDBx mmCIF viewers (likely only the atomic
coordinates will be visible).

The Python package can be used standalone, but is primarily intended for use
within modeling software such as `IMP <https://integrativemodeling.org>`_,
or `HADDOCK <https://haddock.science.uu.nl/>`_. For example, IMP provides
`a class <https://integrativemodeling.org/nightly/doc/ref/classIMP_1_1pmi_1_1mmcif_1_1ProtocolOutput.html>`_
which uses this library to convert an IMP::pmi modeling protocol into an mmCIF
file.
