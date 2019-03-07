Classes, functions and executables for modeling of the Spindle Pole Body

# Integrative Modeling of the Spindle Pole Body Core

Here we determine the molecular architecture of the yeast spindle pole body (SPB), 
the structural and functional equivalent of the metazoan centrosome. 
Data from in vivo FRET and yeast two-hybrid, along with SAXS, X-ray crystallography, 
and electron microscopy were integrated by a Bayesian structure modeling approach. 

This module provides classes, functions and executables for running the sampling, analysis, clustering
and density calculation for this project.

### Prerequisites: 
[MPI](https://integrativemodeling.org/2.7.0/doc/ref/namespaceIMP_1_1mpi.html) must be used to compile the IMP code so that replica exchange can be used.

### Note:
For detailed information on inputs, outputs and how to run each executable, refer to the README and files in the [SPB biosystems page](https://github.com/integrativemodeling/spb).

Here's a brief description of the command line executables. 

# spb {#spb_bin}
This is the main executable for running sampling using Well-Tempered Ensemble Replica Exchange, based on FRET, yeast two-hybrid and other restraints. 

# spb_analysis {#spb_analysis_bin}
After running sampling, we need to rescore the final ensemble (all models at 1 K) based on the EM2D score. 
This step performs the rescoring and attaches a weight to each model (higher the better) based on the Well-tempered ensemble Replica Exchange method. 

# spb_cluster {#spb_cluster_bin}
Models are clustered taking their weight into account. Distance threshold-based clustering is performed with a 1.5 nm threshold. 

# spb_density {#spb_density_bin}
Localization density is calculated per protein for models of a given cluster.

# spb_density_perbead {#spb_density_perbead_bin}
To better localize the protein termini, localization density is calculated per bead per protein for a given cluster.

# spb_test_score {#spb_test_score_bin}
Code to output the restraint values of a model. Takes 2 RMFs as input: one containing structural coordinations and another containing ISD values. 

# Info

_Author(s)_: Massimiliano Bonomi, Shruthi Viswanath

_Maintainer_: `shruthivis`

_License_: [LGPL](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
- S. Viswanath\*, M. Bonomi\*, S.J. Kim, V.A. Klenchin, K.C. Taylor, K.C. Yabut, N.T. Umbreit, H.A. Van Epps, J. Meehl, M.H. Jones, D. Russel, J.A. Velazquez-Muriel, M. Winey, I. Rayment, T.N. Davis, A. Sali, and E.G. Muller. The molecular architecture of the yeast spindle pole body core determined by Bayesian integrative modeling. Mol Biol Cell 28, 3298-3314, 2017.
- See [main IMP papers list](@ref publications).
