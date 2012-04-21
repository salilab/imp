
import IMP
import IMP.atom as atom
import IMP.algebra as alg
import IMP.core as core
import logging
from argminmax import keymax
log = logging.getLogger("buildxlinks")

try:
    import networkx as nx
except:
    print "Script requires the Networkx library to run"
    sys.exit()


def build_xlinks_graph(xlinks):
    """
        Build a set of nodes and edges from the set of crosslinking
        restraints.
        @param xlinks a Xlinks class

    """
    subunits =  set()
    edges = []
    for x in xlinks.keys():
        subunits.add(x[0])
        subunits.add(x[1])
        e = [x[0],x[1]]
        e.sort()
        if e not in edges:
            edges.append(e)
    log.debug("Subunits %s", subunits)
    log.debug("Edges %s", edges)
    return subunits, edges

class Xlinks(dict):
    """
        Description of crosslinking restraints as a python
        dictionary with the keys being a pair of subunits names.
        Note: The pairs are considered in alphabetic order
    """
    def add(self, id1, residue1, id2, residue2, distance):
        """
            Add a restraint
            @param id1 name of the first subunit crosslinked
            @param id2 name of the first subunit crosslinked
            @param residue1 Aminoacid number of the residue 1 (first subunit)
            @param residue2 Aminoacid number of the residue 2 (second subunit)
            @param distance Maximum distance for the cross-link
        """
        if id1 < id2:
            p = (id1, id2)
            if p not in self.keys():
                self[p] = []
            self[p].append((residue1, residue2, distance))
        else:
            p = (id2, id1)
            if p not in self.keys():
                self[p] = []
            self[p].append( (residue2,residue1, distance))

    def get_xlinks_for_pair(self, pair_ids):
        """
            @param pair_ids Ids fo the subunits that are cross-linked
        """
        try:
            return self[pair_ids]
        except KeyError:
            # If not found, invert the pair and the residues list
            xlist = self[(pair_ids[1], pair_ids[0])]
            xlist = [(a[1],a[0],a[2]) for a in xlist]
            return xlist

class DockOrder (object):
    """
        Compute the order of the docking experiments. The order is derived
        from the cross-linking restraints:
        1) The subunit with the highest number of cross-linkns with others
           works as the first receptor (A)
        2) All the subunits cross-linked with A are docked into A.
        3) The next receptor (B) is the subunit that has the highest number of
            cross-links after A.
        4) All the subunits cross-linked to B are docked into B (expect A)
        5) The procedure is repeated until there are no more cross-links
    """

    def __init__(self,):
        self.G = nx.Graph()

    def set_components_and_connections(self, subunits, edges):
        """
            Instead of setting the xlink restraints, init the graph directly
            Example of what to pass with the structure in 3sfd
            G.add_nodes_from(["A", "B", "C", "D"])
            G.add_edges_from([("B","C"), ("B","A"), ("B","D"), ("C","D")])
            ("B","C") means that there are cross-links between B and C
        """
        self.G.add_nodes_from(subunits)
        self.G.add_edges_from(edges)

    def set_xlinks(self, xlinks):
        """
            Sets the xlinks used for computing the docking order
            @param xlinks Xlinks class
        """
        subunits, edges = build_xlinks_graph(xlinks)
        self.set_components_and_connections(subunits, edges)

    def get_docking_order(self):
        """ return the order to dock components from the cross links """
        docking_pairs = []
        degs = self.G.degree(self.G.nodes())
        log.debug("Degrees: %s",degs )
        sorted_degrees = [(v,k) for v,k in zip(degs.values(),degs.keys())]
        sorted_degrees.sort()
        sorted_degrees.reverse() # descending order

        receptors_considered = []
        for degree, node in sorted_degrees:
            for n in self.G.neighbors(node):
                if not n in receptors_considered:
                    docking_pairs.append((node, n))
            receptors_considered.append(node)
        log.info("The suggested order for the docking pairs is %s",
                                                            docking_pairs)
        return docking_pairs


