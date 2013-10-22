
import IMP
import IMP.atom as atom
import IMP.core as core
import IMP.algebra as alg
import IMP.display as display
import IMP.container as container
import string
from math import pi
import logging

log = logging.getLogger("representation")

#########################################
"""

    Functions to deal with the representation of assemblies and managing
    rigid bodies.

"""
#########################################

def create_assembly_from_pdb(model, fn_pdb, names=False):
    """
        Builds the assembly setting the chains in the PDB file as components
    """
    temp = read_component(model, fn_pdb)
    hchains = atom.get_by_type(temp, atom.CHAIN_TYPE)
    ids = [atom.Chain(h).get_id() for h in hchains]
    log.debug("Creating assembly from pdb %s,names: %s. Chains %s",
                                                        fn_pdb, names, ids)
    atom.add_radii(temp)
    if(names):
        for i, h in enumerate(hchains):
            h.set_name(names[i])
    assembly = IMP.atom.Molecule.setup_particle(temp)
    return assembly


def create_assembly(model, fn_pdbs, names=False):
    """ Read all the PDBs given in the list of names fn_pdbs and adds the
        hierarchies to the model
    """
    assembly = IMP.atom.Molecule.setup_particle(IMP.kernel.Particle(model))
    for i, fn_pdb in enumerate(fn_pdbs):
        if(names):
            prot = read_component(model, fn_pdb, names[i])
        else:
            prot = read_component(model, fn_pdb)
        assembly.add_child(prot)
    return assembly


def read_component(model,fn_pdb, name=False):
    """ Read a PDB molecule, add atoms, and set a name
    """
    if name:
        log.debug("reading component %s from %s", name, fn_pdb)
    else:
        log.debug("reading component from %s", fn_pdb)

    hierarchy =  atom.read_pdb(fn_pdb, model,
                                  atom.NonWaterNonHydrogenPDBSelector())
    if name:
        hierarchy.set_name(name)
    atom.add_radii(hierarchy)
    return hierarchy


def create_rigid_bodies(assembly):
    """ set the children of a molecule type hierarchy as rigid bodies
        In this case, all the children are the components of the complex.
        I use the function create_rigid_body(), that creates a lot of
        sub-rigid bodies.

        I have changed the function and now build the rigid body directly from
        the leaves of each of the components. With this I guarantee that the
        number of rigid members is going to be the same if the components have
        the same number of atoms.
    """
    molecule = assembly.get_as_molecule()
    if(not molecule.get_is_valid(True)):
        raise TypeError("create_rigid_bodies(): The argument is not a valid "\
                        "hierarchy")
    rbs = []
    for c in molecule.get_children():
        p = IMP.kernel.Particle(c.get_model())
        core.RigidBody.setup_particle(p, atom.get_leaves(c))
        rb = core.RigidBody(p)
#        rb = atom.create_rigid_body(c)
        rb.set_name( get_rb_name( c.get_name() ) )
        rbs.append(rb)
    return rbs

def rename_chains(assembly):
    """ Rename all the chains of an assembly so there are no conflicts with
        the ids. The names are added sequentially.
    """
    m = assembly.get_as_molecule()
    if(not m.get_is_valid(True)):
        raise TypeError("The argument is not a valid  hierarchy")
    all_chains_as_hierarchies= get_all_chains(m.get_children())
    letters = string.ascii_uppercase
    n_chains = len(all_chains_as_hierarchies)
    if(len(letters) < n_chains):
        raise ValueError("There are more chains than letter ids")
    ids = letters[0:n_chains]
    for h, c_id in zip(all_chains_as_hierarchies, ids):
        chain = atom.Chain( h.get_particle() )
        chain.set_id(c_id)
        chain.set_name("chain %s" % c_id)


