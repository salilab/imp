import IMP
import IMP.test
import IMP.em

# Offset of the title field (szITit) in the Spider header
TITLE_OFFSET = 864


class Tests(IMP.test.TestCase):

    def _make_header(self, title):
        h = IMP.em.ImageHeader()
        h.set_image_type(IMP.em.ImageHeader.VOL_IMPEM)
        h.set_number_of_slices(2)
        h.set_number_of_rows(3)
        h.set_number_of_columns(4)
        h.set_title(title)
        return h

    def _get_title(self, fname, title):
        with open(fname, 'rb') as fh:
            data = fh.read()
        return data[TITLE_OFFSET:TITLE_OFFSET + len(title)]

    def test_reversed_text_fields(self):
        """Text fields of byte-swapped Spider headers should not move"""
        title = b"my test title"
        for reverse in (False, True):
            h = self._make_header(title.decode())
            fname = self.get_tmp_file_name('test%d.spi' % reverse)
            h.write(fname, reverse)
            self.assertEqual(self._get_title(fname, title), title)
            # Read the header back, then write it unswapped
            h2 = IMP.em.ImageHeader()
            h2.read(fname, False, reverse, False)
            fname2 = self.get_tmp_file_name('test%d_out.spi' % reverse)
            h2.write(fname2, False)
            self.assertEqual(self._get_title(fname2, title), title)
            self.assertEqual(h2.get_number_of_columns(), 4)
            self.assertEqual(h2.get_number_of_rows(), 3)
            self.assertEqual(h2.get_number_of_slices(), 2)


if __name__ == '__main__':
    IMP.test.main()
