<?php
  include "functions.inc";
  write_header("index");
?>

<h2>Purpose of IMP</h2>

<p><img class="figure" style="float:right" src="exp_cycle.png"
        alt="experimental cycle">
</p>

<p>Our broad goal is to contribute to a comprehensive structural
characterization of biomolecules ranging in size and complexity from small
peptides to large macromolecular assemblies, such as the ribosome and the
<a href="/npc.html">nuclear pore complex (NPC)</a>.
Detailed structural characterization of assemblies
is generally impossible by any single existing experimental or computational
method. We suggest that this barrier can be overcome by hybrid approaches
that integrate data from diverse biochemical and biophysical experiments
(eg, x-ray crystallography, NMR spectroscopy, electron microscopy,
immuno-electron microscopy, footprinting, chemical cross-linking,
FRET spectroscopy, small angle X-ray scattering, immunoprecipitation,
and genetic interactions). Even a coarse characterization of the configuration
of macromolecular components in a complex (ie, the molecular architecture)
helps to elucidate the principles that underlie cellular processes, in
addition to providing a necessary starting point for a higher resolution
description.</p>

<p><a href="alber_fig_5b.jpg">
<img class="linkfigure" style="float:left" src="alber_fig_5b.jpg" alt="NPC"
     width="434" height="364">
</a></p>

<p>We formulate the hybrid approach to structure determination as an
optimization problem, the solution of which requires three main components:
the representation of the assembly, the scoring function, and the optimization
method. The ensemble of solutions to the optimization problem embodies the
most accurate structural characterization given the available information.
A preliminary version of this approach was used to determine the configuration
of the 456 proteins in the yeast NPC14. The key challenges remain translating
experimental data into restraints on the structure of the assembly, combining
these spatial restraints into a single scoring function, optimizing the
scoring function, and analyzing the resulting ensemble of solutions.</p>

<p>To address these challenges, we are creating an Integrative Modeling
Platform (IMP). IMP is designed to allow mixing and matching of existing
modeling components as well as easy adding of new functionality. It will
support a wide variety of assembly representations and input data. It will
provide infrastructure that encourages and supports contributions from
other laboratories and allows integration with existing modeling packages.</p>

<p style="clear:both">

<?php
  write_footer();
?>
