"""@namespace IMP.pmi.io.crosslink
   Handles cross-link data sets.

   Utilities are also provided to help in the analysis of models that
   contain cross-links.
"""
from __future__ import print_function
import IMP
import IMP.pmi
import IMP.pmi.output
import IMP.atom
import IMP.core
import IMP.algebra
import IMP.rmf
import RMF
import IMP.display
import operator
import math
import sys
import ihm.location
import ihm.dataset
from collections import defaultdict
import numpy

# json default serializations
def set_json_default(obj):
    if isinstance(obj, set):
        return list(obj)
    if isinstance(obj, IMP.pmi.topology.Molecule):
        return str(obj)
    raise TypeError

# Handle and return data that must be a string
if sys.version_info[0] >= 3:
    def _handle_string_input(inp):
        if not isinstance(inp, str):
            raise TypeError("expecting a string")
        return inp
else:
    def _handle_string_input(inp):
        if not isinstance(inp, (str, unicode)):
            raise TypeError("expecting a string or unicode")
        # Coerce to non-unicode representation (str)
        if isinstance(inp, unicode):
            return str(inp)
        else:
            return inp

class _CrossLinkDataBaseStandardKeys(object):
    '''
    This class setup all the standard keys needed to
    identify the crosslink features from the data sets
    '''
    def __init__(self):
        self.type={}
        self.protein1_key="Protein1"
        self.type[self.protein1_key]=str
        self.protein2_key="Protein2"
        self.type[self.protein2_key]=str
        self.residue1_key="Residue1"
        self.type[self.residue1_key]=int
        self.residue2_key="Residue2"
        self.type[self.residue2_key]=int
        self.residue1_amino_acid_key="Residue1AminoAcid"
        self.type[self.residue1_amino_acid_key]=str
        self.residue2_amino_acid_key="Residue2AminoAcid"
        self.type[self.residue2_amino_acid_key]=str
        self.residue1_moiety_key="Residue1Moiety"
        self.type[self.residue1_moiety_key]=str
        self.residue2_moiety_key="Residue2Moiety"
        self.type[self.residue2_moiety_key]=str
        self.site_pairs_key="SitePairs"
        self.type[self.site_pairs_key]=str
        self.unique_id_key="XLUniqueID"
        self.type[self.unique_id_key]=str
        self.unique_sub_index_key="XLUniqueSubIndex"
        self.type[self.unique_sub_index_key]=int
        self.unique_sub_id_key="XLUniqueSubID"
        self.type[self.unique_sub_id_key]=str
        self.data_set_name_key="DataSetName"
        self.type[self.data_set_name_key]=str
        self.cross_linker_chemical_key="CrossLinkerChemical"
        self.type[self.cross_linker_chemical_key]=str
        self.id_score_key="IDScore"
        self.type[self.id_score_key]=float
        self.fdr_key="FDR"
        self.type[self.fdr_key]=float
        self.quantitation_key="Quantitation"
        self.type[self.quantitation_key]=float
        self.redundancy_key="Redundancy"
        self.type[self.redundancy_key]=int
        self.redundancy_list_key="RedundancyList"
        self.type[self.redundancy_key]=list
        self.ambiguity_key="Ambiguity"
        self.type[self.ambiguity_key]=int
        self.residue1_links_number_key="Residue1LinksNumber"
        self.type[self.residue1_links_number_key]=int
        self.residue2_links_number_key="Residue2LinksNumber"
        self.type[self.residue2_links_number_key]=int
        self.type[self.ambiguity_key]=int
        self.state_key="State"
        self.type[self.state_key]=int
        self.sigma1_key="Sigma1"
        self.type[self.sigma1_key]=str
        self.sigma2_key="Sigma2"
        self.type[self.sigma2_key]=str
        self.psi_key="Psi"
        self.type[self.psi_key]=str
        self.distance_key="Distance"
        self.type[self.distance_key]=float
        self.min_ambiguous_distance_key="MinAmbiguousDistance"
        self.type[self.distance_key]=float
        #link types are either Monolink, Intralink or Interlink
        self.link_type_key="LinkType"
        self.type[self.link_type_key]=str

        self.ordered_key_list =[self.data_set_name_key,
                        self.unique_id_key,
                        self.unique_sub_index_key,
                        self.unique_sub_id_key,
                        self.protein1_key,
                        self.protein2_key,
                        self.residue1_key,
                        self.residue2_key,
                        self.residue1_amino_acid_key,
                        self.residue2_amino_acid_key,
                        self.residue1_moiety_key,
                        self.residue2_moiety_key,
                        self.cross_linker_chemical_key,
                        self.id_score_key,
                        self.fdr_key,
                        self.quantitation_key,
                        self.redundancy_key,
                        self.redundancy_list_key,
                        self.state_key,
                        self.sigma1_key,
                        self.sigma2_key,
                        self.psi_key,
                        self.distance_key,
                        self.min_ambiguous_distance_key,
                        self.link_type_key]


class _ProteinsResiduesArray(tuple):
    '''
    This class is inherits from tuple, and it is a shorthand for a cross-link
    (p1,p2,r1,r2) or a monolink (p1,r1) where p1 and p2 are protein1 and protein2, r1 and r2 are
    residue1 and residue2.
    '''

    def __new__(self,input_data):
        '''
        @input_data can be a dict or a tuple
        '''
        self.cldbsk=_CrossLinkDataBaseStandardKeys()
        if type(input_data) is dict:
            monolink=False
            p1=input_data[self.cldbsk.protein1_key]
            try:
                p2=input_data[self.cldbsk.protein2_key]
            except KeyError:
                monolink=True
            r1=input_data[self.cldbsk.residue1_key]
            try:
                r2=input_data[self.cldbsk.residue2_key]
            except KeyError:
                monolink=True
            if not monolink:
                t=(p1,p2,r1,r2)
            else:
                t=(p1,"",r1,None)
        elif type(input_data) is tuple:
            if len(input_data)>4 or len(input_data)==3 or len(input_data)==1:
                raise TypeError("_ProteinsResiduesArray: must have only 4 elements")
            if len(input_data)==4:
                p1 = _handle_string_input(input_data[0])
                p2 = _handle_string_input(input_data[1])
                r1=input_data[2]
                r2=input_data[3]
                if (not (type(r1) is int)) and (not (r1 is None)):
                    raise TypeError("_ProteinsResiduesArray: residue1 must be a integer")
                if (not (type(r2) is int)) and (not (r1 is None)):
                    raise TypeError("_ProteinsResiduesArray: residue2 must be a integer")
                t=(p1,p2,r1,r2)
            if len(input_data) == 2:
                p1 = _handle_string_input(input_data[0])
                r1 = input_data[1]
                if type(r1) is not int:
                    raise TypeError("_ProteinsResiduesArray: residue1 must be a integer")
                t = (p1,"",r1,None)
        else:
            raise TypeError("_ProteinsResiduesArray: input must be a dict or tuple")
        return tuple.__new__(_ProteinsResiduesArray, t)

    def get_inverted(self):
        '''
        Returns a _ProteinsResiduesArray instance with protein1 and protein2 inverted
        '''
        return _ProteinsResiduesArray((self[1],self[0],self[3],self[2]))

    def __repr__(self):
        outstr=self.cldbsk.protein1_key+" "+str(self[0])
        outstr+=" "+self.cldbsk.protein2_key+" "+str(self[1])
        outstr+=" "+self.cldbsk.residue1_key+" "+str(self[2])
        outstr+=" "+self.cldbsk.residue2_key+" "+str(self[3])
        return outstr

    def __str__(self):
        outstr=str(self[0])+"."+str(self[2])+"-"+str(self[1])+"."+str(self[3])
        return outstr

class FilterOperator(object):
    '''
    This class allows to create filter functions that can be passed to the CrossLinkDataBase
    in this way:

    fo=FilterOperator(cldb.protein1_key,operator.eq,"AAA")|FilterOperator(cldb.protein2_key,operator.eq,"BBB")

    where cldb is CrossLinkDataBase instance and it is only needed to get the standard keywords

    A filter operator can be evaluate on a CrossLinkDataBase item xl and returns a boolean

    fo.evaluate(xl)

    and it is used to filter the database
    '''

    def __init__(self, argument1, operator, argument2):
        '''
        (argument1,operator,argument2) can be either a (keyword,operator.eq|lt|gt...,value)
        or  (FilterOperator1,operator.or|and...,FilterOperator2)
        '''
        if isinstance(argument1, FilterOperator):
            self.operations = [argument1, operator, argument2]
        else:
            self.operations = []
            self.values = (argument1, operator, argument2)

    def __or__(self, FilterOperator2):
        return FilterOperator(self, operator.or_, FilterOperator2)

    def __and__(self, FilterOperator2):
        return FilterOperator(self, operator.and_, FilterOperator2)

    def __invert__(self):
        return FilterOperator(self, operator.not_, None)

    def evaluate(self, xl_item):

        if len(self.operations) == 0:
            keyword, operator, value = self.values
            return operator(xl_item[keyword], value)
        FilterOperator1, op, FilterOperator2 = self.operations

        if FilterOperator2 is None:
            return op(FilterOperator1.evaluate(xl_item))
        else:
            return op(FilterOperator1.evaluate(xl_item), FilterOperator2.evaluate(xl_item))

'''
def filter_factory(xl_):

    class FilterOperator(object):
        import operator
        xl = xl_

        def __new__(self,key,value,oper=operator.eq):
            return oper(self.xl[key],value)

    return FilterOperator
'''

