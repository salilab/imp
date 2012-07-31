

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
#    log.debug("Measuring DRMS of the backbone")
    begin_range = 0
    ranges = []
    backbone = []
    h_chains = atom.get_by_type(assembly, atom.CHAIN_TYPE)
    for h in h_chains:
        atoms = representation.get_backbone(h)
        """
        for a in atoms:
            print "atom ===> ",
            at = atom.Atom(a)
            hr = at.get_parent()
            res = atom.Residue(hr)
            at.show()
            print " ",
            res.show()
            print ""
        """
        backbone.extend(atoms)
        end_range = begin_range + len(atoms)
        ranges.append((begin_range, end_range ))
        begin_range = end_range
#    log.debug("Ranges %s number of atoms %s", ranges, len(backbone))
    xyzs = [core.XYZ(l) for l in backbone]
    native_chains = atom.get_by_type(native_assembly, atom.CHAIN_TYPE)
    names = [atom.Chain(ch).get_id() for ch in native_chains]
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
        log.debug("drms = %s",drms)
        atom.write_pdb(native_assembly, "drms_filtering_calphas.pdb")
        raise ValueError("There is a problem with the drms")
    return drms



def get_chains_overlap(hierarchies):
    """ Check if a set of chains use the same region of space. If any of
        the pairs of chains overlap, the function returns True.
        The function converts the atoms into densities and counts the number
        of voxels from each molecule that use the same region of space.
        Only a 10% of overlap of the smaller molecule is allowed
    """
    all_points = []
    for h in hierarchies:
        all_points += [core.XYZ(l).get_coordinates() for l in atom.get_leaves(h)]
    bb = alg.BoundingBox3D(all_points)
    sizes = bb.get_corner(1) - bb.get_corner(0)

    size = max([sizes[0], sizes[1], sizes[2]]) # max dimension (Angstroms)
    voxel_size = 3
    # number of voxels for one dimension
    voxels_padding = 4 # voxels used to pad a little bit the bounding box size
    n = int(size/voxel_size) + voxels_padding

    # Same thing for the origin of the map, this way the coordinates are not
    # exactly in the edges of the box
    padding_dist = voxels_padding*voxel_size/2.
    padding_vector = alg.Vector3D(padding_dist, padding_dist, padding_dist)

    dh = em.DensityHeader()
    dh.update_map_dimensions(n, n, n)
    dh.set_resolution(1.0)

    maps = []
    chain_ids = [atom.Chain(ch).get_id() for ch in hierarchies]
    for ch in hierarchies:
        density_map = em.SampledDensityMap(dh)
        ls = atom.get_leaves(ch)
        density_map.set_particles(ls)
        density_map.update_voxel_size(voxel_size)
        density_map.set_origin(bb.get_corner(0) - padding_vector )
        # print density_map.get_origin(),density_map.get_header().show()
        density_map.resample()
        mrw = em.MRCReaderWriter()
#        em.write_map(density_map, "test-%s.mrc" % ch.get_id(), mrw)
        maps.append(density_map)

    pct = 0.2 # allowed_ratio_of_overlapping
    voxels_above_zero = [ number_of_voxels_above_threshold(m, 0.) for m in maps]
    for j, k in itertools.combinations(range(len(maps)), 2):
        overlapping_voxels = number_of_overlapping_voxels(maps[j],maps[k])
        if (overlapping_voxels/voxels_above_zero[j] > pct or
           overlapping_voxels/voxels_above_zero[k] > pct):
            print "chains",chain_ids[j],chain_ids[k],"overlap"
            return True
    return False

def number_of_overlapping_voxels(map1, map2):
    """
        Computes the overlapping voxels between maps
    """
    overlapping_voxels = 0.0
    voxels = map1.get_header().get_number_of_voxels()
    for i in range(voxels):
        if map1.get_value(i) > 0 and map2.get_value(i) > 0:
            # both maps have values in a voxel above zero. There is
            # density in both of them (overlap)
            overlapping_voxels += 1.
    return overlapping_voxels


def number_of_voxels_above_threshold(density_map, threshold):
    count = 0
    for i in range(density_map.get_header().get_number_of_voxels()):
        if density_map.get_value(i) > threshold:
            count += 1.
    return count
