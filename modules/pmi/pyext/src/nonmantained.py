"""@namespace IMP.pmi.nonmaintained
   Nonmaintained code.
"""

from __future__ import print_function
import IMP
import IMP.atom

class Rods(object):

    def __init__(self, m):
        self.m = m
        self.hier = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
        self.rigid_bodies = []
        self.floppy_bodies = []
        self.maxtrans_rb = 2.0
        self.maxtrans_fb = 2.0
        self.maxrot_rb = 0.15

    def add_protein(self, name, res):
        (firstres, lastres) = res
        from math import pi, cos, sin
        h = IMP.atom.Molecule.setup_particle(IMP.Particle(self.m))
        h.set_name(name)
        nres = lastres - firstres
        radius = (nres) * 5 / 2 / pi

        for res in range(firstres, lastres):
            alpha = 2 * pi / nres * (res - firstres)
            x = radius * cos(alpha)
            y = radius * sin(alpha)
            p = IMP.Particle(self.m)
            r = IMP.atom.Residue.setup_particle(p, IMP.atom.ALA, res)
            d = IMP.core.XYZR.setup_particle(p, 5.0)
            d.set_coordinates(IMP.algebra.Vector3D((x, y, 0)))
            d.set_coordinates_are_optimized(True)
            h.add_child(r)
        self.hier.add_child(h)

    def get_hierarchy(self):
        return self.hier

    def set_rod(self, chainname, res):
        (firstres, lastres) = res
        prb = IMP.Particle(self.m)
        sel = IMP.atom.Selection(
            self.hier,
            molecule=chainname,
            residue_indexes=list(range(
                firstres,
                lastres + 1)))
        ps = sel.get_selected_particles()
        rb = IMP.core.RigidBody.setup_particle(prb, ps)
        self.rigid_bodies.append(rb)

    def get_particles_to_sample(self):
        # get the list of samplable particles with their type
        # and the mover displacement. Everything wrapped in a dictionary,
        # to be used by samplers modules
        ps = {}
        ps["Floppy_Bodies_Rods"] = (self.floppy_bodies, self.maxtrans_fb)
        ps["Rigid_Bodies_Rods"] = (
            self.rigid_bodies,
            self.maxtrans_rb,
            self.maxrot_rb)
        return ps


class Beads(object):

    def __init__(self, m):
        self.m = m
        self.beads = []
        self.nresidues = 0
        self.hier = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
        self.floppy_bodies = []
        self.maxtrans_fb = 0.2
        self.particle_database = {}

    def add_bead(self, radius, label="None", color=None):

        p = IMP.Particle(self.m)
        p.set_name(label)
        self.particle_database[label] = p
        self.floppy_bodies.append(p)
        # set default coordinates 0,0,0
        d = IMP.core.XYZ.setup_particle(p)
        d = IMP.core.XYZR.setup_particle(p, radius)
        d.set_coordinates_are_optimized(True)
        # a=IMP.atom.Atom.setup_particle(p,IMP.atom.AT_CA)
        # p=IMP.Particle(self.m)
        self.nresidues += 1
        # r=IMP.atom.Residue.setup_particle(p,IMP.atom.ALA,self.nresidues)
        # r.add_child(a)
        # self.hier.add_child(r)
        self.hier.add_child(p)
        if not color is None:
            self.set_color(label, color)
        return self.particle_database[label]

    def set_color(self, label, value):
        p = self.particle_database[label]
        clr = IMP.display.get_rgb_color(value)
        IMP.display.Colored.setup_particle(p, clr)

    def set_floppy_bodies_max_trans(self, maxtrans):
        self.maxtrans_fb = maxtrans

    def get_hierarchy(self):
        return self.hier

    def get_bead(self, label):
        return self.particle_database[label]

    def set_maxtrans_fb(self, maxtrans_fb):
        self.maxtrans_fb = maxtrans_fb

    def get_particles_to_sample(self):
        # get the list of samplable particles with their type
        # and the mover displacement. Everything wrapped in a dictionary,
        # to be used by samplers modules
        ps = {}
        ps["Floppy_Bodies_Beads"] = (self.floppy_bodies, self.maxtrans_fb)
        return ps


