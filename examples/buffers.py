## \example buffers.py
# Showing how to read and write from buffers

import RMF

buf = RMF.BufferHandle()

fw = RMF.create_rmf_buffer(buf)

# do stuff
del fw

fr = RMF.open_rmf_buffer_read_only(buf)
# do more stuff
