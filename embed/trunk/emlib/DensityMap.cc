#include "DensityMap.h"

void DensityMap::Read(ifstream &map_file, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  reader.Read(map_file,&data,header);
}
void DensityMap::Write(ostream &file, MapReaderWriter &writer) {
  writer.Write(file,data,header);
}
