## \example add_bonds.py
# Show how to add bonds to a file.
from __future__ import print_function
import RMF


def copy_to_residues(pin, pout, rcf, rf, pcf, pf, acf, af, ccf, cf, bf, prev):
    if rcf.get_is(pin):
        rpin = rcf.get(pin)
        rpout = rf.get(pout)
        rpout.set_residue_type(rpin.get_residue_type())
        rpout.set_residue_index(rpin.get_residue_index())
        mass = 0
        # provide a default for nonstandard residues
        ca = pin.get_children()[0]
        for c in pin.get_children():
            # there is a hetatom
            if acf.get_is(c) and c.get_name().find("CA") != -1:
                ca = c
            mass += pcf.get(c).get_mass()
        ppout = pf.get(pout)
        pca = pcf.get(ca)
        ppout.set_coordinates(pca.get_coordinates())
        ppout.set_mass(mass)
        ppout.set_radius(3)  # garbage value
        if prev:
            bd = bf.get(pout.add_child("bond", RMF.BOND))
            bd.set_bonded_0(prev.get_id().get_index())
            bd.set_bonded_1(pout.get_id().get_index())
        prev = pout
    else:
        if ccf.get_is(pin):
            cf.get(pout).set_chain_id(ccf.get(pin).get_chain_id())
            prev = None
        for inch in pin.get_children():
            if inch.get_type() == RMF.REPRESENTATION:
                outch = pout.add_child(inch.get_name(), inch.get_type())
                prev = copy_to_residues(
                    inch,
                    outch,
                    rcf,
                    rf,
                    pcf,
                    pf,
                    acf,
                    af,
                    ccf,
                    cf,
                    bf,
                    prev)
    return prev
fh = RMF.open_rmf_file_read_only(RMF.get_example_path("3U7W.rmf"))
fh.set_current_frame(RMF.FrameID(0))

outpath = RMF._get_temporary_file_path("3U7W-residues.rmf")
out = RMF.create_rmf_file(outpath)
out.add_frame("nothing", RMF.FRAME)
print("file is", outpath)
bf = RMF.BondFactory(out)

rcf = RMF.ResidueFactory(fh)
rf = RMF.ResidueFactory(out)
pcf = RMF.ParticleFactory(fh)
pf = RMF.ParticleFactory(out)
acf = RMF.AtomFactory(fh)
af = RMF.AtomFactory(out)
ccf = RMF.ChainFactory(fh)
cf = RMF.ChainFactory(out)
bf = RMF.BondFactory(out)
copy_to_residues(
    fh.get_root_node(),
    out.get_root_node(),
    rcf,
    rf,
    pcf,
    pf,
    acf,
    af,
    ccf,
    cf,
    bf,
    None)

RMF.show_hierarchy_with_values(out.get_root_node())
RMF.show_hierarchy_with_decorators(out.get_root_node())
