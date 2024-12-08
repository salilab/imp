Viewing {#viewing}
=======

# Overview #

[TOC]

There are currently several molecular viewers that support RMFs:

# Chimera # {#chimera}

The [Chimera](https://www.cgl.ucsf.edu/chimera/) research group has
provided support for loading and interacting with RMF files. Chimera
supports
- interactive display of restraints and restraint scores
- geometry

While support for RMF was added to the Chimera 1.9 stable release, only the
nightly builds of Chimera work with the very latest RMF features. So if Chimera
is unable to read an RMF file, first try updating to the latest nightly build.

# ChimeraX # {#chimerax}

A plugin to add support for RMF to
[ChimeraX](https://www.rbvi.ucsf.edu/chimerax/) is available as part of
the [ChimeraX Toolshed](https://cxtoolshed.rbvi.ucsf.edu/apps/chimeraxrmf).
It requires a recent version of ChimeraX (>= 0.91) or a nightly build.

# VMD # {#vmd}

We provide a plugin for
[VMD](http://www.ks.uiuc.edu/Research/vmd/). Instructions for how to
install it can be found on the [VMD plugin page](\ref vmdplugin). The
plugin supports
- multiresolution modules using the RMF::decorator::Alternatives decorator
- multiple states stored using the RMF::decorator::State decorator
- fast loading
- displaying restraints as bonds
- static geometry

# Pymol # {#pymol}

We provide a plugin for Pymol. Instruction can be found at the
[Pymol plugin page](\ref pymolplugin). The plugin supports creating
multiple molecules from one RMF based on chains or RMF::decorator::Molecule
labels as well as dynamics geometry.