class CrossLinkDataBaseKeywordsConverter(_CrossLinkDataBaseStandardKeys):
    '''
    This class is needed to convert the keywords from a generic database
    to the standard ones
    '''

    def __init__(self, list_parser=None):
        '''
        @param list_parser an instance of ResiduePairListParser, if any is needed
        '''
        self.converter={}
        self.backward_converter={}
        _CrossLinkDataBaseStandardKeys.__init__(self)
        self.rplp = list_parser
        if self.rplp is None:
            # either you have protein1, protein2, residue1, residue2
            self.compulsory_keys=set([self.protein1_key,
                                  self.protein2_key,
                                  self.residue1_key,
                                  self.residue2_key])
        else:
            self.compulsory_keys=self.rplp.get_compulsory_keys()
        self.setup_keys=set()

    def check_keys(self):
        '''
        Is a function that check whether necessary keys are setup
        '''
        setup_keys=set(self.get_setup_keys())
        if  self.compulsory_keys & setup_keys != self.compulsory_keys:
            raise KeyError("CrossLinkDataBaseKeywordsConverter: must setup all necessary keys")

    def get_setup_keys(self):
        '''
        Returns the keys that have been setup so far
        '''
        return self.backward_converter.keys()

    def set_standard_keys(self):
        """
        This sets up the standard compulsory keys as defined by
        _CrossLinkDataBaseStandardKeys
        """
        for ck in self.compulsory_keys:
            self.converter[ck]=ck
            self.backward_converter[ck]=ck

    def set_unique_id_key(self,origin_key):
        self.converter[origin_key]=self.unique_id_key
        self.backward_converter[self.unique_id_key]=origin_key

    def set_protein1_key(self,origin_key):
        self.converter[origin_key]=self.protein1_key
        self.backward_converter[self.protein1_key]=origin_key

    def set_protein2_key(self,origin_key):
        self.converter[origin_key]=self.protein2_key
        self.backward_converter[self.protein2_key]=origin_key

    def set_residue1_key(self,origin_key):
        self.converter[origin_key]=self.residue1_key
        self.backward_converter[self.residue1_key]=origin_key

    def set_residue2_key(self,origin_key):
        self.converter[origin_key]=self.residue2_key
        self.backward_converter[self.residue2_key]=origin_key

    def set_residue1_amino_acid_key(self, origin_key):
        self.converter[origin_key] = self.residue1_amino_acid_key
        self.backward_converter[self.residue1_amino_acid_key] = origin_key

    def set_residue2_amino_acid_key(self, origin_key):
        self.converter[origin_key] = self.residue2_amino_acid_key
        self.backward_converter[self.residue2_amino_acid_key] = origin_key

    def set_residue1_moiety_key(self, origin_key):
        self.converter[origin_key] = self.residue1_moiety_key
        self.backward_converter[self.residue1_moiety_key] = origin_key

    def set_residue2_moiety_key(self, origin_key):
        self.converter[origin_key] = self.residue2_moiety_key
        self.backward_converter[self.residue2_moiety_key] = origin_key

    def set_site_pairs_key(self,origin_key):
        self.converter[origin_key]=self.site_pairs_key
        self.backward_converter[self.site_pairs_key]=origin_key

    def set_id_score_key(self,origin_key):
        self.converter[origin_key]=self.id_score_key
        self.backward_converter[self.id_score_key]=origin_key

    def set_fdr_key(self,origin_key):
        self.converter[origin_key]=self.fdr_key
        self.backward_converter[self.fdr_key]=origin_key

    def set_quantitation_key(self,origin_key):
        self.converter[origin_key]=self.quantitation_key
        self.backward_converter[self.quantitation_key]=origin_key

    def set_psi_key(self,origin_key):
        self.converter[origin_key]=self.psi_key
        self.backward_converter[self.psi_key]=origin_key

    def set_link_type_key(self,link_type_key):
        self.converter[link_type_key]=self.link_type_key
        self.backward_converter[self.link_type_key]=link_type_key

    def get_converter(self):
        '''
        Returns the dictionary that convert the old keywords to the new ones
        '''
        self.check_keys()
        return self.converter

    def get_backward_converter(self):
        '''
        Returns the dictionary that convert the new keywords to the old ones
        '''
        self.check_keys()
        return self.backward_converter

class ResiduePairListParser(_CrossLinkDataBaseStandardKeys):
    '''
    A class to handle different styles of site pairs parsers.
    Implemented styles:
    MSSTUDIO: [Y3-S756;Y3-K759;K4-S756;K4-K759] for crosslinks
                  [Y3-;K4-] for dead-ends
    QUANTITATION: sp|P33298|PRS6B_YEAST:280:x:sp|P33298|PRS6B_YEAST:337
    QUANTITATION (with ambiguity separator :|:): Fbw7:107:|:StrepII2x-Fbw7fl:408:x:Nedd8:48
    LAN_HUANG: PROT1:C88-PROT2:C448 ambiguous separators | or ;
    '''

    import re

    def __init__(self,style):

        _CrossLinkDataBaseStandardKeys.__init__(self)
        if style is "MSSTUDIO":
            self.style=style
            self.compulsory_keys= set([self.protein1_key,
                                  self.protein2_key,
                                  self.site_pairs_key])
        elif style is "XTRACT" or style is "QUANTITATION":
            self.style=style
            self.compulsory_keys= set([self.site_pairs_key])
        elif style is "LAN_HUANG":
            self.style=style
            self.compulsory_keys= set([self.site_pairs_key])
        else:
            raise Error("ResiduePairListParser: unknown list parser style")

    def get_compulsory_keys(self):
        return self.compulsory_keys

    def get_list(self,input_string):
        '''
        This function returns a list of cross-linked residues and the corresponding list of
        cross-linked chains. The latter list can be empty, if the style doesn't have the
        corresponding information.
        '''
        if self.style == "MSSTUDIO":
            input_string=input_string.replace("[","")
            input_string=input_string.replace("]","")
            input_string_pairs=input_string.split(";")
            residue_pair_indexes=[]
            chain_pair_indexes=[]
            for s in input_string_pairs:
                m1=self.re.search(r'^(A|C|D|E|F|G|H|I|K|L|M|N|O|P|Q|R|S|T|Y|X|W)(\d+)-(A|C|D|E|F|G|H|I|K|L|M|N|O|P|Q|R|S|T|Y|X|W)(\d+)$',s)
                m2=self.re.search(r'^(A|C|D|E|F|G|H|I|K|L|M|N|O|P|Q|R|S|T|Y|X|W)(\d+)-$',s)
                if m1:
                    # cross link
                    residue_type_1,residue_index_1,residue_type_2,residue_index_2=m1.group(1,2,3,4)
                    residue_pair_indexes.append((residue_index_1,residue_index_2))
                elif m2:
                    # dead end
                    residue_type_1,residue_index_1=m2.group(1,2)
            # at this stage chain_pair_indexes is empty
            return  residue_pair_indexes,chain_pair_indexes
        if self.style is "XTRACT" or self.style is "QUANTITATION":
            if ":x:" in input_string:
                # if it is a crosslink....
                input_strings=input_string.split(":x:")
                first_peptides=input_strings[0].split(":|:")
                second_peptides=input_strings[1].split(":|:")
                first_peptides_indentifiers=[(f.split(":")[0],f.split(":")[1]) for f in first_peptides]
                second_peptides_indentifiers=[(f.split(":")[0],f.split(":")[1]) for f in second_peptides]
                residue_pair_indexes=[]
                chain_pair_indexes=[]
                for fpi in first_peptides_indentifiers:
                    for spi in second_peptides_indentifiers:
                        chain1=fpi[0]
                        chain2=spi[0]
                        residue1=fpi[1]
                        residue2=spi[1]
                        residue_pair_indexes.append((residue1,residue2))
                        chain_pair_indexes.append((chain1,chain2))
                return residue_pair_indexes, chain_pair_indexes
            else:
                # if it is a monolink....
                first_peptides = input_string.split(":|:")
                first_peptides_indentifiers = [(f.split(":")[0], f.split(":")[1]) for f in first_peptides]
                residue_indexes = []
                chain_indexes = []
                for fpi in first_peptides_indentifiers:
                    chain1=fpi[0]
                    residue1=fpi[1]
                    residue_indexes.append((residue1,))
                    chain_indexes.append((chain1,))
                return residue_indexes, chain_indexes
        if self.style is "LAN_HUANG":
            input_strings=input_string.split("-")
            chain1,first_series=input_strings[0].split(":")
            chain2,second_series=input_strings[1].split(":")

            first_residues=first_series.replace(";","|").split("|")
            second_residues=second_series.replace(";","|").split("|")
            residue_pair_indexes=[]
            chain_pair_indexes=[]
            for fpi in first_residues:
                for spi in second_residues:
                    residue1=self.re.sub("[^0-9]", "", fpi)
                    residue2=self.re.sub("[^0-9]", "", spi)
                    residue_pair_indexes.append((residue1,residue2))
                    chain_pair_indexes.append((chain1,chain2))
            return residue_pair_indexes, chain_pair_indexes



class FixedFormatParser(_CrossLinkDataBaseStandardKeys):
    '''
    A class to handle different XL format with fixed format
    currently support ProXL
    '''
    def __init__(self,format):

        _CrossLinkDataBaseStandardKeys.__init__(self)
        if format is "PROXL":
            self.format=format
        else:
            raise Error("FixedFormatParser: unknown list format name")


    def get_data(self,input_string):
        if self.format is "PROXL":
            tockens=input_string.split("\t")
            xl={}
            if tockens[0]=="SEARCH ID(S)":
                return None
            else:
                xl[self.protein1_key]=tockens[2]
                xl[self.protein2_key]=tockens[4]
                xl[self.residue1_key]=int(tockens[3])
                xl[self.residue2_key]=int(tockens[5])
                return xl

