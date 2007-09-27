#include "DensityMap.h"

void DensityMap::Read(ifstream &map_file, MapReaderWriter &reader) {
  reader.Read(map_file);
  data = reader.get_data();
  header = reader.get_header();
}
void DensityMap::Write(ostream &file, MapReaderWriter &writer) {
  writer.Write(file);
}
