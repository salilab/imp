import utility
from SCons.Script import File, Dir
import os
import os.path

class IMPData:
    def __init__(self, env):
        self.env=env
    def add_to_alias(self, name, nodes):
        #print "add alias", name
        self.env.Alias(name, nodes)
    def get_alias(self, name):
        #print "get alias", name
        return self.env.Alias(name)

def get(env):
    return env["IMP_DATA"]

def add(env, data= None):
    if not data:
        data=IMPData(env)
    env["IMP_DATA"] = data