def create_simplified_dna(dna_hierarchy, n_res):
    """ Gets a hierarchy containing a molecule of DNA and simplifies it,
        generating a coarse representation of spheres. The function returns
        a hierarchy with the spheres.
        n_res - Number of residues to use per sphere.
    """
    chain = dna_hierarchy.get_as_chain()
    if(not chain.get_is_valid(True)):
        raise TypeError("create_simplified_dna: the hierarchy provided is not a "\
                        "chain.")

    model = dna_hierarchy.get_model()
    ph = IMP.kernel.Particle(model)
    simplified_h = atom.Hierarchy.setup_particle(ph)
    atom.Chain.setup_particle(ph,"0")

    residues = atom.get_by_type(dna_hierarchy , atom.RESIDUE_TYPE)
    l =len(residues)
    # print "the DNA has ",l,"residues"
    for i in range(0, l, n_res):
        xyzrs = []
        equivalent_mass = 0.0
        residues_numbers = []
        for r in residues[i : i+n_res]:
            rr = atom.Residue(r)
            residues_numbers.append( rr.get_index())
            #print "residue",rr.get_name(),rr.get_index()
            residue_xyzrs = [ core.XYZ(a.get_particle()) for a in rr.get_children()]
            xyzrs += residue_xyzrs
#            print "residue",r,"mass",get_residue_mass(r)
            equivalent_mass += get_residue_mass(r)

        s = core.get_enclosing_sphere(xyzrs)
        p = IMP.kernel.Particle(model)
        xyzr = core.XYZR.setup_particle(p)
        xyzr.set_radius(s.get_radius() )
        xyzr.set_coordinates(s.get_center())
        fragment = atom.Fragment.setup_particle(p)
        fragment.set_residue_indexes(residues_numbers)
        atom.Mass.setup_particle(p, equivalent_mass)
        simplified_h.add_child(fragment)
    simplified_h.set_name("DNA")
#    print "simplified_h is valid:",simplified_h.get_is_valid(True)
    return simplified_h


def get_residue_mass(residue):
    r = residue.get_as_residue()
    if(r.get_is_valid(True) == False):
        raise TypeError("The argument is not a residue")
    mass = 0.0
    for l in atom.get_leaves(r):
        ms = atom.Mass(l)
        mass += ms.get_residue_mass()
    return mass


def create_simplified_assembly(assembly, components_rbs, n_res):
    """ Simplifies an assembly, by creating a hierarchy with one ball per
        n_res residues. Each of the chains in the new hierarchy are added to
        the rigid bodies for each of the components.
        There must be correspondence between the children of the assembly
        (components) and the rigid bodies. I check for the ids.
    """
    molecule = assembly.get_as_molecule()
    if(not molecule.get_is_valid(True)):
        raise TypeError( "The argument is not a valid  hierarchy")

    model = assembly.get_model()
    n_children = molecule.get_number_of_children()

    sh = IMP.kernel.Particle(model)
    simplified_hierarchy = atom.Molecule.setup_particle(sh)

    for i in range(n_children): # for all members of the assembly
        component = molecule.get_child(i)
        name = component.get_name()
        rb = components_rbs[i]
        if( rb.get_name() !=  get_rb_name(name)):
            raise ValueError("Rigid body and component do not match")

        hchains = atom.get_by_type(component, atom.CHAIN_TYPE)
        ch = IMP.kernel.Particle(model)
        coarse_component_h = atom.Molecule.setup_particle(ch)
        # simplify all the chains in the member
        for h in hchains:
            chain = atom.Chain(h.get_particle())
            coarse_h = None
            if(name == "DNA"):
            # print "simplifying DNA"
                coarse_h_particle = create_simplified_dna(h, n_res)
                coarse_h = atom.Hierarchy(coarse_h_particle)
            else:
                coarse_h = atom.create_simplified_along_backbone(chain, n_res)

            chain_rb = atom.create_rigid_body(coarse_h) # does not work for DNA
            # chain_rb = atom.setup_as_rigid_body(coarse_h) # works with DNA
            chain_rb.set_name("sub_rb"+name)
            rb.add_member(chain_rb)

            # are required to have excluded volume
            coarse_component_h.add_child( atom.Chain(coarse_h) )
        coarse_component_h.set_name(name)
        simplified_hierarchy.add_child(coarse_component_h)
    return  simplified_hierarchy