class InitialDockingFromXlinks:
    """
        Puts two subunits together using the Xlinkins restraints. The solutions
        offered by this class are just an initial position of the components
        to be fed to HEX
    """
    def clear_xlinks(self):
        self.xlinks = []

    def set_xlinks(self, xl):
        """
            Sets the xlinks used for the computation fo the intial rough
            docking solution
            @param xl a list of (residue1, residue2, distance)
            residue1 belongs to the receptor and residue2 belongs to the ligand
        """
        self.xlinks = xl

    def set_pdbs(self, fn_receptor, fn_ligand):
        """
            Set the name of the PDB files of the receptor and the ligand
            @param fn_receptor
            @param fn_ligand
        """
        sel = atom.ATOMPDBSelector()
        self.m_receptor = IMP.Model()
        self.h_receptor = atom.read_pdb(fn_receptor, self.m_receptor, sel)
        self.m_ligand = IMP.Model()
        self.h_ligand = atom.read_pdb(fn_ligand, self.m_ligand, sel)

    def set_hierarchies(self, h_receptor, h_ligand):
        """
            Set the hierarchies (atom.Hierarchy objects) for the receptor and
            the ligand
            @param h_receptor
            @param h_ligand
        """
        self.h_receptor = h_receptor
        self.h_ligand = h_ligand

    def set_rigid_bodies(self,rb_receptor, rb_ligand):
        """
            Sets the rigid bodies (core.RigidBody objects) for the receptor and
            the ligand
            @param rb_receptor
            @param rb_ligand
        """
        self.rb_receptor = rb_receptor
        self.rb_ligand = rb_ligand

    def move_ligand(self):
        """
            Movest the ligrand close to the receptor based on the xlinks
            provided by set_xlinks()
        """
        log.debug("Moving ligand close to the receptor using the xlinks")
        n = len(self.xlinks)
        if n == 1:
            self.move_one_xlink()
        elif n >= 2:
            self.move_xlinks()

    def get_residue_particle(self, h, res):
        """
            Get the particle representing a residue in a hierarchy
            @param h atom.Hierarchy containing the residue
            @param residue_index index of the residue
        """
        s=IMP.atom.Selection(h, residue_index=res)
        return s.get_selected_particles()[0]

    def get_residue_coordinates(self, h, res):
        """
            Get the coordinates for a residue in a molecular hierarchy
            @param atom.Hierarchy object
            @param res Residue index
        """
        p = self.get_residue_particle(h, res)
        return core.XYZ(p).get_coordinates()

    def write_ligand(self, fn):
        """
            Write a pdb file the coordinates of the ligand
            @param fn
        """
        atom.write_pdb(self.h_ligand, fn)

    def move_one_xlink(self):
        """
            Put the residues in a random distance between 0 and the maximum
            cross-linkin distance
        """
        r1, r2, distance = self.xlinks[0]
        center = self.get_residue_coordinates(self.h_receptor, r1)
        sph = alg.Sphere3D(center, distance)
        v = alg.get_random_vector_in(sph)
        ref = self.rb_ligand.get_reference_frame()
        coords = ref.get_transformation_to().get_translation()
        R = ref.get_transformation_to().get_rotation()
        lig = self.get_residue_coordinates(self.h_ligand, r2)
        log.debug("Ligand residue before moving %s", lig)
        displacement = v - lig
        T = alg.Transformation3D(R, coords + displacement)
        self.rb_ligand.set_reference_frame(alg.ReferenceFrame3D(T))
        log.debug("ligand after moving %s",
                        self.get_residue_coordinates(self.h_ligand,r2))


    def move_xlinks(self, ):
        """
            Function equivalent to move_one_xlink() for the case where there
            are more than one cross-link restraints available.
            Puts the ligand residues as close as possible to the receptor
            residues
        """
        rec_residues = [x[0] for x in self.xlinks]
        lig_residues = [x[1] for x in self.xlinks]
        rec_coords = [self.get_residue_coordinates(self.h_receptor, x)
                                                        for x in rec_residues]
        lig_coords = [self.get_residue_coordinates(self.h_ligand, x)
                                                        for x in lig_residues]
        log.debug( "Receptor residues before moving %s", rec_coords)
        log.debug( "Ligand residues before moving %s", lig_coords)
        ref = self.rb_ligand.get_reference_frame()
        # new coords for the ligand aminoacids
        Tr = alg.get_transformation_aligning_first_to_second(lig_coords,
                                                             rec_coords)
        T = ref.get_transformation_to()
        newT = alg.compose(Tr, T)
        self.rb_ligand.set_reference_frame(alg.ReferenceFrame3D(newT))
        moved_lig_coords = [self.get_residue_coordinates(self.h_ligand, x)
                                                        for x in lig_residues]
        log.debug( "Ligand residues after moving %s", moved_lig_coords)
