"""@namespace IMP.pmi.io.crosslink
   Handles cross-link data sets.

   Utilities are also provided to help in the analysis of models that
   contain cross-links.
"""

import IMP
import IMP.pmi
import operator

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
        self.quantitation_key="Quantitation"
        self.type[self.quantitation_key]=float
        self.redundancy_key="Redundancy"
        self.type[self.redundancy_key]=int
        self.redundancy_list_key="RedundancyList"
        self.type[self.redundancy_key]=list
        self.state_key="State"
        self.type[self.state_key]=int
        self.sigma1_key="Sigma1"
        self.type[self.sigma1_key]=float
        self.sigma2_key="Sigma2"
        self.type[self.sigma2_key]=float
        self.psi_key="Psi"
        self.type[self.psi_key]=float

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
                        self.quantitation_key,
                        self.redundancy_key,
                        self.redundancy_list_key,
                        self.state_key,
                        self.sigma1_key,
                        self.sigma2_key,
                        self.psi_key]


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
        if type(input_data) is dict:
            self.cldbsk=_CrossLinkDataBaseStandardKeys()
            p1=input_data[self.cldbsk.protein1_key]
            p2=input_data[self.cldbsk.protein2_key]
            r1=input_data[self.cldbsk.residue1_key]
            r2=input_data[self.cldbsk.residue2_key]
            t=(p1,p2,r1,r2)
        elif type(input_data) is tuple:
            if len(input_data)>4:
                raise TypeError("_ProteinsResiduesArray: must have only 4 elements")
            if type(input_data[0]) is not str:
                raise TypeError("_ProteinsResiduesArray: input_data[0] must be a string")
            if type(input_data[1]) is not str:
                raise TypeError("_ProteinsResiduesArray: input_data[1] must be a string")
            if type(input_data[2]) is not int:
                raise TypeError("_ProteinsResiduesArray: input_data[2] must be a integer")
            if type(input_data[3]) is not int:
                raise TypeError("_ProteinsResiduesArray: input_data[3] must be a integer")
            t=input_data
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

    def __init__(self):
        self.converter={}
        self.backward_converter={}
        _CrossLinkDataBaseStandardKeys.__init__(self)
        self.compulsory_keys=set([self.protein1_key,
                                  self.protein2_key,
                                  self.residue1_key,
                                  self.residue2_key])
        self.setup_keys=set()

    def check_keys(self):
        '''
        Is a function that check whether necessary keys are setup
        '''
        setup_keys=set(self.get_setup_keys())
        if self.compulsory_keys & setup_keys != self.compulsory_keys:
            raise KeyError("CrossLinkDataBaseKeywordsConverter: must setup all necessary keys")

    def get_setup_keys(self):
        '''
        Returns the keys that have been setup so far
        '''
        return self.backward_converter.keys()

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

    def set_idscore_key(self,origin_key):
        self.converter[origin_key]=self.id_score_key
        self.backward_converter[self.id_score_key]=origin_key

    def set_quantification_key(self,origin_key):
        self.converter[origin_key]=self.quantitation_key
        self.backward_converter[self.quantitation_key]=origin_key

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

class CrossLinkDataBase(_CrossLinkDataBaseStandardKeys):
    import operator
    '''
    this class handles a cross-link dataset and do filtering
    operations, adding cross-links, merge datasets...
    '''

    def __init__(self,CrossLinkDataBaseKeywordsConverter,data_base=None):
        '''
        To be constructed it needs a CrossLinkDataBaseKeywordsConverter instance first

        '''
        if data_base is None:
            self.data_base = {}
        else:
            self.data_base=data_base
        self.cldbkc=CrossLinkDataBaseKeywordsConverter
        _CrossLinkDataBaseStandardKeys.__init__(self)
        self.converter=CrossLinkDataBaseKeywordsConverter.get_converter()
        self.__update__()

    def __update__(self):
        '''
        Update the whole dataset after changes
        '''
        self.update_cross_link_unique_sub_index()
        self.update_cross_link_redundancy()

    def __iter__(self):
        for k in self.data_base:
            for xl in self.data_base[k]:
                yield xl

    def xlid_iterator(self):
        for xlid in self.data_base.keys():
            yield xlid

    def __getitem__(self,xlid):
        return self.data_base[xlid]

    def __len__(self):
        return len([xl for xl in self])

    def get_name(self):
        return self.name

    def set_name(self,name):
        self.name=name

    def get_number_of_xlid(self):
        return len(self.data_base)


    def create_set_from_file(self,csv_file_name):
        xl_list=IMP.pmi.tools.get_db_from_csv(csv_file_name)
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
                new_xl_dict[str(nxl)]=[new_xl]

        self.data_base=new_xl_dict
        self.name=csv_file_name
        self.__update__()

    def update_cross_link_unique_sub_index(self):
        for k in self.data_base:
            for n,xl in enumerate(self.data_base[k]):
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

    def append(self,CrossLinkDataBase2):
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
            new_data_base[k+"."+name1]=self.data_base[k]
        for k in CrossLinkDataBase2.data_base:
            new_data_base[k+"."+name2]=CrossLinkDataBase2.data_base[k]
        self.data_base=new_data_base
        self.__update__()

    def change_value(self,key,old_value,new_value):
        pass

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
        self.__update__()


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
