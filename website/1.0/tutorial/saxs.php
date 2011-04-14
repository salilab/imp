<?php
   include("misc.inc.php");
   print_page_header();
?>
<div id="tutorial">

<h1>Integration of comparative modeling, X-ray crystallography, and SAXS</h1>

<p>The Nup84 complex structure determined <a href="restrainer.html">in the
previous section</a> is consistent with all input information, but for a
detailed understanding of its function, an accurate atomic structure is
required.  Two possible routes to such a structure, depending on the
available information, are (i) fitting atomic structures of the individual
protein subunits into a cryo-EM map of the assembly and (ii) accurately
placing pairs of subunits relative to each other using X-ray crystallography
or molecular docking. For both routes, atomic structures of the subunits
are required; these structures can be obtained via X-ray crystallography
or comparative modeling.</p>

<p>One component of the Nup84 complex is the Nup133 protein; the structure
of this protein has been characterized by both X-ray crystallography and
small angle X-ray (SAXS). SAXS differs from X-ray crystallography in that
it is applied to proteins in solution rather than crystals; thus, it can
be applied to a much wider range of proteins in states more closely
resembling their functional forms than X-ray crystallography, but the
information is rotationally averaged and so the resulting SAXS profile
gives less structural information. IMP contains a method that, given an
atomic protein structure, can calculate its SAXS profile using the Debye
formula, and then fit this profile against the experimentally determined
one. This method is implemented in the IMP
<a href="../doc/html/namespaceIMP_1_1saxs.html">saxs module</a> and so can
be used by writing a suitable Python script.
However, because fitting against a SAXS profile is a common task, we provide
an IMP application, FoXS, which automates this process. FoXS is available
both as a command-line IMP application and
<a href="http://salilab.org/foxs/">a web service</a>.</p>

<p>All input files for this demonstration are available in
<a href="saxs.zip">this zipfile</a>.
The structure of the C-terminal domain of yeast Nup133 is available in the
<a href="http://www.pdb.org/">RCSB Protein Data Bank (PDB)</a> as code
<a href="http://www.pdb.org/pdb/explore/explore.do?structureId=3KFO">3kfo</a>
(file <span class="filename">3KFO.pdb</span>), while the experimental
SAXS profile is given in the
<span class="filename">23922_merge.dat</span> file. The atomic structure
can be fit against the SAXS profile by running FoXS in the directory
containing both files:</p>

<p><tt>foxs 3KFO.pdb 23922_merge.dat</tt></p>

<p>Alternatively, the two files can be submitted to the
<a href="http://salilab.org/foxs/">FoXS web server</a>. FoXS compares the
theoretical profile of the provided structure (solid line in the image
below) with the experimental profile (points), and calculates the quality
of the fit, χ, with smaller values corresponding to closer fits:</p>

<p class="textfigure">
<img class="textfigure" src="images/saxs-fit.png"
     alt="SAXS fit" />
</p>

<p>The fit in this example is not a good one (χ=2.96). To understand why
this is so, we examine the header of the 3kfo PDB file, which reveals two
problems. Several residues at the N and C termini were not resolved in
the X-ray experiment (8 in total, 2 at the N terminus and 6 at the C
terminus), and the sidechains of 16 other residues could also not be
located (REMARK 465 and REMARK 470 lines).</p>

<p>The missing 8 residues and 16 sidechains need to be placed to create a
complete atomic structure. One way to achieve this goal is to build a
comparative model using a package such as
<a href="http://salilab.org/modeller/">MODELLER</a> relying on the original
3kfo structure as a template and the full sequence (including the 8 missing
N and C terminal residues) as the target. The corresponding MODELLER
alignment file (<span class="filename">3KFO-fill.ali</span>) and script file
(<span class="filename">fill.py</span>) are provided in the
<a href="saxs.zip">downloaded zipfile</a>. Each candidate comparative model
can be fitted against the SAXS profile using the FoXS command-line application
or the web service in exactly the same way as the original 3kfo structure;
the best MODELLER model gives a significantly improved fit between the
theoretical and experimental profiles (dashed line in the above plot;
χ=1.21).</p>

<p>Given similar atomic structures of the subunits in the Nup84 complex,
as either crystal structures or comparative models,
<a href="restrainer.html">restrainer</a> can be used to build an atomic model
of the complex. Note, however, that an accurate model of such a complex
would require additional information beyond the proteomics data used above,
since yeast two-hybrid data only show that proteins interact, not the
specific residues in the protein-protein interaction, and thus do not
inform us about the relative orientations of the interacting proteins.
Such information can be obtained, for example, from chemical-crosslinking,
molecular docking, or cryo-EM maps, as illustrated in the
<a href="multifit.html">next section</a>.</p>

<p><a href="multifit.html">Next chapter</a>; <a href="./">Tutorial index</a>.</p>

</div>
<?php
   print_page_footer();
?>
