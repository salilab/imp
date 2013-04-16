The protocol includes a number of tools:
<pre>
\par saxs_score: Scoring with SAXS profile
Usage: \<pdb1\> \<pdb2\> \<trans_file\> \<exp_profile_file\>
saxs_score program scores results of docking of two molecules against the SAXS profile file.

\<pdb1\> - The first molecule that was given to docking program and was kept in place during docking
\<pdb2\> - The second molecule that was given to docking program and was transformed
\<trans_file\> - Docking output file with transformations that bring the transformed molecule to the static molecule.
  The format is as follows:

    1 -0.75020 0.67870 0.34160 82.62000 59.28000 -1.01100
    2 -0.42560 0.55540 0.33110 75.69000 67.15000 -0.30480
    3 -0.48910 0.66950 0.11820 84.94000 58.97000 3.34300

  The first number is the transformation number followed by three rotational and three translational parameters.

\<exp_profile_file\> - The SAXS profile of the complex. see http://modbase.compbio.ucsf.edu/foxs/help.html for format details.


\par em2d_score: Scoring with EM 2D class averages
Usage: \<pdb1\> \<pdb2\> \<trans_file\> \<image1\> \<image2\>...
em2d_score program scores results of docking of two molecules against 2D class averages from EM.

\<pdb1\> \<pdb2\> \<trans_file\> as above
\<image1\> \<image2\>... - complex 2D class averages in PGM format (P2)


\par em3d_score: Scoring with EM 3D density map
Usage: \<pdb1\> \<pdb2\> \<trans_file\> \<em_map\>
em3d_score program scores results of docking of two molecules against 3D density map.

\<pdb1\> \<pdb2\> \<trans_file\> as above
\<em_map\> - density map of the complex in mrc format


\par nmr_rtc_score: Scoring with NMR residue type content term (NMR-RTC)
Usage: \<pdb1\> \<pdb2\> \<trans_file\> \<residue_content_file1\> \<residue_content_file2\>
nmr_rtc_score program scores results of docking of two molecules against NMR residue type content.
NMR residue type content data provides us with information about residue composition at the interface.
For example, we can know that the interface has 2 arginine and 3 histidine residues.

\<pdb1\> \<pdb2\> \<trans_file\> as above
\<residue_content_file1\> - residue content file for pdb1 or '-' if none
\<residue_content_file2\> - residue content file for pdb2 or '-' if none
  The format of the file is as follows:

MET 0
ARG 0
HIS 2

  This would correspond to having 0 methionine, 0 arginine and two histidine residues in the interface.

\par cross_links_score: Scoring with cross links detected by mass spectrometry
Usage: \<pdb1\> \<pdb2\> \<trans_file\> \<cross_links_file\>
cross_links_score program scores results of docking of two molecules against cross linking dataset.

\<pdb1\> \<pdb2\> \<trans_file\> as above
\<cross_links_file\> - file with detected cross links

  The format of the file is as follows:
145 - 1 A 3 27
224 - 1 A 3 27
169 - 1 A 3 27
222 - 1 A 3 27

Each line corresponds to one cross link. Each cross link is defined by residue numbers and chain ids of the cross linked residues, followed by limits on the distance between CA atoms.

'145 - 1 A 3 27' corresponds to a cross link between residue 145 in a chain with blank chain id and residue 1 in chain A. The minimal distance is 3A and the maximal is 27A.

\par combine_scores: Combine scores for different data types
Usage: combine_scores \<file1\> \<weight1\> \<file2\> \<weight2\> \<file3\> \<weight3\>...
combines files with same docking transformations in the following format:

# | score | filtered (+/-) | z-score | ... | Transformation

The programs for scoring of docking transformations (saxs_score, em2d_score, em3d_score, nmr_rtc_score) output results in this format.
</pre>
Only filtered values of + are combined. The output score is the weighted sum of Z-Scores.

_Author(s)_: Dina Schneidman

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - Schneidman-Duhovny D, Hammel M, Sali A. Macromolecular docking restrained by a small angle X-ray scattering profile. J Struct Biol. 2010
 - Schneidman-Duhovny et al. A method for integrative structure determination of protein-protein complexes: application to a therapeutical antibody-antigen complex. submitted