class CrossLinkDataBase(_CrossLinkDataBaseStandardKeys):
    '''
    this class handles a cross-link dataset and do filtering
    operations, adding cross-links, merge datasets...
    '''

    def __init__(self, converter=None, data_base=None, fasta_seq=None, linkable_aa=('K',)):
        '''
        Constructor.
        @param converter an instance of CrossLinkDataBaseKeywordsConverter
        @param data_base an instance of CrossLinkDataBase to build the new database on
        @param fasta_seq an instance of IMP.pmi.topology.Sequences containing protein fasta sequences to check
                crosslink consistency. If not given consistency will not be checked
        @param linkable_aa a tuple containing one-letter amino acids which are linkable by the crosslinker;
                only used if the database DOES NOT provide a value for a certain residueX_amino_acid_key
                and if a fasta_seq is given
        '''

        if data_base is None:
            self.data_base = {}
        else:
            self.data_base=data_base

        _CrossLinkDataBaseStandardKeys.__init__(self)
        if converter is not None:
            self.cldbkc = converter                     #type: CrossLinkDataBaseKeywordsConverter
            self.list_parser=self.cldbkc.rplp
            self.converter = converter.get_converter()

        else:
            self.cldbkc =    None               #type: CrossLinkDataBaseKeywordsConverter
            self.list_parser=None
            self.converter = None

        # default amino acids considered to be 'linkable' if none are given
        self.def_aa_tuple = linkable_aa
        self.fasta_seq = fasta_seq      #type: IMP.pmi.topology.Sequences
        self.dataset = None
        self._update()

    def _update(self):
        '''
        Update the whole dataset after changes
        '''
        self.update_cross_link_unique_sub_index()
        self.update_cross_link_redundancy()
        self.update_residues_links_number()
        self.check_cross_link_consistency()


    def __iter__(self):
        sorted_ids=sorted(self.data_base.keys())
        for k in sorted_ids:
            for xl in self.data_base[k]:
                yield xl

    def xlid_iterator(self):
        sorted_ids=sorted(self.data_base.keys())
        for xlid in sorted_ids:
            yield xlid

    def __getitem__(self,xlid):
        return self.data_base[xlid]

    def __len__(self):
        return len([xl for xl in self])

    def get_name(self):
        return self.name

    def set_name(self,name):
        new_data_base={}
        for k in self.data_base:
            new_data_base[k+"."+name]=self.data_base[k]
        self.data_base=new_data_base
        self.name=name
        self._update()

    def get_number_of_xlid(self):
        return len(self.data_base)


    def create_set_from_file(self,file_name,converter=None,FixedFormatParser=None):
        '''
        if FixedFormatParser is not specified, the file is comma-separated-values
        @param file_name a txt file to be parsed
        @param converter an instance of CrossLinkDataBaseKeywordsConverter
        @param FixedFormatParser a parser for a fixed format
        '''
        if not FixedFormatParser:
            xl_list=IMP.pmi.tools.get_db_from_csv(file_name)

            if converter is not None:
                self.cldbkc = converter
                self.list_parser=self.cldbkc.rplp
                self.converter = converter.get_converter()


            if not self.list_parser:
                # normal procedure without a list_parser
                # each line is a cross-link
                new_xl_dict={}
                for nxl,xl in enumerate(xl_list):
                    new_xl={}
                    for k in xl:
                        if k in self.converter:
                            new_xl[self.converter[k]]=self.type[self.converter[k]](xl[k])
                        else:
                            new_xl[k]=xl[k]
                    if self.unique_id_key in self.cldbkc.get_setup_keys():
                        if new_xl[self.unique_id_key] not in new_xl_dict:
                            new_xl_dict[new_xl[self.unique_id_key]]=[new_xl]
                        else:
                            new_xl_dict[new_xl[self.unique_id_key]].append(new_xl)
                    else:
                        if str(nxl) not in new_xl_dict:
                            new_xl_dict[str(nxl)]=[new_xl]
                        else:
                            new_xl_dict[str(nxl)].append(new_xl)

            else:
                # with a list_parser, a line can be a list of ambiguous crosslinks
                new_xl_dict={}
                for nxl,entry in enumerate(xl_list):

                    # first get the translated keywords
                    new_dict={}
                    if self.site_pairs_key not in self.cldbkc.get_setup_keys():
                        raise Error("CrossLinkDataBase: expecting a site_pairs_key for the site pair list parser")
                    for k in entry:
                        if k in self.converter:
                            new_dict[self.converter[k]]=self.type[self.converter[k]](entry[k])
                        else:
                            new_dict[k]=entry[k]

                    residue_pair_list,chain_pair_list=self.list_parser.get_list(new_dict[self.site_pairs_key])

                    # then create the crosslinks
                    for n,p in enumerate(residue_pair_list):
                        is_monolink=False
                        if len(p)==1:
                            is_monolink=True

                        new_xl={}
                        for k in new_dict:
                            new_xl[k]=new_dict[k]
                        new_xl[self.residue1_key]=self.type[self.residue1_key](p[0])
                        if not is_monolink:
                            new_xl[self.residue2_key]=self.type[self.residue2_key](p[1])

                        if len(chain_pair_list)==len(residue_pair_list):
                            new_xl[self.protein1_key]=self.type[self.protein1_key](chain_pair_list[n][0])
                            if not is_monolink:
                                new_xl[self.protein2_key]=self.type[self.protein2_key](chain_pair_list[n][1])

                        if not is_monolink:
                            new_xl[self.link_type_key]="CROSSLINK"
                        else:
                            new_xl[self.link_type_key]="MONOLINK"

                        if self.unique_id_key in self.cldbkc.get_setup_keys():
                            if new_xl[self.unique_id_key] not in new_xl_dict:
                                new_xl_dict[new_xl[self.unique_id_key]]=[new_xl]
                            else:
                                new_xl_dict[new_xl[self.unique_id_key]].append(new_xl)
                        else:
                            if str(nxl) not in new_xl_dict:
                                new_xl[self.unique_id_key]=str(nxl+1)
                                new_xl_dict[str(nxl)]=[new_xl]
                            else:
                                new_xl[self.unique_id_key]=str(nxl+1)
                                new_xl_dict[str(nxl)].append(new_xl)


        else:
            '''
            if FixedFormatParser  is defined
            '''

            new_xl_dict={}
            f=open(file_name,"r")
            nxl=0
            for line in f:
                xl=FixedFormatParser.get_data(line)
                if xl:
                    xl[self.unique_id_key]=str(nxl+1)
                    new_xl_dict[str(nxl)]=[xl]
                    nxl+=1


        self.data_base=new_xl_dict
        self.name=file_name
        l = ihm.location.InputFileLocation(file_name, details='Crosslinks')
        self.dataset = ihm.dataset.CXMSDataset(l)
        self._update()

    def update_cross_link_unique_sub_index(self):
        for k in self.data_base:
            for n,xl in enumerate(self.data_base[k]):
                xl[self.ambiguity_key]=len(self.data_base[k])
                xl[self.unique_sub_index_key]=n+1
                xl[self.unique_sub_id_key]=k+"."+str(n+1)

    def update_cross_link_redundancy(self):
        redundancy_data_base={}
        for xl in self:
            pra=_ProteinsResiduesArray(xl)
            if pra not in redundancy_data_base:
                redundancy_data_base[pra]=[xl[self.unique_sub_id_key]]
                redundancy_data_base[pra.get_inverted()]=[xl[self.unique_sub_id_key]]
            else:
                redundancy_data_base[pra].append(xl[self.unique_sub_id_key])
                redundancy_data_base[pra.get_inverted()].append(xl[self.unique_sub_id_key])
        for xl in self:
            pra=_ProteinsResiduesArray(xl)
            xl[self.redundancy_key]=len(redundancy_data_base[pra])
            xl[self.redundancy_list_key]=redundancy_data_base[pra]

    def update_residues_links_number(self):
        residue_links={}
        for xl in self:
            (p1,p2,r1,r2)=_ProteinsResiduesArray(xl)
            if (p1,r1) not in residue_links:
                residue_links[(p1,r1)]=set([(p2,r2)])
            else:
                residue_links[(p1,r1)].add((p2,r2))
            if (p2,r2) not in residue_links:
                residue_links[(p2,r2)]=set([(p1,r1)])
            else:
                residue_links[(p2,r2)].add((p1,r1))

        for xl in self:
            (p1,p2,r1,r2)=_ProteinsResiduesArray(xl)
            xl[self.residue1_links_number_key]=len(residue_links[(p1,r1)])
            xl[self.residue2_links_number_key]=len(residue_links[(p2,r2)])

    def check_cross_link_consistency(self):
        """This function checks the consistency of the dataset with the amino acid sequence"""
        if self.cldbkc and self.fasta_seq:
            cnt_matched, cnt_matched_file = 0, 0
            matched = {}
            non_matched = {}
            for xl in self:
                (p1, p2, r1, r2) = _ProteinsResiduesArray(xl)
                b_matched_file = False
                if self.residue1_amino_acid_key in xl:
                    # either you know the residue type and aa_tuple is a single entry
                    aa_from_file = (xl[self.residue1_amino_acid_key].upper(),)
                    b_matched = self._match_xlinks(p1, r1, aa_from_file)
                    b_matched_file = b_matched
                else:
                    # or pass the possible list of types that can be crosslinked
                    b_matched = self._match_xlinks(p1, r1, self.def_aa_tuple)

                matched, non_matched = self._update_matched_xlinks(b_matched, p1, r1, matched, non_matched)

                if self.residue2_amino_acid_key in xl:
                    aa_from_file = (xl[self.residue2_amino_acid_key].upper(), )
                    b_matched = self._match_xlinks(p2, r2, aa_from_file)
                    b_matched_file = b_matched
                else:
                    b_matched = self._match_xlinks(p2, r2, self.def_aa_tuple)

                matched, non_matched = self._update_matched_xlinks(b_matched, p2, r2, matched, non_matched)
                if b_matched: cnt_matched += 1
                if b_matched_file: cnt_matched_file += 1
            if len(self) > 0:
                percentage_matched = round(100*cnt_matched/len(self),1)
                percentage_matched_file = round(100 * cnt_matched_file / len(self), 1)
                #if matched: print "Matched xlinks:", matched
                if matched or non_matched: print("check_cross_link_consistency: Out of %d crosslinks "
                                                 "%d were matched to the fasta sequence (%f %%).\n "
                                                 "%d were matched by using the crosslink file (%f %%)."%
                                                 (len(self),cnt_matched,percentage_matched,cnt_matched_file,
                                                  percentage_matched_file) )
                if non_matched: print("check_cross_link_consistency: Warning: Non matched xlinks:",
                                      [(prot_name, sorted(list(non_matched[prot_name]))) for prot_name in non_matched])
            return matched,non_matched

    def _match_xlinks(self, prot_name, res_index, aa_tuple):
        # returns Boolean whether given aa matches a position in the fasta file
        # cross link files usually start counting at 1 and not 0; therefore subtract -1 to compare with fasta
        amino_dict = IMP.pmi.tools.ThreeToOneConverter()
        res_index -= 1
        for amino_acid in aa_tuple:
            if len(amino_acid) == 3:
                amino_acid = amino_dict[amino_acid.upper()]
            if prot_name in self.fasta_seq.sequences:
                seq = self.fasta_seq.sequences[prot_name]
                # if we are looking at the first amino acid in a given sequence always return a match
                # the first aa should always be the n-terminal aa
                # which may form a crosslink in any case (for BS3-like crosslinkers)
                # for some data sets the first aa is at position 1; todo: check why this is the case
                if res_index == 0 or res_index == 1:
                    return True
                if res_index < len(seq):
                    if amino_acid == seq[res_index]:
                        return True
                    # else:
                    #     print "Could not match", prot, res+1, amino_acid, seq[res]
        return False

    def _update_matched_xlinks(self, b_matched, prot, res, matched, non_matched):
        if b_matched:
            if prot in matched:
                matched[prot].add(res)
            else:
                matched[prot] = set([res])
        else:
            if prot in non_matched:
                non_matched[prot].add(res)
            else:
                non_matched[prot] = set([res])
        return matched, non_matched


    def get_cross_link_string(self,xl):
        string='|'
        for k in self.ordered_key_list:
            try:
                string+=str(k)+":"+str(xl[k])+"|"
            except KeyError:
                continue

        for k in xl:
            if k not in self.ordered_key_list:
                string+=str(k)+":"+str(xl[k])+"|"

        return string

    def get_short_cross_link_string(self,xl):

        string='|'
        list_of_keys=[self.data_set_name_key,
                      self.unique_sub_id_key,
                      self.protein1_key,
                      self.residue1_key,
                      self.protein2_key,
                      self.residue2_key,
                      self.state_key,
                      self.psi_key]

        for k in list_of_keys:
            try:
                string+=str(xl[k])+"|"
            except KeyError:
                continue

        return string

    def filter(self,FilterOperator):
        new_xl_dict={}
        for id in self.data_base.keys():
            for xl in self.data_base[id]:
                if FilterOperator.evaluate(xl):
                    if id not in new_xl_dict:
                        new_xl_dict[id]=[xl]
                    else:
                        new_xl_dict[id].append(xl)
        self._update()
        cdb = CrossLinkDataBase(self.cldbkc,new_xl_dict)
        cdb.dataset = self.dataset
        return cdb

    def filter_score(self,score):
        '''Get all crosslinks with score greater than an input value'''
        FilterOperator=IMP.pmi.io.crosslink.FilterOperator(self.id_score_key,operator.gt,score)
        return self.filter(FilterOperator)

    def merge(self,CrossLinkDataBase1,CrossLinkDataBase2):
        '''
        This function merges two cross-link datasets so that if two conflicting crosslinks have the same
        cross-link UniqueIDS, the cross-links will be appended under the same UniqueID slots
        with different SubIDs
        '''
        pass

    def append_database(self,CrossLinkDataBase2):
        '''
        This function append one cross-link dataset to another. Unique ids will be renamed to avoid
        conflicts.
        '''
        name1=self.get_name()
        name2=CrossLinkDataBase2.get_name()
        if name1 == name2:
            name1=id(self)
            name2=id(CrossLinkDataBase2)
            self.set_name(name1)
            CrossLinkDataBase2.set_name(name2)

        #rename first database:
        new_data_base={}
        for k in self.data_base:
            new_data_base[k]=self.data_base[k]
        for k in CrossLinkDataBase2.data_base:
            new_data_base[k]=CrossLinkDataBase2.data_base[k]
        self.data_base=new_data_base
        self._update()

    def set_value(self,key,new_value,FilterOperator=None):
        '''
        This function changes the value for a given key in the database
        For instance one can change the name of a protein
        @param key: the key in the database that must be changed
        @param new_value: the new value of the key
        @param FilterOperator: optional FilterOperator to change the value to
                               a subset of the database

        example: `cldb1.set_value(cldb1.protein1_key,'FFF',FO(cldb.protein1_key,operator.eq,"AAA"))`
        '''

        for xl in self:
            if FilterOperator is not None:
                if FilterOperator.evaluate(xl):
                    xl[key]=new_value
            else:
                xl[key]=new_value
        self._update()

    def get_values(self,key):
        '''
        this function returns the list of values for a given key in the database
        alphanumerically sorted
        '''
        values=set()
        for xl in self:
            values.add(xl[key])
        return sorted(list(values))

    def offset_residue_index(self,protein_name,offset):
        '''
        This function offset the residue indexes of a given protein by a specified value
        @param protein_name: the protein name that need to be changed
        @param offset: the offset value
        '''

        for xl in self:
            if xl[self.protein1_key] == protein_name:
                xl[self.residue1_key]=xl[self.residue1_key]+offset
            if xl[self.protein2_key] == protein_name:
                xl[self.residue2_key]=xl[self.residue2_key]+offset
        self._update()

    def create_new_keyword(self,keyword,values_from_keyword=None):
        '''
        This function creates a new keyword for the whole database and set the values from
        and existing keyword (optional), otherwise the values are set to None
        @param keyword the new keyword name:
        @param values_from_keyword the keyword from which we are copying the values:
        '''
        for xl in self:
            if values_from_keyword is not None:
                xl[keyword] = xl[values_from_keyword]
            else:
                xl[keyword] = None
        self._update()

    def rename_proteins(self,old_to_new_names_dictionary, protein_to_rename="both"):
        '''
        This function renames all proteins contained in the input dictionary
        from the old names (keys) to the new name (values)
        @param old_to_new_names_dictionary dictionary for converting old to new names
        @param protein_to_rename specify whether to rename both or protein1 or protein2 only
        '''

        for old_name in old_to_new_names_dictionary:
            new_name=old_to_new_names_dictionary[old_name]
            if protein_to_rename == "both" or protein_to_rename == "protein1":
                fo2=FilterOperator(self.protein1_key,operator.eq,old_name)
                self.set_value(self.protein1_key,new_name,fo2)
            if protein_to_rename == "both" or protein_to_rename == "protein2":
                fo2=FilterOperator(self.protein2_key,operator.eq,old_name)
                self.set_value(self.protein2_key,new_name,fo2)

    def classify_crosslinks_by_score(self,number_of_classes):
        '''
        This function creates as many classes as in the input (number_of_classes: integer)
        and partition crosslinks according to their identification scores. Classes are defined in the psi key.
        '''

        if self.id_score_key is not None:
            scores=self.get_values(self.id_score_key)
        else:
            raise ValueError('The crosslink database does not contain score values')
        minscore=min(scores)
        maxscore=max(scores)
        scoreclasses=numpy.linspace(minscore, maxscore, number_of_classes+1)
        if self.psi_key is None:
            self.create_new_keyword(self.psi_key,values_from_keyword=None)
        for xl in self:
            score=xl[self.id_score_key]
            for n,classmin in enumerate(scoreclasses[0:-1]):
                if score>=classmin and score<=scoreclasses[n+1]:
                    xl[self.psi_key]=str("CLASS_"+str(n))
        self._update()

    def clone_protein(self,protein_name,new_protein_name):
        new_xl_dict={}
        for id in self.data_base.keys():
            new_data_base=[]
            for xl in self.data_base[id]:
                new_data_base.append(xl)
                if xl[self.protein1_key]==protein_name and xl[self.protein2_key]!=protein_name:
                    new_xl=dict(xl)
                    new_xl[self.protein1_key]=new_protein_name
                    new_data_base.append(new_xl)
                elif xl[self.protein1_key]!=protein_name and xl[self.protein2_key]==protein_name:
                    new_xl=dict(xl)
                    new_xl[self.protein2_key]=new_protein_name
                    new_data_base.append(new_xl)
                elif xl[self.protein1_key]==protein_name and xl[self.protein2_key]==protein_name:
                    new_xl=dict(xl)
                    new_xl[self.protein1_key]=new_protein_name
                    new_data_base.append(new_xl)
                    new_xl=dict(xl)
                    new_xl[self.protein2_key]=new_protein_name
                    new_data_base.append(new_xl)
                    new_xl=dict(xl)
                    new_xl[self.protein1_key]=new_protein_name
                    new_xl[self.protein2_key]=new_protein_name
                    new_data_base.append(new_xl)
            self.data_base[id]=new_data_base
        self._update()

    def filter_out_same_residues(self):
        '''
        This function remove cross-links applied to the same residue
        (ie, same chain name and residue number)
        '''
        new_xl_dict={}
        for id in self.data_base.keys():
            new_data_base=[]
            for xl in self.data_base[id]:
                if xl[self.protein1_key]==xl[self.protein2_key] and xl[self.residue1_key]==xl[self.residue2_key]:
                    continue
                else:
                    new_data_base.append(xl)
            self.data_base[id]=new_data_base
        self._update()


    def jackknife(self,percentage):
        '''
        this method returns a CrossLinkDataBase class containing
        a random subsample of the original cross-link database.
        @param percentage float between 0 and 1, is the percentage of
                          of spectra taken from the original list
        '''
        import random
        if percentage > 1.0 or percentage < 0.0:
            raise ValueError('the percentage of random cross-link spectra should be between 0 and 1')
        nspectra=self.get_number_of_xlid()
        nrandom_spectra=int(nspectra*percentage)
        random_keys=random.sample(self.data_base.keys(),nrandom_spectra)
        new_data_base={}
        for k in random_keys:
            new_data_base[k]=self.data_base[k]
        return CrossLinkDataBase(self.cldbkc,new_data_base)

    def __str__(self):
        outstr=''
        sorted_ids=sorted(self.data_base.keys())

        for id in sorted_ids:
            outstr+=id+"\n"
            for xl in self.data_base[id]:
                for k in self.ordered_key_list:
                    try:
                        outstr+="--- "+str(k)+" "+str(xl[k])+"\n"
                    except KeyError:
                        continue

                for k in xl:
                    if k not in self.ordered_key_list:
                        outstr+="--- "+str(k)+" "+str(xl[k])+"\n"
                outstr+="-------------\n"
        return outstr


    def plot(self,filename,**kwargs):
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib.colors



        if kwargs["type"] == "scatter":
            cmap=plt.get_cmap("rainbow")
            norm=matplotlib.colors.Normalize(vmin=0.0, vmax=1.0)
            xkey=kwargs["xkey"]
            ykey=kwargs["ykey"]
            if "colorkey" in kwargs:
                colorkey=kwargs["colorkey"]
            if "sizekey" in kwargs:
                sizekey=kwargs["sizekey"]
            if "logyscale" in kwargs:
                logyscale=kwargs["logyscale"]
            else:
                logyscale=False
            xs=[]
            ys=[]
            colors=[]
            for xl in self:
                try:
                    xs.append(float(xl[xkey]))
                    if logyscale:
                        ys.append(math.log10(float(xl[ykey])))
                    else:
                        ys.append(float(xl[ykey]))
                    colors.append(float(xl[colorkey]))
                except ValueError:
                    print("CrossLinkDataBase.plot: Value error for cross-link %s" % (xl[self.unique_id_key]))
                    continue

            cs=[]
            for color in colors:
                cindex=(color-min(colors))/(max(colors)-min(colors))
                cs.append(cmap(cindex))

            fig = plt.figure()
            ax = fig.add_subplot(111)
            ax.scatter(xs, ys, s=50.0, c=cs, alpha=0.8,marker="o")
            ax.set_xlabel(xkey)
            ax.set_ylabel(ykey)
            plt.savefig(filename)
            plt.show()
            plt.close()

        if kwargs["type"] == "residue_links":
            #plot the number of distinct links to a residue
            #in an histogram
            #molecule name
            molecule=kwargs["molecule"]
            if type(molecule) is IMP.pmi.topology.Molecule:
                length=len(molecule.sequence)
                molecule=molecule.get_name()
            else:
                #you need a IMP.pmi.topology.Sequences object
                sequences_object=kwargs["sequences_object"]
                sequence=sequences_object.sequences[molecule]
                length=len(sequence)

            histogram=[0]*length
            for xl in self:
                if xl[self.protein1_key]==molecule:
                    histogram[xl[self.residue1_key]-1]=xl[self.residue1_links_number_key]
                if xl[self.protein2_key]==molecule:
                    histogram[xl[self.residue2_key]-1]=xl[self.residue2_links_number_key]
            rects = plt.bar(range(1,length+1), histogram)
                 #bar_width,
                 #alpha=opacity,
                 #color='b',
                 #yerr=std_men,
                 #error_kw=error_config,
                 #label='Men')
            plt.savefig(filename)
            plt.show()
            plt.close()



        if kwargs["type"] == "histogram":
            valuekey=kwargs["valuekey"]
            reference_xline=kwargs["reference_xline"]
            valuename=valuekey
            bins=kwargs["bins"]
            values_list=[]
            for xl in self:
                try:
                    values_list.append(float(xl[valuekey]))
                except ValueError:
                    print("CrossLinkDataBase.plot: Value error for cross-link %s" % (xl[self.unique_id_key]))
                    continue
            IMP.pmi.output.plot_field_histogram(
                  filename, [values_list], valuename=valuename, bins=bins,
                  colors=None, format="pdf",
                  reference_xline=None, yplotrange=None,
                  xplotrange=None,normalized=True,
                  leg_names=None)

    def dump(self,json_filename):
        import json
        with open(json_filename, 'w') as fp:
            json.dump(self.data_base, fp, sort_keys=True, indent=2, default=set_json_default)

    def load(self,json_filename):
        import json
        with open(json_filename, 'r') as fp:
            self.data_base = json.load(fp)
        self._update()
        #getting rid of unicode
        # (can't do this in Python 3, since *everything* is Unicode there)
        if sys.version_info[0] < 3:
            for xl in self:
                for k,v in xl.iteritems():
                    if type(k) is unicode: k=str(k)
                    if type(v) is unicode: v=str(v)
                    xl[k]=v

    def save_csv(self,filename):

        import csv

        data=[]
        sorted_ids=None
        sorted_group_ids=sorted(self.data_base.keys())
        for group in sorted_group_ids:
            group_block=[]
            for xl in self.data_base[group]:
                if not sorted_ids:
                    sorted_ids=sorted(xl.keys())
                values=[xl[k] for k in sorted_ids]
                group_block.append(values)
            data+=group_block


        with open(filename, 'w') as fp:
            a = csv.writer(fp, delimiter=',')
            a.writerow(sorted_ids)
            a.writerows(data)

    def get_number_of_unique_crosslinked_sites(self):
        """
        Returns the number of non redundant crosslink sites
        """
        praset=set()
        for xl in self:
            pra=_ProteinsResiduesArray(xl)
            prai=pra.get_inverted()
            praset.add(pra)
            praset.add(prai)
        return len(list(praset))

