"""pdb reader"""

import IMP
import modeller.scripts

def copy_residue(r, model):
    """Copy residue information from modeller to imp"""
    #print "residue "+str(r)
    p=IMP.Particle()
    model.add_particle(p)
    hp= IMP.MolecularHierarchyDecorator.create(p)
    rp= IMP.ResidueDecorator.create(p)
    rp.set_type(IMP.ResidueType(r.name))
    rp.set_index(r.index)
    if rp.get_is_amino_acid():
        hp.set_type(IMP.MolecularHierarchyDecorator.RESIDUE)
    elif rp.get_is_nucleic_acid():
        hp.set_type(IMP.MolecularHierarchyDecorator.NUCLEICACID)
    else:
        hp.set_type(IMP.MolecularHierrchyDecorator.MOLECULE)
    IMP.NameDecorator.create(p).set_name(str("residue "+r.num));
    return p


def copy_atom(a, model):
    """Copy atom information from modeller"""
    #print "atom "+str(a)
    p=IMP.Particle()
    model.add_particle(p)
    ap= IMP.AtomDecorator.create(p)
    ap.set_x(a.x)
    ap.set_y(a.y)
    ap.set_z(a.z)
    hp= IMP.MolecularHierarchyDecorator.create(p)
    hp.set_type(IMP.MolecularHierarchyDecorator.ATOM)
    ap.set_type(IMP.AtomType(a.name))
    #IMP.NameDecorator.create(p).set_name(str("atom "+a._atom__get_num()));
    if (a.charge != 0):
        ap.set_charge(a.charge)
    if (a.mass != 0):
        ap.set_mass(a.mass)
    return p


def copy_bonds(pdb, atoms, model):
    for b in pdb.bonds:
        maa= b[0]
        mab= b[1]
        pa=atoms[maa.index]
        pb=atoms[mab.index]
        if IMP.BondedDecorator.is_instance_of(pa):
            ba= IMP.BondedDecorator.cast(pa)
        else:
            ba= IMP.BondedDecorator.create(pa)
        if IMP.BondedDecorator.is_instance_of(pb):
            bb= IMP.BondedDecorator.cast(pb)
        else:
            bb= IMP.BondedDecorator.create(pb)
        bp= IMP.bond(ba, bb, IMP.BondDecorator.COVALENT)

def read_pdb(name, model, special_patches=None):
    """Construct a MolecularHierarchyDecorator from a pdb file.
       The highest level hierarchy node is a PROTEIN. `special_patches`
       can be a function that applies patches (e.g. nucleic acid termini)
       to the Modeller model."""
    e = modeller.environ()
    e.libs.topology.read('${LIB}/top_heav.lib')
    e.libs.parameters.read('${LIB}/par.lib')
    e.io.hetatm=True
    pdb = modeller.scripts.complete_pdb(e, name,
                                        special_patches=special_patches)
    pp= IMP.Particle()
    model.add_particle(pp)
    hpp= IMP.MolecularHierarchyDecorator.create(pp)
    hpp.set_type(IMP.MolecularHierarchyDecorator.PROTEIN)
    IMP.NameDecorator.create(pp).set_name(name)
    atoms={}
    for chain in pdb.chains:
        cp=IMP.Particle()
        model.add_particle(cp)
        hcp= IMP.MolecularHierarchyDecorator.create(cp)
        # We don't really know the type yet
        hcp.set_type(IMP.MolecularHierarchyDecorator.FRAGMENT)
        hpp.add_child(hcp)
        IMP.NameDecorator.create(cp).set_name(chain.name)
        for residue in chain.residues:
            rp= copy_residue(residue, model)
            hrp= IMP.MolecularHierarchyDecorator.cast(rp)
            hcp.add_child(hrp)
            for atom in residue.atoms:
                ap= copy_atom(atom, model)
                hap= IMP.MolecularHierarchyDecorator.cast(ap)
                hrp.add_child(hap)
                atoms[atom.index]=ap
            lastres=hrp
        # set the type for real
        if lastres.get_type() == IMP.MolecularHierarchyDecorator.RESIDUE:
            hcp.set_type(IMP.MolecularHierarchyDecorator.CHAIN)
        elif lastres.get_type() ==\
                IMP.MolecularHierarchyDecorator.NUCLEICACID:
            hcp.set_type(IMP.MolecularHierarchyDecorator.NUCLEOTIDE)
        else:
            hcp.set_type(IMP.MolecularHierarchyDecorator.MOLECULE)
    copy_bonds(pdb,atoms, model)
    return hpp
