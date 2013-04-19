This is a protocol for pairwise protein docking, in which additional
experimental information about the protein-protein complex is incorporated
into the docking procedure to greatly improve the accuracy of predictions.
This method succeeds in producing a near-native model among the top 10 models
in 42–82% of cases, while state-of-the-art docking methods succeed only in
30–40% of cases, depending on the benchmark and accuracy criterion.

The protocol can currently incorporate data from the following sources:
 - the pair-distance distribution function of the complex from a SAXS profile
 - 2D class average images of the complex from negative-stain EM micrographs
   (EM2D)
 - a 3D density map of the complex from single-particle negative-stain EM
   micrographs (EM3D)
 - residue type content at the protein interface from NMR spectroscopy
   (NMR-RTC)
 - chemical cross-linking detected by mass spectrometry (CXMS).

The protocol proceeds by first sampling complex models using
[PatchDock](http://bioinfo3d.cs.tau.ac.il/) for pairwise protein
docking, followed by
[filtering based on fit to the experimental data](@ref idock_progs),
conformational refinement using
[FiberDock](http://bioinfo3d.cs.tau.ac.il/) and composite scoring.
Third, good-scoring representatives of clusters of models are picked as
final models.

_Examples_:
 - [Docking of PCSK9](@ref idock_pcsk9)

## Info

_Author(s)_: Dina Schneidman

_Maintainer_: `duhovka`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Schneidman-Duhovny D, Hammel M, Sali A. Macromolecular docking restrained by a small angle X-ray scattering profile. J Struct Biol. 2010
 - Schneidman-Duhovny et al. A method for integrative structure determination of protein-protein complexes: application to a therapeutical antibody-antigen complex. submitted
