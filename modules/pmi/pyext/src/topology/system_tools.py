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
        mh = IMP.atom.read_pdb(pdb_fn,mdl,sel)
    else:
        mhs = IMP.atom.read_multimodel_pdb(pdb_fn,mdl,sel)
        if model_num>=len(mhs):
            raise Exception("you requested model num "+str(model_num)+\
                            " but the PDB file only contains "+str(len(mhs))+" models")
        mh = mhs[model_num]

    if res_range==[] or res_range is None:
        sel = IMP.atom.Selection(mh,chain=chain_id,atom_type=IMP.atom.AtomType('CA'))
    else:
        sel = IMP.atom.Selection(mh,chain=chain_id,residue_indexes=range(res_range[0],res_range[1]+1),
                               atom_type=IMP.atom.AtomType('CA'))
    ret=[]

    # return final and apply offset
    for p in sel.get_selected_particles():
        res = IMP.atom.Residue(IMP.atom.Atom(p).get_parent())
        res.set_index(res.get_index() + offset)
        ret.append(res)
    return ret

def build_bead(model,residues,input_coord=None):
    """Generates a single bead"""

    ds_frag = (residues[0].get_index(), residues[-1].get_index())
    prt = IMP.Particle(model)
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

def build_necklace(model,residues, resolution, input_coord=None):
    """Generates a string of beads with given length"""
    out_hiers = []
    for chunk in list(IMP.pmi.tools.list_chunks_iterator(residues, resolution)):
        out_hiers.append(build_bead(model,chunk, input_coord=input_coord))
    return out_hiers

def build_ca_centers(model,residues):
    """Create a bead on the CA position with coarsened size and mass"""
    out_hiers = []
    for tempres in residues:
        residue = tempres.get_hierarchy()
        rp1 = IMP.Particle(model)
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

def build_ideal_helix(model, residues, resolution):
    """Creates an ideal helix from the specified residue range
    Residues MUST be contiguous and be implemented at resolution = 1"""

    from math import pi, cos, sin

    protein_h = self.hier_dict[name]
    out_hiers = []

    start = resrange[0]
    end = resrange[1]
    self.elements[name].append((start, end, " ", "helix"))
    c0 = IMP.atom.Chain.setup_particle(IMP.Particle(self.m), "X")
    for n, res in enumerate(range(start, end + 1)):
        if name in self.sequence_dict:
            try:
                rtstr = self.onetothree[
                    self.sequence_dict[name][res-1]]
            except:
                rtstr = "UNK"
            rt = IMP.atom.ResidueType(rtstr)
        else:
            rt = IMP.atom.ResidueType("ALA")

        # get the residue volume
        try:
            vol = IMP.atom.get_volume_from_residue_type(rt)
            # mass=IMP.atom.get_mass_from_residue_type(rt)
        except IMP.ValueException:
            vol = IMP.atom.get_volume_from_residue_type(
            IMP.atom.ResidueType("ALA"))
            # mass=IMP.atom.get_mass_from_residue_type(IMP.atom.ResidueType("ALA"))
        radius = IMP.algebra.get_ball_radius_from_volume_3d(vol)

        r = IMP.atom.Residue.setup_particle(IMP.Particle(self.m), rt, res)
        p = IMP.Particle(self.m)
        d = IMP.core.XYZR.setup_particle(p)
        x = 2.3 * cos(n * 2 * pi / 3.6)
        y = 2.3 * sin(n * 2 * pi / 3.6)
        z = 6.2 / 3.6 / 2 * n * 2 * pi / 3.6
        d.set_coordinates(IMP.algebra.Vector3D(x, y, z))
        d.set_radius(radius)
        # print d
        a = IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
        r.add_child(a)
        c0.add_child(r)

    out_hiers += self.coarse_hierarchy(name, start, end,
                                      resolutions, False, c0, protein_h, "helix", color)

    if show:
        IMP.atom.show_molecular_hierarchy(protein_h)

    del c0
    return out_hiers

def recursive_show_representations(root):
    pass

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
    ret = []
    atomic_res = 0
    ca_res = 1
    mdl = parent.get_model()

    # first get the primary representation (currently, the smallest bead size)
    #  eventually we won't require beads to be present at all
    primary_resolution = min(rep.bead_resolutions)

    # if collective densities, will return single node with everything
    # below we sample or read the GMMs and add them as representation
    single_node = False
    if rep.density_residues_per_component!=None:
        single_node = True
        num_components = len(rep.residues)//rep.density_residues_per_component+1
        rep_dict = defaultdict(list)
        root_representation = IMP.atom.Representation.setup_particle(IMP.Particle(mdl),
                                                                     primary_resolution)
        parent.add_child(root_representation)
        res_nums = [r.get_index() for r in rep.residues]
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
            IMP.isd.gmm_tools.write_gmm_to_map(density_ps,rep.density_prefix+'.mrc',rep.density_voxel_size)

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
    for nr,r in enumerate(rsort):
        if r.get_index()!=prev_idx+1 or r.get_has_structure()!=prev_structure or \
           r.get_index() in rep.bead_extra_breaks:
            segments.append(cur_seg)
            cur_seg = []
        cur_seg.append(r)
        prev_idx = r.get_index()
        prev_structure = r.get_has_structure()
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
            parent.add_child(this_representation)
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
                #uncomment this when we can write self-densities to RMF
                #this_representation.add_representation(this_resolution,
                #                                       IMP.atom.DENSITIES,
                #                                       resolution)

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