class JaccardDistanceMatrix(object):
    """This class allows to compute and plot the distance between datasets"""

    def __init__(self,cldb_dictionary):
        """Input a dictionary where keys are cldb names and values are cldbs"""
        import scipy.spatial.distance
        self.dbs=cldb_dictionary
        self.keylist=list(self.dbs.keys())
        self.distances=list()


        for i,key1 in enumerate(self.keylist):
            for key2 in self.keylist[i+1:]:
                distance=self.get_distance(key1,key2)
                self.distances.append(distance)

        self.distances=scipy.spatial.distance.squareform(self.distances)

    def get_distance(self,key1,key2):
        return 1.0-self.jaccard_index(self.dbs[key1],self.dbs[key2])

    def jaccard_index(self,CrossLinkDataBase1,CrossLinkDataBase2):
        """Similarity index between two datasets
        https://en.wikipedia.org/wiki/Jaccard_index"""

        set1=set()
        set2=set()
        for xl1 in CrossLinkDataBase1:
            a1f=_ProteinsResiduesArray(xl1)
            a1b=a1f.get_inverted()
            set1.add(a1f)
            set1.add(a1b)
        for xl2 in CrossLinkDataBase2:
            a2f=_ProteinsResiduesArray(xl2)
            a2b=a2f.get_inverted()
            set2.add(a2f)
            set2.add(a2b)
        return float(len(set1&set2)/2)/(len(set1)/2+len(set2)/2-len(set1&set2)/2)

    def plot_matrix(self,figurename="clustermatrix.pdf"):
        import matplotlib as mpl
        import numpy
        mpl.use('Agg')
        import matplotlib.pylab as pl
        from scipy.cluster import hierarchy as hrc

        raw_distance_matrix=self.distances
        labels=self.keylist

        fig = pl.figure()
        #fig.autolayout=True

        ax = fig.add_subplot(1,1,1)
        dendrogram = hrc.dendrogram(
            hrc.linkage(raw_distance_matrix),
            color_threshold=7,
            no_labels=True)
        leaves_order = dendrogram['leaves']
        ax.set_xlabel('Dataset')
        ax.set_ylabel('Jaccard Distance')
        pl.tight_layout()
        pl.savefig("dendrogram."+figurename, dpi=300)
        pl.close(fig)

        fig = pl.figure()
        #fig.autolayout=True

        ax = fig.add_subplot(1,1,1)
        cax = ax.imshow(
            raw_distance_matrix[leaves_order,
                                     :][:,
                                        leaves_order],
            interpolation='nearest')
        cb = fig.colorbar(cax)
        cb.set_label('Jaccard Distance')
        ax.set_xlabel('Dataset')
        ax.set_ylabel('Dataset')
        ax.set_xticks(range(len(labels)))
        ax.set_xticklabels(numpy.array(labels)[leaves_order], rotation='vertical')
        ax.set_yticks(range(len(labels)))
        ax.set_yticklabels(numpy.array(labels)[leaves_order], rotation='horizontal')
        pl.tight_layout()
        pl.savefig("matrix."+figurename, dpi=300)
        pl.close(fig)


