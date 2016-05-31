from __future__ import print_function, division
import IMP
import IMP.atom
import IMP.isd.gmm_tools
import IMP.pmi
import IMP.pmi.tools
from collections import defaultdict
from math import pi
import os

def resnums2str(res):
    """Take iterable of TempResidues and return compatified string"""
    if len(res)==0:
        return ''
    idxs = [r.get_index() for r in res]
    idxs.sort()
    all_ranges=[]
    cur_range=[idxs[0],idxs[0]]
    for idx in idxs[1:]:
        if idx!=cur_range[1]+1:
            all_ranges.append(cur_range)
            cur_range=[idx,idx]
        cur_range[1]=idx
    all_ranges.append(cur_range)
    ret = ''
    for nr,r in enumerate(all_ranges):
        ret+='%i-%i'%(r[0],r[1])
        if nr<len(all_ranges)-1:
            ret+=', '
    return ret

def get_structure(mdl,pdb_fn,chain_id,res_range=None,offset=0,model_num=None,ca_only=False):
    """read a structure from a PDB file and return a list of residues
    @param mdl The IMP model
    @param pdb_fn    The file to read
    @param chain_id  Chain ID to read
    @param res_range Add only a specific set of residues. None gets you all
    @param offset    Apply an offset to the residue indexes of the PDB file
    @param model_num Read multi-model PDB and return that model
    @param ca_only Read only CA atoms (by default, all non-waters are read)
    """
    sel = IMP.atom.get_default_pdb_selector()
    if ca_only:
        sel = IMP.atom.CAlphaPDBSelector()
    if model_num is None:
        mh = IMP.atom.read_pdb(pdb_fn,mdl,
                               IMP.atom.AndPDBSelector(IMP.atom.ChainPDBSelector(chain_id), sel))

    else:
        mhs = IMP.atom.read_multimodel_pdb(pdb_fn,mdl,sel)
        if model_num>=len(mhs):
            raise Exception("you requested model num "+str(model_num)+\
                            " but the PDB file only contains "+str(len(mhs))+" models")
        mh = IMP.atom.Selection(mhs[model_num],chain=chain_id,with_representation=True)

    if res_range==[] or res_range is None:
        sel = IMP.atom.Selection(mh,chain=chain_id,atom_type=IMP.atom.AtomType('CA'))
    else:
        start = res_range[0]
        end = res_range[1]
        if end==-1:
            end = IMP.atom.Residue(mh.get_children()[0].get_children()[-1]).get_index()
        sel = IMP.atom.Selection(mh,chain=chain_id,residue_indexes=range(start,end+1),
                                 atom_type=IMP.atom.AtomType('CA'))
    ret=[]

    # return final and apply offset
    for p in sel.get_selected_particles():
        res = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
        res.set_index(res.get_index() + offset)
        ret.append(res)
    return ret

def build_bead(mdl,residues,input_coord=None):
    """Generates a single bead"""

    ds_frag = (residues[0].get_index(), residues[-1].get_index())
    prt = IMP.Particle(mdl)
    IMP.core.XYZR.setup_particle(prt)
    ptem = IMP.core.XYZR(prt)
    mass = IMP.atom.get_mass_from_number_of_residues(len(residues))

    if ds_frag[0] == ds_frag[-1]:
        rt = residues[0].get_residue_type()
        h = IMP.atom.Residue.setup_particle(prt, rt, ds_frag[0])
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
        if tuple(input_coord) is not None:
            ptem.set_coordinates(input_coord)
    except TypeError:
        pass
    return h

def build_necklace(mdl,residues, resolution, input_coord=None):
    """Generates a string of beads with given length"""
    out_hiers = []
    for chunk in list(IMP.pmi.tools.list_chunks_iterator(residues, resolution)):
        out_hiers.append(build_bead(mdl,chunk, input_coord=input_coord))
    return out_hiers

def build_ca_centers(mdl,residues):
    """Create a bead on the CA position with coarsened size and mass"""
    out_hiers = []
    for tempres in residues:
        residue = tempres.get_hierarchy()
        rp1 = IMP.Particle(mdl)
        rp1.set_name("Residue_%i"%residue.get_index())
        rt = residue.get_residue_type()
        this_res = IMP.atom.Residue.setup_particle(rp1,residue)
        try:
            vol = IMP.atom.get_volume_from_residue_type(rt)
        except IMP.ValueException:
            vol = IMP.atom.get_volume_from_residue_type(
                IMP.atom.ResidueType("ALA"))
        try:
            mass = IMP.atom.get_mass(rt)
        except:
            mass = IMP.atom.get_mass(IMP.atom.ResidueType("ALA"))
        calpha = IMP.atom.Selection(residue,atom_type=IMP.atom.AT_CA). \
                 get_selected_particles()[0]
        radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)
        shape = IMP.algebra.Sphere3D(IMP.core.XYZ(calpha).get_coordinates(),radius)
        IMP.core.XYZR.setup_particle(rp1,shape)
        IMP.atom.Mass.setup_particle(rp1,mass)
        out_hiers.append(this_res)
    return out_hiers

