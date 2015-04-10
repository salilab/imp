"""@namespace IMP.pmi.sampling_tools
   Useful tools for setting up sampling"""

import IMP
import IMP.atom

class SampleObjects(object):
    """Hack class to provide things to sample for PMI::samplers """
    def __init__(self,dict_name,pack_in_dict):
        self.d={dict_name:pack_in_dict}
    def get_particles_to_sample(self):
        return self.d

def create_floppy_bodies(particles,max_step):
    for p in particles:
        IMP.core.XYZ(p).set_coordinates_are_optimized(True)
    return [SampleObjects('Floppy_Bodies_SimplifiedModel',[particles,max_step])]

def enable_md_sampling(mdl,
                       hier=None,
                       particles=None,
                       hierarchies=None,
                       include_siblings=False,
                       exclude_backbone=False):
    """Add necessary attributes to the selected residues for MD sampling.
    @param mdl              The IMP model
    @param hier             Hierarchy to sample
    @param particles        Particles to sample
    @param include_siblings Get the siblings of the passed particles and sample them too
    @param exclude_backbone Don't sample backbone atoms
    """
    vxkey = IMP.FloatKey('vx')
    vykey = IMP.FloatKey('vy')
    vzkey = IMP.FloatKey('vz')
    backbone = [IMP.atom.AT_C,IMP.atom.AT_N, IMP.atom.AT_CA]
    if particles is None:
        particles=[]
    if hier is not None:
        particles+=IMP.atom.get_leaves(hier)
    if hierarchies is not None:
        particles+=[h.get_particle() for h in hierarchies]
    all_ps=[]
    for p in particles:
        if include_siblings:
            ps=[x.get_particle() for x in IMP.atom.Hierarchy(p).get_parent().get_children()]
        else:
            ps=[p]
        for pp in ps:
            if exclude_backbone and IMP.atom.Atom(mdl,pp.get_index()).get_atom_type() in backbone:
                continue
            IMP.core.XYZ(mdl,pp.get_index()).set_coordinates_are_optimized(True)
            mdl.add_attribute(vxkey,pp.get_index(),0.0)
            mdl.add_attribute(vykey,pp.get_index(),0.0)
            mdl.add_attribute(vzkey,pp.get_index(),0.0)
            all_ps.append(pp)
    return [SampleObjects('Floppy_Bodies_SimplifiedModel',[all_ps])]