class MapCrossLinkDataBaseOnStructure(object):
    '''
    This class maps a CrossLinkDataBase on a given structure
    and save an rmf file with color-coded crosslinks
    '''


    def __init__(self,CrossLinkDataBase,rmf_or_stat_handler):
        self.CrossLinkDataBase=CrossLinkDataBase
        if type(rmf_or_stat_handler) is IMP.pmi.output.RMFHierarchyHandler or \
                type(rmf_or_stat_handler) is IMP.pmi.output.StatHierarchyHandler:
            self.prots=rmf_or_stat_handler
        self.distances=defaultdict(list)
        self.array_to_id={}
        self.id_to_array={}

        print("computing distances fro all crosslinks and all structures")
        for i in self.prots[::10]:
            self.compute_distances()
            for key,xl in enumerate(self.CrossLinkDataBase):
                array=_ProteinsResiduesArray(xl)
                self.array_to_id[array]=key
                self.id_to_array[key]=array
                if xl["MinAmbiguousDistance"] is not 'None':
                    self.distances[key].append(xl["MinAmbiguousDistance"])

    def compute_distances(self):
        data=[]
        sorted_ids=None
        sorted_group_ids=sorted(self.CrossLinkDataBase.data_base.keys())
        for group in sorted_group_ids:
            #group_block=[]
            group_dists=[]
            for xl in self.CrossLinkDataBase.data_base[group]:
                if not sorted_ids:
                    sorted_ids=sorted(xl.keys())
                    data.append(sorted_ids+["UniqueID","Distance","MinAmbiguousDistance"])
                (c1,c2,r1,r2)=_ProteinsResiduesArray(xl)
                try:
                    (mdist,p1,p2),(state1,copy1,state2,copy2)=self._get_distance_and_particle_pair(r1,c1,r2,c2)
                except:
                    mdist="None"
                    state1="None"
                    copy1="None"
                    state2="None"
                    copy2="None"
                try:
                    # sometimes keys get "lost" in the database, not really sure why
                    values=[xl[k] for k in sorted_ids]
                    values += [group, mdist]
                except KeyError as e:
                    print("MapCrossLinkDataBaseOnStructure KeyError: {0} in {1}".format(e, xl))
                group_dists.append(mdist)
                #group_block.append(values)
                xl["Distance"]=mdist
                xl["State1"]=state1
                xl["Copy1"]=copy1
                xl["State2"]=state2
                xl["Copy2"]=copy2

            for xl in self.CrossLinkDataBase.data_base[group]:
                xl["MinAmbiguousDistance"]=min(group_dists)

    def _get_distance_and_particle_pair(self,r1,c1,r2,c2):
        '''more robust and slower version of above'''
        sel=IMP.atom.Selection(self.prots,molecule=c1,residue_index=r1,resolution=1)
        selpart_1=sel.get_selected_particles()
        if len(selpart_1)==0:
            print("MapCrossLinkDataBaseOnStructure: Warning: no particle selected for first site")
            return None
        sel=IMP.atom.Selection(self.prots,molecule=c2,residue_index=r2,resolution=1)
        selpart_2=sel.get_selected_particles()
        if len(selpart_2)==0:
            print("MapCrossLinkDataBaseOnStructure: Warning: no particle selected for second site")
            return None
        results=[]
        for p1 in selpart_1:
            for p2 in selpart_2:
                if p1 == p2 and r1 == r2: continue
                d1=IMP.core.XYZ(p1)
                d2=IMP.core.XYZ(p2)
                #round distance to second decimal
                dist=float(int(IMP.core.get_distance(d1,d2)*100.0))/100.0
                h1=IMP.atom.Hierarchy(p1)
                while not IMP.atom.Molecule.get_is_setup(h1.get_particle()):
                    h1=h1.get_parent()
                copy_index1=IMP.atom.Copy(h1).get_copy_index()
                while not IMP.atom.State.get_is_setup(h1.get_particle()):
                    h1=h1.get_parent()
                state_index1=IMP.atom.State(h1).get_state_index()
                h2=IMP.atom.Hierarchy(p2)
                while not IMP.atom.Molecule.get_is_setup(h2.get_particle()):
                    h2=h2.get_parent()
                copy_index2=IMP.atom.Copy(h2).get_copy_index()
                while not IMP.atom.State.get_is_setup(h2.get_particle()):
                    h2=h2.get_parent()
                state_index2=IMP.atom.State(h2).get_state_index()

                results.append((dist,state_index1,copy_index1,state_index2,copy_index2,p1,p2))
        if len(results)==0: return None
        results_sorted = sorted(results, key=operator.itemgetter(0,1,2,3,4))
        return (results_sorted[0][0],results_sorted[0][5],results_sorted[0][6]),(results_sorted[0][1],results_sorted[0][2],results_sorted[0][3],results_sorted[0][4])

    def save_rmf_snapshot(self,filename,color_id=None):
        sorted_group_ids=sorted(self.CrossLinkDataBase.data_base.keys())
        list_of_pairs=[]
        color_scores=[]
        for group in sorted_group_ids:
            group_dists_particles=[]
            for xl in self.CrossLinkDataBase.data_base[group]:
                xllabel=self.CrossLinkDataBase.get_short_cross_link_string(xl)
                (c1,c2,r1,r2)=_ProteinsResiduesArray(xl)
                try:
                    (mdist,p1,p2),(state1,copy1,state2,copy2)=self._get_distance_and_particle_pair(r1,c1,r2,c2)
                except TypeError:
                    print("TypeError or missing chain/residue ",r1,c1,r2,c2)
                    continue
                if color_id is not None:
                    group_dists_particles.append((mdist,p1,p2,xllabel,float(xl[color_id])))
                else:
                    group_dists_particles.append((mdist,p1,p2,xllabel,0.0))
            if group_dists_particles:
                (minmdist,minp1,minp2,minxllabel,mincolor_score)=min(group_dists_particles, key = lambda t: t[0])
                color_scores.append(mincolor_score)
                list_of_pairs.append((minp1,minp2,minxllabel,mincolor_score))
            else:
                continue

        linear = IMP.core.Linear(0, 0.0)
        linear.set_slope(1.0)
        dps2 = IMP.core.DistancePairScore(linear)
        rslin = IMP.RestraintSet(self.prots.get_model(), 'linear_dummy_restraints')
        sgs=[]
        offset=min(color_scores)
        maxvalue=max(color_scores)
        for pair in list_of_pairs:
            pr = IMP.core.PairRestraint(self.prots.get_model(), dps2, (pair[0], pair[1]))
            pr.set_name(pair[2])
            if offset<maxvalue:
                factor=(pair[3]-offset)/(maxvalue-offset)
            else:
                factor=0.0
            c=IMP.display.get_rgb_color(factor)
            seg=IMP.algebra.Segment3D(IMP.core.XYZ(pair[0]).get_coordinates(),IMP.core.XYZ(pair[1]).get_coordinates())
            rslin.add_restraint(pr)
            sgs.append(IMP.display.SegmentGeometry(seg,c,pair[2]))

        rh = RMF.create_rmf_file(filename)
        IMP.rmf.add_hierarchies(rh, [self.prots])
        IMP.rmf.add_restraints(rh,[rslin])
        IMP.rmf.add_geometries(rh, sgs)
        IMP.rmf.save_frame(rh)
        del rh

    def boxplot_crosslink_distances(self,filename):
        import numpy
        keys=[self.id_to_array[k] for k in self.distances.keys()]
        medians=[numpy.median(self.distances[self.array_to_id[k]]) for k in keys]
        dists=[self.distances[self.array_to_id[k]] for k in keys]
        distances_sorted_by_median=[x for _,x in sorted(zip(medians,dists))]
        keys_sorted_by_median=[x for _,x in sorted(zip(medians,keys))]
        IMP.pmi.output.plot_fields_box_plots(filename,
                                         distances_sorted_by_median,
                                         range(len(distances_sorted_by_median)),
                                         xlabels=keys_sorted_by_median,scale_plot_length=0.2)

    def get_crosslink_violations(self,threshold):
        violations=defaultdict(list)
        for k in self.distances:
            #viols=map(lambda x:1.0*(x<= threshold), self.distances[k])
            viols=self.distances[k]
            violations[self.id_to_array[k]]=viols
        return violations