def setup_bead_as_gaussian(mh):
    """Setup bead as spherical gaussian, using radius as variance"""
    p = mh.get_particle()
    center = IMP.core.XYZ(p).get_coordinates()
    rad = IMP.core.XYZR(p).get_radius()
    mass = IMP.atom.Mass(p).get_mass()
    trans = IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),center)
    shape = IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans),[rad]*3)
    IMP.core.Gaussian.setup_particle(p,shape)


def show_representation(node):
    print(node)
    if IMP.atom.Representation.get_is_setup(node):
        repr = IMP.atom.Representation(node)
        resolutions = repr.get_resolutions()
        for r in resolutions:
            print('---- resolution %i ----' %r)
            IMP.atom.show_molecular_hierarchy(repr.get_representation(r))
        return True
    else:
        return False

def build_representation(parent,rep,coord_finder):
    """Create requested representation.
    For beads, identifies continuous segments and sets up as Representation.
    If any volume-based representations (e.g.,densities) are requested,
    will instead create a single Representation node.
    All reps are added as children of the passed parent.
    @param parent The Molecule to which we'll add add representations
    @param rep What to build. An instance of pmi::topology::_Representation
    @param coord_finder A _FindCloseStructure object to help localize beads
    """
    built_reps = []
    atomic_res = 0
    ca_res = 1
    mdl = parent.get_model()
    if rep.color is not None:
        if type(rep.color) is float:
            color = IMP.display.get_rgb_color(rep.color)
        elif hasattr(rep.color,'__iter__') and len(rep.color)==3:
            color = IMP.display.Color(*rep.color)
        elif type(rep.color) is IMP.display.Color:
            color = rep.color
        elif type(rep.color) is str:
            color = IMP.display.Color(*IMP.pmi.tools.color2rgb(rep.color))
        else:
            raise Exception("Color must be float or (r,g,b) tuple")
    else:
        color = None

    # first get the primary representation (currently, the smallest bead size)
    #  eventually we won't require beads to be present at all
    primary_resolution = min(rep.bead_resolutions)

    # if collective densities, will return single node with everything
    # below we sample or read the GMMs and add them as representation
    single_node = False # flag indicating grouping nonlinear segments with one GMM
    if rep.density_residues_per_component:
        single_node = True
        num_components = len(rep.residues)//rep.density_residues_per_component+1
        rep_dict = defaultdict(list)
        segp = IMP.Particle(mdl)
        root_representation = IMP.atom.Representation.setup_particle(segp,
                                                                     primary_resolution)
        built_reps.append(root_representation)
        res_nums = [r.get_index() for r in rep.residues]
        IMP.atom.Fragment.setup_particle(segp,res_nums)
        density_frag = IMP.atom.Fragment.setup_particle(IMP.Particle(mdl),res_nums)
        density_frag.get_particle().set_name("Densities %i"%rep.density_residues_per_component)
        density_ps = []

        if os.path.exists(rep.density_prefix+'.txt') and not rep.density_force_compute:
            IMP.isd.gmm_tools.decorate_gmm_from_text(rep.density_prefix+'.txt',
                                                     density_ps,
                                                     mdl)
        if len(density_ps)!=num_components or not os.path.exists(rep.density_prefix+'.txt') or rep.density_force_compute:
            density_ps = []
            fit_coords = []
            for r in rep.residues:
                fit_coords += [IMP.core.XYZ(p).get_coordinates() for p in IMP.core.get_leaves(r.hier)]
            IMP.isd.gmm_tools.fit_gmm_to_points(fit_coords,
                                                num_components,
                                                mdl,
                                                density_ps)
            IMP.isd.gmm_tools.write_gmm_to_text(density_ps,rep.density_prefix+'.txt')
            if rep.density_voxel_size>0.0:
                IMP.isd.gmm_tools.write_gmm_to_map(density_ps,rep.density_prefix+'.mrc',
                                                   rep.density_voxel_size,fast=True)

        for d in density_ps:
            density_frag.add_child(d)
        root_representation.add_representation(density_frag,
                                               IMP.atom.DENSITIES,
                                               rep.density_residues_per_component)

    # get continuous segments from residues
    segments = []
    rsort = sorted(list(rep.residues),key=lambda r:r.get_index())
    prev_idx = rsort[0].get_index()-1
    prev_structure = rsort[0].get_has_structure()
    cur_seg = []
    force_break = False
    for nr,r in enumerate(rsort):
        if r.get_index()!=prev_idx+1 or r.get_has_structure()!=prev_structure or force_break:
            segments.append(cur_seg)
            cur_seg = []
            force_break = False
        cur_seg.append(r)
        prev_idx = r.get_index()
        prev_structure = r.get_has_structure()
        if r.get_index()-1 in rep.bead_extra_breaks:
            force_break = True
    if cur_seg!=[]:
        segments.append(cur_seg)

    # for each segment, merge into beads
    name_all = 'frags:'
    name_count = 0
    for frag_res in segments:
        res_nums = [r.get_index() for r in frag_res]
        rrange = "%i-%i"%(res_nums[0],res_nums[-1])
        name = "Frag_"+rrange
        if name_count<3:
            name_all +=rrange+','
        elif name_count==3:
            name_all +='...'
        name_count+=1
        segp = IMP.Particle(mdl,name)
        this_segment = IMP.atom.Fragment.setup_particle(segp,res_nums)
        if not single_node:
            this_representation = IMP.atom.Representation.setup_particle(segp,primary_resolution)
            built_reps.append(this_representation)
        for resolution in rep.bead_resolutions:
            fp = IMP.Particle(mdl)
            this_resolution = IMP.atom.Fragment.setup_particle(fp,res_nums)
            this_resolution.set_name("%s: Res %i"%(name,resolution))
            if frag_res[0].get_has_structure():
                # if structured, merge particles as needed
                if resolution==atomic_res:
                    for residue in frag_res:
                        this_resolution.add_child(residue.get_hierarchy())
                elif resolution==ca_res and rep.bead_ca_centers:
                    beads = build_ca_centers(mdl,frag_res)
                    for bead in beads:
                        this_resolution.add_child(bead)
                else:
                    tempc = IMP.atom.Chain.setup_particle(IMP.Particle(mdl),"X")
                    for residue in frag_res:
                        tempc.add_child(IMP.atom.create_clone(residue.hier))
                    beads = IMP.atom.create_simplified_along_backbone(tempc,resolution)
                    for bead in beads.get_children():
                        this_resolution.add_child(bead)
                    del tempc
                    del beads
            else:
                # if unstructured, create necklace
                input_coord = coord_finder.find_nearest_coord(min(r.get_index() for r in frag_res))
                if input_coord is None:
                    input_coord = rep.bead_default_coord
                beads = build_necklace(mdl,
                                       frag_res,
                                       resolution,
                                       input_coord)
                for bead in beads:
                    this_resolution.add_child(bead)

            # if requested, color all resolutions the same
            if color:
                for lv in IMP.core.get_leaves(this_resolution):
                    IMP.display.Colored.setup_particle(lv,color)

            # finally decide where to put this resolution
            #  if volumetric, collect resolutions from different segments together
            if single_node:
                rep_dict[resolution]+=this_resolution.get_children()
            else:
                if resolution==primary_resolution:
                    this_representation.add_child(this_resolution)
                else:
                    this_representation.add_representation(this_resolution,
                                                           IMP.atom.BALLS,
                                                           resolution)
            # if individual beads to be setup as Gaussians:
            if rep.setup_particles_as_densities:
                for p in IMP.core.get_leaves(this_resolution):
                    setup_bead_as_gaussian(p)
                this_resolution.set_name(this_resolution.get_name()+' Densities %i'%resolution)
                this_representation.add_representation(this_resolution,
                                                       IMP.atom.DENSITIES,
                                                       resolution)

    if single_node:
        root_representation.set_name(name_all.strip(',')+": Base")
        d = root_representation.get_representations(IMP.atom.DENSITIES)
        d[0].set_name('%s: '%name_all + d[0].get_name())
        for resolution in rep.bead_resolutions:
            this_resolution = IMP.atom.Fragment.setup_particle(
                IMP.Particle(mdl),
                [r.get_index() for r in rep.residues])
            this_resolution.set_name("%s: Res %i"%(name_all,resolution))
            for hier in rep_dict[resolution]:
                this_resolution.add_child(hier)
            if resolution==primary_resolution:
                root_representation.add_child(this_resolution)
            else:
                root_representation.add_representation(this_resolution,
                                                       IMP.atom.BALLS,
                                                       resolution)
    return built_reps
