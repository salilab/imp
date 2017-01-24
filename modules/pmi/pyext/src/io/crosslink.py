"""@namespace IMP.pmi.io.crosslink
   Handles cross-link data sets.

   Utilities are also provided to help in the analysis of models that
   contain cross-links.
"""

import IMP
import IMP.pmi
import operator
import sys

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
            return str(inp):
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

        self.ordered_key_list =[self.data_set_name_key,
                        self.unique_id_key,
                        self.unique_sub_index_key,
                        self.unique_sub_id_key,
                        self.protein1_key,
                        self.protein2_key,
                        self.residue1_key,
                        self.residue2_key,
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
                        self.min_ambiguous_distance_key]


class _ProteinsResiduesArray(tuple):
    '''
    This class is inherits from tuple, and it is a shorthand for a cross-link
    (p1,p2,r1,r2) where p1 and p2 are protein1 and protein2, r1 and r2 are
    residue1 and residue2.
    '''

    def __new__(self,input_data):
        '''
        @input_data can be a dict or a tuple
        '''
        self.cldbsk=_CrossLinkDataBaseStandardKeys()
        if type(input_data) is dict:
            p1=input_data[self.cldbsk.protein1_key]
            p2=input_data[self.cldbsk.protein2_key]
            r1=input_data[self.cldbsk.residue1_key]
            r2=input_data[self.cldbsk.residue2_key]
            t=(p1,p2,r1,r2)
        elif type(input_data) is tuple:
            if len(input_data)>4:
                raise TypeError("_ProteinsResiduesArray: must have only 4 elements")
            p1 = _handle_string_input(input_data[0])
            p2 = _handle_string_input(input_data[1])
            r1=input_data[2]
            r2=input_data[3]
            if type(r1) is not int:
                raise TypeError("_ProteinsResiduesArray: residue1 must be a integer")
            if type(r2) is not int:
                raise TypeError("_ProteinsResiduesArray: residue2 must be a integer")
            t=(p1,p2,r1,r2)
        else:
            raise TypeError("_ProteinsResiduesArray: input must be a dict or tuple")
        return tuple.__new__(_ProteinsResiduesArray, t)

    def get_inverted(self):
        '''
        Returns a _ProteinsResiduesArray instance with protein1 and protein2 inverted
        '''
        return _ProteinsResiduesArray((self[1],self[0],self[3],self[2]))

    def __str__(self):
        outstr=self.cldbsk.protein1_key+" "+str(self[0])
        outstr+=" "+self.cldbsk.protein2_key+" "+str(self[1])
        outstr+=" "+self.cldbsk.residue1_key+" "+str(self[2])
        outstr+=" "+self.cldbsk.residue2_key+" "+str(self[3])
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

        we need to implement a NOT
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

    def evaluate(self, xl_item):

        if len(self.operations) == 0:
            keyword, operator, value = self.values
            return operator(xl_item[keyword], value)
        FilterOperator1, op, FilterOperator2 = self.operations

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
    '''

    import re

    def __init__(self,style):

        _CrossLinkDataBaseStandardKeys.__init__(self)
        if style is "MSSTUDIO":
            self.style=style
            self.compulsory_keys= set([self.protein1_key,
                                  self.protein2_key,
                                  self.site_pairs_key])
        elif style is "QUANTITATION":
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
        if self.style == "QUANTITATION":
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
            return residue_pair_indexes,chain_pair_indexes


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
    import operator
    '''
    this class handles a cross-link dataset and do filtering
    operations, adding cross-links, merge datasets...
    '''

    def __init__(self, converter=None, data_base=None):
        '''
        Constructor.
        @param converter an instance of CrossLinkDataBaseKeywordsConverter
        @param data_base an instance of CrossLinkDataBase to build the new database on
        '''
        if data_base is None:
            self.data_base = {}
        else:
            self.data_base=data_base

        _CrossLinkDataBaseStandardKeys.__init__(self)
        if converter is not None:
            self.cldbkc = converter
            self.list_parser=self.cldbkc.rplp
            self.converter = converter.get_converter()

        else:
            self.cldbkc =    None
            self.list_parser=None
            self.converter = None

        self._update()

    def _update(self):
        '''
        Update the whole dataset after changes
        '''
        self.update_cross_link_unique_sub_index()
        self.update_cross_link_redundancy()

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
                        new_xl={}
                        for k in new_dict:
                            new_xl[k]=new_dict[k]
                        new_xl[self.residue1_key]=self.type[self.residue1_key](p[0])
                        new_xl[self.residue2_key]=self.type[self.residue2_key](p[1])

                        if len(chain_pair_list)==len(residue_pair_list):
                            new_xl[self.protein1_key]=self.type[self.protein1_key](chain_pair_list[n][0])
                            new_xl[self.protein2_key]=self.type[self.protein2_key](chain_pair_list[n][1])

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
        return CrossLinkDataBase(self.cldbkc,new_xl_dict)


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

    def rename_proteins(self,old_to_new_names_dictionary):
        '''
        This function renames all proteins contained in the input dictionary
        from the old names (keys) to the new name (values)
        '''

        for old_name in old_to_new_names_dictionary:
            new_name=old_to_new_names_dictionary[old_name]
            fo2=FilterOperator(self.protein1_key,operator.eq,old_name)
            self.set_value(self.protein1_key,new_name,fo2)
            fo2=FilterOperator(self.protein2_key,operator.eq,old_name)
            self.set_value(self.protein2_key,new_name,fo2)

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
        import matplotlib.pyplot as plt
        import matplotlib
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
            xs=[]
            ys=[]
            colors=[]
            for xl in self:
                try:
                    xs.append(float(xl[xkey]))
                    ys.append(float(xl[ykey]))
                    colors.append(float(xl[colorkey]))
                except ValueError:
                    print("Value error for cross-link %s" % (xl[self.unique_id_key]))
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
                    print("Value error for cross-link %s" % (xl[self.unique_id_key]))
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
            a.writerows(data)

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
            self.model=self.representation.m
        elif system is not None:
            #PMI 2.0 mode
            self.system=system
            self.model=self.system.mdl
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
                for moleculename,molecules in state.get_molecules().iteritems():
                    for molecule in molecules:
                        # we are saving a dictionary with protein name, residue number and random reactivity of the residue
                        seq=molecule.sequence
                        residues=[i for i in range(1,len(seq)+1) if ((seq[i-1] in self.residue_types_1) or (seq[i-1] in self.residue_types_2))]

                        for r in residues:
                            # uniform random reactivities
                            #self.reactivity_dictionary[(protein,r)]=random.uniform(reactivity_range[0],reactivity_range[1])
                            # getting reactivities from the CDF of an exponential distribution
                            rexp=numpy.random.exponential(0.1)
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



    def get_data_base(self,total_number_of_spectra,
                                  ambiguity_probability=0.1,
                                  noise=0.01,
                                  distance=21,
                                  max_delta_distance=10.0,
                                  xwalk_bin_path=None):
        import math
        from random import random,uniform
        number_of_spectra=1

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
            new_xl["Reactivity"]=1.0-math.exp(-math.log(1.0/(1.0-r1))*math.log(1.0/(1.0-r2))/math.log(1.0/(1.0-r1)*1.0/(1.0-r2)))
            if noisy:
                new_xl["Score"]=uniform(-1.0,1.0)
            else:
                new_xl["Score"]=new_xl["Reactivity"]+uniform(0.0,2.0)
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
                        prob=1.0-((particle_distance-distance)/max_delta_distance)**(0.3)
                        if uniform(0.0,1.0)<prob: break



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
