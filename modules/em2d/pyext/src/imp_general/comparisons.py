

import IMP
import IMP.algebra as alg
import IMP.core as core
import IMP.em as em
import IMP.atom as atom
import IMP.em2d.imp_general.alignments as alignments
import IMP.em2d.imp_general.representation as representation

import random
import itertools
import math
import csv
import itertools
import logging
log = logging.getLogger("comparisons")

"""
    Compute discrepancy measures between two PDBS. The first one is
    considered the reference, and for the second one, calculate:
        - RMSD
        - Distance error of each of the chains
        - Rotation error of each of the chains
        - Average distance and rotation errors

"""
class ModelInfo:
    rmsd = 0.0
    nat_over = 0.0
    rmsd_calpha = 0.0

    def __init__(self):
        self.chains_infos = []

    def get_average_placement_distance(self):
        dist = sum([c.placement_distance for c in self.chains_infos])
        return dist/len(self.chains_infos)

    def get_average_placement_angle(self):
        ang = sum([c.placement_angle for c in self.chains_infos])
        return ang/len(self.chains_infos)

    def get_info(self):
        info= [ "model",
                           "%d" % self.model_id,
                           "%f" % self.rmsd,
                           "%f" % self.rmsd_calpha,
                           "%f" % self.nat_over,
                           "%f" % self.get_average_placement_distance(),
                           "%f" % self.get_average_placement_angle(),
                           "%f" % self.drms_alpha,
                           "%f" % self.radius_g,
              ]
        return info

    def get_text(self):
        return "|".join( self.get_info())

    def get_comment_line(self):
        line = " | ".join(["model", "rmsd", "rmsd_calpha",
                    "native_overlap", "placement_distance", "placement_angle",
                    "drms_alpha","radius gyration"])
        line = "# "+line
        return line



class ChainInfo:
    chain_id = ''
    phi = 0
    theta = 0
    psi = 0
    trans_x = 0
    trans_y = 0
    trans_z = 0
    rmsd = 0
    nat_over = 0
    placement_distance = 0
    placement_angle = 0
    transformation = None

    def get_info(self):
        info= [ "chain",
                           "%d" % self.model_id,
                           "%s" % self.chain_id,
                           "%f" % self.phi,
                           "%f" % self.theta,
                           "%f" % self.psi,
                           "%f" % self.trans_x,
                           "%f" % self.trans_y,
                           "%f" % self.trans_z,
                           "%f" % self.rmsd,
                           "%f" % self.nat_over,
                           "%f" % self.placement_distance,
                           "%f" % self.placement_angle,
              ]
        return info

    def get_text(self):
        return "|".join( self.get_info())

    def get_comment_line(self):
        line = " | ".join(["chain", "rotation ZYZ (radians)",
                    "translation (Angstrom)", "rmsd",
                    "native_overlap", "placement_distance", "placement_angle"])
        line = "# "+line
        return line


def get_native_model_info(fn_pdb1):
    """
        Fills the information for the native structure
    """
    model_info = ModelInfo()
    sel = atom.ATOMPDBSelector()
    m1 = IMP.Model()
    hierarchy1 =  atom.read_pdb(fn_pdb1, m1, sel)
    sel = atom.CAlphaPDBSelector()
    malphas1 = IMP.Model()
    alphas1 =  atom.read_pdb(fn_pdb1, malphas1, sel)
    axyzs1 = [core.XYZ(l) for l in atom.get_leaves(alphas1)]
    model_info.rmsd_calpha = 0.0
    model_info.drms_alpha = 0.0
    model_info.rmsd = 0.0
    model_info.nat_over = 100.
    xyzs1 = [core.XYZ(l) for l in atom.get_leaves(hierarchy1)]
    model_info.radius_g = atom.get_radius_of_gyration(xyzs1)
    h_chains1 = atom.get_by_type(hierarchy1, atom.CHAIN_TYPE)
    for hc1 in h_chains1:
        ch1 = atom.Chain(hc1)
        chain_info = ChainInfo()
        chain_info.chain_id = ch1.get_id()
        chain_info.placement_distance = 0.0
        chain_info.placement_angle = 0.0
        chain_info.rmsd =  0.0
        native_overlap_threshold = 10
        chain_info.nat_over = 0.0
        chain_info.model_id = -1
        model_info.chains_infos.append(chain_info)
    model_info.model_id = -1
    return model_info


