import IMP
import IMP.core
import IMP.algebra
import IMP.domino
import IMP.atom
import IMP.spb
import IMP.container
import math

def create_representation():
    m=IMP.Model()
    mp0= IMP.atom.read_pdb('2nr9_A.pdb', m, IMP.atom.NonWaterNonHydrogenPDBSelector())
    chain=IMP.atom.get_by_type(mp0, IMP.atom.CHAIN_TYPE)[0]
    rb=IMP.atom.create_rigid_body(chain)
    return (m, chain)

def create_restraints(m, chain, up, down):
    hf = IMP.core.Harmonic(0,1)
    f = IMP.FloatKey("z")
    lpc= IMP.container.ListPairContainer(m)
    for i in range(len(up)-1):
# fix particles up
        s0=IMP.atom.Selection(chain, residue_index=up[i], atom_type = IMP.atom.AT_CA)
        s1=IMP.atom.Selection(chain, residue_index=up[i+1], atom_type = IMP.atom.AT_CA)
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        lpc.add_particle_pair([p0,p1])
# fix particles down
        s0=IMP.atom.Selection(chain, residue_index=down[i], atom_type = IMP.atom.AT_CA)
        s1=IMP.atom.Selection(chain, residue_index=down[i+1], atom_type = IMP.atom.AT_CA)
        p0=s0.get_selected_particles()[0]
        p1=s1.get_selected_particles()[0]
        lpc.add_particle_pair([p0,p1])
    df= IMP.spb.AttributeDistancePairScore(hf,f)
    pr= IMP.container.PairsRestraint(df, lpc)
    m.add_restraint(pr)

tmb=[10,63,86,116,137,166]
tme=[26,84,108,131,156,191]

up=[26,63,108,116,156,166]
down=[10,84,86,131,137,191]

print "creating representation"
(m,chain)=create_representation()

print "creating score function"
create_restraints(m,chain,up,down)

print "SCORE BEFORE ::", m.evaluate(False)

print "optimizing"
cg= IMP.core.ConjugateGradients()
cg.set_model(m)
cg.optimize(10000)

print "SCORE AFTER ::", m.evaluate(False)

print "writing pdb"
IMP.atom.write_pdb(chain,"oriented.pdb")
