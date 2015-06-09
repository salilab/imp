from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container

import IMP.pmi.io.crosslink

class TestIOCrosslink(IMP.test.TestCase):

    def test_setup_cldbkc(self):

        expected_converter={'Residue 2': 'Residue2',
                            'Residue 1': 'Residue1',
                            'Unique ID': 'XLUniqueID',
                            'Quantification': 'Quantitation',
                            'ID Score': 'IDScore',
                            'Protein 1': 'Protein1',
                            'Protein 2': 'Protein2'}

        expected_backward_converter={'XLUniqueID': 'Unique ID',
                            'IDScore': 'ID Score',
                            'Residue1': 'Residue 1',
                            'Protein2': 'Protein 2',
                            'Protein1': 'Protein 1',
                            'Residue2': 'Residue 2',
                            'Quantitation': 'Quantification'}

        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertTrue(errorstatus)
        cldbkc.set_unique_id_key("Unique ID")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertTrue(errorstatus)
        cldbkc.set_protein1_key("Protein 1")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertTrue(errorstatus)
        cldbkc.set_protein2_key("Protein 2")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertTrue(errorstatus)
        cldbkc.set_residue1_key("Residue 1")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertTrue(errorstatus)
        cldbkc.set_residue2_key("Residue 2")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertFalse(errorstatus)
        cldbkc.set_idscore_key("ID Score")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertFalse(errorstatus)
        cldbkc.set_quantification_key("Quantification")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertFalse(errorstatus)
        c=cldbkc.get_converter()
        bc=cldbkc.get_backward_converter()
        self.assertEqual(c,expected_converter)
        self.assertEqual(bc,expected_backward_converter)

    def setup_cldb(self,input_data_set):
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("prot1")
        cldbkc.set_protein2_key("prot2")
        cldbkc.set_residue1_key("res1")
        cldbkc.set_residue2_key("res2")
        cldbkc.set_unique_id_key("id")
        cldbkc.set_idscore_key("score")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name(input_data_set))
        return cldb

    def test_setup_cldb(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")


        expected_dict={'1': [{'XLUniqueID': '1', 'IDScore': 10.0,
                            'Residue1': 1, 'Protein2': 'AAA', 'Protein1':
                            'AAA', 'Residue2': 10, 'sample': 'yeast'},
                            {'XLUniqueID': '1', 'IDScore': 9.0, 'Residue1':
                            15, 'Protein2': 'AAA', 'Protein1': 'AAA',
                            'Residue2': 30, 'sample': 'yeast'}, {'XLUniqueID':
                            '1', 'IDScore': 8.0, 'Residue1': 5, 'Protein2':
                            'AAA', 'Protein1': 'BBB', 'Residue2': 21,
                            'sample': 'yeast'}], '3': [{'XLUniqueID': '3',
                            'IDScore': 12.0, 'Residue1': 30, 'Protein2':
                            'BBB', 'Protein1': 'AAA', 'Residue2': 40,
                            'sample': 'bacteria'}], '2': [{'XLUniqueID':
                            '2', 'IDScore': 11.0, 'Residue1': 2, 'Protein2':
                            'BBB', 'Protein1': 'BBB', 'Residue2': 10, 'sample':
                            'bacteria'}], '5': [{'XLUniqueID': '5', 'IDScore':
                            10.0, 'Residue1': 10, 'Protein2': 'AAA', 'Protein1':
                            'AAA', 'Residue2': 30, 'sample': 'yeast'}], '4':
                            [{'XLUniqueID': '4', 'IDScore': 11.0, 'Residue1':
                            40, 'Protein2': 'AAA', 'Protein1': 'BBB', 'Residue2':
                            50, 'sample': 'human'}], '7': [{'XLUniqueID': '7',
                            'IDScore': 6.0, 'Residue1': 1, 'Protein2': 'CCC',
                            'Protein1': 'CCC', 'Residue2': 4, 'sample': 'yeast'}],
                            '6': [{'XLUniqueID': '6', 'IDScore': 7.0, 'Residue1':
                            30, 'Protein2': 'AAA', 'Protein1': 'CCC', 'Residue2':
                            50, 'sample': 'yeast'}]}

        # test content
        #self.assertEqual(cldb.data_base,expected_dict)

        # test iterator
        expected=[]
        actual=[]
        for k in cldb.data_base:
            for xl in cldb.data_base[k]:
                expected.append(xl)
        for xl in cldb:
            actual.append(xl)

        for n,xl in enumerate(expected):
            self.assertEqual(xl,actual[n])

        # test proteins_residues_array class

        for xl in cldb:

            pra=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
            self.assertEqual(pra[0],xl[cldb.protein1_key])
            self.assertEqual(pra[1],xl[cldb.protein2_key])
            self.assertEqual(pra[2],xl[cldb.residue1_key])
            self.assertEqual(pra[3],xl[cldb.residue2_key])
            prai=pra.get_inverted()
            self.assertEqual(prai[1],xl[cldb.protein1_key])
            self.assertEqual(prai[0],xl[cldb.protein2_key])
            self.assertEqual(prai[3],xl[cldb.residue1_key])
            self.assertEqual(prai[2],xl[cldb.residue2_key])

    def test_FilterOperator(self):
        import operator
        from IMP.pmi.io.crosslink import FilterOperator as FO
        cldb=self.setup_cldb("xl_dataset_test.dat")
        fo=FO(cldb.protein1_key,operator.eq,"AAA")

        for xl in cldb:
            self.assertEqual(fo.evaluate(xl),xl[cldb.protein1_key]=="AAA")

        fo=FO(cldb.protein1_key,operator.eq,"AAA")|FO(cldb.protein2_key,operator.eq,"BBB")

        for xl in cldb:
            self.assertEqual(fo.evaluate(xl),((xl[cldb.protein1_key]=="AAA")|(xl[cldb.protein2_key]=="BBB") ))

        fo=(FO(cldb.protein1_key,operator.eq,"AAA")|FO(cldb.protein2_key,operator.eq,"BBB"))&FO("sample",operator.eq,"human")

        for xl in cldb:
            self.assertEqual(fo.evaluate(xl),((xl[cldb.protein1_key]=="AAA")|(xl[cldb.protein2_key]=="BBB") )&(xl["sample"]=="human"))

        fo=(FO(cldb.residue1_key,operator.gt,30)|FO(cldb.protein2_key,operator.eq,"BBB"))

        for xl in cldb:
            self.assertEqual(fo.evaluate(xl),((xl[cldb.residue1_key]>30)|(xl[cldb.protein2_key]=="BBB") ))


    def test_filter_cldbkc(self):
        import operator
        from IMP.pmi.io.crosslink import FilterOperator as FO
        cldb=self.setup_cldb("xl_dataset_test.dat")
        fo=FO(cldb.protein1_key,operator.eq,"AAA")

        cldb1=cldb.filter(fo)
        nentry=len([xl for xl in cldb if (xl[cldb.protein1_key]=="AAA")])
        self.assertEqual(len(cldb1),nentry)

    def test_clone_protein(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")
        expected_crosslinks=[]
        for xl in cldb:
            array=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
            if array[0] == "AAA" and array[1] != "AAA":
                expected_crosslinks.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(("AAC",array[1],array[2],array[3])))
            elif array[0] != "AAA" and array[1] == "AAA":
                expected_crosslinks.append(IMP.pmi.io.crosslink._ProteinsResiduesArray((array[0],"AAC",array[2],array[3])))
            elif array[0] == "AAA" and array[1] == "AAA":
                expected_crosslinks.append(IMP.pmi.io.crosslink._ProteinsResiduesArray((array[0],"AAC",array[2],array[3])))
                expected_crosslinks.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(("AAC",array[1],array[2],array[3])))
                expected_crosslinks.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(("AAC","AAC",array[2],array[3])))
            expected_crosslinks.append(array)

        cldb.clone_protein("AAA","AAC")

        for xl in cldb:
            array=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
            expected_crosslinks.remove(array)

        self.assertEqual(len(expected_crosslinks),0)

    def test_redundancy(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")
        pass

    def test_merge_cldbkc(self):
        pass

    def test_append_cldbkc(self):
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test_2.dat")
        cldb1.append(cldb2)
        pass



if __name__ == '__main__':
    IMP.test.main()
