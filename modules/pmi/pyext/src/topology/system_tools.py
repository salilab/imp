"""@namespace IMP.pmi.topology.system_tools
   Tools to help build structures
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.pmi
from math import pi

def list_chunks_iterator(input_list, length):
    """ Yield successive length-sized chunks from a list.
    """
    for i in range(0, len(input_list), length):
        yield input_list[i:i + length]

def get_structure(mdl,pdb_fn,chain_id,res_range=[],offset=0,model_num=None,ca_only=False):
    """read a structure from a PDB file and return a list of residues
    @param mdl The IMP model
    @param pdb_fn    The file to read
    @param chain_id  Chain ID to read
    @param res_range Add only a specific set of residues
    @param offset    Apply an offset to the residue indexes of the PDB file
    @param model_num Read multi-model PDB and return that model
    @param ca_only Read only CA atoms (by default, all non-waters are read)
    """
    sel = IMP.atom.get_default_pdb_selector()
    if ca_only:
        sel = IMP.atom.CAlphaPDBSelector()
    if model_num is None:
        mh=IMP.atom.read_pdb(pdb_fn,mdl,sel)
    else:
        mhs=IMP.atom.read_multimodel_pdb(pdb_fn,mdl,sel)
        if model_num>=len(mhs):
            raise Exception("you requested model num "+str(model_num)+\
                            " but the PDB file only contains "+str(len(mhs))+" models")
        mh=mhs[model_num]

    # first update using offset:
    for rr in IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE):
        IMP.atom.Residue(rr).set_index(IMP.atom.Residue(rr).get_index()+offset)

    if res_range==[]:
        sel=IMP.atom.Selection(mh,chain=chain_id,atom_type=IMP.atom.AtomType('CA'))
    else:
        sel=IMP.atom.Selection(mh,chain=chain_id,residue_indexes=range(res_range[0],res_range[1]+1),
                               atom_type=IMP.atom.AtomType('CA'))
    ret=[]
    for p in sel.get_selected_particles():
        res=IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
        ret.append(res)
    return ret

def build_bead(model,residues,input_coord=None):
    """Generates a single bead
    """

    ds_frag = (residues[0].get_index(), residues[-1].get_index())
    prt = IMP.Particle(model)
    IMP.core.XYZR.setup_particle(prt)
    ptem = IMP.core.XYZR(prt)
    mass = IMP.atom.get_mass_from_number_of_residues(len(residues))

    if ds_frag[0] == ds_frag[-1]:
        rt=residues[0].get_residue_type()
        h =IMP.atom.Residue.setup_particle(prt, rt, ds_frag[0])
        h.set_name('%i_bead' % (ds_frag[0]))
        prt.set_name('%i_bead' % (ds_frag[0]))
        try:
            vol = IMP.atom.get_volume_from_residue_type(rt)
        except IMP.ValueException:
            vol = IMP.atom.get_volume_from_residue_type(
                IMP.atom.ResidueType("ALA"))
        radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
        ptem.set_radius(radius)
    else:
        h = IMP.atom.Fragment.setup_particle(prt)
        h.set_name('%i-%i_bead' % (ds_frag[0], ds_frag[-1]))
        prt.set_name('%i-%i_bead' % (ds_frag[0], ds_frag[-1]))
        h.set_residue_indexes(range(ds_frag[0], ds_frag[-1] + 1))
        volume = IMP.atom.get_volume_from_mass(mass)
        radius = 0.8 * (3.0 / 4.0 / pi * volume) ** (1.0 / 3.0)
        ptem.set_radius(radius)

    IMP.atom.Mass.setup_particle(prt, mass)
    try:
        if not tuple(input_coord) is None:
            ptem.set_coordinates(input_coord)
    except TypeError:
        pass

    return h

def build_necklace(model,residues, resolution, input_coord=None):
    '''
    Generates a string of beads with given length
    '''
    outhiers = []
    for chunk in list(list_chunks_iterator(residues, resolution)):
        outhiers.append(build_bead(model,chunk, input_coord=input_coord))
    return outhiers



def build_along_backbone(mdl,root,residues,rep_type,ca_centers=True):
    """Group residues along the backbone, adding beads as needed.
    Currently this first groups into contiguous fragment regions ('folders')
    with identical sets of resolutions. However this behavior may
    The current resolutions used are 0 for atomic, and N for N residues
    per ball (see @ref pmi_resolution).
    @param mdl        the model
    @param root       the hierarchy to which all the fragments and resolutions will be added
    @param residues   list of PMI Residues, labeled with resolution
    @param rep_type   Representation type (currently supports IMP.atom.BALLS)
    @param ca_centers If true, when making residue beads, will place the bead at the CA position
    """
    allowed_reps=[IMP.atom.BALLS]
    if rep_type not in allowed_reps:
        print("Only supported representation types are",allowed_types)
    prev_rep = None
    prev_atomic = False
    cur_fragment=[]
    fragments=[]

    # group into fragments with identical resolutions
    for res in residues:
        if prev_rep is None:
            prev_rep = res.representations
            prev_atomic = res.get_has_coordinates()
        rep = res.representations
        if rep==prev_rep and res.get_has_coordinates()==prev_atomic:
            cur_fragment.append(res)
        else:
            fragments.append(cur_fragment)
            cur_fragment=[res]
        prev_rep=rep
        prev_atomic=res.get_has_coordinates()
    fragments.append(cur_fragment)

    # build the representations within each fragment

    for frag_res in fragments:
        if len(frag_res[0].representations)==0:
            continue
        res_nums=[r.get_index() for r in frag_res]
        name = "frag_%i-%i"%(res_nums[0],res_nums[-1])
        if frag_res[0].get_has_coordinates():
            this_rep=frag_res[0].representations
            if len(this_rep)==0:
                continue
            frag = IMP.atom.Fragment.setup_particle(mdl,mdl.add_particle(name),res_nums)
            root.add_child(frag)

            primary_rep_num=min(this_rep['balls'])
            frep = IMP.atom.Representation.setup_particle(frag,primary_rep_num)
            if 'balls' in this_rep:
                # if atomic, add the residues as child (inside another fragment - check RMF)
                if 0 in this_rep['balls']:
                    f=IMP.atom.Fragment.setup_particle(mdl,mdl.add_particle("resolution 0"),
                                                       res_nums)
                    for residue in frag_res:
                        f.add_child(residue.hier)
                    frag.add_child(f)

                # add one-residue-per-bead
                if 1 in this_rep['balls']:
                    res1=IMP.atom.Fragment.setup_particle(mdl,mdl.add_particle("resolution 1"),res_nums)
                    for residue in frag_res:
                        if ca_centers==True:
                            rp1 = IMP.Particle(mdl)
                            rp1.set_name("Residue_%i"%residue.get_index())
                            rt=residue.get_residue_type()
                            res1.add_child(IMP.atom.Residue.setup_particle(rp1,residue.hier))
                            try:
                                vol = IMP.atom.get_volume_from_residue_type(rt)
                            except IMP.ValueException:
                                vol = IMP.atom.get_volume_from_residue_type(
                                    IMP.atom.ResidueType("ALA"))
                            try:
                                mass = IMP.atom.get_mass(rt)
                            except:
                                mass = IMP.atom.get_mass(IMP.atom.ResidueType("ALA"))
                            calpha = IMP.atom.Selection(residue.hier,atom_type=IMP.atom.AT_CA). \
                                       get_selected_particles()[0]
                            radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
                            shape = IMP.algebra.Sphere3D(IMP.core.XYZ(calpha).get_coordinates(),radius)
                            IMP.core.XYZR.setup_particle(rp1,shape)
                            IMP.atom.Mass.setup_particle(rp1,mass)
                    if primary_rep_num==1:
                        frag.add_child(res1)
                    else:
                        frep.add_representation(res1,IMP.atom.BALLS,1)

                # add all other resolutions
                for resolution in set(this_rep['balls']) - set([0,1]):
                    resx=IMP.atom.Fragment.setup_particle(mdl,mdl.add_particle("resolution "+str(resolution)),res_nums)

                    # we create a dummy chain hierarchy and copy
                    # residues in it, beacuse the chain is required by simplified_along_backbone
                    c=IMP.atom.Chain.setup_particle(mdl,mdl.add_particle("dummy chain"),"X")
                    for residue in frag_res:
                        c.add_child(residue.hier)
                    sh  =IMP.atom.create_simplified_along_backbone(c,resolution)
                    for ch in sh.get_children():
                        resx.add_child(ch)
                    if primary_rep_num==resolution:
                        frag.add_child(resx)
                    else:
                        frep.add_representation(resx,IMP.atom.BALLS,resolution)
                    del sh
                    del c


        else:
            # frag_res is a continuous list of non-atomic residues with the same resolutions
            this_resolutions=frag_res[0].representations['balls']
            # check that we have only one resolution for now
            if len(this_resolutions) > 1 :
                raise ValueError("build_along_backbone Error: residues with missing atomic coordinate should be associated with only one resolution")
            if len(this_resolutions) == 0 :
                print("build_along_backbone Error: no resolution associated with that residue")
                exit()
            this_resolution=list(this_resolutions)[0]
            # create a root hierarchy node for the beads
            frag = IMP.atom.Fragment.setup_particle(mdl,mdl.add_particle(name),res_nums)
            root.add_child(frag)
            frep = IMP.atom.Representation.setup_particle(frag,this_resolution)
            hiers=build_necklace(mdl,frag_res,this_resolution)
            for h in hiers:
                frag.add_child(h)

def show_representation(node):
    print(node)
    if IMP.atom.Representation.get_is_setup(node):
        repr=IMP.atom.Representation(node)
        resolutions=repr.get_resolutions()
        for r in resolutions:
            print('---- resolution %i ----' %r)
            IMP.atom.show_molecular_hierarchy(repr.get_representation(r))
        return True
    else:
        return False

def recursive_show_representations(root):
    pass
