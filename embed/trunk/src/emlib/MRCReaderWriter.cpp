#include "MRCReaderWriter.h"

void MRCReaderWriter::Read(const char *fn_in, float **data, DensityHeader &head)
{
  // Read file
  filename.assign(fn_in);
  read(data);
  // Translate header to DensityHeader
  header.ToDensityHeader(head);
}

void MRCReaderWriter::Write(const char *fn_out, const float *data,
                            const DensityHeader &head)
{
  // Translate DensityHeader to MRCHeader and write
  header.FromDensityHeader(head);
  // Write
  write(fn_out,data);
}

void MRCReaderWriter::read(float **pt)
{
  fs.open(filename.c_str(), std::fstream::in | std::fstream::binary);
  if (!fs.fail()) {
    // Read header
    read_header();
    // Allocate memory
    size_t n = header.nx*header.ny*header.nz;
    (*pt)= new float[n]; 
    read_data(*pt);
  }
  // TODO - add exception
  fs.close();
}

void MRCReaderWriter::read_data(float *pt)
{
  if(header.mode==0) {
    return read_8_data(pt);
  }
  else if(header.mode==2) {
    return read_32_data(pt);
  }
  else {
    std::ostringstream msg;
    msg << "MRCReaderWriter::read_data >> This routine can only read "
    << "8-bit or 32-bit MRC files. Unknown mode for " << filename <<  "\n";
    throw EMBED_IOException(msg.str().c_str());
  }
}


/** Read the density data of a 8-bit MRC file */
void MRCReaderWriter::read_8_data(float *pt)
{
  seek_to_data();
  size_t n = header.nx*header.ny*header.nz;
  unsigned char *grid_8bit= new unsigned char [n]; // memory
  read_grid(grid_8bit,sizeof(unsigned char), n);
  // Transfer to floats
  for(size_t i=0;i<n;i++)
    pt[i]=(float)grid_8bit[i];
  delete(grid_8bit);
  std::cout << "MRC file read in 8-bit mode: grid "
  << header.nx << "x" << header.ny << "x" << header.nz << "\n";
}


/** Read the density data of a 32-bit MRC file */
void MRCReaderWriter::read_32_data(float *pt)
{
  int needswap;
  seek_to_data();
  size_t n = header.nx*header.ny*header.nz; // size of the grid
  read_grid(pt,sizeof(float), n);
  // Check for the necessity of changing the endian
  needswap = 0;
  for(size_t i=0;i<n;i++)
    if (pt[i] > 1e10) {
      // Really big values usually result if the endian is not correct
      needswap = 1;
      break;
    }
  /* Change endian if necessary */
  if (needswap == 1) {
    unsigned char *ch = (unsigned char *)pt;
    byte_swap(ch, n);
  }
  std::cout << "MRC file read in 32-bit mode: grid " << header.nx << "x"
            << header.ny << "x" << header.nz << std::endl;
}

void MRCReaderWriter::read_grid(void *pt,size_t size,size_t n)
{
  fs.read((char *)pt,size*n);
  size_t val = fs.gcount();
  if (val != size*n) { // If the values read are not the amount requested
    std::ostringstream msg;
    msg << "MRCReaderWriter::read_grid >> The values read "
                           "are not the amount requested \n";
    throw EMBED_IOException(msg.str().c_str());
  }
}

void MRCReaderWriter::seek_to_data(void)
{
  fs.seekg(sizeof(MRCHeader)+header.nsymbt, std::ios::beg);
  if(fs.fail()) {
    std::ostringstream msg;
     msg << "MRCReaderWriter::seek_to_data. Cannot find MRC data in file "
      << filename <<  "\n";
    throw EMBED_IOException(msg.str().c_str());
   }
}

void  MRCReaderWriter::read_header(void)
{
  // Read header
  fs.read((char *) &header,sizeof(MRCHeader));
  if(fs.gcount()!=sizeof(MRCHeader)) {
    std::ostringstream msg;
    msg << "MRCReaderWriter::read_header >> Error reading MRC header of file: "
     << filename <<  "\n";
   throw EMBED_IOException(msg.str().c_str());
  }

  // Check for endian
  unsigned char *ch = (unsigned char *) &header;
  if ((ch[0] == 0 && ch[1] == 0) + is_bigendian() == 1) {
    int machinestamp = header.machinestamp;
    byte_swap(ch, 56);
    header.machinestamp = machinestamp;
  }
  if (header.mapc != 1 || header.mapr != 2 || header.maps != 3) {
    std::ostringstream msg;
    msg << "MRCReaderWriter::read_header >> Error reading MRC header of file: "
     << filename <<  "\nNon-standard MRC file: column, row, section indices " 
     <<"are not (1,2,3) but (" << header.mapc << "," << header.mapr << 
     "," << header.maps << ")." << 
     " Resulting density data may be incorrectly oriented.\n";
    throw EMBED_IOException(msg.str().c_str());
  }
}

