## \example domino/rigid_body_excluded_volume.py
## This example shows using two rigid bodies and doing excluded volume with them.

import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import math

# create a rigid body per helix
def create_representation():
    m=IMP.Model()
    h0= IMP.atom.read_pdb(IMP.domino.get_example_path('helix_0.pdb'), m, IMP.atom.CAlphaPDBSelector())
    h1= IMP.atom.read_pdb(IMP.domino.get_example_path('helix_1.pdb'), m, IMP.atom.CAlphaPDBSelector())
    for h in [h0, h1]:
        IMP.atom.create_rigid_body(h)
    return (m, [h0, h1])

def create_excluded_volume(m, helices):
    # this is the interesting function:
    # it uses a KClosePairsPair score to generate the list of close atoms on the fly
    all=[]
    for h in helices:
        all.extend(IMP.atom.get_by_type(h, IMP.atom.ATOM_TYPE))
    lsc= IMP.container.ListSingletonContainer(all)
    evr= IMP.core.ExcludedVolumeRestraint(lsc, 1)
    evr.set_model(m)
    evr.set_maximum_score(.01)
    return [evr]


# creating the discrete states for domino
def  create_discrete_states(m,helices):
    pst= IMP.domino.ParticleStatesTable()
    rot00=  IMP.algebra.get_rotation_about_axis(IMP.algebra.Vector3D(0,1,0), math.pi/2.0)
    trs=[]
    zv=IMP.algebra.Vector3D(0,0,0)
    trs.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot00,zv)))
    for dx in range(0,15):
        tr=IMP.algebra.Vector3D(1.0*dx,0,0)
        trs.append(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot00,tr)))
    pstate= IMP.domino.RigidBodyStates(trs)
    for h in helices:
        pst.set_particle_states(IMP.core.RigidMember(h).get_rigid_body(), pstate)
    return pst

# setting up domino (and filters)
def create_sampler(m, rs, pst):
    s=IMP.domino.DominoSampler(m, pst)
    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    rc= IMP.domino.RestraintCache(pst)
    rc.add_restraints(rs)
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(rc))
    states= IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)
    s.set_assignments_table(states)
    s.set_subset_filter_tables(filters)
    return s

def display(m,helices,name):
    m.update()
    w= IMP.display.PymolWriter(name)
    for i,h in enumerate(helices):
        g= IMP.atom.HierarchyGeometry(h)
        g.set_color(IMP.display.get_display_color(i))
        w.add_geometry(g)

IMP.base.set_log_level(IMP.base.SILENT)
print "creating representation"
(m,helices)=create_representation()

print "creating score function"
rs=create_excluded_volume(m,helices)

print "creating discrete states"
pst=create_discrete_states(m,helices)

print "creating sampler"
s=create_sampler(m, rs, pst)
m.set_log_level(IMP.base.SILENT)
IMP.base.set_log_level(IMP.base.VERBOSE)
print "sampling"
cs=s.get_sample()

print "found ", cs.get_number_of_configurations(), "solutions"
score=[]
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    ss=m.evaluate(False)
    score.append(ss)
    print "** solution number:",i," is:",ss
    display(m,helices,"sol_"+str(i)+".pym")
