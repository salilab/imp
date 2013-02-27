#!/usr/bin/env python

import IMP
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import os
import sys
import time
import logging
log = logging.getLogger("score_model")

def score_model(complete_fn_model,
                images_sel_file,
                pixel_size,
                n_projections=20,
                resolution=1,
                images_per_batch=250):
    """ Score a model

    Scores a model against the images in the selection file.
    Reads the images in batchs to avoid memory problems
    resolution and pixel_size are used for generating projections of the model
    The finder is an em2d.ProjectionFinder used for projection matching and optimizations
    """
    print "SCORING MODEL:",complete_fn_model
    cwd = os.getcwd()

    images_dir, nil  =  os.path.split(images_sel_file)
    images_names = em2d.read_selection_file(images_sel_file)
    n_images = len(images_names)
    if(n_images == 0):
        raise ValueError(" Scoring with a empty set of images")

    # TYPICAL OPTIMIZER PARAMETERS
    params = em2d.Em2DRestraintParameters(pixel_size, resolution)
    params.coarse_registration_method = em2d.ALIGN2D_PREPROCESSING
    params.optimization_steps = 4
    params.simplex_initial_length = 0.1
    params.simplex_minimum_size=0.02
    params.save_match_images = True
    score_function = em2d.EM2DScore()
    finder = em2d.ProjectionFinder()
    finder.setup(score_function, params)

    # Get the number of rows and cols from the 1st image
    srw = em2d.SpiderImageReaderWriter()
    test_imgs = em2d.read_images([os.path.join(images_dir, images_names[0])], srw)
    rows = test_imgs[0].get_header().get_number_of_columns()
    cols = test_imgs[0].get_header().get_number_of_rows()

    model = IMP.Model()
    ssel = atom.ATOMPDBSelector()
    prot =  atom.read_pdb(complete_fn_model, model, ssel)
    particles = IMP.core.get_leaves(prot)
    # generate projections
    proj_params = em2d.get_evenly_distributed_registration_results(n_projections)
    opts = em2d.ProjectingOptions(pixel_size, resolution)
    projections = em2d.get_projections(particles, proj_params, rows, cols, opts)

    finder.set_model_particles(particles)
    finder.set_projections(projections)
    optimized_solutions= 2
    finder.set_fast_mode(optimized_solutions)
    # read the images in blocks to avoid memory problems
    all_registration_results = []
    init_set = 0
    init_time = time.time()
    while(init_set < n_images):
        end_set = min( init_set + images_per_batch, n_images )
        if(images_dir != ""):
            os.chdir(images_dir)
        subjects = em2d.read_images(images_names[init_set:end_set], srw)
        # register
        finder.set_subjects(subjects)
        os.chdir(cwd)
        finder.get_complete_registration()
        # Recover the registration results:
        registration_results = finder.get_registration_results()
        for reg in registration_results:
            all_registration_results.append(reg)
        init_set += images_per_batch
    os.chdir(cwd)
    em2d.write_registration_results("registration.params", all_registration_results)
    print "score_model: time complete registration",time.time()-init_time
    print "coarse registration time",finder.get_coarse_registration_time()
    print "fine registration time",finder.get_fine_registration_time()
    return all_registration_results

def parse_args():
    parser = IMP.OptionParser(usage="""%prog model selfile pxsize nproj res nimg

model:   PDB file of the model to score
selfile: Selection file containing the images used for scoring
pxsize:  Pixel size of the images in Angstrom/pixel
nproj:   Number of projections of the model used for start registering
res:     Resolution to generate the projections
nimg:    Images per batch used when scoring a lot of images (to avoid memory
         problems)
""",
                              description="EMageFit scoring",
                              imp_module=em2d)
    opts, args = parser.parse_args()
    if len(args) != 6:
        parser.error("wrong number of arguments")
    return args


if __name__ == "__main__":
    args = parse_args()
    complete_fn_model  = args[0]
    images_sel_file = args[1]
    pixel_size = float(args[2])
    n_projections= int(args[3])
    resolution= float(args[4])
    images_per_batch= int(args[5])

    all_regs =  score_model(complete_fn_model,
                images_sel_file,
                pixel_size,
                n_projections,
                resolution,
                images_per_batch)
    for i, reg in enumerate(all_regs):
        print "Score for image %s: %f" % (i, reg.get_score())
    print "Global score ", em2d.get_global_score(all_regs)
