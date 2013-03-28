
import IMP
import IMP.atom as atom
import IMP.algebra as alg
import IMP.core as core
import sys
import logging
from IMP.em2d.argminmax import keymax
log = logging.getLogger("buildxlinks")

import networkx as nx

try:
    set = set
except NameError:
    from sets import Set as set




class Xlink:
    """
        Class defining a cross-link
    """

    def __init__(self, id1, chain1, residue1,
                       id2, chain2, residue2,
                       distance ):
        """
            Initialize the class
            @param id1 Id of the first component of the cross-link
            @param chain1 Chain of the first component
            @param residue1 Residue cross-linked in the first component
            @param id2   Id of the second component of the cross-link
            @param chain2   chain1 Chain of the second component
            @param residue2   Residue cross-linked in the second component
            @param distance Maximum distance
        """
        self.first_id = id1
        self.first_chain = chain1
        self.first_residue = residue1
        self.second_id = id2
        self.second_chain = chain2
        self.second_residue = residue2
        self.distance = distance

    def clone(self):
        xl = Xlink( self.first_id, self.first_chain, self.first_residue,
                    self.second_id, self.second_chain, self.second_residue,
                    self.distance)
        return xl

    def __eq__(self, other):
        if self.first_id != other.first_id or \
           self.second_id != other.second_id or \
           self.first_chain != other.first_chain or \
           self.second_chain != other.second_chain or \
           self.first_residue != other.first_residue or \
           self.second_residue != other.second_residue or \
           abs(other.distance-self.distance) > 0.01:
            return False
        return True

    def swap(self):
        """
            swaps the order of the residues in the restraint
            @param
        """
        self.first_id, self.second_id = self.second_id, self.first_id
        self.first_residue, self.second_residue = \
                                    self.second_residue, self.first_residue
        self.first_chain, self.second_chain = \
                                self.second_chain, self.first_chain


    def show(self):
        s = "Cross Link: %s %s %d - %s %s %d. Distance %f" % (self.first_id,
                                self.first_chain, self.first_residue,
                                self.second_id, self.second_chain,
                                self.second_residue, self.distance )
        return s

    def get_name(self):
        """
            Generate a unique name for the restraint.
            @note: The name cannot start with a number, upsets sqlite
        """
        name = "cl_%s_%s%d_%s_%s%d" % (self.first_id,  self.first_chain,
                                  self.first_residue, self.second_id,
                                  self.second_chain, self.second_residue)
        return name



def build_xlinks_graph(xlinks_dict):
    """
        Build a set of nodes and edges from the set of crosslinking
        restraints.
        @param xlinks_dict a XlinksDict class

    """
    subunits =  set()
    edges = []
    for key in xlinks_dict.keys():
        subunits.add(key[0])
        subunits.add(key[1])
        edge = [key[0],key[1]]
        edge.sort()
        if edge not in edges:
            edges.append(edge)
    log.debug("Subunits %s", subunits)
    log.debug("Edges %s", edges)
    return subunits, edges

