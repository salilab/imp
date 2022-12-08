## \example atom/charmm_topology.py
# This example shows how to use IMP's CHARMM topology reader to
# build an atomic structure (in this case, alanine dipeptide) from
# primary sequence, and to apply topology patches.
#

from __future__ import print_function
import IMP.atom
import sys

IMP.setup_from_argv(sys.argv, "CHARMM topology")

# Read CHARMM non-hydrogen parameters
ff = IMP.atom.get_heavy_atom_CHARMM_parameters()

# Start building a single segment (chain) topology
st = IMP.atom.CHARMMSegmentTopology()

# Get the topology of an ideal alanine from the CHARMM topology
ideal_ala = ff.get_residue_topology(IMP.atom.ALA)

# Use ideal topology as a template to make the real alanine topology
# ("real" topology can be modified, e.g. by patching)
ala = IMP.atom.CHARMMResidueTopology(ideal_ala)

# Apply ACE and CT3 patches to our topology. Normally a residue can only
# be patched once, so reset the patched flag after the first patch.
ace = ff.get_patch("ACE")
ct3 = ff.get_patch("CT3")
ace.apply(ala)
ala.set_patched(False)
ct3.apply(ala)

# Add the residue and chain to the top-level topology
st.add_residue(ala)
t = IMP.atom.CHARMMTopology(ff)
t.add_segment(st)

# Make a Hierarchy using this topology
m = IMP.Model()
h = t.create_hierarchy(m)
IMP.atom.show(h)
