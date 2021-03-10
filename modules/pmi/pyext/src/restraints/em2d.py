"""@namespace IMP.pmi.restraints.em2d
Restraints for handling electron microscopy images.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.pmi.tools
import ihm.location
import ihm.dataset


class ElectronMicroscopy2D(object):
    """Fit particles against a set of class averages by principal components.
    Compares how well the principal components of the segmented class
    average fit to the principal components of the particles.
    """
    def __init__(self, hier, images, pixel_size, image_resolution,
                 projection_number, resolution, micrographs_number=None,
                 n_components=1):
        """Constructor.
        @param hier The root hierarchy for applying the restraint
        @param images 2D class average filenames in PGM text format
        @param pixel_size Pixel size in angstroms
        @param image_resolution Estimated resolution of the images
               in angstroms
        @param projection_number Number of projections of the model
               to generate and fit to images. The lower the number, the
               faster the evaluation, but the lower the accuracy
        @param resolution Which level of
               [model representation](@ref pmi_resolution) to use in the fit
        @param micrographs_number Number of micrograph particles that
               were used to generate the class averages, if known
        @param n_components Number of the largest components to be
               considered for the EM image
        """

        import IMP.em2d

        self.datasets = []
        for image in images:
            loc = ihm.location.InputFileLocation(
                image, details="Electron microscopy class average")
            d = ihm.dataset.EM2DClassDataset(loc)
            self.datasets.append(d)

        for p, state in IMP.pmi.tools._all_protocol_outputs(hier):
            for i in range(len(self.datasets)):
                p.add_em2d_restraint(state, self, i, resolution, pixel_size,
                                     image_resolution, projection_number,
                                     micrographs_number)

        # PMI2 selection
        self.m = hier.get_model()
        particles = IMP.atom.Selection(
                hier, resolution=resolution).get_selected_particles()
        self.weight = 1.0
        self.rs = IMP.RestraintSet(self.m, 'em2d')
        self.label = "None"

        # read PGM FORMAT images
        # format conversion recommendation - first run
        # "e2proc2d.py $FILE ${NEW_FILE}.pgm"
        # then, run "convert ${NEW_FILE}.pgm -compress none ${NEW_FILE2}.pgm"

        # Number of the largest components to be considered for the EM image
        if n_components >= 2:
            em2d = IMP.em2d.PCAFitRestraint(
                particles, images, pixel_size, image_resolution,
                projection_number, True, n_components)
        else:
            em2d = IMP.em2d.PCAFitRestraint(
                particles, images, pixel_size, image_resolution,
                projection_number, True)
        self._em2d_restraint = em2d
        self._num_images = len(images)
        self.rs.add_restraint(em2d)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs, add_to_rmf=True)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        output = {}
        score = self.weight*self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ElectronMicroscopy2D_" + self.label] = str(score)
        # For each image, get the transformation that places the
        # model on the image, and its cross correlation coefficient
        for i in range(self._num_images):
            prefix = 'ElectronMicroscopy2D_%s_Image%d' % (self.label, i+1)
            ccc = self._em2d_restraint.get_cross_correlation_coefficient(i)
            output[prefix + '_CCC'] = str(ccc)
            tran = self._em2d_restraint.get_transformation(i)
            r = tran.get_rotation().get_quaternion()
            t = tran.get_translation()
            for j in range(4):
                output[prefix + '_Rotation%d' % j] = str(r[j])
            for j in range(3):
                output[prefix + '_Translation%d' % j] = str(t[j])
        return output


class ElectronMicroscopy2D_FFT(object):
    """FFT based image alignment, developed by Javier Velazquez-Muriel"""
    def __init__(self, hier, images=None, pixel_size=None,
                 image_resolution=None, projection_number=None,
                 resolution=None):
        """Constructor.
        @param hier The root hierarchy for applying the restraint
        @param images SPIDER FORMAT images (format conversion should be done
               through EM2EM)
        @param pixel_size sampling rate of the available EM images (angstroms)
        @param image_resolution resolution at which you want to generate the
               projections of the model. In principle you want "perfect"
               projections, so use the highest resolution
        @param projection_number Number of projections of the model (coarse
               registration) to estimate the registration parameters
        @param resolution Which level of
               [model representation](@ref pmi_resolution) to use in the fit
        @param n_components Number of the largest components to be
               considered for the EM image
        """

        import IMP.em2d

        # check input
        if images is None:
            raise Exception("EM2D_FFT: must pass images")
        if pixel_size is None:
            raise Exception("EM2D_FFT: must pass pixel size")
        if image_resolution is None:
            raise Exception("EM2D_FFT: must pass image resolution")
        if projection_number is None:
            raise Exception("EM2D_FFT: must pass projection_number")

        # PMI2 selection
        self.m = hier.get_model()
        particles = IMP.atom.Selection(
                hier, resolution=resolution).get_selected_particles()

        self.weight = 1.0
        self.rs = IMP.RestraintSet(self.m, 'em2d_FFT')
        self.label = "None"

        # read
        srw = IMP.em2d.SpiderImageReaderWriter()
        imgs = IMP.em2d.read_images(images, srw)

        params = IMP.em2d.Em2DRestraintParameters(
            pixel_size, image_resolution, projection_number)

        # This method (recommended) uses preprocessing of the images and
        # projections to speed-up the registration
        params.coarse_registration_method = IMP.em2d.ALIGN2D_PREPROCESSING
        params.optimization_steps = 50
        params.simplex_initial_length = 0.1
        params.simplex_minimum_size = 0.02

        # use true if you want to save the projections from the model that
        # best match the EM images
        params.save_match_images = False

        ######################
        # set up the em2D restraint
        ######################
        score_function = IMP.em2d.EM2DScore()
        em2d_restraint = IMP.em2d.Em2DRestraint(self.m)
        em2d_restraint.setup(score_function, params)
        em2d_restraint.set_images(imgs)
        em2d_restraint.set_fast_mode(5)
        em2d_restraint.set_name("em2d_restraint")

        print("len(particles) = ", len(particles))
        container = IMP.container.ListSingletonContainer(self.m, particles)
        em2d_restraint.set_particles(container)

        self.rs.add_restraint(em2d_restraint)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(self.weight)

    def get_output(self):
        output = {}
        score = self.weight*self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ElectronMicroscopy2D_FFT_" + self.label] = str(score)
        return output