class MultipleStates(object):

    def __init__(self, nstates, m):
        global itertools, tools, restraints

        import itertools
        import IMP.pmi.tools as tools
        import IMP.pmi.restraints as restraints

        self.floppy_bodies = []
        self.rigid_bodies = []
        for ncopy in range(nstates):
            self.floppy_bodies.append([])
            self.rigid_bodies.append([])

        self.rigid_bodies_are_sampled = True
        self.floppy_bodies_are_sampled = True
        self.prot = []
        self.refprot = []
        self.prot_lowres = {}
        self.nstates = nstates
        self.label = "None"

        # model decorator list
        self.xyzmodellist = []
        self.xyzreflist = []
        self.maxtrans_rb = 0.15
        self.maxrot_rb = 0.03
        self.maxtrans_fb = 0.15
        self.m = m

    def get_model(self):
        return self.m

    def set_label(self, label):
        self.label = label

    def set_rigid_bodies_are_sampled(self, input=True):
        self.rigid_bodies_are_sampled = input

    def set_floppy_bodies_are_sampled(self, input=True):
        self.floppy_bodies_are_sampled = input

    def get_rigid_bodies(self):
        return self.rigid_bodies

    def set_rigid_bodies_max_trans(self, maxtrans):
        self.maxtrans_rb = maxtrans

    def set_rigid_bodies_max_rot(self, maxrot):
        self.maxrot_rb = maxrot

    def set_floppy_bodies_max_trans(self, maxtrans):
        self.maxtrans_fb = maxtrans

    def get_hierarchies(self):
        return self.prot

    def destroy_residues(self, segments):
        # segments are defined as a list of tuples ex [(res1,res2,chain),....]
        # this function must be called before the rigid body definition!
        for prot in self.prot:
            for segment in segments:
                # rinterval=[(segment[0],segment[1]+1)]
                if (segment[0] == -1 or segment[1] == -1):
                    s = IMP.atom.Selection(prot, chains=segment[2])
                else:
                    s = IMP.atom.Selection(
                        prot,
                        chains=segment[2],
                        residue_indexes=list(range(segment[0],
                                              segment[1] + 1)))
                for p in s.get_selected_particles():
                    if IMP.core.RigidMember.get_is_setup(p):
                        print("MultipleStates: one particle was not destroied because it was a RigidMember.")
                    else:
                        # destroy the residue and the associated atom
                        a = IMP.atom.Atom(p)
                        r = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
                        IMP.atom.destroy(r)
                        # IMP.atom.destroy(a)
                        # IMP.atom.destroy(p)
            IMP.atom.show_molecular_hierarchy(prot)

    def add_residues_to_chains(
        self,
        residuechainlist,
            residue_type=IMP.atom.LYS):
        # add a list of residues to the corresponding list
        # for instance residuechainlist=[(35,"A"),(100,"B")] will add
        # residue 35 to chain A and residue 100 to chain B
        for rc in residuechainlist:

            s = IMP.atom.Selection(
                self.prot[0],
                chains=rc[1],
                residue_index=rc[0],
                atom_type=IMP.atom.AT_CA)

            print(s.get_selected_particles())
            if len(s.get_selected_particles()) == 0:
                for prot in self.prot:
                    print("adding " + str(rc))
                    p = IMP.Particle(self.m)
                    # set default coordinates 0,0,0
                    d = IMP.core.XYZ.setup_particle(p)
                    IMP.core.XYZR.setup_particle(p, 0.0)
                    d.set_coordinates_are_optimized(True)
                    a = IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
                    p = IMP.Particle(self.m)
                    r = IMP.atom.Residue.setup_particle(p, residue_type, rc[0])
                    r.add_child(a)
                    p = IMP.Particle(self.m)
                    c = IMP.atom.Chain.setup_particle(p, rc[1])
                    c.add_child(r)
                    prot.add_child(c)
                    print(tools.get_residue_index_and_chain_from_particle(a))

            else:
                p = s.get_selected_particles()[0]
                # , tools.get_residue_index_and_chain_from_particle(s.get_selected_particles()[0])
                print(rc, s.get_selected_particles()[0])

            # test that that was indeed added:
            s = IMP.atom.Selection(
                self.prot[0],
                chains=rc[1],
                residue_index=rc[0],
                atom_type=IMP.atom.AT_CA)

            print(s.get_selected_particles())

    def add_beads(self, segments, xyzs=None, radii=None, colors=None):
        '''
        this method generate beads in missing portions.
        The segments argument must be a list of selections
        in the form [(firstres,lastres,chain)]
        each selection will generate a bead
        '''
        if xyzs is None:
            xyzs = []
        if radii is None:
            radii = []
        if colors is None:
            colors = []

        from math import pi

        for n, s in enumerate(segments):
            firstres = s[0]
            lastres = s[1]
            chainid = s[2]
            nres = s[1] - s[0]
            for prot in self.prot:
                for prot in self.prot:
                    cps = IMP.atom.get_by_type(prot, IMP.atom.CHAIN_TYPE)
                    for c in cps:
                        chain = IMP.atom.Chain(c)
                        if chain.get_id() == chainid:
                            p = IMP.Particle(self.m)
                            f = IMP.atom.Fragment.setup_particle(p)
                            rindexes = list(range(firstres, lasteres + 1))
                            f.set_residue_indexes(rindexes)
                            f.set_name(
                                "Fragment_" + '%i-%i' %
                                (firstres, lastres))
                            chain.add_child(f)
                            mass = len(rindexes) * 110.0
                            vol = IMP.atom.get_volume_from_mass(mass)
                            if n + 1 > len(radii):
                                mass = len(rindexes) * 110.0
                                vol = IMP.atom.get_volume_from_mass(mass)
                                radius = (3 * vol / math.pi) ** (1 / 3)
                            else:
                                radius = radii[n]

                            if n + 1 > len(xyzs):
                                x = 0
                                y = 0
                                z = 0
                            else:
                                x = xyzs[n][0]
                                y = xyzs[n][1]
                                z = xyzs[n][2]

                            if n + 1 <= len(colors):
                                clr = IMP.display.get_rgb_color(colors[n])
                                IMP.display.Colored.setup_particle(prt, clr)

                            d = IMP.atom.XYZR.setup_particle(
                                p, IMP.algebra.Sphere3D(x, y, z, radius))

    def renumber_residues(self, chainid, newfirstresiduenumber):
        for prot in self.prot:
            cps = IMP.atom.get_by_type(prot, IMP.atom.CHAIN_TYPE)
            for c in cps:
                if IMP.atom.Chain(c).get_id() == chainid:
                    ps = c.get_children()
                    r = IMP.atom.Residue(ps[0])
                    ri = r.get_index()
                    offs = newfirstresiduenumber - ri
                    for p in ps:
                        r = IMP.atom.Residue(p)
                        ri = r.get_index()
                        r.set_index(ri + offs)

    def destroy_everything_but_the_residues(self, segments):
        # segments are defined as a list of tuples ex [(res1,res2,chain),....]
        for prot in self.prot:
            pstokeep = []
            for segment in segments:

            # rinterval=[(segment[0],segment[1]+1)]
                if (segment[0] == -1 or segment[1] == -1):
                    s = IMP.atom.Selection(prot, chains=segment[2])
                else:
                    s = IMP.atom.Selection(
                        prot,
                        chains=segment[2],
                        residue_indexes=list(range(segment[0],
                                              segment[1] + 1)))
                pstokeep += s.get_selected_particles()

            for p in IMP.atom.get_leaves(prot):
                if p not in pstokeep:
                    if IMP.core.RigidMember.get_is_setup(p):
                        print("MultipleStates: one particle was not destroied because it was a RigidMember.")
                    else:
                        # destroy the residue and the associated atom
                        a = IMP.atom.Atom(p).get_parent()
                        r = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
                        # IMP.atom.destroy(a)
                        IMP.atom.destroy(r)
                        # self.m.remove_particle(p)

    def generate_linkers_restraint_and_floppy_bodies(self, segment):
        '''
        this methods automatically links the particles consecutively
        according to the sequence. The restraint applied is a harmonic upper bound,
        with a distance that is proportional to the number of residues
        in the gap.
        '''
        # this function will create floppy bodies where there are not
        # rigid bodies and moreover create a linker restraint between them
        linker_restraint_objects = []
        for ncopy, prot in enumerate(self.prot):
            if (segment[0] == -1 or segment[1] == -1):
                s = IMP.atom.Selection(prot, chains=segment[2])
            else:
                s = IMP.atom.Selection(
                    prot,
                    chains=segment[2],
                    residue_indexes=list(range(segment[0],
                                          segment[1] + 1)))
            residue_indexes = []
            for p in s.get_selected_particles():

                (r, c) = tools.get_residue_index_and_chain_from_particle(p)

                if IMP.core.RigidMember.get_is_setup(p):
                    Floppy = False
                else:
                    (r, c) = tools.get_residue_index_and_chain_from_particle(p)
                    p.set_name(str(r) + ":" + str(c))
                    tools.set_floppy_body(p)
                    self.floppy_bodies[ncopy].append(p)
                    Floppy = True
                residue_indexes.append((r, Floppy, c, p))

            residue_indexes.sort()

            pruned_residue_list = []
            r0 = residue_indexes[0]
            pruned_residue_list.append(r0)

            # generate the list of residues that define the intervals
            # between rigid bodies and floppy bodies
            for i in range(1, len(residue_indexes)):
                r = residue_indexes[i]
                if r[1] == r0[1] and r[1] == False and IMP.core.RigidMember(r[3]).get_rigid_body() == IMP.core.RigidMember(r0[3]).get_rigid_body():
                    r0 = r
                elif r[1] == r0[1] and r[1] == False and IMP.core.RigidMember(r[3]).get_rigid_body() != IMP.core.RigidMember(r0[3]).get_rigid_body():
                    pruned_residue_list.append(r0)
                    pruned_residue_list.append(r)
                    r0 = r
                elif r[1] != r0[1] and r0[1] == False:
                    pruned_residue_list.append(r0)
                    pruned_residue_list.append(r)
                    r0 = r
                elif r[1] == r0[1] and r0[1]:
                    pruned_residue_list.append(r)
                    r0 = r
                elif r[1] != r0[1] and r[1] == False:
                    pruned_residue_list.append(r)
                    r0 = r

            r0 = pruned_residue_list[0]
            linkdomaindef = []

            for i in range(1, len(pruned_residue_list)):
                r = pruned_residue_list[i]
                if r[1] == r0[1] and r[1] == False and IMP.core.RigidMember(r[3]).get_rigid_body() == IMP.core.RigidMember(r0[3]).get_rigid_body():
                    r0 = r
                else:
                    linkdomaindef.append((r0[0], r[0], r[2]))
                    r0 = r

            print(" creating linker between atoms defined by: " + str(linkdomaindef))

            ld = restraints.LinkDomains(prot, linkdomaindef, 1.0, 3.0)
            ld.set_label(str(ncopy))
            ld.add_to_model()
            linker_restraint_objects.append(ld)
            prs = ld.get_pairs()

        return linker_restraint_objects

    def get_ref_hierarchies(self):
        return self.refprot

    def get_number_of_states(self):
        return self.nstates

    def get_rigid_bodies(self):
        rblist = []
        for rbl in self.rigid_bodies:
            for rb in rbl:
                rblist.append(rb)
        return rblist

    def get_floppy_bodies(self):
        fblist = []
        for fbl in self.floppy_bodies:
            for fb in fbl:
                fblist.append(fb)
        return fblist

    def set_rigid_bodies(self, rigid_body_list):
        if len(self.prot) == 0:
            print("MultipleStates.set_rigid_bodies: hierarchy was not initialized")
            exit()
        for ncopy, prot in enumerate(self.prot):
            rbl = []
            for element in rigid_body_list:
                atoms = []
                for interval in element:
                # rinterval upper bound is incremented by one because the
                # residue_indexes attribute cuts the upper edge
                    # rinterval=[(interval[0],interval[1]+1)]
                    if (interval[0] == -1 or interval[1] == -1):
                        s = IMP.atom.Selection(prot, chains=interval[2])
                    else:
                        s = IMP.atom.Selection(
                            prot,
                            chains=interval[2],
                            residue_indexes=list(range(interval[0],
                                                  interval[1] + 1)))
                    for p in s.get_selected_particles():
                        atoms.append(IMP.core.XYZR(p))

                    # add low resolution representation to the rigid bodies
                    for key in self.prot_lowres:
                        if (interval[0] == -1 or interval[1] == -1):
                            s = IMP.atom.Selection(
                                self.prot_lowres[key][ncopy],
                                chains=interval[2])
                        else:
                            s = IMP.atom.Selection(
                                self.prot_lowres[key][
                                    ncopy], chains=interval[2],
                                residue_indexes=list(range(interval[0], interval[1] + 1)))
                        for p in s.get_selected_particles():
                            atoms.append(IMP.core.XYZR(p))

                if len(atoms) > 0:
                    prb = IMP.Particle(self.m)
                    rb = IMP.core.RigidBody.setup_particle(prb, atoms)
                    rb.set_name(str(element))
                    rbl.append(rb)
                else:
                    print("MultipleStates.set_rigid_bodies: selection " + str(interval) + "  has zero elements")
            self.rigid_bodies[ncopy] += rbl

    def set_floppy_bodies(self, floppy_body_list):
        # define flexible regions within rigid bodies

        if len(self.prot) == 0:
            print("MultipleStates: hierarchy was not initialized")
            exit()

        for ncopy, prot in enumerate(self.prot):
            atoms = []
            for element in floppy_body_list:

                for interval in element:
                # rinterval upper bound is incremented by one because the
                # residue_indexes attribute cuts the upper edge
                    # rinterval=[(interval[0],interval[1]+1)]
                    if (interval[0] == -1 or interval[1] == -1):
                        s = IMP.atom.Selection(prot, chains=interval[2])
                    else:
                        s = IMP.atom.Selection(
                            prot,
                            chains=interval[2],
                            residue_indexes=list(range(interval[0],
                                                  interval[1] + 1)))
                    for p in s.get_selected_particles():
                        (r, c) = tools.get_residue_index_and_chain_from_particle(
                            p)
                        tools.set_floppy_body(p)
                        p.set_name(str(r) + ":" + str(c))
                        atoms.append(IMP.core.XYZR(p))
            self.floppy_bodies[ncopy] += atoms

    def get_particles_to_sample(self):
        # get the list of samplable particles with their type
        # and the mover displacement. Everything wrapped in a dictionary,
        # to be used by samplers modules
        ps = {}
        rblist = self.get_rigid_bodies()
        fblist = self.get_floppy_bodies()
        if self.rigid_bodies_are_sampled:
            ps["Rigid_Bodies_MultipleStates"] = (
                rblist,
                self.maxtrans_rb,
                self.maxrot_rb)
        if self.floppy_bodies_are_sampled:
            ps["Floppy_Bodies_MultipleStates"] = (fblist, self.maxtrans_fb)
        return ps

    def set_hierarchy_from_pdb(self, pdblistoflist):
        "the input is a list of list of pdbs"
        "one list for each copy"
        # eg [["pdb1_copy0","pdb2_copy0"],["pdb1_copy1","pdb2_copy1"]]"
        for copy in range(0, self.nstates):
            prot = self.read_pdbs(pdblistoflist[copy])
            self.prot.append(prot)
            xyz = IMP.core.XYZs(IMP.atom.get_leaves(prot))
            self.xyzmodellist.append(xyz)

    def set_ref_hierarchy_from_pdb(self, pdblistoflist):
        "the input is a list of list of pdbs"
        "one list for each copy"
        # eg [["pdb1_copy0","pdb2_copy0"],["pdb1_copy1","pdb2_copy1"]]"
        for copy in range(0, self.nstates):
            prot = self.read_pdbs(pdblistoflist[copy])
            self.refprot.append(prot)
            xyz = IMP.core.XYZs(IMP.atom.get_leaves(prot))
            self.xyzreflist.append(xyz)

    def read_pdbs(self, list_pdb_file):
        """read pdbs from an external list file
        create a simplified representation
        if the pdbs are given a individual strings, it will read the
        pdbs and give the chain name as specified in the pdb
        If it s a tuple like (filename,chainname) it will read
        the pdb and assing a name chainname
        to the chain"""

        # create an empty hierarchy
        hier = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.m))
        for pdb in list_pdb_file:
            if type(pdb) == str:
                h = IMP.atom.read_pdb(
                    pdb, self.m, IMP.atom.AndPDBSelector(
                        IMP.atom.CAlphaPDBSelector(),
                        IMP.atom.ATOMPDBSelector()))

                '''
                #destroy CA atoms, for the future
                for p in IMP.atom.get_leaves(h):
                    coor=IMP.core.XYZ(p).get_coordinates()
                    r=IMP.atom.Hierarchy(p).get_parent()
                    IMP.core.XYZ.setup_particle(r,coor)
                    IMP.atom.destroy(p)
                '''

                cps = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)

                '''
                #consolidate the chains
                for c in cps:
                    cid=c.get_id()
                    s0=IMP.atom.Selection(hier, chains=cid)
                    try:
                      p=s0.get_selected_particles()[0]
                      re=IMP.atom.Residue(IMP.atom.Atom(p).get_parent()
                      ch=IMP.atom.Chain(re).get_parent())

                    except:
                      continue
                '''

                hier.add_child(h)  # add read chains into hierarchy
            if type(pdb) == tuple:
                h = IMP.atom.read_pdb(
                    pdb[0], self.m, IMP.atom.AndPDBSelector(
                        IMP.atom.CAlphaPDBSelector(),
                        IMP.atom.ATOMPDBSelector()))

                '''
                #destroy CA atoms, for the future
                for p in IMP.atom.get_leaves(h):
                    coor=IMP.core.XYZ(p).get_coordinates()
                    r=IMP.atom.Hierarchy(p).get_parent()
                    IMP.core.XYZ.setup_particle(r,coor)
                    IMP.atom.destroy(p)
                '''

                cps = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
                for cp in cps:
                    IMP.atom.Chain(cp).set_id(pdb[1])
                hier.add_child(h)  # add read chains into hierarchy

        return hier

    def recenter(self, prot):
        "recenter the hierarchy"
        ps = IMP.atom.get_leaves(prot)
        center = IMP.algebra.get_zero_vector_3d()
        for l in ps:
            center += IMP.core.XYZ(l).get_coordinates()
        center /= len(ps)
        for l in ps:
            d = IMP.core.XYZ(l)
            d.set_coordinates(d.get_coordinates() - center)
            d.set_coordinates_are_optimized(True)

        '''
        # bug generating code: keeping it for history

        rb=IMP.atom.create_rigid_body(prot)
        rbcoord=rb.get_coordinates()
        rot=IMP.algebra.get_identity_rotation_3d()
        tmptrans=IMP.algebra.Transformation3D(rot,rbcoord)
        trans=tmptrans.get_inverse()
        IMP.core.transform(rb,trans)
        IMP.core.RigidBody.teardown_particle(rb)
        self.m.remove_particle(rb)
        '''

    def shuffle_configuration(self, bounding_box_length):
        "shuffle configuration, used to restart the optimization"
        "it only works if rigid bodies were initialized"
        if len(self.rigid_bodies) == 0:
            print("MultipleStates: rigid bodies were not intialized")
        hbbl = bounding_box_length / 2
        for rbl in self.rigid_bodies:
            for rb in rbl:
                ub = IMP.algebra.Vector3D(-hbbl, -hbbl, -hbbl)
                lb = IMP.algebra.Vector3D(hbbl, hbbl, hbbl)
                bb = IMP.algebra.BoundingBox3D(ub, lb)
                translation = IMP.algebra.get_random_vector_in(bb)
                rotation = IMP.algebra.get_random_rotation_3d()
                transformation = IMP.algebra.Transformation3D(
                    rotation,
                    translation)
                rb.set_reference_frame(
                    IMP.algebra.ReferenceFrame3D(transformation))

    def generate_simplified_hierarchy(self, nres):
        # generate a new multistate hierarchy
        self.prot_lowres[nres] = []
        for prot in self.prot:
            sh = IMP.atom.create_simplified_along_backbone(prot, nres, False)
            print(IMP.atom.get_leaves(sh))
            # for p in IMP.atom.get_leaves(sh):
            #    IMP.atom.Atom.setup_particle(p,IMP.atom.AT_CA)
            # s=IMP.atom.Selection(sh, chains="A",
            #              residue_index=958)
            # print s.get_selected_particles()[0]
            self.prot_lowres[nres].append(sh)

    def get_simplified_hierarchy(self, nres):
        return self.prot_lowres[nres]

    def calculate_drms(self):
        # calculate DRMSD matrix

        if len(self.xyzmodellist) == 0:
            print("MultipleStates: hierarchies were not intialized")

        if len(self.xyzreflist) == 0:
            print("MultipleStates: reference hierarchies were not intialized")

        drmsd = {}
        for i in range(len(self.xyzreflist)):
            for j in range(len(self.xyzmodellist)):
                try:
                    drmsdval = IMP.atom.get_drmsd(
                        self.xyzmodellist[j],
                        self.xyzreflist[i])
                except:
                    drmsdval = tools.get_drmsd(
                        self.xyzmodellist[j],
                        self.xyzreflist[i])
                drmsd["MultipleStates_DRMSD_" +
                      str(i) + "-Model_" + str(j)] = drmsdval

        # calculate model-template assignment that gives minimum total drmsd
        min_drmsd = []
        for assign in itertools.permutations(list(range(len(self.xyzreflist)))):
            s = 0.
            for i, j in enumerate(assign):
                s += drmsd["MultipleStates_DRMSD_" +
                           str(j) + "-Model_" + str(i)]
            min_drmsd.append(s)

        drmsd["MultipleStates_Total_DRMSD"] = min(min_drmsd)
        return drmsd

    def get_output(self):
        output = {}
        if len(self.refprot) != 0:
            drms = self.calculate_drms()
            output.update(drms)
        output["MultipleStates_Total_Score_" +
               self.label] = str(self.m.evaluate(False))
        return output


