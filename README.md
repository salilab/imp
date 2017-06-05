Integrative Modeling of the Spindle Pole Body Core

Here we determine the molecular architecture of the yeast spindle pole body (SPB), 
the structural and functional equivalent of the metazoan centrosome. 
Data from in vivo FRET and yeast two-hybrid, along with SAXS, X-ray crystallography, 
and electron microscopy were integrated by a Bayesian modeling approach. 

This module provides classes, functions and executables for running the sampling, analysis, clustering
and density calculation for this project.

We provide a number of command line executables :

# spb {#spb_bin}
This is the main executable for running sampling, based on the FRET and yeast two-hybrid restraints. 
It should be compiled with MPI since it runs Replica Exchange sampling.
It reads an input file where all the parameters of the modeling are specified and writes several output files:
- log. Once per replica, with information about the model score and ISD particle values for each
  model written to rmf file.
- traj.rmf. Once per replica, where the model coordinates are saved
- trajisd.rmf. Once per replica, where the ISD particles are saved

The code expects the following files in the directory from which it is executed:
config.ini (input configuration file), 3OA7_A.pdb, 3OA7_B.pdb, 4DS7_Cmd1_swapped.pdb, 4DS7_Spc110_swapped.pdb, 
CC_120_A.pdb, CC_120_A.pdb, CC_78_A.pdb, CC_78_B.pdb 
(these are needed to create the represantation of the few components with X-ray structure),
fret_2014.dat, fret_new_exp.dat (FRET_R exp data).

After reading the input file, the code creates all the particles and, if specified in input,
initializes their values from the last frame of a rmf file (useful for restarting from a previous modeling run)
Then it adds all the restraints specified in the input file and starts the sampling part.
Output is written every # steps specified in the input file.

# spb_analysis {#spb_analysis_bin}
After running sampling, we need to rescore the final ensemble (all models at 1 K) based on the EM2D score. 
This step performs the rescoring and attaches a weight to each model (higher the better) based on the Well-tempered ensemble Replica Exchange scheme. 

# spb_cluster {#spb_cluster_bin}
Models are clustered taking their weight into account. Distance threshold-based clustering is performed with a 1.5 nm threshold. 

# spb_density {#spb_density_bin}
Localization density is calculated per protein for a given cluster.

# spb_density_perbead {#spb_density_perbead_bin}
To better localize the protein termini, localization density is calculated per bead per protein for a given cluster.

# spb_test_score {#spb_test_score_bin}
Code to output the restraint values of a model. Takes 2 RMFs as input: one containing structural coordinations and another containing  ISD values. 

# Info

_Author(s)_: Massimiliano Bonomi, Shruthi Viswanath

_Maintainer_: `shruthivis`

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
- M. Bonomi, S. Viswanath, S.J. Kim et. al., Molecular Architecture of the Spindle Body Core determined by an Integrative Bayesian approach, submitted.  
- See [main IMP papers list](@ref publications).
