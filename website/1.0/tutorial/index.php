<?php
   include("misc.inc.php");
   print_page_header();
?>
<div id="tutorial">

<h1>Tutorial</h1>

<p>
This tutorial aims to give a simple introduction to the IMP 1.0 software
itself and some specific applications. It cannot cover all parts of IMP in
detail; for more information, please refer to the <a href="../doc/html/">IMP
manual</a>.
</p>

<p>
In order to follow along with the tutorial, you will first need to
<a href="../../download.html">download and install IMP 1.0</a>.
Please note that if you choose to use a later version of IMP, some scripts may
need some modification to work correctly.</p>

<ol>
<li><a href="intro.html">Introduction.</a><br />
Introduction to the problems that IMP is designed to solve, and the high-level
design of the software.
</li>

<li><a href="library.html">The IMP C++/Python library.</a><br />
Simple usage of basic parts of the IMP library.
</li>

<li><a href="restrainer.html">The restrainer high-level interface.</a><br />
Demonstration of the IMP.restrainer module for generating a structure of a
subcomplex of the Nuclear Pore Complex.
</li>

<li><a href="saxs.html">Simple integrative example.</a><br />
A simple example of the use of comparative modeling, X-ray crystallography,
and small angle X-ray data.
</li>

<li><a href="multifit.html">MultiFit.</a><br />
Demonstration of the MultiFit method, which determines macromolecular
assembly structures by fitting multiple structures into an electron density
map, on the ARP2/3 complex.
</li>

</ol>

</div>
<?php
   print_page_footer();
?>
