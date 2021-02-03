from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import os

import IMP.pmi.io.crosslink

class Tests(IMP.test.TestCase):

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
        cldbkc.set_id_score_key("ID Score")
        errorstatus=False
        try:
            cldbkc.check_keys()
        except KeyError:
            errorstatus=True
        self.assertFalse(errorstatus)
        cldbkc.set_quantitation_key("Quantification")
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
        cldbkc.set_id_score_key("score")
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
                            'Residue2': 30, 'sample': 'yeast'},
                            {'XLUniqueID':'1', 'IDScore': 8.0, 'Residue1': 5, 'Protein2':
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
        for k in sorted(cldb.data_base.keys()):
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

    def test_msstudio_style(self):
        rplp=IMP.pmi.io.crosslink.ResiduePairListParser("MSSTUDIO")
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter(rplp)
        cldbkc.set_protein1_key("prot1")
        cldbkc.set_protein2_key("prot2")
        cldbkc.set_site_pairs_key("site pairs")
        cldbkc.set_unique_id_key("id")
        cldbkc.set_id_score_key("score")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_crs.dat"))

        expected_list=[('1',1,"AAA","AAA",1,10),
                       ('1',2,"AAA","AAA",5,100),
                       ('2',1,"BBB","AAA",5,21),
                       ('2',2,"BBB","AAA",100,3),
                       ('2',3,"BBB","AAA",1,100),
                       ('3',1,"CCC","AAA",7,11)]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.protein2_key],e[3])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.residue2_key],e[5])
            nxl+=1

    def test_xtract_style(self):
        rplp=IMP.pmi.io.crosslink.ResiduePairListParser("XTRACT")
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter(rplp)
        cldbkc.set_site_pairs_key("uID")
        cldbkc.set_unique_id_key("dbindex")
        cldbkc.set_id_score_key("pvalue")
        cldbkc.set_quantitation_key("log2ratio")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_quant.dat"))

        expected_list=[('1',1,"AAA","AAA",1,10,0.001,1.0,"CROSSLINK"),
                       ('1',2,"AAA","AAA",5,100,0.001,1.0,"CROSSLINK"),
                       ('2',1,"BBB","AAA",5,21,0.001,1.0,"CROSSLINK"),
                       ('2',2,"BBB","AAA",100,3,0.001,1.0,"CROSSLINK"),
                       ('2',3,"BBB","AAA",1,100,0.001,1.0,"CROSSLINK"),
                       ('3',1,"CCC","AAA",7,11,0.001,1.0,"CROSSLINK")]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.protein2_key],e[3])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.residue2_key],e[5])
            self.assertEqual(xl[cldb.id_score_key],e[6])
            self.assertEqual(xl[cldb.quantitation_key],e[7])
            self.assertEqual(xl[cldb.link_type_key],e[8])
            nxl+=1

    def test_xtract_style_monolinks(self):
        rplp=IMP.pmi.io.crosslink.ResiduePairListParser("XTRACT")
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter(rplp)
        cldbkc.set_site_pairs_key("uID")
        cldbkc.set_unique_id_key("dbindex")
        cldbkc.set_id_score_key("pvalue")
        cldbkc.set_quantitation_key("log2ratio")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_quant_monolink.dat"))

        expected_list=[('1',1,"AAA","",1,None,0.001,1.0,"MONOLINK"),
                       ('1',2,"AAA","",5,None,0.001,1.0,"MONOLINK"),
                       ('2',1,"BBB","",5,None,0.001,1.0,"MONOLINK"),
                       ('2',2,"BBB","",100,None,0.001,1.0,"MONOLINK"),
                       ('2',3,"BBB","",1,None,0.001,1.0,"MONOLINK"),
                       ('3',1,"CCC","",7,None,0.001,1.0,"MONOLINK")]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.id_score_key],e[6])
            self.assertEqual(xl[cldb.quantitation_key],e[7])
            self.assertEqual(xl[cldb.link_type_key],e[8])
            nxl+=1



    def test_msstudio_style_no_id(self):
        rplp=IMP.pmi.io.crosslink.ResiduePairListParser("MSSTUDIO")
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter(rplp)
        cldbkc.set_protein1_key("prot1")
        cldbkc.set_protein2_key("prot2")
        cldbkc.set_site_pairs_key("site pairs")
        cldbkc.set_id_score_key("score")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_crs.dat"))

        expected_list=[('1',1,"AAA","AAA",1,10),
                       ('1',2,"AAA","AAA",5,100),
                       ('2',1,"BBB","AAA",5,21),
                       ('2',2,"BBB","AAA",100,3),
                       ('2',3,"BBB","AAA",1,100),
                       ('3',1,"CCC","AAA",7,11)]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.protein2_key],e[3])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.residue2_key],e[5])
            nxl+=1

    def test_xlpro_fixed_format_delimited(self):

        ffp=IMP.pmi.io.crosslink.FixedFormatParser("PROXL")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase()
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_proxl.dat"),FixedFormatParser=ffp)


        expected_list=[('1',1,"A","B",24,39),
                       ('2',1,"A","A",24,125),
                       ('3',1,"B","A",37,45),
                       ('4',1,"C","A",37,93)]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.protein2_key],e[3])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.residue2_key],e[5])
            nxl+=1

    def test_align_sequence(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")
        cldb.align_sequence(self.get_input_file_name('xl_dataset_test_alignment.dat'))


        expected_list=[('1',1,"AAA","AAA",1,6),
                       ('1',2,"AAA","AAA",12,28),
                       ('1',3,"BBB","AAA",7,19),
                       ('10',1,"AAA","AAA",1,6),
                       ('2',1,"BBB","BBB",6,22),
                       ('3',1,"AAA","BBB",28,50),
                       ('4',1,"BBB","AAA",50,48),
                       ('5',1,"AAA","AAA",6,28),
                       ('6',1,"CCC","AAA",34,48),
                       ('7',1,"CCC","CCC",2,5),
                       ('8',1,"BBB","AAA",50,48),
                       ('9',1,"AAA","BBB",48,50)]

        nxl=0
        for xl in cldb:
            e=expected_list[nxl]
            self.assertEqual(xl[cldb.unique_id_key],e[0])
            self.assertEqual(xl[cldb.unique_sub_index_key],e[1])
            self.assertEqual(xl[cldb.protein1_key],e[2])
            self.assertEqual(xl[cldb.protein2_key],e[3])
            self.assertEqual(xl[cldb.residue1_key],e[4])
            self.assertEqual(xl[cldb.residue2_key],e[5])
            nxl+=1

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

        fo=~(FO(cldb.residue1_key,operator.gt,30)|FO(cldb.protein2_key,operator.eq,"BBB"))

        for xl in cldb:
            self.assertEqual(fo.evaluate(xl),((xl[cldb.residue1_key]<=30)&(xl[cldb.protein2_key]!="BBB") ))


    def test_filter_cldbkc(self):
        import operator
        from IMP.pmi.io.crosslink import FilterOperator as FO
        cldb=self.setup_cldb("xl_dataset_test.dat")
        fo=FO(cldb.protein1_key,operator.eq,"AAA")

        cldb1=cldb.filter(fo)
        nentry=len([xl for xl in cldb if (xl[cldb.protein1_key]=="AAA")])
        self.assertEqual(len(cldb1),nentry)

    def test_filter_score(self):
        """Test CrossLinkDatabase.filter_score()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb1 = cldb.filter_score(10)
        self.assertEqual(len(cldb1), 5)

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


    def test_filter_out_same_residues(self):
        cldb=self.setup_cldb("xl_dataset_test_same_residues.dat")
        test_list=[]
        for xl in cldb:
            if xl[cldb.protein1_key] == xl[cldb.protein2_key] and xl[cldb.residue1_key] == xl[cldb.residue2_key]:
                continue
            else:
                test_list.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(xl))
        cldb.filter_out_same_residues()
        final_list=[]
        for xl in cldb:
            final_list.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(xl))
        self.assertEqual(test_list,final_list)

    def test_str(self):
        """Test CrossLinkDatabase.__str__"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        self.assertEqual(str(cldb)[:129], """1
--- XLUniqueID 1
--- XLUniqueSubIndex 1
--- XLUniqueSubID 1.1
--- Protein1 AAA
--- Protein2 AAA
--- Residue1 1
--- Residue2 10
""")

    def test_dump_load(self):
        """Test JSON dump/load"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.dump('test_dump_load.json')
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.load('test_dump_load.json')
        os.unlink('test_dump_load.json')

    def test_save_csv(self):
        """Test CrossLinkDatabase.save_csv()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.save_csv("test_save_csv.csv")
        os.unlink("test_save_csv.csv")

    def test_get_number_of_unique_crosslinked_sites(self):
        """Test CrossLinkDatabase.get_number_of_unique_crosslinked_sites()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        self.assertEqual(cldb.get_number_of_unique_crosslinked_sites(), 18)

    def test_jackknife(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")
        self.assertRaises(ValueError, cldb.jackknife, -1.0)
        # assess size
        cldb100=cldb.jackknife(1.0)
        self.assertEqual(cldb.get_number_of_xlid(),cldb100.get_number_of_xlid())
        cldb75=cldb.jackknife(0.75)
        self.assertEqual(int(cldb.get_number_of_xlid()*0.75),cldb75.get_number_of_xlid())
        cldb50=cldb.jackknife(0.5)
        self.assertEqual(int(cldb.get_number_of_xlid()*0.5),cldb50.get_number_of_xlid())
        cldb25=cldb.jackknife(0.25)
        self.assertEqual(int(cldb.get_number_of_xlid()*0.25),cldb25.get_number_of_xlid())
        # assess randomness
        cldb50_1=cldb.jackknife(0.5)
        cldb50_1_set=set([xlid for xlid in cldb50_1.xlid_iterator()])
        cldb50_2=cldb.jackknife(0.5)
        cldb50_2_set=set([xlid for xlid in cldb50_2.xlid_iterator()])
        self.assertNotEqual(cldb50_1_set,cldb50_2_set)
        # content of new databases must be identical for jackknifed crosslinks
        for xlid in cldb50_1.xlid_iterator():
            for n,xl in enumerate(cldb50_1[xlid]):
                for key in xl:
                    self.assertEqual(xl[key],cldb.data_base[xlid][n][key])
        for xlid in cldb50_2.xlid_iterator():
            for n,xl in enumerate(cldb50_2[xlid]):
                for key in xl:
                    self.assertEqual(xl[key],cldb.data_base[xlid][n][key])


    def test_redundancy(self):
        cldb=self.setup_cldb("xl_dataset_test.dat")
        pass

    def test_merge_cldbkc(self):
        """Test CrossLinkDatabase.merge()"""
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        self.assertRaises(NotImplementedError, cldb1.merge, None, None)

    def test_append_cldbkc(self):
        """Test CrossLinkDatabase.append_database()"""
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test_2.dat")
        cldb1.append_database(cldb2)
        self.assertEqual(len(cldb1.data_base), 10)

        # Test operator
        cldb3=self.setup_cldb("xl_dataset_test.dat")
        cldb4=self.setup_cldb("xl_dataset_test_2.dat")
        cldb3 += cldb4
        self.assertEqual(cldb1.data_base, cldb3.data_base)

    def test_set_value(self):
        import operator
        from IMP.pmi.io.crosslink import FilterOperator as FO
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test.dat")
        cldb1.set_value(cldb1.protein1_key,'FFF',FO(cldb1.protein1_key,operator.eq,"AAA"))
        cldb1.set_value(cldb1.protein2_key,'HHH',FO(cldb1.protein2_key,operator.eq,"AAA"))
        cldb1.set_value(cldb1.id_score_key, 0)

        protein_names_1=[]
        for xl in cldb1:
            protein_names_1.append((xl[cldb1.protein1_key],xl[cldb1.protein2_key]))

        protein_names_2=[]
        for xl in cldb2:
            protein_names_2.append((xl[cldb1.protein1_key],xl[cldb1.protein2_key]))

        new_protein_names=[]
        for n,p in enumerate(protein_names_2):

            p1=p[0]
            p2=p[1]
            if p1 == 'AAA':
                p1 = 'FFF'
            if p2 == 'AAA':
                p2 = 'HHH'
            new_protein_names.append((p1,p2))

        self.assertEqual(protein_names_1,new_protein_names)

    def test_get_values(self):
        """Test CrossLinkDatabase.get_values()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        self.assertEqual(cldb.get_values(cldb.protein1_key),
                ['AAA', 'BBB', 'CCC'])

    def test_offset_residue(self):
        import operator
        from IMP.pmi.io.crosslink import FilterOperator as FO
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test.dat")
        cldb1.offset_residue_index('AAA',100)

        records_1=[]
        for xl in cldb1:
            records_1.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(xl))

        records_2=[]
        for xl in cldb2:
            records_2.append(IMP.pmi.io.crosslink._ProteinsResiduesArray(xl))

        for n,r in enumerate(records_1):
            if r[0]=='AAA':
                self.assertEqual(r[2],records_2[n][2]+100)
            if r[1]=='AAA':
                self.assertEqual(r[3],records_2[n][3]+100)

    def test_create_new_keyword(self):
        """Test CrossLinkDatabase.create_new_keyword()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.create_new_keyword("myp1", cldb.protein1_key)
        cldb.create_new_keyword("newkey")
        self.assertEqual(cldb.get_values("myp1"), ['AAA', 'BBB', 'CCC'])
        self.assertEqual(cldb.get_values("newkey"), [None])

    def test_rename_proteins(self):
        """Test CrossLinkDatabase.rename_proteins()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.rename_proteins({'AAA':'XXX'}, "protein1")
        self.assertEqual(cldb.get_values(cldb.protein1_key),
                ['BBB', 'CCC', 'XXX'])
        self.assertEqual(cldb.get_values(cldb.protein2_key),
                ['AAA', 'BBB', 'CCC'])
        cldb.rename_proteins({'BBB':'XXX', 'CCC':'YYY'}, "protein2")
        self.assertEqual(cldb.get_values(cldb.protein1_key),
                ['BBB', 'CCC', 'XXX'])
        self.assertEqual(cldb.get_values(cldb.protein2_key),
                ['AAA', 'XXX', 'YYY'])
        cldb.rename_proteins({'XXX':'ZZZ'})
        self.assertEqual(cldb.get_values(cldb.protein1_key),
                ['BBB', 'CCC', 'ZZZ'])
        self.assertEqual(cldb.get_values(cldb.protein2_key),
                ['AAA', 'YYY', 'ZZZ'])

    def test_classify_crosslinks_by_score(self):
        """Test CrossLinkDatabase.classify_crosslinks_by_score()"""
        cldb = self.setup_cldb("xl_dataset_test.dat")
        cldb.classify_crosslinks_by_score(3)
        self.assertEqual(cldb.get_values(cldb.psi_key),
                ['CLASS_0', 'CLASS_1', 'CLASS_2'])

    def test_jaccard_distance(self):
        try:
            import scipy.spatial
        except ImportError:
            self.skipTest("no scipy spatial")

        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test.dat")
        cldb3=self.setup_cldb("xl_dataset_test.dat")
        cldb3.offset_residue_index('AAA',100)

        ddb={1:cldb1,2:cldb2,3:cldb3}
        jdm=IMP.pmi.io.crosslink.JaccardDistanceMatrix(ddb)
        self.assertAlmostEqual(jdm.get_distance(1,2), 0.0, delta=1e-3)
        self.assertAlmostEqual(jdm.get_distance(1,3), 0.875, delta=1e-3)

    def test_plot_matrix(self):
        """Test JaccardDistanceMatrix.plot_matrix()"""
        try:
            import matplotlib
            import scipy.spatial
        except ImportError:
            self.skipTest("no scipy spatial or matplotlib")
        cldb1=self.setup_cldb("xl_dataset_test.dat")
        cldb2=self.setup_cldb("xl_dataset_test.dat")
        cldb3=self.setup_cldb("xl_dataset_test.dat")
        cldb3.offset_residue_index('AAA',100)

        ddb={1:cldb1,2:cldb2,3:cldb3}
        jdm=IMP.pmi.io.crosslink.JaccardDistanceMatrix(ddb)
        jdm.plot_matrix("test_plot_matrix.pdf")
        os.unlink("matrix.test_plot_matrix.pdf")
        os.unlink("dendrogram.test_plot_matrix.pdf")

    def test_check_consistency(self):
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name("proteasome.fasta"))

        cldbkc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_unique_id_key("linkage ID")
        cldbkc.set_protein1_key("Linked protein 1")
        cldbkc.set_protein2_key("Linked protein 2")
        cldbkc.set_residue1_key("linked resid 1")
        cldbkc.set_residue2_key("linked resid 2")
        cldbkc.set_residue1_amino_acid_key("Cross-linked residue 1 in 4CR2")
        cldbkc.set_residue2_amino_acid_key("Cross-linked residue 2 in 4CR2")
        # instead of a score we're using the number of events here, which 'might' resemble a score
        cldbkc.set_id_score_key("Number of quantified event")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc, fasta_seq=seqs, linkable_aa=('K', 'S', 'T', 'Y'))
        cldb.set_name("master")
        cldb.create_set_from_file(self.get_input_file_name("proteasome_xlinks.csv"))
        matched,non_matched=cldb.check_cross_link_consistency()


        calc_matched=dict([(prot,len(matched[prot])) for prot in matched])
        calc_non_matched=dict([(prot,len(non_matched[prot])) for prot in non_matched])
        # expected results; summed up crosslinks
        exp_matched={'RPN3': 11, 'RPN5': 14, 'RPN6': 12, 'RPN7': 16, 'RPN8': 15, 'RPN9': 11, 'RPN11': 11, 'SEM1': 6}
        exp_non_matched={'RPN7': 1}

        for prot_name, amount in calc_matched.items():
            self.assertEqual(amount, exp_matched[prot_name])
        for prot_name, amount in calc_non_matched.items():
            self.assertEqual(amount, exp_non_matched[prot_name])

    def test_map_crosslink_database(self):
        model=IMP.Model()

        cldbkc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("P1")
        cldbkc.set_protein2_key("P2")
        cldbkc.set_residue1_key("R1")
        cldbkc.set_residue2_key("R2")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_io_crosslink_map.txt"))

        rmf_name=self.get_input_file_name("pmi2_sample_0/rmfs/0.rmf3")

        rmfh=IMP.pmi.output.RMFHierarchyHandler(model,rmf_name)

        mcldb=IMP.pmi.io.crosslink.MapCrossLinkDataBaseOnStructure(cldb,rmfh)

        for i in rmfh:
            mcldb.compute_distances()
            for xl in cldb:
                (prot1,prot2,res1,res2)=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
                p1=IMP.atom.Selection(rmfh,molecule=prot1,residue_index=res1,resolution=1).get_selected_particles()[0]
                p2=IMP.atom.Selection(rmfh,molecule=prot2,residue_index=res2,resolution=1).get_selected_particles()[0]
                v1=IMP.core.XYZ(p1).get_coordinates()
                v2=IMP.core.XYZ(p2).get_coordinates()
                dist=IMP.algebra.get_distance(v1,v2)
                d1 = xl["MinAmbiguousDistance"]
                self.assertAlmostEqual(dist,d1,1)

    def test_ProteinsResiduesArray(self):
        cldbkc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("P1")
        cldbkc.set_protein2_key("P2")
        cldbkc.set_residue1_key("R1")
        cldbkc.set_residue2_key("R2")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(self.get_input_file_name("xl_dataset_test_io_crosslink_map.txt"))
        for xl in cldb:
            pra=IMP.pmi.io.crosslink._ProteinsResiduesArray(xl)
            self.assertEqual(pra[0],xl[cldb.protein1_key])
            self.assertEqual(pra[1],xl[cldb.protein2_key])
            self.assertEqual(pra[2],xl[cldb.residue1_key])
            self.assertEqual(pra[3],xl[cldb.residue2_key])

if __name__ == '__main__':
    IMP.test.main()