void MRCReaderWriter::write(const char *fn,const float *pt)
{
  std::ofstream s(fn, std::ofstream::out | std::ofstream::binary);
  if(!s.fail()) {
    write_header(s);
    write_data(s,pt);
  }
  s.close();
}

void MRCReaderWriter::write_header(std::ofstream &s)
{
  int wordsize=4; 
  s.write((char *) &header.nx,wordsize);
  s.write((char *) &header.ny,wordsize);
  s.write((char *) &header.nz,wordsize);
  s.write((char *) &header.mode,wordsize);
  s.write((char *) &header.nxstart,wordsize);
  s.write((char *) &header.nystart,wordsize);
  s.write((char *) &header.nzstart,wordsize);
  s.write((char *) &header.mx,wordsize);
  s.write((char *) &header.my,wordsize);
  s.write((char *) &header.mz,wordsize);
  s.write((char *) &header.xlen,wordsize);
  s.write((char *) &header.ylen,wordsize);
  s.write((char *) &header.zlen,wordsize);
  s.write((char *) &header.alpha,wordsize);
  s.write((char *) &header.beta,wordsize);
  s.write((char *) &header.gamma,wordsize);
  s.write((char *) &header.mapc,wordsize);
  s.write((char *) &header.mapr,wordsize);
  s.write((char *) &header.maps,wordsize);
  s.write((char *) &header.dmin,wordsize);
  s.write((char *) &header.dmax,wordsize);
  s.write((char *) &header.dmean,wordsize);
  s.write((char *) &header.ispg,wordsize);
  s.write((char *) &header.nsymbt,wordsize);
  s.write((char *) &header.user,wordsize*MRC_USER);
  s.write((char *) &header.xorigin,wordsize);
  s.write((char *) &header.yorigin,wordsize);
  s.write((char *) &header.zorigin,wordsize);
  s.write((char *) &header.map,wordsize);
  s.write((char *) &header.machinestamp,wordsize);
  s.write((char *) &header.rms,wordsize);
  s.write((char *) &header.nlabl,wordsize);
  s.write((char *) &header.labels,sizeof(char)*MRC_NUM_LABELS*MRC_LABEL_SIZE);
  if(s.bad()) {
    std::ostringstream msg;
    msg << "MRCReaderWriter::write_header >> Error writing MRC header\n";
    throw EMBED_IOException(msg.str().c_str());
  }
}

/* Writes the grid of values of an EM map to a MRC file */
void MRCReaderWriter::write_data(std::ofstream &s,const float *pt)
{

  s.write((char *)pt,sizeof(float)*header.nx * header.ny * header.nz);
  if(s.bad()){
    std::ostringstream msg;
    msg << "MRCReaderWriter::write_data >> Error writing MRC data.\n";
    throw EMBED_IOException(msg.str().c_str());
  }
  std::cout << "MRC file written: grid " << header.nx << "x" << header.ny
            << "x" << header.nz << std::endl;
}


/** Returns a CCP4 convention machine stamp: 0x11110000 for big endian, or
    0x44440000 for little endian */
int get_machine_stamp(void)
{
  int retval;
  unsigned char *ch;
  ch = (unsigned char *)&retval;
  ch[0] = ch[1] = (is_bigendian()? 0x11 : 0x44);
  ch[2] = ch[3] = 0;
  return retval;
}

int is_bigendian(void)
{
  static const int ival = 50;
  char *ch = (char *)&ival;
  return (ch[0] == 0 && ch[1] == 0);
}


/** Swaps the byte order in an array of 32-bit ints */
void byte_swap(unsigned char *ch, int n_array)
{
  int i;
  unsigned char tmp;

  for (i = n_array * 4 - 4; i >= 0; i -= 4) {
    tmp = ch[i];
    ch[i] = ch[i + 3];
    ch[i + 3] = tmp;
    tmp = ch[i + 1];
    ch[i + 1] = ch[i + 2];
    ch[i + 2] = tmp;
  }
}

