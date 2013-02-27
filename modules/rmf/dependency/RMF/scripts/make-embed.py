#!/usr/bin/env python
import sys
import os.path
import os
# first is directory, second data, third subnamespace
output_directory=sys.argv[1]
data_path= sys.argv[2]
namespace = sys.argv[3]

data= open(data_path, "r").read()
quoted_data=data.replace("\"", "\\\"").replace("\n"," ")

file_name= os.path.split(data_path)[1]
name=file_name.replace(".", "_")

try:
    os.makedirs(output_directory)
except:
    pass

header= """#ifndef RMF_%(NAME)s_H
#define RMF_%(NAME)s_H
#include <RMF/config.h>
#include <string>
namespace RMF {
namespace %(namespace)s {
extern RMFEXPORT std::string %(name)s;
}
}
#endif
"""%{"name":name, "NAME":name.upper(), "namespace":namespace}
open(os.path.join(output_directory, file_name+".h"), "w").write(header);

header_path= output_directory[output_directory.rfind("src")+4:]+"/"+file_name+".h"

cpp= """#include <%(header_path)s>
namespace RMF {
namespace %(namespace)s {
std::string %(name)s="%(data)s";
}
}
"""%{"name":name, "NAME":name.upper(), "namespace":namespace,
     "header_path":header_path, "data":quoted_data}
open(os.path.join(output_directory, file_name+".cpp"), "w").write(cpp);