class XlinksDict(dict):
    """
        Description of crosslinking restraints as a python
        dictionary.
        The keys are a pair with the ids of the cross-linked subunits.
        Note: The pairs are considered in alphabetic order
    """
    def add(self, xlink):
        """
            Add a xlink. It is ensured that the id of the first element is
            is lower that the second
            @param
        """
        if xlink.second_id < xlink.first_id:
            xlink.swap()
        key = (xlink.first_id, xlink.second_id)
        if key not in self.keys():
            self[key] = []
        self[key].append(xlink)

    def get_xlinks_for_pair(self, pair_ids):
        """
            @param pair_ids Ids fo the subunits that are cross-linked
        """
        try:
            xlinks_list = self[pair_ids]
            ys = [xl.clone() for xl in xlinks_list]

        except KeyError:
            try:
                xlinks_list = self[(pair_ids[1], pair_ids[0])]
                ys = [xl.clone() for xl in xlinks_list]
                # Ensure that the first element of the pair corresponds to the
                # first id in the xlink
                for y in ys:
                    y.swap()
            except KeyError,e:
                raise e
        return ys



        """
        try:
            return self[pair_ids]
        except KeyError:
            print pair_ids,"NOT FOUND, swapping to",(pair_ids[1], pair_ids[0])
            # If not found, invert the pair
            xlinks_list = self[(pair_ids[1], pair_ids[0])]
            # swap the xlinks, so the first element of the pair corresponds to the
            # first id in the xlink
            for xl in xlinks_list:
                xl.swap()
            for xl in xlinks_list:
                print xl.show()
            return xlinks_list
        """

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

    def set_xlinks(self, xlinks_dict):
        """
            Sets the xlinks used for computing the docking order
            @param xlinks_dict XlinksDict class
        """
        subunits, edges = build_xlinks_graph(xlinks_dict)
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
        self.xlinks_list = []

    def set_xlinks(self, xlinks_list):
        """
            Sets the xlinks used for the computation fo the intial rough
            docking solution
            @param xlinks_list A list of Xlink classes
            residue1 belongs to the receptor and residue2 belongs to the ligand
        """
        self.xlinks_list = xlinks_list

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
            Movest the ligand close to the receptor based on the xlinks
            provided by set_xlinks()
        """
        log.debug("Moving ligand close to the receptor using the xlinks")
        n = len(self.xlinks_list)
        if n == 1:
            self.move_one_xlink()
        elif n >= 2:
            self.move_xlinks()

    def get_residue_particle(self, h, ch, res):
        """
            Get the particle representing a residue in a hierarchy
            @param h atom.Hierarchy containing the residue
            @param ch The chain id
            @param res index of the residue
        """
        s=IMP.atom.Selection(h, chain=ch, residue_index=res)
        return s.get_selected_particles()[0]

    def get_residue_coordinates(self, h, ch, res):
        """
            Get the coordinates for a residue in a molecular hierarchy
            @param h atom.Hierarchy object
            @param ch The chain id
            @param res Residue index
        """
        p = self.get_residue_particle(h, ch, res)
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
        xl = self.xlinks_list[0]
        center = self.get_residue_coordinates(self.h_receptor, xl.first_chain,
                                                xl.first_residue)
        sph = alg.Sphere3D(center, xl.distance)
        v = alg.get_random_vector_in(sph)
        ref = self.rb_ligand.get_reference_frame()
        coords = ref.get_transformation_to().get_translation()
        R = ref.get_transformation_to().get_rotation()
        lig = self.get_residue_coordinates(self.h_ligand, xl.second_chain,
                                                          xl.second_residue)
        log.debug("Ligand residue before moving %s", lig)
        displacement = v - lig
        T = alg.Transformation3D(R, coords + displacement)
        self.rb_ligand.set_reference_frame(alg.ReferenceFrame3D(T))
        new_coords = self.get_residue_coordinates(self.h_ligand, xl.second_chain,
                                                          xl.second_residue)
        log.debug("ligand after moving %s", new_coords)


    def move_xlinks(self, ):
        """
            Function equivalent to move_one_xlink() for the case where there
            are more than one cross-link restraints available.
            Puts the ligand residues as close as possible to the receptor
            residues
        """
        rec_coords = []
        lig_coords = []
        for xl in self.xlinks_list:
            c = self.get_residue_coordinates(self.h_receptor, xl.first_chain,
                                                          xl.first_residue)
            rec_coords.append(c)
            c = self.get_residue_coordinates(self.h_ligand, xl.second_chain,
                                                          xl.second_residue)
            lig_coords.append(c)
        log.debug( "Receptor residues before moving %s", rec_coords)
        log.debug( "Ligand residues before moving %s", lig_coords)
        ref = self.rb_ligand.get_reference_frame()
        Tr = alg.get_transformation_aligning_first_to_second(lig_coords,
                                                             rec_coords)
        T = ref.get_transformation_to()
        newT = alg.compose(Tr, T)
        self.rb_ligand.set_reference_frame(alg.ReferenceFrame3D(newT))

        moved_lig_coords = []
        for xl in self.xlinks_list:
            c = self.get_residue_coordinates(self.h_ligand, xl.second_chain,
                                                          xl.second_residue)
            moved_lig_coords.append(c)
        log.debug( "Ligand residues after moving %s", moved_lig_coords)