def get_placement_score(reference_rb, rb):
    reference_centroid = reference_rb.get_coordinates()
    centroid = rb.get_coordinates()
    translation_vector = reference_centroid - centroid
    distance = translation_vector.get_magnitude()
    ref_coords =  [m.get_coordinates() for m in reference_rb.get_members()]
    coords = [m.get_coordinates() for m in rb.get_members()]
    if(len(ref_coords) != len(coords) ):
        raise ValueError(
          "Mismatch in the number of members: reference model %d, " \
                "model to measure %d " % (len(ref_coords), len(coords)) )
    TT = alg.get_transformation_aligning_first_to_second(coords, ref_coords)
    P = alg.get_axis_and_angle( TT.get_rotation() )
    angle = P.second
    return distance, angle


def get_placement_scores(reference_rbs, rbs):
    distances = []
    angles = []
    for reference_rb, rb in zip(reference_rbs, rbs):
        distance, angle = get_placement_score(reference_rb, rb)
        distances.append(distance)
        angles.append(angle)
    return distances, angles

def get_placement_scores_from_coordinates(model_components_coords,
                                         native_components_coords):
    distances = []
    angles = []
    for model_coords, native_coords  in zip(
                        model_components_coords,native_components_coords):
        distance, angle = get_placement_score_from_coordinates(model_coords,
                                                                native_coords)
        distances.append(distance)
        angles.append(angle)
    return distances, angles


def get_placement_score_from_coordinates(model_coords, native_coords):
    """
        Computes the position error (placement distance) and the orientation
        error (placement angle) of the coordinates in model_coords respect to
        the coordinates in native_coords.
        placement distance - translation between the centroids of the
                            coordinates
        placement angle - Angle in the axis-angle formulation of the rotation
        aligning the two rigid bodies.
    """
    native_centroid = alg.get_centroid(native_coords)
    model_centroid = alg.get_centroid(model_coords)
    translation_vector = native_centroid - model_centroid
    distance = translation_vector.get_magnitude()
    if(len(model_coords) != len(native_coords) ):
        raise ValueError(
          "Mismatch in the number of members %d %d " % (
                                                len(model_coords),
                                                len(native_coords)) )
    TT = alg.get_transformation_aligning_first_to_second(model_coords,
                                                            native_coords)
    P = alg.get_axis_and_angle( TT.get_rotation() )
    angle = P.second
    return distance, angle



def align_and_get_placement_scores(reference_rbs, rbs):
    """
        reference_rbs - Reference rigid bodies.
        The rigid bodies are aligned using their centroids.
        NOTE: The function changes but restores the reference frames of rbs.
                 The rigid bodies refs_rbs of the reference are not changed.
    """
    saved_refs = [rb.get_reference_frame() for rb in rbs]
    best_rmsd, new_refs = \
        alignments.get_reference_frames_aligning_rbs(rbs, reference_rbs)
    representation.set_reference_frames(rbs, new_refs)
    distances, angles = get_placement_scores(reference_rbs, rbs)
    representation.set_reference_frames(rbs, saved_refs)
    return distances, angles


def get_rmsd(hierarchy1, hierarchy2):
    xyz1 = [core.XYZ(l) for l in atom.get_leaves(hierarchy1)]
    xyz2 = [core.XYZ(l) for l in atom.get_leaves(hierarchy2)]
    return atom.get_rmsd(xyz1, xyz2)