void MRCHeader::FromDensityHeader(const DensityHeader &h)
{
  std::string empty;

  nz=h.nz;   ny=h.ny;  nx=h.nx; // map size
  // mode
  if(h.data_type==0) // data type not initialized
    mode = 2;
  if(h.data_type==1)
    mode=0; // 8-bits
  else if(h.data_type==2)
    mode=1;// 16-bits
  else if(h.data_type==5)
    mode=2;// 32-bits

  // number of first columns in map (default = 0)
  nxstart=h.nxstart ; nystart=h.nystart ; nzstart=h.nzstart;

  mx=h.mx ; my=h.my ; mz=h.mz; // Number of intervals along each dimension
  xlen=h.xlen ; ylen=h.ylen ; zlen=h.zlen; // Cell dimensions (angstroms)
  alpha=h.alpha ; beta=h.beta ; gamma=h.gamma; // Cell angles (degrees)
  /* Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
     (1,2,3 for x,y,z) */
  mapc=h.mapc;
  mapr=h.mapr;
  maps=h.maps; 
  /* Minimum, maximum and mean density value */
  dmin=h.dmin;
  dmax=h.dmax;
  dmean=h.dmean; 
  ispg=h.ispg; // Sapce group number 0 or 1 (default 0) 
  nsymbt=h.nsymbt; // Number of bytes used for symmetry data (0 or 80)

  // extra space used for anything - 0 by default
  for(int i=0;i<MRC_USER;i++)
    user[i]=h.user[i];
  strcpy(map,"MAP \0"); // character string 'MAP ' to identify file type
  // Origin used for transforms 
  xorigin=h.get_xorigin() ; yorigin=h.get_yorigin() ; zorigin=h.get_zorigin();
  // machine stamp (0x11110000 bigendian, 0x44440000 little)
  machinestamp=h.machinestamp;
  rms=h.rms; // RMS deviation of map from mean density
  nlabl=h.nlabl; // Number of labels being used
  // Copy comments
  for(int i=0;i<nlabl;i++)
    strcpy(labels[i],h.comments[i]);
  // Fill empty coments with null character
  const char *c="\0";
  empty.resize(MRC_LABEL_SIZE,*c);
  for(int i=nlabl;i<MRC_NUM_LABELS;i++)
    strcpy(labels[i],empty.c_str());
}

void MRCHeader::ToDensityHeader(DensityHeader &h)
{
  std::string empty;
  h.nz=nz; h.ny=ny; h.nx=nx; // map size
  // mode
  if(mode==0)
    h.data_type=1;
  else if(mode==1)
    h.data_type=2;
  else if(mode==2)
    h.data_type=5;
  // number of first columns in map (default = 0)
  h.nxstart=nxstart ; h.nystart=nystart ; h.nzstart=nzstart;
  h.mx=mx ; h.my=my ; h.mz=mz; // Number of intervals along each dimension
  h.xlen=xlen ; h.ylen=ylen ; h.zlen=zlen; // Cell dimensions (angstroms)
  h.alpha=alpha ; h.beta=beta ; h.gamma=gamma; // Cell angles (degrees)
  /* Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
     (1,2,3 for x,y,z) */
  h.mapc=mapc;
  h.mapr=mapr;
  h.maps=maps; 
  /* Minimum, maximum and mean density value */
  h.dmin=dmin;
  h.dmax=dmax;
  h.dmean=dmean; 
  h.ispg=ispg; // Sapce group number 0 or 1 (default 0) 
  h.nsymbt=nsymbt; // Number of bytes used for symmetry data (0 or 80)
  // extra space used for anything - 0 by default
  for(int i=0;i<MRC_USER;i++)
    h.user[i]=user[i];
  strcpy(h.map,"MAP \0"); // character string 'MAP ' to identify file type
  // Origin used for transforms 
  h.set_xorigin(xorigin) ; h.set_yorigin(yorigin) ; h.set_zorigin(zorigin);
  // machine stamp (0x11110000 bigendian, 0x44440000 little)
  h.machinestamp=machinestamp;
  h.rms=rms; // RMS deviation of map from mean density
  h.nlabl=nlabl; // Number of labels being used

  // Copy comments  
  for(int i=0;i<h.nlabl;i++)
    strcpy(h.comments[i],labels[i]);


  // Fill empty coments with null character
  const char *c="\0";
  empty.resize(MRC_LABEL_SIZE,*c);
  for(int i=h.nlabl;i<MRC_NUM_LABELS;i++) 
    strcpy(h.comments[i],empty.c_str());
}