#

class LinkDomains(object):

    def __init__(self, prot, resrangelist, kappa, length=5.0):
        # generate a linker between residues using HarmonicUpperBound
        # restraints. Define a list of linked residues,
        # e.g.   [(3,5,"A"),(9,10,"B")]
        # will link residues 3 and 5 belonging to chain A and
        # residues 9 and 10 belonging to chain B
        self.rs = IMP.RestraintSet('linker')
        self.prot = prot
        self.kappa = kappa
        self.resrangelist = resrangelist
        self.length = length
        self.label = "None"

        self.m = self.prot.get_model()
        self.pairs = []

        for pair in self.resrangelist:
            c0 = pair[2]
            r0 = pair[0]
            c1 = pair[2]
            r1 = pair[1]
            try:
                s0 = IMP.atom.Selection(
                    self.prot,
                    chains=c0,
                    residue_index=r0,
                    atom_type=IMP.atom.AT_CA)
                p0 = s0.get_selected_particles()[0]
            except:
                "LinkDomains: error"
                continue
            try:
                s1 = IMP.atom.Selection(
                    self.prot,
                    chains=c1,
                    residue_index=r1,
                    atom_type=IMP.atom.AT_CA)
                p1 = s1.get_selected_particles()[0]
            except:
                "LinkDomains: error"
                continue
            # check this is the residue length (should be 4, but I use a larger
            # length)
            dist0 = float(pair[1] - pair[0]) * self.length
            h = IMP.core.HarmonicUpperBound(dist0, self.kappa)
            dps = IMP.core.DistancePairScore(h)
            pr = IMP.core.PairRestraint(dps, IMP.ParticlePair(p0, p1))
            pr.set_name(
                "LinkDomains_" + str(pair[0]) + "-" + str(pair[1]) + "_" + str(pair[2]))
            self.rs.add_restraint(pr)
            self.pairs.append((p0, p1, r0, c0, r1, c1, pr))

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_pairs(self):
        return self.pairs

    def get_hierarchy(self):
        return self.prot

    def get_kappa(self):
        return self.kappa

    def get_residue_ranges(self):
        return self.resrangelist

    def get_length(self):
        return self.length

    def get_restraint(self):
        return self.rs

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def print_chimera_pseudobonds(self, filesuffix, model=0):
        f = open(filesuffix + ".chimera", "w")
        atype = "ca"
        for p in self.pairs:
            s = "#" + str(model) + ":" + str(p[2]) + "." + p[3] + "@" + atype + \
                " #" + str(model) + ":" + str(p[4]) + "." + p[5] + "@" + atype
            f.write(s + "\n")
        f.close()

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["LinkDomains_" + self.label] = str(score)
        for rst in self.rs.get_restraints():
            # output["LinkDomains_"+
            #        IMP.core.PairRestraint.get_from(rst).get_name()+
            #           "_"+self.label]=IMP.core.PairRestraint.get_from(rst).evaluate(False)
            output["LinkDomains_" + rst.get_name() +
                   "_" + self.label] = rst.unprotected_evaluate(None)

        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            r0 = self.pairs[i][2]
            c0 = self.pairs[i][3]
            r1 = self.pairs[i][4]
            c1 = self.pairs[i][5]

            label = str(r0) + ":" + c0 + "_" + str(r1) + ":" + c1

            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["LinkDomains_Distance_" + label + "_" +
                   self.label] = str(IMP.core.get_distance(d0, d1))

        return output


#


class UpperBound(object):

    def __init__(self, prot, respairs, kappa, length=5.0):
        # generate a restraint between residues using HarmonicUpperBound
        # restraints. Define a list of linked residues,
        # e.g.   [(3,"A",5,"B"),(9,"B",10,"C")]
        # will link residues 3 and 5 belonging to chain A and B and
        # residues 9 and 10 belonging to chain B and C
        self.rs = IMP.RestraintSet('upperbound')

        self.prot = prot
        self.kappa = kappa
        self.respairs = respairs
        self.length = length
        self.label = "None"

        self.m = self.prot.get_model()

        for pair in self.respairs:
            try:
                s0 = IMP.atom.Selection(
                    self.prot,
                    chains=pair[1],
                    residue_index=pair[0],
                    atom_type=IMP.atom.AT_CA)
                p0 = s0.get_selected_particles()[0]
            except:
                "UpperBound: error"
                continue
            try:
                s1 = IMP.atom.Selection(
                    self.prot,
                    chains=pair[3],
                    residue_index=pair[2],
                    atom_type=IMP.atom.AT_CA)
                p1 = s1.get_selected_particles()[0]
            except:
                "UpperBound: error"
                continue
            # check this is the residue length (should be 4, but I use a larger
            # length)

            h = IMP.core.HarmonicUpperBound(self.length, self.kappa)
            dps = IMP.core.DistancePairScore(h)
            pr = IMP.core.PairRestraint(dps, IMP.ParticlePair(p0, p1))
            pr.set_name(
                "UpperBound_" + str(pair[0]) + "-" + str(pair[1]) + "_" + str(pair[2]))
            self.rs.add_restraint(pr)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_hierarchy(self):
        return self.prot

    def get_kappa(self):
        return self.kappa

    def get_residue_ranges(self):
        return self.respairs

    def get_length(self):
        return self.length

    def get_restraint(self):
        return self.rs

    def get_output(self):
        output = {}
        self.m.update()
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["UpperBound_" + self.label] = str(score)
        return output

#


