import IMP
import IMP.pmi
import IMP.pmi.dof
import IMP.test


class TestDOF(IMP.test.TestCase):

    def init_topology(self):
        return hierarchy

    def test_constraint_symmetry(self):
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        selections=[]
        s0=IMP.atom.Selection(hierarchy,molecule=?,copy=0)
        s1=IMP.atom.Selection(hierarchy,molecule=?,copy=1)
        dof.create_symmetry_contraint([s0,s1],IMP.core.Transformation)

    def test_mc_compound_body(self):
        # compund body is a mix of rigid and flexible parts
        # Do we use System???? Do we need a new decorator that says
        # where the structure is coming from????
        # IMP.atom.Source(p)
        # IMP.atom.Source.get_pdb_id()
        # IMP.atom.Source.get_pdb_id()
        # source can be Modeller, PDB, emdb, Coarse-grained, No-source
        # Invent StringKeys
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        s=IMP.atom.Selection(hierarchy,molecule=?,resid=range(1,10))
        structured_handle,unstructures_handle=dof.create_compound_body(s)

    def test_mc_rigid_body(self):
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        selections=[]
        s=IMP.atom.Selection(hierarchy,molecule=?,resid=range(1,10))
        selection
        for s in selections:
            # rigid_body is a subclass of dof
            rigid_body=dof.create_rigid_body(s)
            # IMP mover
            rigid_body.get_mover()
            # IMP rigid_body
            rb=rigid_body.get_rigid_body()
            # setting non-rigid-members (use)
            # rb.set_is_rigid_member(p.get_index(), False)
            non_rigid_members=rigid_body.create_non_rigid_members(selection/handle/particles)
            nrmmv=non_rigid_members[0].get_mover()

    def test_mc_super_rigid_body(self):
        hierarchy=self.init_topology()
        dof=IMP.pmi.dof.DegreesOfFreedom()
        selections=[]
        s=IMP.atom.Selection(hierarchy,molecule=?,resid=range(1,10))
        dof.create_super_rigid_body(IMP.atom.Selections/PMI_light_handles/Particles/)
        mvs=dof.get_movers()

    def test_mc_flexible_beads(self):
        pass

    def test_mc_kinematic(self):
        pass

    def test_md(self):
        pass
