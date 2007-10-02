#include "DensityMap.h"

void DensityMap::Read(char *filename, MapReaderWriter &reader) {
  //TODO: we need to decide who does the allocation ( mapreaderwriter or density)? if we keep the current implementation ( mapreaderwriter ) we need to pass a pointer to data
  reader.Read(filename,&data,header);
}
void DensityMap::Write(char *filename, MapReaderWriter &writer) {
  writer.Write(filename,data,header);
}