class ExcludedVolumeResidue(object):

    def __init__(self, prot, kappa):
        self.rs = IMP.RestraintSet('excluded_volume')
        self.prot = prot
        self.kappa = kappa
        self.label = "None"
        self.m = self.prot.get_model()

        atoms = IMP.atom.get_by_type(self.prot, IMP.atom.ATOM_TYPE)
        for atom in atoms:
            restype = IMP.atom.Residue(
                IMP.atom.Atom(atom).get_parent()).get_residue_type()
            vol = IMP.atom.get_volume_from_residue_type(restype)
            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
            IMP.core.XYZR(atom).set_radius(radius)
        lsa = IMP.container.ListSingletonContainer(self.m)
        lsa.add_particles(atoms)

        evr = IMP.core.ExcludedVolumeRestraint(lsa, self.kappa)
        self.rs.add_restraint(evr)

    def set_label(self, label):
        self.label = label

    def add_excluded_particle_pairs(self, excluded_particle_pairs):
        # add pairs to be filtered when calculating  the score
        lpc = IMP.container.ListPairContainer(self.m)
        lpc.add_particle_pairs(excluded_particle_pairs)
        icpf = IMP.container.InContainerPairFilter(lpc)
        IMP.core.ExcludedVolumeRestraint.get_from(
            self.rs.get_restraints()[0]).add_pair_filter(icpf)

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_hierarchy(self):
        return self.prot

    def get_kappa(self):
        return self.kappa

    def get_restraint(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ExcludedVolumeResidue_" + self.label] = str(score)
        return output

#


class BipartiteExcludedVolumeResidue(object):

    def __init__(self, prot1, prot2, kappa):
        self.rs = IMP.RestraintSet('bipartite_excluded_volume')
        self.prot1 = prot1
        self.prot2 = prot2
        self.kappa = kappa
        self.label = "None"
        self.m = self.prot.get_model()

        atoms1 = IMP.atom.get_by_type(prot1, IMP.atom.ATOM_TYPE)
        ls1 = IMP.container.ListSingletonContainer(self.m)
        ls1.add_particles(atoms1)
        for atom in atoms1:
            restype = IMP.atom.Residue(
                IMP.atom.Atom(atom).get_parent()).get_residue_type()
            vol = IMP.atom.get_volume_from_residue_type(restype)
            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
            IMP.core.XYZR(atom).set_radius(radius)

        atoms2 = IMP.atom.get_by_type(prot2, IMP.atom.ATOM_TYPE)
        ls2 = IMP.container.ListSingletonContainer(self.m)
        ls2.add_particles(atoms2)
        for atom in atoms2:
            restype = IMP.atom.Residue(
                IMP.atom.Atom(atom).get_parent()).get_residue_type()
            vol = IMP.atom.get_volume_from_residue_type(restype)
            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
            IMP.core.XYZR(atom).set_radius(radius)

        cbpc = IMP.container.CloseBipartitePairContainer(
            ls_ref,
            ls_symm,
            kappa,
            10.0)
        ssps = IMP.core.SoftSpherePairScore(kappa)
        evr3 = IMP.container.PairsRestraint(ssps, cbpc)
        self.rs.add_restraint(evr3)
        self.m.add_restraint(rs)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_hierarchy(self):
        return self.prot1, self.prot2

    def get_kappa(self):
        return self.kappa

    def get_restraint(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["BipartiteExcludedVolumeResidue_" + self.label] = str(score)
        return output

#


class TemplateRestraint(object):

    def __init__(self, ps1, ps2, cutoff=6.5, kappa=1.0, forcerb=False):
        self.m = ps1[0].get_model()
        self.label = "None"
        self.cutoff = cutoff
        self.kappa = kappa
        # this parameter ovverides the rigid body filter below
        self.forcerb = forcerb
        self.rs = IMP.RestraintSet('template_restraint')
        for p1 in ps1:
            for p2 in ps2:
                # check that the two particles are not in the same rigid body
                if(IMP.core.RigidMember.get_is_setup(p1) and IMP.core.RigidMember.get_is_setup(p2) and
                   IMP.core.RigidMember(p1).get_rigid_body() == IMP.core.RigidMember(p2).get_rigid_body()) and not self.forcerb:
                    continue
                d0 = IMP.core.XYZ(p1)
                d1 = IMP.core.XYZ(p2)
                dist = IMP.core.get_distance(d0, d1)
                if dist <= self.cutoff:
                    hf = IMP.core.Harmonic(dist, self.kappa)
                    dps = IMP.core.DistancePairScore(hf)
                    pr = IMP.core.PairRestraint(dps, IMP.ParticlePair(p1, p2))
                    self.rs.add_restraint(pr)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_cutoff(self):
        return self.cutoff

    def get_kappa(self):
        return self.kappa

    def get_restraint(self):
        return self.rs

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["TemplateRestraint_" + self.label] = str(score)
        return output

#


class MarginalChi3Restraint(object):

    def __init__(self, part1, part2):
        global impisd2, tools
        import IMP.isd2 as impisd2
        import IMP.pmi.tools as tools

        self.m = part1.get_model()
        self.label = "None"
        self.rs = IMP.RestraintSet('chi3_restraint')
        self.sigmamaxtrans = 0.1

        self.ps1 = IMP.atom.get_leaves(part1)
        self.ps2 = IMP.atom.get_leaves(part2)
        self.sigma = tools.SetupNuisance(
            self.m,
            1.0,
            0.1,
            100.0,
            True).get_particle(
        )

        for i in range(len(self.ps1)):
            mc = impisd2.MarginalChi3Restraint(
                self.ps1[i],
                self.ps2[i],
                self.sigma)
            self.rs.add_restraint(mc)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def get_particles_to_sample(self):
        ps = {}
        ps["Nuisances_MarginalChi3Restraint_Sigma_" +
            self.label] = ([self.sigma], self.sigmamaxtrans)
        return ps

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["MarginalChi3Restraint_" + self.label] = str(score)
        output["MarginalChi3Restraint_Sigma_" +
               self.label] = str(self.sigma.get_scale())
        return output

#


class CrossLinkMS(object):

    '''
    this class initialize a CrossLinkMS restraint and contains
    all useful informations, such as the cross-link database, contained in self.pairs
    If restraint_file=None, it will proceed creating simulated data
    '''

    def __init__(self, prots,
                 listofxlinkertypes=["BS3", "BS2G", "EGS"], map_between_protein_names_and_chains=None,
                 sigmamin=1.0, sigmamax=1.0, sigmagrid=1, sigmaissampled=False, typeofprofile="gofr"):
        global impisd2, tools
        import IMP.isd2 as impisd2
        import IMP.pmi.tools as tools

        if map_between_protein_names_and_chains is None:
            map_between_protein_names_and_chains = {}

        self.rs = IMP.RestraintSet('data')
        self.rs2 = IMP.RestraintSet('prior')
        self.prots = prots
        self.label = "None"
        self.pairs = []
        self.m = self.prots[0].get_model()
        self.sigmamin = sigmamin
        self.sigmamax = sigmamax
        self.sigmagrid = sigmagrid
        self.sigmaissampled = sigmaissampled

        self.sigmatrans = 0.1
        self.sigmaglobal = tools.SetupNuisance(self.m, self.sigmamin,
                                               self.sigmamin, self.sigmamax, self.sigmaissampled).get_particle()
        self.outputlevel = "low"
        self.listofxlinkertypes = listofxlinkertypes
        # simulated reaction rates
        self.reaction_rates = None
        self.allpairs_database = None
        self.residue_list = None

        self.crosslinker_dict = self.get_crosslinker_dict(typeofprofile)
        # this map is used in the new cross-link file reader
        #{"Nsp1":"A","Nup82":"B"} etc.
        self.mbpnc = map_between_protein_names_and_chains
        # check whther the file was initialized

    #-------------------------------
    def get_crosslinker_dict(self, typeofprofile="gofr"):
        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only yht needed crosslinks
        # type of profile can be gofr or pfes

        disttuple = (0.0, 200.0, 1000)
        omegatuple = (1.0, 1000.0, 30)
        sigmatuple = (self.sigmamin, self.sigmamax, self.sigmagrid)

        crosslinker_dict = {}
        if "BS3" in self.listofxlinkertypes:
            crosslinker_dict["BS3"] = tools.get_cross_link_data("bs3l",
                                                                "pmf_bs3l_tip3p.txt.standard", disttuple, omegatuple, sigmatuple,
                                                                don=None, doff=None, prior=0, type_of_profile=typeofprofile)
        if "BS2G" in self.listofxlinkertypes:
            crosslinker_dict["BS2G"] = tools.get_cross_link_data("bs2gl",
                                                                 "pmf_bs2gl_tip3p.txt.standard", disttuple, omegatuple, sigmatuple,
                                                                 don=None, doff=None, prior=0, type_of_profile=typeofprofile)
        if "EGS" in self.listofxlinkertypes:
            crosslinker_dict["EGS"] = tools.get_cross_link_data("egl",
                                                                "pmf_egl_tip3p.txt.standard", disttuple, omegatuple, sigmatuple,
                                                                don=None, doff=None, prior=0, type_of_profile=typeofprofile)
        if "Short" in self.listofxlinkertypes:
            # setup a "short" xl with an half length of 10 Ang
            crosslinker_dict["Short"] = tools.get_cross_link_data_from_length(
                10.0,
                disttuple,
                omegatuple,
                sigmatuple)
        return crosslinker_dict

    #-------------------------------

    def add_restraints(self, restraint_file=None, oldfile=True):

        if restraint_file is None:
            # get the restraints from simulated data
            restraint_list = self.allpairs_database

        else:
            # get the restraints from external file
            f = open(restraint_file)
            restraint_list = f.readlines()

        self.index = 0

        self.added_pairs_list = []
        self.missing_residues = []
        for line in restraint_list:
            # force_restraint=True makes all intra rigid body restraint to be
            # accepted
            force_restraint = False

            if restraint_file is None:
                if line["Is_Detected"]:
                    crosslinker = line["Crosslinker"]
                    (r1, c1) = line["Identified_Pair1"]
                    (r2, c2) = line["Identified_Pair2"]
                    index += 1
                else:
                    continue

            elif oldfile:
                tokens = line.split()
                # skip character
                if (tokens[0] == "#"):
                    continue
                r1 = int(tokens[0])
                c1 = tokens[1]
                r2 = int(tokens[2])
                c2 = tokens[3]
                crosslinker = tokens[4]

                # two restraints with the same index will be ambiguous
                self.index = int(tokens[5])

                # force restraint even if it belong to the same rigid body, use
                # it for ambiguous restraints
                if (tokens[len(tokens) - 1] == "F"):
                    force_restraint = True

            else:
                # read with the new file parser
                totallist = eval(line)
                self.add_crosslink_according_to_new_file(totallist)
                # skip the rest
                continue

            print('''CrossLinkMS: attempting to add restraint between
                     residue %d of chain %s and residue %d of chain %s''' % (r1, c1, r2, c2))

            p1s = []
            p2s = []

            try:
                s1 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r1,
                    chains=c1,
                    atom_type=IMP.atom.AT_CA)
                p1 = (s1.get_selected_particles()[0])
            except:
                print("CrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue
            try:
                s2 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r2,
                    chains=c2,
                    atom_type=IMP.atom.AT_CA)
                p2 = (s2.get_selected_particles()[0])
            except:
                print("CrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            for copy in self.prots:
                s1 = IMP.atom.Selection(
                    copy,
                    residue_index=r1,
                    chains=c1,
                    atom_type=IMP.atom.AT_CA)
                p1s.append(s1.get_selected_particles()[0])
                s2 = IMP.atom.Selection(
                    copy,
                    residue_index=r2,
                    chains=c2,
                    atom_type=IMP.atom.AT_CA)
                p2s.append(s2.get_selected_particles()[0])

            # check whether the atom pair belongs to the same rigid body
            if(IMP.core.RigidMember.get_is_setup(p1s[0]) and
               IMP.core.RigidMember.get_is_setup(p2s[0]) and
               IMP.core.RigidMember(p1s[0]).get_rigid_body() ==
               IMP.core.RigidMember(p2s[0]).get_rigid_body() and not force_restraint):
                print('''CrossLinkMS: WARNING> residue %d of chain %s and
                       residue %d of chain %s belong to the same rigid body''' % (r1, c1, r2, c2))
                continue

            # this list contains the list of symmetric pairs to avoid
            # duplicates
            if (p1s[0], p2s[0], crosslinker) in self.added_pairs_list:
                print("CrossLinkMS: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue
            if (p2s[0], p1s[0], crosslinker) in self.added_pairs_list:
                print("CrossLinkMS: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue

            print("CrossLinkMS: added pair %d %s %d %s" % (r1, c1, r2, c2))
            self.added_pairs_list.append((p1s[0], p2s[0], crosslinker))

            rs_name = 'restraint_' + str(index)

            ln = impisd2.CrossLinkMSRestraint(
                self.sigmaglobal,
                self.crosslinker_dict[crosslinker])
            for i in range(len(p1s)):
                ln.add_contribution(p1s[i], p2s[i])

            for i in range(len(self.prots)):
                self.pairs.append((p1s[i], p2s[i],
                                   crosslinker, rs_name,
                                   100, 100, (r1, c1, i), (r2, c2, i),
                                   crosslinker, i, ln))

            self.rs.add_restraint(ln)

        # self.rs2.add_restraint(impisd2.JeffreysRestraint(self.sigmaglobal))
        self.rs2.add_restraint(
            impisd2.UniformPrior(self.sigmaglobal,
                                 1000.0,
                                 self.sigmaglobal.get_upper() - 1.0,
                                 self.sigmaglobal.get_lower() + 0.1))
        print("CrossLinkMS: missing residues")
        for ms in self.missing_residues:
            print("CrossLinkMS:missing " + str(ms))

#---------------------------------
    def add_crosslink_according_to_new_file(self, totallist):
        force_restraint = False
        ambiguous_list = totallist[0]
        crosslinker = totallist[1]
        if (totallist[2] == "F"):
            force_restraint = True

        p1s = []
        p2s = []
        r1s = []
        r2s = []
        c1s = []
        c2s = []
        self.index += 1
        for pair in ambiguous_list:
            error = False

            try:
                c1 = self.mbpnc[pair[0][0]]
            except:
                "CrossLinkMS: WARNING> protein name " + \
                    pair[0][0] + " was not defined"
                continue
            try:
                c2 = self.mbpnc[pair[1][0]]
            except:
                "CrossLinkMS: WARNING> protein name " + \
                    pair[1][0] + " was not defined"
                continue
            r1 = int(pair[0][1])
            r2 = int(pair[1][1])

            print('''CrossLinkMS: attempting to add restraint between
                     residue %d of chain %s and residue %d of chain %s''' % (r1, c1, r2, c2))

            try:
                s1 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r1,
                    chains=c1,
                    atom_type=IMP.atom.AT_CA)
                p1 = (s1.get_selected_particles()[0])
            except:
                print("CrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                error = True
                self.missing_residues.append((r1, c1))
            try:
                s2 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r2,
                    chains=c2,
                    atom_type=IMP.atom.AT_CA)
                p2 = (s2.get_selected_particles()[0])
            except:
                print("CrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                error = True
                self.missing_residues.append((r2, c2))
            if error:
                continue

            s1 = IMP.atom.Selection(
                self.prots[0],
                residue_index=r1,
                chains=c1,
                atom_type=IMP.atom.AT_CA)
            p1 = s1.get_selected_particles()[0]
            s2 = IMP.atom.Selection(
                self.prots[0],
                residue_index=r2,
                chains=c2,
                atom_type=IMP.atom.AT_CA)
            p2 = s2.get_selected_particles()[0]
            # this list contains the list of symmetric pairs to avoid
            # duplicates
            if (p1, p2, crosslinker) in self.added_pairs_list:
                print("CrossLinkMS: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue
            if (p2, p1, crosslinker) in self.added_pairs_list:
                print("CrossLinkMS: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue

            # check whether the atom pair belongs to the same rigid body
            if(IMP.core.RigidMember.get_is_setup(p1) and
               IMP.core.RigidMember.get_is_setup(p2) and
               IMP.core.RigidMember(p1).get_rigid_body() ==
               IMP.core.RigidMember(p2).get_rigid_body() and not force_restraint):
                print('''CrossLinkMS: WARNING> residue %d of chain %s and
                       residue %d of chain %s belong to the same rigid body''' % (r1, c1, r2, c2))
                continue

            p1s.append(p1)
            p2s.append(p2)
            r1s.append(r1)
            r2s.append(r2)
            c1s.append(c1)
            c2s.append(c2)

            print("CrossLinkMS: added pair %d %s %d %s" % (r1, c1, r2, c2))

            self.added_pairs_list.append((p1, p2, crosslinker))

        if len(p1s) > 0:
            rs_name = '{:05}'.format(self.index % 100000)

            ln = impisd2.CrossLinkMSRestraint(
                self.sigmaglobal,
                self.crosslinker_dict[crosslinker])
            for i in range(len(p1s)):
                print(rs_name, i)
                ln.add_contribution(p1s[i], p2s[i])
                self.pairs.append((p1s[i], p2s[i], crosslinker, rs_name,
                                   100, 100, (r1s[i], c1s[i], i), (r2s[i], c2s[i], i), crosslinker, i, ln))
            self.rs.add_restraint(ln)

#---------------------------------

    def simulate_data(self, crosslinker, weights, sensitivity_threshold=0.1,
                      false_positive_half=0.02, elapsed_time=0.01,
                      ratemin=100, ratemax=100):

        from random import choice
        from random import randrange
        from itertools import combinations
        from numpy.random import binomial

        self.weights = weights
        self.sensitivity_threshold = sensitivity_threshold
        self.false_positive_half = false_positive_half
        self.elapsed_time = elapsed_time
        self.ratemin = ratemin
        self.ratemax = ratemax

        # dictionary of random reaction rates
        # check if they were already initialized
        if self.reaction_rates is None:
            self.reaction_rates = {}

            s0 = IMP.atom.Selection(self.prots[0],
                                    residue_type=IMP.atom.get_residue_type(
                                        "K"),
                                    atom_type=IMP.atom.AT_CA)
            self.residue_list = []

            for p1 in s0.get_selected_particles():
                (r1, c1) = tools.get_residue_index_and_chain_from_particle(p1)
                self.residue_list.append((r1, c1))
                if self.ratemin != self.ratemax:
                    self.reaction_rates[(
                        r1,
                        c1)] = randrange(self.ratemin,
                                         self.ratemax,
                                         1)
                else:
                    self.reaction_rates[(r1, c1)] = self.ratemax

        if self.allpairs_database is None:
            self.allpairs_database = []

        # generate the restraints
        allcomb = list(combinations(self.residue_list, 2))
        for ((r1, c1), (r2, c2)) in allcomb:

            p1s = []
            p2s = []

            for copy in self.prots:
                s1 = IMP.atom.Selection(copy, residue_index=r1,
                                        chains=c1, atom_type=IMP.atom.AT_CA)
                p1s.append(s1.get_selected_particles()[0])
                s2 = IMP.atom.Selection(copy, residue_index=r2,
                                        chains=c2, atom_type=IMP.atom.AT_CA)
                p2s.append(s2.get_selected_particles()[0])

            ln = impisd2.CrossLinkMSRestraint(
                self.sigmaglobal,
                self.crosslinker_dict[crosslinker])
            for i in range(len(p1s)):
                ln.add_contribution(p1s[i], p2s[i])
                d1 = IMP.core.XYZ(p1s[i])
                d2 = IMP.core.XYZ(p2s[i])
                dist = IMP.core.get_distance(d1, d2)
                reactionrate1 = self.reaction_rates[(r1, c1)]
                reactionrate2 = self.reaction_rates[(r2, c2)]
                prob = ln.get_marginal_probabilities()[i]
                effrate = float(reactionrate1 * reactionrate2) / \
                    (reactionrate1 + reactionrate2)
                probt = self.weights[i] * \
                    (1 - exp(-effrate * prob * elapsed_time))
                falsepositiveprob = exp(-probt / false_positive_half)
                falsepositivebool = False
                falsepositive = binomial(n=1, p=falsepositiveprob)
                if (falsepositive == 1):
                    falsepositivebool = True
                    randompair = choice(allcomb)
                    randpair1 = randompair[0]
                    randpair2 = randompair[1]
                else:
                    randpair1 = (r1, c1)
                    randpair2 = (r2, c2)
                if (probt > sensitivity_threshold):
                    detectedbool = True
                else:
                    detectedbool = False

                self.allpairs_database.append({})
                self.allpairs_database[-1]["Particle1"] = p1s[i]
                self.allpairs_database[-1]["Particle2"] = p2s[i]
                self.allpairs_database[-1]["Distance"] = dist
                self.allpairs_database[-1]["Crosslinker"] = crosslinker
                self.allpairs_database[-1]["IMPRestraint"] = ln
                self.allpairs_database[-1]["IMPRestraint_Probability"] = prob
                self.allpairs_database[-1]["Reaction_Rate1"] = reactionrate1
                self.allpairs_database[-1]["Reaction_Rate2"] = reactionrate2
                self.allpairs_database[-1]["Effective_Rate"] = effrate
                self.allpairs_database[-1]["CrossLink_Fraction"] = probt
                self.allpairs_database[
                    -1]["Resid1_Chainid1_Copy1"] = (r1, c1, i)
                self.allpairs_database[
                    -1]["Resid2_Chainid2_Copy2"] = (r2, c2, i)
                self.allpairs_database[
                    -1]["Is_False_Positive"] = falsepositivebool
                self.allpairs_database[-1]["Identified_Pair1"] = randpair1
                self.allpairs_database[-1]["Identified_Pair2"] = randpair2
                self.allpairs_database[-1]["Is_Detected"] = detectedbool

    def set_hierarchy(self, prots):
        # we use it to change the hierarchy
        self.prots = prots

    def initialize_simulated_database(self):
        # we use it to restart the simulation
        self.allpairs_database = None

    def get_number_detected_inter(self, xl_type):
        # we use it to see ho many xls of a give type (eg. BS3) were detected
        # as inter-chain
        ndetected = 0
        for el in self.allpairs_database:
            if el["Is_Detected"] and \
                ( el["Identified_Pair1"][1] != el["Identified_Pair2"][1] ) and \
                    el["Crosslinker"] == xl_type:
                ndetected += 1
        return ndetected

    def get_number_detected_inter_false_positive(self, xl_type):
        # we use it to see ho many xls of a give type (eg. BS3) were detected
        # as inter-chain
        ndetectedfp = 0
        for el in self.allpairs_database:
            if el["Is_Detected"] and \
                ( el["Identified_Pair1"][1] != el["Identified_Pair2"][1] ) and \
                    el["Crosslinker"] == xl_type and el["Is_False_Positive"]:
                ndetectedfp += 1
        return ndetectedfp

    def show_simulated_data(self, what="Inter"):
        #"what" can be "All", "Detected", "FalsePositive", "TruePositive", "Intra", "Inter"
        if not self.allpairs_database is None:
            detectedlist = []
            for el in self.allpairs_database:
                printbool = False
                if el["Is_Detected"]:
                    p1 = el["Identified_Pair1"]
                    p2 = el["Identified_Pair2"]
                    isf = el["Is_False_Positive"]
                    isinter = (
                        el["Identified_Pair1"][1] != el["Identified_Pair2"][1])
                    cl = el["Crosslinker"]
                    detectedlist.append((p1, p2, isf, cl, isinter))

                if el["Is_Detected"] and what == "Detected":
                    printbool = True
                if el["Is_Detected"] and el["Is_False_Positive"] and what == "FalsePositive":
                    printbool = True
                if el["Is_Detected"] and el["Is_False_Positive"] == False and what == "TruePositive":
                    printbool = True
                if el["Is_Detected"] and what == "Intra" and \
                        (el["Identified_Pair1"][1] == el["Identified_Pair2"][1]):
                    printbool = True
                if el["Is_Detected"] and what == "Inter" and \
                        (el["Identified_Pair1"][1] != el["Identified_Pair2"][1]):
                    printbool = True
                if what == "All":
                    printbool = True

                if printbool:
                    print("Residue1: %6s, chainid1: %6s, copy1: %6d" % el["Resid1_Chainid1_Copy1"])
                    print("Residue2: %6s, chainid2: %6s, copy2: %6d" % el["Resid2_Chainid2_Copy2"])
                    keylist = list(el.keys())
                    keylist.sort()
                    for k in keylist:
                        print("----", k, el[k])
            print("r1 c1 r2 c2 FP XL Inter")
            for d in detectedlist:
                print(d[0][0], d[0][1], d[1][0], d[1][1], d[2], d[3], d[4])
        else:
            print("CrossLinkMS: Simulated data not initialized")
            exit()

    def dump_simulated_data(self, filename="simulated_cross_link.dat"):
            # dump the whole simulated xl database on a file
        sclf = open(filename, "w")
        for el in self.allpairs_database:
            sclf.write(str(el))
            sclf.write("\n")
        sclf.close()

    def write_simulated_data(self, filename="detected_cross_link.dat"):
        # dump the whole simulated xl database on a file
        sclf = open(filename, "w")
        index = 0
        for el in self.allpairs_database:
            if el["Is_Detected"]:
                index += 1
                p1 = el["Identified_Pair1"]
                p2 = el["Identified_Pair2"]
                isf = el["Is_False_Positive"]
                cl = el["Crosslinker"]
                s = " ".join(
                    [str(p1[0]),
                     p1[1],
                        str(p2[0]),
                        p2[1],
                        cl,
                        str(index),
                        str(isf),
                        "T"])
                sclf.write(s)
                sclf.write("\n")
        sclf.close()

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)
        self.m.add_restraint(self.rs2)

    def get_hierarchies(self):
        return self.prots

    def get_particles(self):
        return self.sigmaglobal

    def get_restraint_sets(self):
        return self.rs, self.rs2

    def get_restraint(self):
        tmprs = IMP.RestraintSet('xlms')
        tmprs.add_restraint(self.rs)
        tmprs.add_restraint(self.rs2)
        return tmprs

    def set_output_level(self, level="low"):
        # this might be "low" or "high"
        self.outputlevel = level

    def print_chimera_pseudobonds(self, filesuffix, model=0):
        f = open(filesuffix + ".chimera", "w")
        atype = "ca"
        for p in self.pairs:
            s = "#" + str(model) + ":" + str(p[6][0]) + "." + p[6][1] + "@" + atype + \
                " #" + str(model) + ":" + \
                str(p[7][0]) + "." + p[7][1] + "@" + atype
            f.write(s + "\n")
        f.close()

    def get_particles_to_sample(self):
        ps = {}
        if self.sigmaissampled:
            ps["Nuisances_CrossLinkMS_Sigma_" +
                self.label] = ([self.sigmaglobal], self.sigmatrans)
        return ps

    def get_output(self):
        # content of the crosslink database pairs
        #self.pairs.append((p1s[i], p2s[i], crosslinker, rs_name, 100, 100, (r1,c1,i),  (r2,c2,i), crosslinker, i, ln))
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        score2 = self.rs2.unprotected_evaluate(None)
        output["_TotalScore"] = str(score + score2)

        output["CrossLinkMS_Likelihood_" + self.label] = str(score)
        output["CrossLinkMS_Prior_" + self.label] = str(score2)
        output["CrossLinkMS_Sigma"] = str(self.sigmaglobal.get_scale())

        if self.outputlevel == "high":
            for i in range(len(self.pairs)):

                p0 = self.pairs[i][0]
                p1 = self.pairs[i][1]
                crosslinker = self.pairs[i][2]
                ln = self.pairs[i][10]
                index = self.pairs[i][9]
                rsname = self.pairs[i][3]
                resid1 = self.pairs[i][6][0]
                chain1 = self.pairs[i][6][1]
                copy1 = self.pairs[i][6][2]
                resid2 = self.pairs[i][7][0]
                chain2 = self.pairs[i][7][1]
                copy2 = self.pairs[i][7][2]
                label_copy = str(rsname) + ":" + str(index) + "-" + str(resid1) + \
                    ":" + chain1 + "_" + "-" + \
                    str(resid2) + ":" + chain2 + "_" + crosslinker
                output["CrossLinkMS_Partial_Probability_" +
                       label_copy] = str(ln.get_marginal_probabilities()[index])

                if copy1 == 0:
                    label = str(resid1) + ":" + chain1 + \
                        "_" + str(resid2) + ":" + chain2
                    # output["CrossLinkMS_Combined_Probability_"+str(rsname)+"_"+crosslinker+"_"+label]=str(ln.get_probability())
                    output["CrossLinkMS_Score_" +
                           str(rsname) + "_" + crosslinker + "_" + label] = str(ln.unprotected_evaluate(None))

                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
                output["CrossLinkMS_Distance_" +
                       label_copy] = str(IMP.core.get_distance(d0, d1))

        return output

#


class BinomialXLMSRestraint(object):

    def __init__(self, m, prots,
                 listofxlinkertypes=["BS3", "BS2G", "EGS"], map_between_protein_names_and_chains=None, typeofprofile='pfes'):

        if map_between_protein_names_and_chains is None:
            map_between_protein_names_and_chains = {}

        global impisd2, tools, exp
        import IMP.isd2 as impisd2
        import IMP.pmi.tools as tools

        self.setup = 0

        self.label = "None"
        self.rs = IMP.RestraintSet('xlms')
        self.rs2 = IMP.RestraintSet('jeffreys')
        self.m = m
        self.prots = prots
        self.pairs = []

        self.weightmaxtrans = 0.05
        self.weightissampled = False

        self.sigmainit = 5.0
        self.sigmamin = 1.0
        self.sigmaminnuis = 0.0
        self.sigmamax = 10.0
        self.sigmamaxnuis = 11.0
        self.nsigma = 100
        self.sigmaissampled = True
        self.sigmamaxtrans = 0.1

        self.betainit = 1.0
        self.betamin = 1.0
        self.betamax = 4.0
        if self.setup == 1:
            self.betaissampled = True
            print("BinomialXLMSRestraint: beta is sampled")
        if self.setup == 0:
            self.betaissampled = False
            print("BinomialXLMSRestraint: beta is NOT sampled")
        self.betamaxtrans = 0.01

        '''
        self.deltainit=0.001
        self.deltamin=0.001
        self.deltamax=0.1
        self.deltaissampled=False
        self.deltamaxtrans=0.001

        self.laminit=5.0
        self.lammin=0.01
        self.lamminnuis=0.00001
        self.lammax=10.0
        self.lammaxnuis=100.0
        self.lamissampled=False
        self.lammaxtrans=0.1
        '''

        self.epsilon = 0.01
        self.psi_dictionary = {}

        self.sigma = tools.SetupNuisance(self.m, self.sigmainit,
                                         self.sigmaminnuis, self.sigmamaxnuis, self.sigmaissampled).get_particle()

        self.beta = tools.SetupNuisance(self.m, self.betainit,
                                        self.betamin, self.betamax, self.betaissampled).get_particle()

        '''
        self.delta=tools.SetupNuisance(self.m,self.deltainit,
             self.deltamin,self.deltamax,self.deltaissampled).get_particle()

        self.lam=tools.SetupNuisance(self.m,self.laminit,
             self.lamminnuis,self.lammaxnuis,self.lamissampled).get_particle()

        self.weight=tools.SetupWeight(m,False).get_particle()

        for n in range(len(self.prots)):
            self.weight.add_weight()
        '''
        self.outputlevel = "low"
        self.listofxlinkertypes = listofxlinkertypes
        # simulated reaction rates
        self.reaction_rates = None
        self.allpairs_database = None
        self.residue_list = None

        self.crosslinker_dict = self.get_crosslinker_dict(typeofprofile)
        # this map is used in the new cross-link file reader
        #{"Nsp1":"A","Nup82":"B"} etc.
        self.mbpnc = map_between_protein_names_and_chains
        # check whther the file was initialized

    def create_psi(self, index, value):
        if value is None:
            self.psiinit = 0.01
            self.psiissampled = True
            print("BinomialXLMSRestraint: psi " + str(index) + " is sampled")
        else:
            self.psiinit = value
            self.psiissampled = False
            print("BinomialXLMSRestraint: psi " + str(index) + " is NOT sampled")
        self.psiminnuis = 0.0000001
        self.psimaxnuis = 0.4999999
        self.psimin = 0.01
        self.psimax = 0.49
        self.psitrans = 0.01
        self.psi = tools.SetupNuisance(self.m, self.psiinit,
                                       self.psiminnuis, self.psimaxnuis, self.psiissampled).get_particle()
        self.psi_dictionary[index] = (
            self.psi,
            self.psitrans,
            self.psiissampled)

    def get_psi(self, index, value):
        if not index in self.psi_dictionary:
            self.create_psi(index, value)
        return self.psi_dictionary[index]

    def get_crosslinker_dict(self, typeofprofile):
        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only
        # yht needed crosslinks

        disttuple = (0.0, 200.0, 500)
        omegatuple = (0.01, 1000.0, 30)
        sigmatuple = (self.sigmamin, self.sigmamax, self.nsigma)

        crosslinker_dict = {}
        if "BS3" in self.listofxlinkertypes:
            crosslinker_dict["BS3"] = tools.get_cross_link_data("bs3l",
                                                                "pmf_bs3l_tip3p.txt.standard", disttuple, omegatuple, sigmatuple, don=None, doff=None, prior=1, type_of_profile=typeofprofile)
        if "BS2G" in self.listofxlinkertypes:
            crosslinker_dict["BS2G"] = tools.get_cross_link_data("bs2gl",
                                                                 "pmf_bs2gl_tip3p.txt.standard", disttuple, omegatuple, sigmatuple, don=None, doff=None, prior=1, type_of_profile=typeofprofile)
        if "EGS" in self.listofxlinkertypes:
            crosslinker_dict["EGS"] = tools.get_cross_link_data("egl",
                                                                "pmf_egl_tip3p.txt.standard", disttuple, omegatuple, sigmatuple, don=None, doff=None, prior=1, type_of_profile=typeofprofile)
        if "Short" in self.listofxlinkertypes:
            # setup a "short" xl with an half length of 10 Ang
            crosslinker_dict["Short"] = tools.get_cross_link_data_from_length(
                10.0,
                disttuple,
                omegatuple,
                sigmatuple)
        return crosslinker_dict

    def add_restraints(self, restraint_file=None, oldfile=False):

        # get the restraints from external file
        f = open(restraint_file)
        restraint_list = f.readlines()

        self.index = 0

        self.added_pairs_list = []
        self.missing_residues = []
        for line in restraint_list:
            # force_restraint=True makes all intra rigid body restraint to be
            # accepted
            force_restraint = False

            # read with the new file parser
            totallist = eval(line)
            self.add_crosslink_according_to_new_file(totallist)

        self.rs2.add_restraint(
            impisd2.UniformPrior(
                self.sigma,
                1000000000.0,
                self.sigmamax,
                self.sigmamin))
        # self.rs2.add_restraint(impisd2.JeffreysRestraint(self.sigma))

        for psiindex in self.psi_dictionary:
            if self.psi_dictionary[psiindex][2]:
                psip = self.psi_dictionary[psiindex][0]

                if self.setup == 0:
                    print("BinomialXLMSRestraint: setup 0, adding BinomialJeffreysPrior to psi particle " + str(psiindex))
                    self.rs2.add_restraint(impisd2.BinomialJeffreysPrior(psip))
                    # self.rs2.add_restraint(impisd2.JeffreysRestraint(psip))

                self.rs2.add_restraint(
                    impisd2.UniformPrior(
                        psip,
                        1000000000.0,
                        self.psimax,
                        self.psimin))

    def add_crosslink_according_to_new_file(self, totallist, constructor=0):
        # the constructor variable specify what constroctor to use in
        # the restraint   0: use predetermined f.p.r. (psi)
        # 1: infer the f.p.r. defining a psi nuisance defined by a class
        force_restraint = False
        ambiguous_list = totallist[0]
        crosslinker = totallist[1]
        if (totallist[2] == "F"):
            force_restraint = True

        p1s = []
        p2s = []
        r1s = []
        r2s = []
        c1s = []
        c2s = []
        psis = []
        psivalues = []
        self.index += 1
        for pair in ambiguous_list:
            error = False

            try:
                c1 = self.mbpnc[pair[0][0]]
            except:
                print("BinomialXLMSRestraint: WARNING> protein name " + pair[0][0] + " was not defined")
                continue
            try:
                c2 = self.mbpnc[pair[1][0]]
            except:
                print("BinomialXLMSRestraint: WARNING> protein name " + pair[1][0] + " was not defined")
                continue

            r1 = int(pair[0][1])
            r2 = int(pair[1][1])
            psi = float(pair[2])
            try:
                psivalue = float(pair[3])
            except:
                psivalue = None

            print('''CrossLinkMS: attempting to add restraint between
                     residue %d of chain %s and residue %d of chain %s''' % (r1, c1, r2, c2))

            try:
                s1 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r1,
                    chains=c1,
                    atom_type=IMP.atom.AT_CA)
                p1 = (s1.get_selected_particles()[0])
            except:
                print("BinomialXLMSRestraint: WARNING> residue %d of chain %s is not there" % (r1, c1))
                error = True
                self.missing_residues.append((r1, c1))
            try:
                s2 = IMP.atom.Selection(
                    self.prots[0],
                    residue_index=r2,
                    chains=c2,
                    atom_type=IMP.atom.AT_CA)
                p2 = (s2.get_selected_particles()[0])
            except:
                print("BinomialXLMSRestraint: WARNING> residue %d of chain %s is not there" % (r2, c2))
                error = True
                self.missing_residues.append((r2, c2))
            if error:
                continue

            s1 = IMP.atom.Selection(
                self.prots[0],
                residue_index=r1,
                chains=c1,
                atom_type=IMP.atom.AT_CA)
            p1 = s1.get_selected_particles()[0]
            s2 = IMP.atom.Selection(
                self.prots[0],
                residue_index=r2,
                chains=c2,
                atom_type=IMP.atom.AT_CA)
            p2 = s2.get_selected_particles()[0]
            # this list contains the list of symmetric pairs to avoid
            # duplicates
            if (p1, p2, crosslinker) in self.added_pairs_list:
                print("BinomialXLMSRestraint: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue
            if (p2, p1, crosslinker) in self.added_pairs_list:
                print("BinomialXLMSRestraint: WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue

            # check whether the atom pair belongs to the same rigid body
            if(IMP.core.RigidMember.get_is_setup(p1) and
               IMP.core.RigidMember.get_is_setup(p2) and
               IMP.core.RigidMember(p1).get_rigid_body() ==
               IMP.core.RigidMember(p2).get_rigid_body() and not force_restraint):
                print('''BinomialXLMSRestraint: WARNING> residue %d of chain %s and
                       residue %d of chain %s belong to the same rigid body''' % (r1, c1, r2, c2))
                continue

            p1s.append(p1)
            p2s.append(p2)
            r1s.append(r1)
            r2s.append(r2)
            c1s.append(c1)
            c2s.append(c2)
            psis.append(psi)
            psivalues.append(psivalue)

            print("BinomialXLMSRestraint: added pair %d %s %d %s" % (r1, c1, r2, c2))

            self.added_pairs_list.append((p1, p2, crosslinker))

        if len(p1s) > 0:
            rs_name = '{:05}'.format(self.index % 100000)

            if self.setup == 0:
                print("BinomialXLMSRestraint: constructor 0")
                ln = impisd2.BinomialCrossLinkMSRestraint(
                    self.m,
                    self.sigma,
                    self.epsilon,
                    self.crosslinker_dict[crosslinker])

            if self.setup == 1:
                print("BinomialXLMSRestraint: constructor 1")
                ln = impisd2.BinomialCrossLinkMSRestraint(
                    self.m,
                    self.sigma,
                    self.beta,
                    self.epsilon,
                    self.crosslinker_dict[crosslinker])

            for i in range(len(p1s)):
                ln.add_contribution()

                psi = self.get_psi(psis[i], psivalues[i])

                ln.add_particle_pair(
                    i,
                    (p1s[i].get_index(),
                     p2s[i].get_index()),
                    psi[0].get_particle().get_index())
                self.pairs.append((p1s[i], p2s[i], crosslinker, rs_name,
                                   self.index, 100, (r1s[i], c1s[i], i), (r2s[i], c2s[i], i), crosslinker, i, ln))

                h = IMP.core.Linear(0, 0.03)
                dps = IMP.core.DistancePairScore(h)
                pr = IMP.core.PairRestraint(
                    dps, IMP.ParticlePair(p1s[i], p2s[i]))
                self.rs2.add_restraint(pr)

            self.rs.add_restraint(ln)

        print("BinomialXLMSRestraint: missing residues")
        for ms in self.missing_residues:
            print("BinomialXLMSRestraint:missing " + str(ms))

        # self.rs2.add_restraint(impisd2.IntensityThresholdRestraint(self.delta))
        # self.rs2.add_restraint(impisd2.UniformPrior(self.delta,1000000000.0,self.delta.get_upper(),self.delta.get_lower()))
        # exit()
            # self.rs2.add_restraint(impisd2.UniformPrior(psip,1000000000.0,0.5,0.01))
    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)
        self.m.add_restraint(self.rs2)

    def get_hierarchies(self):
        return self.prots

    def get_particles(self):
        return self.sigmaglobal

    def get_restraint_sets(self):
        return self.rs, self.rs2

    def get_restraint(self):
        tmprs = IMP.RestraintSet('xlms')
        tmprs.add_restraint(self.rs)
        tmprs.add_restraint(self.rs2)
        return tmprs

    def set_output_level(self, level="low"):
        # this might be "low" or "high"
        self.outputlevel = level

    def print_chimera_pseudobonds(self, filesuffix, model=0):
        f = open(filesuffix + ".chimera", "w")
        atype = "ca"
        for p in self.pairs:
            s = "#" + str(model) + ":" + str(p[6][0]) + "." + p[6][1] + "@" + atype + \
                " #" + str(model) + ":" + \
                str(p[7][0]) + "." + p[7][1] + "@" + atype
            f.write(s + "\n")
        f.close()

    def print_chimera_pseudobonds_with_psiindexes(self, filesuffix, model=0):

        f = open(filesuffix + ".chimera", "w")
        atype = "ca"
        for p in self.pairs:
            s = "#" + str(model) + ":" + str(p[6][0]) + "." + p[6][1] + "@" + atype + \
                " #" + str(model) + ":" + \
                str(p[7][0]) + "." + p[7][1] + "@" + atype
            f.write(s + "\n")
        f.close()

    def get_output(self):
        # content of the crosslink database pairs
        #self.pairs.append((p1s[i], p2s[i], crosslinker, rs_name, psi, 100, (r1,c1,i),  (r2,c2,i), crosslinker, i, ln))
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        score2 = self.rs2.unprotected_evaluate(None)
        output["_TotalScore"] = str(score + score2)
        output["CrossLinkMS_Likelihood_" + self.label] = str(score)
        output["CrossLinkMS_Prior_" + self.label] = str(score2)

        if self.outputlevel == "high":
            for i in range(len(self.pairs)):

                p0 = self.pairs[i][0]
                p1 = self.pairs[i][1]
                crosslinker = self.pairs[i][2]
                psiindex = self.pairs[i][4]
                ln = self.pairs[i][10]
                index = self.pairs[i][9]
                rsname = self.pairs[i][3]
                resid1 = self.pairs[i][6][0]
                chain1 = self.pairs[i][6][1]
                copy1 = self.pairs[i][6][2]
                resid2 = self.pairs[i][7][0]
                chain2 = self.pairs[i][7][1]
                copy2 = self.pairs[i][7][2]
                label_copy = str(rsname) + ":" + str(index) + "-" + str(resid1) + \
                    ":" + chain1 + "_" + "-" + \
                    str(resid2) + ":" + chain2 + "_" + crosslinker

                if copy1 == 0:
                    label = str(resid1) + ":" + chain1 + \
                        "_" + str(resid2) + ":" + chain2
                    # output["CrossLinkMS_Combined_Probability_"+str(rsname)+"_"+crosslinker+"_"+label]=str(ln.get_probability())
                    output["CrossLinkMS_Score_" + str(rsname) + "_" + str(index) + "_" +
                           crosslinker + "_" + label] = str(ln.unprotected_evaluate(None))

                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
                output["CrossLinkMS_Distance_" +
                       str(index) + "_" + label_copy] = str(IMP.core.get_distance(d0, d1))

        output["CrossLinkMS_Sigma_" + self.label] = str(self.sigma.get_scale())
        '''
        output["CrossLinkMS_Delta_"+self.label]=str(self.delta.get_scale())
        output["CrossLinkMS_Lambda_"+self.label]=str(self.lam.get_scale())
        '''
        output["CrossLinkMS_Beta_" + self.label] = str(self.beta.get_scale())
        for psiindex in self.psi_dictionary:
            output["CrossLinkMS_Psi_" +
                   str(psiindex) + "_" + self.label] = str(self.psi_dictionary[psiindex][0].get_scale())
        '''
        for n in range(self.weight.get_number_of_states()):
           output["CrossLinkMS_weights_"+str(n)+"_"+self.label]=str(self.weight.get_weight(n))
        '''
        return output

    def get_particles_to_sample(self):
        ps = {}
        if self.sigmaissampled:
            ps["Nuisances_CrossLinkMS_Sigma_" +
                self.label] = ([self.sigma], self.sigmamaxtrans)
        '''
        if self.deltaissampled:
           ps["Nuisances_CrossLinkMS_Delta_"+self.label]=([self.delta],self.deltamaxtrans)
        if self.lamissampled:
           ps["Nuisances_CrossLinkMS_Lambda_"+self.label]=([self.lam],self.lammaxtrans)
        '''
        if self.betaissampled:
            ps["Nuisances_CrossLinkMS_Beta_" +
                self.label] = ([self.beta], self.betamaxtrans)

        for psiindex in self.psi_dictionary:
            if self.psi_dictionary[psiindex][2]:
                ps["Nuisances_CrossLinkMS_Psi_" +
                    str(psiindex) + "_" + self.label] = ([self.psi_dictionary[psiindex][0]], self.psi_dictionary[psiindex][1])

        '''
        if self.weightissampled:
           ps["Weights_CrossLinkMS_"+self.label]=([self.weight],self.weightmaxtrans)
        '''
        return ps

#


class CrossLinkMSSimple(object):

    def __init__(self, prot, restraints_file, TruncatedHarmonic=True):
        """read crosslink restraints between two residue
        of different chains from an external text file
        syntax: part_name_1 part_name_2 distance error
        example:     0 1 1.0 0.1"""
        self.prot = prot
        self.rs = IMP.RestraintSet('xlms')
        self.pairs = []
        self.m = self.prot.get_model()

        # crosslinker="BS3"
        # this is an harmonic potential with mean 12 and sigma=25, therefore
        # k=1/sigma^2

        if TruncatedHarmonic:
                # use truncated harmonic to account for outliers
            hf = IMP.core.TruncatedHarmonicBound(12.0, 1.0 / 25.0, 15.0, 5)
        else:
            hf = IMP.core.Harmonic(12.0, 1.0 / 25.0)
        dps = IMP.core.DistancePairScore(hf)

        # small linear contribution for long range
        h = IMP.core.Linear(0, 0.03)
        dps2 = IMP.core.DistancePairScore(h)

        index = 0

        addedd_pairs_list = []
        for line in open(restraints_file):

        # force_restraint=True makes all intra rigid body restraint to be
        # accepted
            force_restraint = True
            tokens = line.split()

            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[0])
            c1 = tokens[1]
            r2 = int(tokens[2])
            c2 = tokens[3]
            crosslinker = tokens[4]

            # two restraints with the same index will be ambiguous
            index = int(tokens[5])

            # force restraint even if it belong to the same rigid body, use it
            # for ambiguous restraints
            if (tokens[len(tokens) - 1] == "F"):
                force_restraint = True

            print("attempting to add restraint between residue %d of chain %s and residue %d of chain %s" % (r1, c1, r2, c2))

            p1s = []
            p2s = []

            # apply the cross-link to the main copy
            try:
                s1 = IMP.atom.Selection(
                    self.prot,
                    residue_index=r1,
                    chains=c1,
                    atom_type=IMP.atom.AT_CA)
                p1 = (s1.get_selected_particles()[0])
            except:
                print("WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue
            try:
                s2 = IMP.atom.Selection(
                    self.prot,
                    residue_index=r2,
                    chains=c2,
                    atom_type=IMP.atom.AT_CA)
                p2 = (s2.get_selected_particles()[0])
            except:
                print("WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            # part1=[]
            # part2=[]
            # this list contain the list of simmetric pairs to avoid
            # duplications

            # this is the map between particle pairs and the restraints (there
            # might be more than one restraint per particle pair if you have
            # more than one cross-link type

            print("attempting to add restraint between residue %d of chain %s and residue %d of chain %s" % (r1, c1, r2, c2))

            # check whether the atom pair belongs to the same rigid body
            if(IMP.core.RigidMember.get_is_setup(p1) and IMP.core.RigidMember.get_is_setup(p2) and
               IMP.core.RigidMember(p1).get_rigid_body() == IMP.core.RigidMember(p2).get_rigid_body() and not force_restraint):
                print("WARNING> residue %d of chain %s and residue %d of chain %s belong to the same rigid body" % (r1, c1, r2, c2))
                continue

                # this list contain the list of simmetric pairs to avoid
                # duplications
            if (p1, p2, crosslinker) in addedd_pairs_list:
                print("WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue
            if (p2, p1, crosslinker) in addedd_pairs_list:
                print("WARNING> pair %d %s %d %s already there" % (r1, c1, r2, c2))
                continue

            print("added pair %d %s %d %s" % (r1, c1, r2, c2))
            index += 1
            addedd_pairs_list.append((p1, p2, crosslinker))

            rs_name = 'restraint_' + str(index)

            ln = IMP.core.PairRestraint(dps, IMP.ParticlePair(p1, p2))
            ln.set_name("CrossLinkMSSimple_" + str(r1)
                        + ":" + str(c1) + "-" + str(r2) + ":" + str(c2))
            ln.set_weight(1.0)

            self.rs.add_restraint(ln)

            pr = IMP.core.PairRestraint(dps2, IMP.ParticlePair(p1, p2))

            self.rs.add_restraint(pr)

            self.pairs.append(
                (p1,
                 p2,
                 crosslinker,
                 rs_name,
                 100,
                 100,
                 (r1,
                  c1),
                    (r2,
                     c2),
                    crosslinker,
                    ln,
                    pr))

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def print_chimera_pseudobonds(self, filesuffix, model=0):
        f = open(filesuffix + ".chimera", "w")
        atype = "ca"
        for p in self.pairs:
            s = "#" + str(model) + ":" + str(p[6][0]) + "." + p[6][1] + "@" + atype + \
                " #" + str(model) + ":" + \
                str(p[7][0]) + "." + p[7][1] + "@" + atype
            f.write(s + "\n")
        f.close()

    def get_output(self):
        # content of the crosslink database pairs
        #self.pairs.append((p1s[i], p2s[i], crosslinker, rs_name, 100, 100, (r1,c1),  (r2,c2), crosslinker, ln,pr))
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CrossLinkMSSimple_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = self.pairs[i][2]
            ln = self.pairs[i][9]
            pr = self.pairs[i][10]
            resid1 = self.pairs[i][6][0]
            chain1 = self.pairs[i][6][1]
            resid2 = self.pairs[i][7][0]
            chain2 = self.pairs[i][7][1]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["CrossLinkMSSimple_Score_" + crosslinker +
                   "_" + label] = str(ln.unprotected_evaluate(None))
            output["CrossLinkMSSimple_Score_Linear_" + crosslinker +
                   "_" + label] = str(pr.unprotected_evaluate(None))
            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["CrossLinkMSSimple_Distance_" +
                   label] = str(IMP.core.get_distance(d0, d1))

        return output


def get_residue_index_and_chain_from_particle(p):
    rind = IMP.atom.Residue(IMP.atom.Atom(p).get_parent()).get_index()
    c = IMP.atom.Residue(IMP.atom.Atom(p).get_parent()).get_parent()
    cid = IMP.atom.Chain(c).get_id()
    return rind, cid


def select_calpha_or_residue(
    prot,
    chain,
    resid,
    ObjectName="None:",
        SelectResidue=False):
    # use calphas
    p = None
    s = IMP.atom.Selection(prot, chains=chain,
                           residue_index=resid, atom_type=IMP.atom.AT_CA)

    ps = s.get_selected_particles()
    # check if the calpha selection is empty
    if ps:
        if len(ps) == 1:
            p = ps[0]
        else:
            print(ObjectName + " multiple residues selected for selection residue %s chain %s " % (resid, chain))
    else:
        # use the residue, in case of simplified representation
        s = IMP.atom.Selection(prot, chains=chain,
                               residue_index=resid)
        ps = s.get_selected_particles()
        # check if the residue selection is empty
        if ps:
            if len(ps) == 1:
                p = ps[0]
            else:
                print(ObjectName + " multiple residues selected for selection residue %s chain %s " % (resid, chain))

        else:
            print(ObjectName + " residue %s chain %s does not exist" % (resid, chain))
    return p




class SetupMembranePoreRestraint(object):
    import math

    def __init__(
        self,
        representation,
        selection_tuples_outside=None,
        selection_tuples_membrane=None,
        selection_tuples_inside=None,
        center=(0.0,0.0,0.0),
        z_tickness=100,
        radius=100,
        membrane_tickness=40.0,
        resolution=1,
        label="None"):

        self.weight = 1.0
        self.label = label
        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, label)
        self.representation=representation
        self.center=center
        self.radius=radius
        self.z_tickness=z_tickness
        self.membrane_tickness=membrane_tickness





    def create_representation(self):
        p=IMP.Particle(self.m)
        h=IMP.atom.Hierarchy.setup_particle(p)
        h.set_name("Membrane_Pore"+self.label)
        self.representation.prot.add_child(h)

        inner=self.z_tickness/2-self.membrane_tickness/2
        outer=self.z_tickness/2+self.membrane_tickness/2

        surface_1=[]
        surface_2=[]
        particles_surface_1=[]
        particles_surface_2=[]

        for i in range(10):
            for j in range(10):
                v=self.math.pi/10.0*float(i)+self.math.pi/2.0
                u=2.0*self.math.pi/10.0*float(j)
                x=(self.radius+inner*self.math.cos(v))*self.math.cos(u)
                y=(self.radius+inner*self.math.cos(v))*self.math.sin(u)
                z=inner*self.math.sin(v)
                p2=IMP.Particle(self.m)
                IMP.atom.Mass.setup_particle(p2,100)
                d=IMP.core.XYZR.setup_particle(p2)
                d.set_coordinates((x,y,z))
                d.set_radius(1)
                h2=IMP.atom.Hierarchy.setup_particle(p2)
                h.add_child(h2)
                surface_1.append((x,y,z))
                particles_surface_1.append(p2)

                x=(self.radius+outer*self.math.cos(v))*self.math.cos(u)
                y=(self.radius+outer*self.math.cos(v))*self.math.sin(u)
                z=outer*self.math.sin(v)
                p2=IMP.Particle(self.m)
                IMP.atom.Mass.setup_particle(p2,100)
                d=IMP.core.XYZR.setup_particle(p2)
                d.set_coordinates((x,y,z))
                d.set_radius(1)
                h2=IMP.atom.Hierarchy.setup_particle(p2)
                h.add_child(h2)
                surface_2.append((x,y,z))
                particles_surface_2.append(p2)

        for i in range(10):
            for j in range(10):
                x=3.0*self.radius/10.0*float(i)-self.radius*1.5
                y=3.0*self.radius/10.0*float(j)-self.radius*1.5
                if(self.math.sqrt(x**2+y**2)<self.radius):
                    continue
                else:
                    for n,z in enumerate([self.z_tickness/2+self.membrane_tickness/2,
                              self.z_tickness/2-self.membrane_tickness/2,
                              -self.z_tickness/2+self.membrane_tickness/2,
                              -self.z_tickness/2-self.membrane_tickness/2]):
                        p2=IMP.Particle(self.m)
                        IMP.atom.Mass.setup_particle(p2,100)
                        d=IMP.core.XYZR.setup_particle(p2)
                        d.set_coordinates((x,y,z))
                        d.set_radius(1)
                        h2=IMP.atom.Hierarchy.setup_particle(p2)
                        h.add_child(h2)
                        if n == 0 or n == 3:
                            surface_1.append((x,y,z))
                            particles_surface_1.append(p2)
                        if n == 1 or n == 2:
                            surface_2.append((x,y,z))
                            particles_surface_2.append(p2)

            from scipy.spatial import Delaunay
            tri = Delaunay(surface_1)

            '''
            edge_points = []
            edges = set()

            def add_edge(i, j):
                """Add a line between the i-th and j-th points, if not in the list already"""
                if (i, j) in edges or (j, i) in edges:
                    # already added
                    return
                edges.add( (i, j) )
                edge_points.append(points[ [i, j] ])

            # loop over triangles:
            # ia, ib, ic = indices of corner points of the triangle
            for ia, ib, ic in tri.vertices:
                add_edge(ia, ib)
                add_edge(ib, ic)
                add_edge(ic, ia)
            '''

            '''
            edges=set()

            for ia, ib, ic, id in tri.simplices:
                edges.add((ia,ib))
                edges.add((ib,ic))
                edges.add((ic,id))

            for e in edges:
                p1=particles_surface_1[e[0]]
                p2=particles_surface_1[e[1]]
                print(p1,p2,e[0],e[1])
                IMP.atom.Bonded.setup_particle(p1)
                IMP.atom.Bonded.setup_particle(p2)
                IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)
            '''

            for i in range(len(particles_surface_1)-1):
                p1=particles_surface_1[i]
                p2=particles_surface_1[i+1]
                if not IMP.atom.Bonded.get_is_setup(p1):
                    IMP.atom.Bonded.setup_particle(p1)
                if not IMP.atom.Bonded.get_is_setup(p2):
                    IMP.atom.Bonded.setup_particle(p2)
                IMP.atom.create_bond(IMP.atom.Bonded(p1),IMP.atom.Bonded(p2),1)
