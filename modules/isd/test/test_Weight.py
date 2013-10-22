#!/usr/bin/env python

#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Weight

#unit testing framework
import IMP.test

class TestWeightParam(IMP.test.TestCase):
    """tests weight setup"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.kernel.Model()
        self.w = Weight.setup_particle(IMP.kernel.Particle(self.m))

    def test_Setup_iw(self):
        "Test weight_initial_values"
        w = Weight.setup_particle(IMP.kernel.Particle(self.m))      
        w.add_weight()  
        for n in range(19):
          for k in range(n+1):
            self.assertEqual(w.get_weight(k),1.0/(n+1))
          w.add_weight()

    def test_Setup_number(self):
        "Test weights_initial_values"
        w = Weight.setup_particle(IMP.kernel.Particle(self.m))      
        w.add_weight()  
        for n in range(19):
          ws=w.get_weights()
          self.assertEqual(len(ws),n+1)
          for k in range(n+1):
            self.assertEqual(ws[k],1.0/(n+1))
          w.add_weight()
        
    def test_Setup_size(self):
        "Test weights_size"
        w = Weight.setup_particle(IMP.kernel.Particle(self.m))   
        w.add_weight()     
        for n in range(19):
          ws=w.get_weights()
          ns=w.get_number_of_states()
          self.assertEqual(n+1,ns)          
          self.assertEqual(len(ws),ns)  
          w.add_weight()        

    def test_assign_values(self):
        "Test weights_set"
        w = Weight.setup_particle(IMP.kernel.Particle(self.m)) 
        w.add_weight()
        ws=[1.0]       
        for n in range(19):
          w.set_weights(ws)
          ws2=w.get_weights()
          for k in range(n+1):
             self.assertEqual(ws[k],ws2[k])  
          ws.append(1.0)
          w.add_weight()
        
    def test_set_optimized(self):
        "Test weights_optimized"
        w = Weight.setup_particle(IMP.kernel.Particle(self.m))   
        w.add_weight()    
        for n in range(19):
          w.set_weights_are_optimized(True)
          for k in range(n+1):
            b=w.get_is_optimized(w.get_weight_key(k))
            self.assertEqual(b,True)
          w.set_weights_are_optimized(False)
          for k in range(n+1):
            b=w.get_is_optimized(w.get_weight_key(k))
            self.assertEqual(b,False)                  
          w.add_weight()
          

if __name__ == '__main__':
    IMP.test.main()
