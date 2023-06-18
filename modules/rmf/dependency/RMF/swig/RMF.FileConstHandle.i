namespace RMF {
%rename(_get_keys) FileConstHandle::get_keys;
}

%extend RMF::FileConstHandle {
   %pythoncode %{
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    def get_frames(self):
        class MyRange:
           def __init__(self, mx):
               self.max = mx
           def __getitem__(self, i):
               if i >= self.max:
                    raise IndexError()
               else:
                    return FrameID(i)
           def __len__(self):
               return self.max
        return MyRange(self.get_number_of_frames())
    def get_node_ids(self):
        class MyRange:
           def __init__(self, mx):
               self.max = mx
           def __getitem__(self, i):
               if i >= self.max:
                    raise IndexError()
               else:
                    return NodeID(i)
           def __len__(self):
               return self.max
        return MyRange(self.get_number_of_nodes())
    def get_keys(self, kc):
        ret=[]
        for t in _tag_list:
           ret.extend(self._get_keys(kc, t))
        return ret
  %}
}
%include "RMF/FileConstHandle.h"