def get_component(assembly, name):
    """ Select a component of the assembly using the name """
    for c in assembly.get_children():
        if (c.get_name() == name):
            return c
    raise ValueError("The requested component %s is not in the assembly" % name)


def get_rigid_body(rigid_bodies, name):
    """ Select a rigid body from the rigid_bodies using the name """
    for rb in rigid_bodies:
        if (rb.get_name() == name):
            return rb
    raise ValueError("This rigid body is not in the set: %s" % name)


def get_rb_name(name):
    """ Name to use for the rigid body of a hierarch"""
    return "rb_"+name


def get_selection_rigid_body(model, S):
    """ Build the rigid body for all the particles in the selection S """
    ps = S.get_selected_particles()
    xyzrs = [core.XYZR(p) for p in ps]
    p_rbS = IMP.kernel.Particle(model)
    rbS = core.RigidBody.setup_particle(p_rbS,xyzrs)
    return rbS

def get_selection_as_hierarchy(model,S):
    ph = IMP.kernel.Particle(model)
    h = core.Hierarchy.setup_particle(ph)
    for p in S.get_selected_particles():
        x = core.Hierarchy.setup_particle(p)
        h.add_child(x)
    return h

def get_selection_as_atom_hierarchy(model,S):
    """ Gets a selection of particles and decorates them as Atoms.
        Then all of them are put into a big residue. I have this to use
        with the multifit.create_coarse_molecule_from_molecule() function
    """
    ph = IMP.kernel.Particle(model)
    h = atom.Residue.setup_particle(ph)
    for p in S.get_selected_particles():
        h.add_child( atom.Atom(p) )
    return h

def get_coarse_selection(coarse_h, residues_numbers):
    """ The function returns the particles (fragments) in the coarse hierarchy
        that were created by summarizing the residues_numbers.

        Coarse hierarchy - Hierarchy formed by a bunch of
        fragments. Each fragment must have the residue numbers that it contains
        residue_numbers - list with the number of the residues that need to
        be recovered.
        The function returns the set of particles that are atom.Fragments
    """
    particles = []
    fragments =  atom.get_by_type(coarse_h, atom.FRAGMENT_TYPE)
    for f in fragments:
        ff = atom.Fragment(f)
        residues_in_f = ff.get_residue_indexes()
        for number in residues_in_f:
            if number in residues_numbers:
                particles.append( ff.get_particle() )
                break
    return particles

def apply_rotation_around_centroid(rb,rot):
    """
        Rotates the reference frame of a rigid body around the centroid
    """
    c = rb.get_coordinates()
    ref = rb.get_reference_frame()
    R = ref.get_transformation_to().get_rotation()
    R2 = alg.compose(rot, R)
    T = alg.Transformation3D(R2, c)
    ref = alg.ReferenceFrame3D(T)
    rb.set_reference_frame(ref)

def apply_transformation_around_centroid(rb, T):
    """
        Aplies a transformation around the centroid of a rigid body.
        First does the rotation around the centroid and
        then applies the transformation.
        @param rb A IMP.core.RigidBody object
        @param T a IMP.algebra.Transformation3D object
    """
    apply_rotation_around_centroid(rb, T.get_rotation())
    rb.set_coordinates( rb.get_coordinates() + T.get_translation())


def get_residue_particle(h, chain_id=False, res=1):
    """
        Get the particle for a residue in a hierarchy.
        @param h The hierarchy
        @param chain_id If chain_id == False, just search for the residue
        @param res Number of residue in the chain
    """
#    log.debug("get_residue_particle: chain_id %s, res %s",chain_id, res)
    if(chain_id):
        s=IMP.atom.Selection(h, chain=chain_id, residue_index=res)
    else:
        s=IMP.atom.Selection(h, residue_index=res)
    return s.get_selected_particles()[0]

def get_residue_coordinates(h, chain_id=False, res=1):
    """
        Get the coordinates of a residue (the coordinates of the first particle)
        @param h Hierarchy
        @param chain See help for get_residue_particle()
        @param res See help for get_residue_particle()
    """
    p = get_residue_particle(h, chain_id, res)
    return core.XYZ(p).get_coordinates()