class CrossLinkDataBaseFromStructure(object):
    '''
    This class generates a CrossLinkDataBase from a given structure
    '''
    def __init__(self,representation=None,
                      system=None,
                      residue_types_1=["K"],
                      residue_types_2=["K"],
                      reactivity_range=[0,1],
                      kt=1.0):

        import numpy.random
        import math
        cldbkc=CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("Protein1")
        cldbkc.set_protein2_key("Protein2")
        cldbkc.set_residue1_key("Residue1")
        cldbkc.set_residue2_key("Residue2")
        self.cldb=CrossLinkDataBase(cldbkc)
        if representation is not None:
            #PMI 1.0 mode
            self.mode="pmi1"
            self.representation=representation
            self.model=self.representation.model
        elif system is not None:
            #PMI 2.0 mode
            self.system=system
            self.model=self.system.model
            self.mode="pmi2"
        else:
            print("Argument error: please provide either a representation object or a IMP.Hierarchy")
            raise
        self.residue_types_1=residue_types_1
        self.residue_types_2=residue_types_2
        self.kt=kt
        self.indexes_dict1={}
        self.indexes_dict2={}
        self.protein_residue_dict={}
        self.reactivity_dictionary={}
        self.euclidean_interacting_pairs=None
        self.xwalk_interacting_pairs=None
        import random

        if self.mode=="pmi1":
            for protein in self.representation.sequence_dict.keys():
            # we are saving a dictionary with protein name, residue number and random reactivity of the residue
                seq=self.representation.sequence_dict[protein]
                residues=[i for i in range(1,len(seq)+1) if ((seq[i-1] in self.residue_types_1) or (seq[i-1] in self.residue_types_2))]

                for r in residues:
                    # uniform random reactivities
                    #self.reactivity_dictionary[(protein,r)]=random.uniform(reactivity_range[0],reactivity_range[1])
                    # getting reactivities from the CDF of an exponential distribution
                    rexp=numpy.random.exponential(0.1)
                    prob=1.0-math.exp(-rexp)
                    self.reactivity_dictionary[(protein,r)]=prob


                residues1=[i for i in range(1,len(seq)+1) if seq[i-1] in self.residue_types_1]
                residues2=[i for i in range(1,len(seq)+1) if seq[i-1] in self.residue_types_2]
                for r in residues1:
                    h=IMP.pmi.tools.select_by_tuple(self.representation,(r,r,protein),resolution=1)[0]
                    p=h.get_particle()
                    index=p.get_index()
                    self.indexes_dict1[index]=(protein,r)
                    self.protein_residue_dict[(protein,r)]=index
                for r in residues2:
                    h=IMP.pmi.tools.select_by_tuple(self.representation,(r,r,protein),resolution=1)[0]
                    p=h.get_particle()
                    index=p.get_index()
                    self.indexes_dict2[index]=(protein,r)
                    self.protein_residue_dict[(protein,r)]=index

        if self.mode=="pmi2":
            for state in self.system.get_states():
                for moleculename,molecules in state.get_molecules().items():
                    for molecule in molecules:
                # we are saving a dictionary with protein name, residue number and random reactivity of the residue
                        seq=molecule.sequence
                        residues=[i for i in range(1,len(seq)+1) if ((seq[i-1] in self.residue_types_1) or (seq[i-1] in self.residue_types_2))]

                        for r in residues:
                            # uniform random reactivities
                            #self.reactivity_dictionary[(protein,r)]=random.uniform(reactivity_range[0],reactivity_range[1])
                            # getting reactivities from the CDF of an exponential distribution
                            rexp=numpy.random.exponential(0.00000001)
                            prob=1.0-math.exp(-rexp)
                            self.reactivity_dictionary[(molecule,r)]=prob

                        residues1=[i for i in range(1,len(seq)+1) if seq[i-1] in self.residue_types_1]
                        residues2=[i for i in range(1,len(seq)+1) if seq[i-1] in self.residue_types_2]
                        for r in residues1:
                            s=IMP.atom.Selection(molecule.hier,residue_index=r,resolution=1)
                            ps=s.get_selected_particles()
                            for p in ps:
                                index=p.get_index()
                                self.indexes_dict1[index]=(molecule,r)
                                self.protein_residue_dict[(molecule,r)]=index
                        for r in residues2:
                            s=IMP.atom.Selection(molecule.hier,residue_index=r,resolution=1)
                            ps=s.get_selected_particles()
                            for p in ps:
                                index=p.get_index()
                                self.indexes_dict2[index]=(molecule,r)
                                self.protein_residue_dict[(molecule,r)]=index


    def get_all_possible_pairs(self):
        n=float(len(self.protein_residue_dict.keys()))
        return n*(n-1.0)/2.0

    def get_all_feasible_pairs(self,distance=21):
        import itertools
        particle_index_pairs=[]
        nxl=0
        for a,b in itertools.combinations(self.protein_residue_dict.keys(),2):

            new_xl={}
            index1=self.protein_residue_dict[a]
            index2=self.protein_residue_dict[b]
            particle_distance=IMP.core.get_distance(IMP.core.XYZ(IMP.get_particles(self.model,[index1])[0]),IMP.core.XYZ(IMP.get_particles(self.model,[index2])[0]))
            if particle_distance <= distance:
                particle_index_pairs.append((index1,index2))
                if self.mode=="pmi1":
                    new_xl[self.cldb.protein1_key]=a[0]
                    new_xl[self.cldb.protein2_key]=b[0]
                elif self.mode=="pmi2":
                    new_xl[self.cldb.protein1_key]=a[0].get_name()
                    new_xl[self.cldb.protein2_key]=b[0].get_name()
                    new_xl["molecule_object1"]=a[0]
                    new_xl["molecule_object2"]=b[0]
                new_xl[self.cldb.residue1_key]=a[1]
                new_xl[self.cldb.residue2_key]=b[1]
                self.cldb.data_base[str(nxl)]=[new_xl]
                nxl+=1
        self.cldb._update()
        return self.cldb




    def get_data_base(self,total_number_of_spectra,
                                  ambiguity_probability=0.1,
                                  noise=0.01,
                                  distance=21,
                                  max_delta_distance=10.0,
                                  xwalk_bin_path=None,
                                  confidence_false=0.75,
                                  confidence_true=0.75):
        import math
        from random import random,uniform
        import numpy as np
        number_of_spectra=1

        self.beta_true=-1.4427*math.log(0.5*(1.0-confidence_true))
        self.beta_false=-1.4427*math.log(0.5*(1.0-confidence_false))
        self.cldb.data_base[str(number_of_spectra)]=[]
        self.sites_weighted=None

        while number_of_spectra<total_number_of_spectra:
            if random() > ambiguity_probability and len(self.cldb.data_base[str(number_of_spectra)]) != 0:
                    # new spectrum
                number_of_spectra+=1
                self.cldb.data_base[str(number_of_spectra)]=[]
            noisy=False
            if random() > noise:
                # not noisy crosslink
                pra,dist=self.get_random_residue_pair(distance,xwalk_bin_path,max_delta_distance)
            else:
                # noisy crosslink
                pra,dist=self.get_random_residue_pair(None,None,None)
                noisy=True

            new_xl={}
            if self.mode=="pmi1":
                new_xl[self.cldb.protein1_key]=pra[0]
                new_xl[self.cldb.protein2_key]=pra[1]
            elif self.mode=="pmi2":
                new_xl[self.cldb.protein1_key]=pra[0].get_name()
                new_xl[self.cldb.protein2_key]=pra[1].get_name()
                new_xl["molecule_object1"]=pra[0]
                new_xl["molecule_object2"]=pra[1]
            new_xl[self.cldb.residue1_key]=pra[2]
            new_xl[self.cldb.residue2_key]=pra[3]
            new_xl["Noisy"]=noisy
            # the reactivity is defined as r=1-exp(-k*Delta t)
            new_xl["Reactivity_Residue1"]=self.reactivity_dictionary[(pra[0],pra[2])]
            new_xl["Reactivity_Residue2"]=self.reactivity_dictionary[(pra[1],pra[3])]
            r1=new_xl["Reactivity_Residue1"]
            r2=new_xl["Reactivity_Residue2"]
            #combined reactivity 1-exp(-k12*Delta t),
            # k12=k1*k2/(k1+k2)
            #new_xl["Reactivity"]=1.0-math.exp(-math.log(1.0/(1.0-r1))*math.log(1.0/(1.0-r2))/math.log(1.0/(1.0-r1)*1.0/(1.0-r2)))
            if noisy:
                #new_xl["Score"]=uniform(-1.0,1.0)
                new_xl["Score"]=np.random.beta(1.0,self.beta_false)
            else:
                #new_xl["Score"]=new_xl["Reactivity"]+uniform(0.0,2.0)
                new_xl["Score"]=1.0-np.random.beta(1.0,self.beta_true)
            new_xl["TargetDistance"]=dist
            new_xl["NoiseProbability"]=noise
            new_xl["AmbiguityProbability"]=ambiguity_probability
            # getting if it is intra or inter rigid body
            (p1,p2)=IMP.get_particles(self.model,[self.protein_residue_dict[(pra[0],pra[2])],
                                                                    self.protein_residue_dict[(pra[1],pra[3])]])
            if(IMP.core.RigidMember.get_is_setup(p1) and
               IMP.core.RigidMember.get_is_setup(p2) and
               IMP.core.RigidMember(p1).get_rigid_body() ==
               IMP.core.RigidMember(p2).get_rigid_body()):
                new_xl["InterRigidBody"] = False
            elif (IMP.core.RigidMember.get_is_setup(p1) and
               IMP.core.RigidMember.get_is_setup(p2) and
               IMP.core.RigidMember(p1).get_rigid_body() !=
               IMP.core.RigidMember(p2).get_rigid_body()):
                new_xl["InterRigidBody"] = True
            else:
                new_xl["InterRigidBody"] = None

            self.cldb.data_base[str(number_of_spectra)].append(new_xl)
        self.cldb._update()
        return self.cldb


    def get_random_residue_pair(self,distance=None,xwalk_bin_path=None,max_delta_distance=None):
        import IMP.pmi.tools
        import math
        from random import choice,uniform
        if distance is None:
        # get a random pair
            while True:
                if self.mode=="pmi1":
                    protein1=choice(self.representation.sequence_dict.keys())
                    protein2=choice(self.representation.sequence_dict.keys())
                    seq1=self.representation.sequence_dict[protein1]
                    seq2=self.representation.sequence_dict[protein2]
                    residue1=choice([i for i in range(1,len(seq1)+1) if seq1[i-1] in self.residue_types_1])
                    residue2=choice([i for i in range(1,len(seq2)+1) if seq2[i-1] in self.residue_types_2])
                    h1=IMP.pmi.tools.select_by_tuple(self.representation,(residue1,residue1,protein1),resolution=1)[0]
                    h2=IMP.pmi.tools.select_by_tuple(self.representation,(residue2,residue2,protein2),resolution=1)[0]
                    particle_distance=IMP.core.get_distance(IMP.core.XYZ(h1.get_particle()),IMP.core.XYZ(h2.get_particle()))
                    if (protein1,residue1) != (protein2,residue2):
                        break
                elif self.mode=="pmi2":
                    (protein1,residue1)=choice(self.protein_residue_dict.keys())
                    (protein2,residue2)=choice(self.protein_residue_dict.keys())
                    index1=self.protein_residue_dict[(protein1,residue1)]
                    index2=self.protein_residue_dict[(protein2,residue2)]
                    particle_distance=IMP.core.get_distance(IMP.core.XYZ(IMP.get_particles(self.model,[index1])[0]),IMP.core.XYZ(IMP.get_particles(self.model,[index2])[0]))
                    if (protein1,residue1) != (protein2,residue2):
                        break
        else:
            # get a pair of residues whose distance is below the threshold
            if not xwalk_bin_path:
                gcpf = IMP.core.GridClosePairsFinder()
                gcpf.set_distance(distance+max_delta_distance)

                while True:
                    #setup the reaction rates lists
                    if not self.sites_weighted:
                        self.sites_weighted=[]
                        for key in self.reactivity_dictionary:
                            r=self.reactivity_dictionary[key]
                            self.sites_weighted.append((key,r))
                    #get a random reaction site
                    first_site=self.weighted_choice(self.sites_weighted)
                    #get all distances
                    if not self.euclidean_interacting_pairs:
                        self.euclidean_interacting_pairs=gcpf.get_close_pairs(self.model,
                                    self.indexes_dict1.keys(),
                                    self.indexes_dict2.keys())
                    #get the partner for the first reacted site
                    first_site_pairs = [pair for pair in self.euclidean_interacting_pairs
                                     if self.indexes_dict1[pair[0]] == first_site or
                                        self.indexes_dict2[pair[1]] == first_site]
                    if len(first_site_pairs)==0: continue
                    #build the list of second reaction sites
                    second_sites_weighted=[]
                    for pair in first_site_pairs:
                        if self.indexes_dict1[pair[0]] == first_site: second_site = self.indexes_dict2[pair[1]]
                        if self.indexes_dict2[pair[1]] == first_site: second_site = self.indexes_dict1[pair[0]]
                        r=self.reactivity_dictionary[second_site]
                        second_sites_weighted.append((second_site,r))
                    second_site=self.weighted_choice(second_sites_weighted)
                    """
                    interacting_pairs_weighted=[]
                    for pair in self.euclidean_interacting_pairs:
                        r1=self.reactivity_dictionary[self.indexes_dict1[pair[0]]]
                        r2=self.reactivity_dictionary[self.indexes_dict2[pair[1]]]
                        #combined reactivity 1-exp(-k12*Delta t),
                        # k12=k1*k2/(k1+k2)
                        #print(r1,r2,dist)
                        r12=1.0-math.exp(-math.log(1.0/(1.0-r1))*math.log(1.0/(1.0-r2))/math.log(1.0/(1.0-r1)*1.0/(1.0-r2)))
                        interacting_pairs_weighted.append((pair,r12))
                        #weight1=math.exp(-self.reactivity_dictionary[self.indexes_dict1[pair[0]]]/self.kt)
                        #weight2=math.exp(-self.reactivity_dictionary[self.indexes_dict2[pair[1]]]/self.kt)
                        #interacting_pairs_weighted.append((pair,weight1*weight2))

                    while True:
                        pair=self.weighted_choice(interacting_pairs_weighted)
                        protein1,residue1=self.indexes_dict1[pair[0]]
                        protein2,residue2=self.indexes_dict2[pair[1]]
                        particle_pair=IMP.get_particles(self.model,pair)
                        particle_distance=IMP.core.get_distance(IMP.core.XYZ(particle_pair[0]),IMP.core.XYZ(particle_pair[1]))
                        if particle_distance<distance and (protein1,residue1) != (protein2,residue2):
                            break
                        elif particle_distance>=distance and (protein1,residue1) != (protein2,residue2) and max_delta_distance:
                            #allow some flexibility
                            prob=1.0-((particle_distance-distance)/max_delta_distance)**(0.3)
                            if uniform(0.0,1.0)<prob: break
                    """
                    protein1,residue1=first_site
                    protein2,residue2=second_site
                    print("CrossLinkDataBaseFromStructure.get_random_residue_pair:",
                          "First site",first_site,self.reactivity_dictionary[first_site],
                          "Second site",second_site,self.reactivity_dictionary[second_site])
                    particle_pair=IMP.get_particles(self.model,[self.protein_residue_dict[first_site],self.protein_residue_dict[second_site]])
                    particle_distance=IMP.core.get_distance(IMP.core.XYZ(particle_pair[0]),IMP.core.XYZ(particle_pair[1]))

                    if particle_distance<distance and (protein1,residue1) != (protein2,residue2):
                        break
                    elif particle_distance>=distance and (protein1,residue1) != (protein2,residue2) and max_delta_distance:
                        #allow some flexibility
                        #prob=1.0-((particle_distance-distance)/max_delta_distance)**(0.3)
                        #if uniform(0.0,1.0)<prob: break
                        if particle_distance-distance <  max_delta_distance: break



            else:
                if not self.xwalk_interacting_pairs:
                    self.xwalk_interacting_pairs=self.get_xwalk_distances(xwalk_bin_path,distance)
                interacting_pairs_weighted=[]

                for pair in self.xwalk_interacting_pairs:
                    protein1=pair[0]
                    protein2=pair[1]
                    residue1=pair[2]
                    residue2=pair[3]
                    weight1=math.exp(-self.reactivity_dictionary[(protein1,residue1)]/self.kt)
                    weight2=math.exp(-self.reactivity_dictionary[(protein2,residue2)]/self.kt)
                    interacting_pairs_weighted.append((pair,weight1*weight2))

                pair=self.weighted_choice(interacting_pairs_weighted)
                protein1=pair[0]
                protein2=pair[1]
                residue1=pair[2]
                residue2=pair[3]
                particle_distance=float(pair[4])



        return ((protein1,protein2,residue1,residue2)),particle_distance

    def get_xwalk_distances(self,xwalk_bin_path,distance):
        import IMP.pmi.output
        import os
        o=IMP.pmi.output.Output(atomistic=True)
        o.init_pdb("xwalk.pdb",self.representation.prot)
        o.write_pdb("xwalk.pdb")
        namechainiddict=o.dictchain["xwalk.pdb"]
        chainiddict={}

        for key in namechainiddict: chainiddict[namechainiddict[key]]=key
        xwalkout=os.popen('java -Xmx256m -cp ' + xwalk_bin_path +' Xwalk -infile xwalk.pdb -aa1 lys -aa2 lys -a1 cb -a2 cb -max '+str(distance)+' -bb').read()

        output_list_of_distance=[]
        for line in xwalkout.split("\n")[0:-2]:
            tockens=line.split()
            first=tockens[2]
            second=tockens[3]
            distance=float(tockens[6])
            fs=first.split("-")
            ss=second.split("-")
            chainid1=fs[2]
            chainid2=ss[2]
            protein1=chainiddict[chainid1]
            protein2=chainiddict[chainid2]
            residue1=int(fs[1])
            residue2=int(ss[1])
            output_list_of_distance.append((protein1,protein2,residue1,residue2,distance))
        return output_list_of_distance


    def weighted_choice(self,choices):
        import random
        # from http://stackoverflow.com/questions/3679694/a-weighted-version-of-random-choice
        total = sum(w for c, w in choices)
        r = random.uniform(0, total)
        upto = 0
        for c, w in choices:
            if upto + w > r:
                return c
            upto += w
        assert False, "Shouldn't get here"

    def save_rmf_snapshot(self,filename,color_id=None):
        import IMP.rmf
        import RMF
        if color_id is None:
            color_id="Reactivity"
        sorted_ids=None
        sorted_group_ids=sorted(self.cldb.data_base.keys())
        list_of_pairs=[]
        color_scores=[]
        for group in sorted_group_ids:
            group_xls=[]
            group_dists_particles=[]
            for xl in self.cldb.data_base[group]:
                xllabel=self.cldb.get_short_cross_link_string(xl)
                (c1,c2,r1,r2)=(xl["molecule_object1"],xl["molecule_object2"],xl[self.cldb.residue1_key],xl[self.cldb.residue2_key])
                try:
                    index1=self.protein_residue_dict[(c1,r1)]
                    index2=self.protein_residue_dict[(c2,r2)]
                    p1,p2=IMP.get_particles(self.model,[index1])[0],IMP.get_particles(self.model,[index2])[0]
                    mdist=xl["TargetDistance"]
                except TypeError:
                    print("TypeError or missing chain/residue ",r1,c1,r2,c2)
                    continue
                group_dists_particles.append((mdist,p1,p2,xllabel,float(xl[color_id])))
            if group_dists_particles:
                (minmdist,minp1,minp2,minxllabel,mincolor_score)=min(group_dists_particles, key = lambda t: t[0])
                color_scores.append(mincolor_score)
                list_of_pairs.append((minp1,minp2,minxllabel,mincolor_score))
            else:
                continue


        m=self.model
        linear = IMP.core.Linear(0, 0.0)
        linear.set_slope(1.0)
        dps2 = IMP.core.DistancePairScore(linear)
        rslin = IMP.RestraintSet(m, 'linear_dummy_restraints')
        sgs=[]
        offset=min(color_scores)
        maxvalue=max(color_scores)
        for pair in list_of_pairs:
            pr = IMP.core.PairRestraint(m, dps2, (pair[0], pair[1]))
            pr.set_name(pair[2])
            factor=(pair[3]-offset)/(maxvalue-offset)
            c=IMP.display.get_rgb_color(factor)
            seg=IMP.algebra.Segment3D(IMP.core.XYZ(pair[0]).get_coordinates(),IMP.core.XYZ(pair[1]).get_coordinates())
            rslin.add_restraint(pr)
            sgs.append(IMP.display.SegmentGeometry(seg,c,pair[2]))

        rh = RMF.create_rmf_file(filename)
        IMP.rmf.add_hierarchies(rh, [self.system.hier])
        IMP.rmf.add_restraints(rh,[rslin])
        IMP.rmf.add_geometries(rh, sgs)
        IMP.rmf.save_frame(rh)
        del rh
