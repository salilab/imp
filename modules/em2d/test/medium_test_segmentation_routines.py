import IMP
import IMP.test
import IMP.em2d
import os
from math import *
import random




class Tests(IMP.test.TestCase):

    def test_diffusion_filtering(self):
        """Test the diffusion filtering on Nup84 class average"""
        input_dir=self.get_input_file_name("")
        sub_dir = input_dir+"segmentation/"
        os.chdir(sub_dir)
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image("nup84.spi",srw)
        stored=IMP.em2d.Image("nup84-diff.spi",srw)
        filtered=IMP.em2d.Image()
        pixelsize = 3.9
        beta = 45
        time_steps = 400
        IMP.em2d.apply_diffusion_filter(img,filtered,beta, pixelsize,time_steps)
        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(filtered(i,j),stored(i,j),delta=0.001,
              msg="Diffusion filtering test: filteredis different from stored" \
                      " row %d col %d" % (i,j))
        os.chdir(input_dir)


    def test_fill_holes(self):
        """ test the do_fill_holes_routine """
        input_dir=self.get_input_file_name("")
        sub_dir = input_dir+"segmentation/"
        os.chdir(sub_dir)
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image("nup84.spi",srw)
        filtered=IMP.em2d.Image()
        n_stddevs = 1
        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())
        # test fill holes"
        stored=IMP.em2d.Image("nup84-fh.spi",srw)
        IMP.em2d.do_fill_holes(img,filtered,n_stddevs)
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(filtered(i,j),stored(i,j),delta=0.001,
                      msg="Fill holes test: filtered is different from stored" \
                      " row %d col %d" % (i,j))
        # test fill holes and threshold
        n_stddevs = 0.5
        stored=IMP.em2d.Image("nup84-fh-th.spi",srw)
        IMP.em2d.do_combined_fill_holes_and_threshold(img,filtered,n_stddevs)
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(filtered(i,j),stored(i,j),delta=0.001,
            msg="Fill holes and treshold: filtered is different from stored" \
                      " row %d col %d" % (i,j))
        os.chdir(input_dir)


    def test_removing_small_objects(self):
        """ test the labeling function"""
        input_dir=self.get_input_file_name("")
        sub_dir = input_dir+"segmentation/"
        os.chdir(sub_dir)
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image("nup84-open-binary.spi",srw)
        stored=IMP.em2d.Image("nup84-cleaned.spi",srw)
        background=0
        foreground=1
        percentage=0.3
        IMP.em2d.do_remove_small_objects(img,
                                         percentage,
                                         background,
                                         foreground)
        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(img(i,j),stored(i,j),delta=0.001,
                      msg="Remove objects test: filtered is different " \
                      "from stored row %d col %d" % (i,j))
        os.chdir(input_dir)


    def test_segmentation(self):
        """ test the segmentation of particles from negative stained classes"""
        input_dir=self.get_input_file_name("")
        sub_dir = input_dir+"segmentation/"
        os.chdir(sub_dir)
        srw = IMP.em2d.SpiderImageReaderWriter()
        img=IMP.em2d.Image("img-to-segment.spi",srw)
        stored=IMP.em2d.Image("img-segmented.spi",srw)
        segmented=IMP.em2d.Image()
        params=IMP.em2d.SegmentationParameters()
        params.image_pixel_size=2.7
        params.diffusion_timesteps=0
        params.diffusion_beta =0
        params.fill_holes_stddevs=0.5
        IMP.em2d.do_segmentation(img,segmented,params)
        rows=int(img.get_header().get_number_of_rows())
        cols=int(img.get_header().get_number_of_columns())
        for i in range(0,rows):
            for j in range(0,cols):
                self.assertAlmostEqual(segmented(i,j),stored(i,j),delta=0.001,
                      msg="Remove objects test: filtered is different " \
                      "from stored row %d col %d" % (i,j))
        os.chdir(input_dir)


if __name__ == '__main__':
    IMP.test.main()