def get_ccc(native_assembly, assembly, resolution, voxel_size,
                            threshold, write_maps=False):
    """
        Threshold - threshold used for the map of the native assembly. Pixels
        with values above this threshold in the native map are used for the
        calculation of the cross_correlation_coefficient
    """
    import IMP.em as em
    particles_native = atom.get_leaves(native_assembly)
    particles_solution = atom.get_leaves(assembly)
    bb_native = core.get_bounding_box(core.XYZs(particles_native))
    bb_solution = core.get_bounding_box(core.XYZs(particles_solution))
    # bounding box enclosing both the particleso of the native assembly and
    #  and the particles of the model
    bb_union = alg.get_union(bb_native, bb_solution)
    # add border of 4 voxels
    border = 4*voxel_size
    bottom = bb_union.get_corner(0)
    bottom += alg.Vector3D(-border, -border, -border)
    top = bb_union.get_corner(1)
    top += alg.Vector3D(border, border, border)
    bb_union = alg.BoundingBox3D(bottom, top)


    mrw = em.MRCReaderWriter()
    header = em.create_density_header(bb_union, voxel_size)
    header.set_resolution(resolution)

    map_native = em.SampledDensityMap(header)
    map_native.set_particles(particles_native)
    map_native.resample()

    map_solution = em.SampledDensityMap(header)
    map_solution.set_particles(particles_solution)
    map_solution.resample()

    if(write_maps):
        em.write_map(map_solution, "map_solution.mrc", mrw)
        em.write_map(map_native, "map_native.mrc", mrw)
    map_native.calcRMS(), map_solution.calcRMS()
    coarse_cc = em.CoarseCC()
    # base the calculation of the cross_correlation coefficient on the threshold]
    # for the native map, because the threshold for the map of the model changes
    # with each model
#    lccc = coarse_cc.local_cross_correlation_coefficient(map_solution,
#                                                        map_native ,0.25)
    ccc = coarse_cc.cross_correlation_coefficient(map_solution,
                                                        map_native, threshold)
    log.debug(
        "cross_correlation_coefficient (based on native_map treshold) %s", ccc)
    return ccc


def get_drms_for_backbone(assembly, native_assembly):
    """
        Measure the DRMS ob the backbone between two assemblies.
        @param assembly The DRMS is computed for this assembly
        @param native_assembly The assembly that acts as a reference

       Notes:
          1) The components of the assembly can be proteins or nucleic acids
          2) If a protein, the c-alphas are used for calculating the drms
          3) If a nucleic acid, the backbone of C4' atoms is used
          4) The chains are treated as rigid bodies to speed the calculation.

        WARNING: if the function fails with a segmentation fault, one of the
        possible problems is that IMP reads some HETATM as calphas. Check that
        the chain does not have heteroatoms.
    """
    begin_range = 0
    ranges = []
    backbone = []
    h_chains = atom.get_by_type(assembly, atom.CHAIN_TYPE)
#    log.debug("Hierarchy 1 Chains %s",len(h_chains1))
    for h in h_chains:
        atoms = representation.get_backbone(h)
        backbone.extend(atoms)
        end_range = begin_range + len(atoms)
        ranges.append((begin_range, end_range ))
        begin_range = end_range
#    log.debug("Ranges %s number of CA %s", ranges, len(calphas1))

    xyzs = [core.XYZ(l) for l in backbone]
    native_chains = atom.get_by_type(native_assembly, atom.CHAIN_TYPE)
    native_backbone = []
    for h in native_chains:
        native_backbone.extend( representation.get_backbone(h))
    native_xyzs = [core.XYZ(l) for l in native_backbone]
    if(len(xyzs) != len(native_xyzs)):
        raise ValueError(
            "Cannot compute DRMS for sets of atoms of different size")
    drms = atom.get_rigid_bodies_drms(xyzs, native_xyzs, ranges)
    if(drms < 0 or math.isnan(drms) or drms > 100):
        log.debug("len(xyzs) = %s. len(native_xyzs) = %s",len(xyzs), len(native_xyzs))
        atom.write_pdb(native_assembly, "drms_filtering_calphas.pdb")
        raise ValueError("There is a problem with the drms")
    return drms
