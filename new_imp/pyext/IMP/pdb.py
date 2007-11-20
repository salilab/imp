"""pdb reader"""

import IMP
import modeller.scripts

def copy_residue(r, model):
    """Copy residue information from modeller to imp"""
    #print "residue "+str(r)
    p=IMP.Particle()
    model.add_particle(p)
    hp= IMP.MolecularHierarchyDecorator.create(p)
    hp.set_type(IMP.MolecularHierarchyDecorator.RESIDUE)
    rp= IMP.ResidueDecorator.create(p)
    rp.set_type(IMP.ResidueType(r._residue__get_name()))
    rp.set_index(int(r._residue__get_num()))
    IMP.NameDecorator.create(p).set_name(str("residue "+r._residue__get_num()));
    return p


def copy_atom(a, model):
    """Copy atom information from modeller"""
    #print "atom "+str(a)
    p=IMP.Particle()
    model.add_particle(p)
    ap= IMP.AtomDecorator.create(p)
    ap.set_x(a._atom__get_x())
    ap.set_y(a._atom__get_y())
    ap.set_z(a._atom__get_z())
    hp= IMP.MolecularHierarchyDecorator.create(p)
    hp.set_type(IMP.MolecularHierarchyDecorator.ATOM)
    ap.set_type(IMP.AtomType(a._atom__get_type()._AtomType__get_name()))
    #IMP.NameDecorator.create(p).set_name(str("atom "+a._atom__get_num()));
    if (a._atom__get_charge() != 0):
        ap.set_charge(a._atom__get_charge())
    return p


def copy_bonds(pdb, atoms, model):
    for b in pdb.bonds:
        maa= b[0]
        mab= b[1]
        pa=atoms[maa._atom__get_index()]
        pb=atoms[mab._atom__get_index()]
        ba= IMP.BondedDecorator.create(pa);
        bb= IMP.BondedDecorator.create(pb);
        bp= IMP.bond(ba, bb, IMP.BondDecorator.COVALENT)

def read_pdb(name, model):
    """Construct a hierarchy from a pdb file"""
    e = modeller.environ()
    e.libs.topology.read('${LIB}/top_heav.lib')
    e.libs.parameters.read('${LIB}/par.lib')
    pdb = modeller.scripts.complete_pdb(e, name)
    residues={}
    pp= IMP.Particle()
    model.add_particle(pp)
    hpp= IMP.MolecularHierarchyDecorator.create(pp)
    hpp.set_type(IMP.MolecularHierarchyDecorator.PROTEIN)
    IMP.NameDecorator.create(pp).set_name(name)
    #IMP.show_molecular(hpp)
    atoms={}
    for a in pdb.atoms:
        ap= copy_atom(a, model)
        atoms[a._atom__get_index()]=ap
        r= a._atom__get_residue()
        hp= IMP.MolecularHierarchyDecorator.cast(ap)
        r= a._atom__get_residue()
        ri=r._residue__get_num()
        if (ri not in residues):
            #print str(residues.keys())
            rp= copy_residue(r, model)
            residues[ri]=rp
            hrp= IMP.MolecularHierarchyDecorator.cast(rp)
            hpp.add_child(hrp)
            #print "Post residue"
            #IMP.show_molecular(hpp)
        hrp = IMP.MolecularHierarchyDecorator.cast(residues[ri])
        hrp.add_child(hp)
        #print "Post atom"
        #IMP.show_molecular(hpp)
    copy_bonds(pdb,atoms, model)
    return hpp.get_particle()

#m=IMP.Model()
#p=read_pdb("test/pdb/single_protein.pdb", m)


#e = modeller.environ()
#e.libs.topology.read('${LIB}/top_heav.lib')
#e.libs.parameters.read('${LIB}/par.lib')
#pdb = modeller.scripts.complete_pdb(e, "test/pdb/single_protein.pdb")
