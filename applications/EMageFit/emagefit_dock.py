#!/usr/bin/env python

import IMP
import IMP.algebra as alg
import IMP.atom as atom
import IMP.em2d
import IMP.em2d.imp_general.representation as representation
import IMP.em2d.imp_general.io as io
import IMP.em2d.csv_related as csv_related
import csv
import sys
import logging
import subprocess
import errno
log = logging.getLogger("emagefit_dock")


"""
    This script can be used for:
    1) docking:
        $IMP python docking_related.py --dock --hex hex_transforms.txt
                --int internal_transforms.txt receptor.pdb ligand.pdb
    2) Writing docking solutions
        $IMP python docking_related.py --w 10 --int hex_transforms.txt
                                                    receptor.pdb ligand.pdb

    NOTE: The script requires the HEX docking program to perform the docking.
"""



def check_for_hexdock():
    log.debug("Checking for HEXDOCK")
    try:
        subprocess.Popen(["hex"],stdin=subprocess.PIPE ,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except OSError, e:
        if e.errno == errno.ENOENT:
            print "Docking requires the program HEXDOCK."
            print "Make sure that the command hex is available in your path."
            sys.exit()
        else:
            raise e


class HexDocking(object):

    def __init__(self, ):
        ##### commands for HEX (the %s are filled when calling self.dock)
        self.text_base = \
        """
        open_receptor %s
        open_ligand %s
        docking_correlation 1
        docking_fft_device 2
        docking_fft_type 5
        max_docking_solutions 100000
        receptor_range_angle 90
        ligand_range_angle 90
        docking_receptor_stepsize 10.0
        docking_ligand_stepsize 10.0
        docking_alpha_stepsize 6.0
        docking_r12_range 60
        docking_r12_step 2.0
        docking_main_scan 20
        activate_docking
        save_transform %s
        save_ligand %s
        """

    def dock(self, fn_receptor, fn_ligand, fn_transforms,
                                                fn_docked="docked.pdb",
                                                write_hex_dump=False ):
        """
            Docks the ligand into the receptor using HEX.
            @param fn_receptor PDB of the receptor
            @param fn_ligand PDB of the ligand
            @param fn_transformations Output fiole with the results, which
                        are the transformations of the ligand to dock it
                        on the receptor.
            @param fn_docked PDB file of the first solution
            @param write_hex_dump For debugging. If True, the output of HEX
                        is written to a file with .dump extension
        """
        log.info("===> Docking %s into %s with HEX",fn_ligand, fn_receptor)
        hex_commands = self.text_base % (fn_receptor, fn_ligand,
                                        fn_transforms, fn_docked)
        pid = subprocess.Popen(["hex"],stdin=subprocess.PIPE ,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # send the commands to hex with the PIPE file object for stdin
        pid.stdin.write(hex_commands)
        out, err = pid.communicate()
        if err != "": # there is some error output
            fn = "hexdock_errors.txt"
            f = open(fn, "w")
            f.write(err)
            f.close()
            raise IOError("Error running HEX. See file hexdoc_errors.txt")

        if write_hex_dump:
            fn = "%s-%s.dump" % (fn_receptor, fn_ligand)
            f = open(fn, "w")
            f.write(out)
            f.close()


def parse_hex_transform(row):
    """
        Returns an HEX transform with the conventions used by IMP.
        @param row A list containing a splitted line from the HEX output file
        NOTE: An HEX rotation is the inverse of the IMP convention
    """
    euler = map(float,row[8:11])
    xyz = map(float, row[5:8])
    euler = [-euler[0], -euler[1], -euler[2]]
    R = alg.get_rotation_from_fixed_zyz(*euler)
    R = R.get_inverse()
    t = alg.Vector3D(*xyz)
    return alg.Transformation3D(R, t)

def read_hex_transforms(fn_transforms):
    """
        Parses a file of hex transforms. It returns the transformations as
        alg.Transformation3D objects
        @param fn_transforms
    """
    rows = csv_related.read_csv(fn_transforms, delimiter=" ")
    return [parse_hex_transform(row) for row in rows]

def get_xlinks_are_good(coords_rec, coords_lig, distances):
    """
        Compare distances between points in coords_rec and coords_lig
        @param coords_rec A set of Vector3D objects containing coordinates from
                        the receptor
        @param coords_lig Coordinates from the ligand
        @param distances The set of maximum distances for each pair of
                                (coord_rec, coord_lig)

        @return True if all the distances between the pairs of coordinates are
                below the thresholds contained in the argument "distances""
    """
    for vi,vj,d in zip(coords_rec, coords_lig, distances):
        if alg.get_distance(vi, vj) > d:
            return False
    return True

def filter_docking_results(h_receptor, h_ligand,
                        list_xlinks, fn_transforms,
                        fn_filtered, max_number=False):
    """
        Check if the set of transforms proposed by docking with HEX is
        compatible with the distances between aminoacids from crosslinking

        @param h_receptor atom.Hierarchy for the receptor
        @param h_ligand atom.Hierarchy for the ligand
        @param list_xlinks - list of tuples with the format
                                (residue receptor, residue ligand, distance)
        @param list_xlinks - list of Xlink class
        @param fn_transforms File of transforms as given by HEX
        @param fn_filtered Output file that will contain only the
                                transformations satisfying the cross-linking
                                restraints
    """
    log.info("Filtering results of docking in %s with links:", fn_transforms)
    coords_rec = []
    coords_lig = []
    threshold_distances = []
    for xl in list_xlinks:
        log.info("%s", xl.show())
        coords_rec.append(representation.get_residue_coordinates(
                    h_receptor, xl.first_chain, xl.first_residue))
        coords_lig.append(representation.get_residue_coordinates(
                    h_ligand, xl.second_chain, xl.second_residue))
        threshold_distances.append(xl.distance);
    rows = csv_related.read_csv(fn_transforms, delimiter=" ",
                                                        max_number=max_number)
    good = []
    for r in rows:
        T = parse_hex_transform(r)
        transformed = [T.get_transformed(c) for c in coords_lig ]
        if get_xlinks_are_good(coords_rec, transformed, threshold_distances):
            good.append(r)

    # If there are not good transforms because all were filtered, the best
    # bet is to keep all of them and trust the randomization procedure.
    f_output = open(fn_filtered, "w")
    w = csv.writer(f_output, delimiter=" ")
    if len(good) ==0:
        log.warning("No docking solution satisfies all the x-linking " \
                                            "restraints. Keeping all of them")
        w.writerows(rows)
    else:
        w.writerows(good)
    f_output.close()


def get_internal_transform(Thex, rb_receptor, rb_ligand):
    """
        Get the internal transform (Ti) resulting from applying
        the transformation Thex (from HEX) that docks the ligand.
        The relationship betwen the reference frame of the ligand docked
        (Tdock)  and the internal transformation (Ti) respect to the receptor
        (Trec) is Tdock = Trec * Tinternal

        @param Thex HEX transformation
        @param rb_receptor Rigid body of the receptor
        @param rb_ligand Rigid body of the ligand
        @return The internal transformation
    """
    Tlig = rb_ligand.get_reference_frame().get_transformation_to()
    Trec = rb_receptor.get_reference_frame().get_transformation_to()
    Tdock = alg.compose(Thex, Tlig)
    Ti = alg.compose(Trec.get_inverse(), Tdock)
    return Ti

def get_docked_reference_frames(hex_transforms, rb_ligand):
    """
        @param hex_transforms A list of HEX transformations
        @param rb_ligand The rigid body of the ligand
        @return The reference frames that the rigid body of the ligand has
        when docked. The docked reference frame has transformation:
            Tdock = Thex * Tinitial
    """
    Trb = rb_ligand.get_reference_frame().get_transformation_to()
    docked_ref_frames = []
    for Thex in hex_transforms:
        Tdock = alg.compose(Thex, Trb)
        docked_rf = alg.ReferenceFrame3D(Tdock)
        docked_ref_frames.append(docked_rf)
    return docked_ref_frames


if __name__ == "__main__":
    parser = IMP.OptionParser(
                      usage="""%prog [options] fn_receptor fn_ligand

fn_receptor and fn_ligand are the filenames of the PDB files for the
receptor and ligand, respectively.""",
                      description="Functions related to docking with HEX",
                      imp_module=IMP.em2d)
    parser.add_option("--dock",
                      action="store_true", dest="dock", default=False,
                      help="Dock the ligand into the receptor")
    parser.add_option("--hex", dest="fn_transforms", default=False,
                      help="File where to write the hex transforms.")
    parser.add_option("--w", type=int, dest="write", default=False,
                      help="Write a number of docking transformations" )
    parser.add_option("--int", dest="fn_internal_transforms", default=False,
                      help="Input/Output file of internal transformations")
    parser.add_option("--log", dest="log", default=None,
                      help="File for logging ")

    opts, args = parser.parse_args()
    if len(args) != 2:
        parser.error("Wrong number of arguments")
    fn_receptor, fn_ligand = args

    if(opts.log):
        logging.basicConfig(filename=opts.log, filemode="w")
    else:
        logging.basicConfig(stream=sys.stdout)
    logging.root.setLevel(logging.DEBUG)

    sel = atom.ATOMPDBSelector()
    m = IMP.Model()
    h_receptor =  atom.read_pdb(fn_receptor, m, sel)
    rb_receptor = atom.create_rigid_body(h_receptor)
    h_ligand =  atom.read_pdb(fn_ligand, m, sel)
    rb_ligand = atom.create_rigid_body(h_ligand)
    if opts.dock:
        check_for_hexdock()
        if not opts.fn_transforms or not opts.fn_internal_transforms:
            raise IOError("Docking requires the --int and --hex arguments")
        hex_docking = HexDocking()
        hex_docking.dock(fn_receptor, fn_ligand, opts.fn_transforms)
        # read the HEX file of solutions and get the internal transformations
        # giving the relative orientation of the ligand respect to the receptor
        Ts = read_hex_transforms(opts.fn_transforms)
        Tis = [get_internal_transform(T, rb_receptor, rb_ligand) for T in Ts]
        io.write_transforms(Tis, opts.fn_internal_transforms)
    elif opts.write:
        # To write the positions correctly, the script requires that the
        # ligand file is the same that was used for the docking
        Tinternal = io.read_transforms(opts.fn_internal_transforms)
        max_number = min(opts.write, len(Tinternal))
        Trec = rb_receptor.get_reference_frame().get_transformation_to()
        for i in range(max_number):
            Tdock = alg.compose(Trec, Tinternal[i])
            ref = alg.ReferenceFrame3D(Tdock)
            rb_ligand.set_reference_frame(ref)
            atom.write_pdb(h_ligand,"docked-%03d.pdb" % i)