def get_residues_distance(hierarchy1, chain_id1, residue1,
                          hierarchy2, chain_id2, residue2):
    """
        Distance between two residues. See the help for get_residue_particle()
        @param hierarchy1
        @param chain_id1
        @param residue1
        @param hierarchy2
        @param chain_id2
        @param residue2
    """
    coords1 = get_residue_coordinates(hierarchy1, chain_id1, residue1)
    coords2 = get_residue_coordinates(hierarchy2, chain_id2, residue2)
    d = alg.get_distance(coords1, coords2)
    return d


def get_all_chains(hierarchies):
    """ Gets all the chains in a set of hierarchies
        @param hierarchies A set of IMP.atom.Hierarchy objects
    """
    chains=[]
    for h in hierarchies:
        chains_in_h = atom.get_by_type(h, atom.CHAIN_TYPE)
        for ch in chains_in_h:
            chains.append(ch.get_as_chain())
    return chains

def set_reference_frames(rigid_bodies, reference_frames):
    for ref, rb  in zip(reference_frames,rigid_bodies):
        rb.set_reference_frame(ref)

def get_nucleic_acid_backbone(hierarchy, backbone='minimal'):
    """
        Returns the atoms in the backbone of the nucleic acid contained
        in the hierarchy.
        backbone 'minimal' returns the atoms: ["P", "O5'", "C5'", "C4'", "C3'", "O3'"]
        backbone 'trace' returns the atoms C4'
    """
#    log.debug("get_nucleic_acid_backbone")
    backbone_atoms = []
    if backbone == 'minimal':
        backbone_atoms = ["P", "O5'", "C5'", "C4'", "C3'", "O3'"]
    elif backbone == 'trace':
        backbone_atoms = ["C4'"]
    else:
        raise ValueError("Wrong value for the type of backbone")
    backbone_atom_types = [atom.AtomType(t) for t in backbone_atoms]
    h_chains = atom.get_by_type(hierarchy, atom.CHAIN_TYPE)
    backbone = []
    if len(h_chains) > 1:
        raise ValueError("The hierarchy mas more than one chain")
    h_residues = atom.get_by_type(hierarchy, atom.RESIDUE_TYPE)
    for hr in h_residues:
        res = atom.Residue(hr)
        if not (res.get_is_dna() or res.get_is_rna()):
            raise ValueError("Residue is not part of a nucleic acid")
        h_atoms = atom.get_by_type(hr, atom.ATOM_TYPE)
        for at in h_atoms:
            if atom.Atom(at).get_atom_type() in backbone_atom_types:
                backbone.append(at)
    return backbone


def get_calphas(chain_hierarchy):
    h_residues = atom.get_by_type(chain_hierarchy, atom.RESIDUE_TYPE)
    cas = [atom.get_atom(atom.Residue(r), atom.AtomType("CA"))
                                                        for r in h_residues]
    return cas

def get_backbone(hierarchy):
    """
        Get the backbone atoms for a hierarchy. It can be a protein or a
        nucleic acid
    """
    h_residues = atom.get_by_type(hierarchy, atom.RESIDUE_TYPE)
    if len(h_residues) == 0:
        raise ValueError("No residues!")
    atoms = []
    res = atom.Residue(h_residues[0])
    if res.get_is_dna() or res.get_is_rna():
        atoms = get_nucleic_acid_backbone(hierarchy, 'trace')
    else:
        atoms = get_calphas(hierarchy)
    return atoms

def get_all_members(rigid_bodies):
    """
        Gets all the members of a set of rigid bodies, removing the subrigid
        bodies. Returns all the plain atom or bead members
    """

    members = []
    for rb  in rigid_bodies:
        members += get_simple_members(rb)
    return members


def get_simple_members(rb):
    # Add members if they are not sub-rigid bodies
    members = filter(
       lambda m: not core.RigidBody.get_is_setup(m.get_particle()),
       rb.get_members() )
    return members
