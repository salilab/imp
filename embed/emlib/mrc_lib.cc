
#include "mrc_lib.h"

// Reads an MRC file
int mrc::read(float **pt)
{
	fs.open(filename, fstream::in | fstream::binary);
	if(!fs.fail())
	{
		// Read header
		if(read_header()==1)
		{
			cout << "mrc::read >> Error reading MRC header of file " << filename << endl;
			return 1;
		}
		// Allocate memory
		size_t n = header.nx*header.ny*header.nz;
		(*pt)= new float [n]; 
		// read 
		if(read_data(*pt)==1)
		{
		    cout << "mrc::read >> Error reading MRC data of file " << filename << endl;
		    return 1;
		}
	}
	fs.close();
	return 0;
}


// Reads the grid of data from a MRC file
int mrc::read_data(float *pt)
{
	if(header.mode==0)
		return read_8_data(pt);
	else if(header.mode==2)
		return read_32_data(pt);
	else
	    cout << "mrc::read_data >> This routine can only read 8-bit or 32-bit MRC files. Unknown mode for " << filename << endl;
    return 1;
}




/** Read the density data of a 8-bit MRC file */
int mrc::read_8_data(float *pt)
{
	int ierr;
	if (seek_to_data()!= 0)
		return 1;

	size_t n = header.nx*header.ny*header.nz;
	unsigned char *grid_8bit= new unsigned char [n]; // memory
	ierr=read_grid(grid_8bit,sizeof(unsigned char), n);
	if(ierr==1)
		return ierr; // There is an error
	// Transfer	to floats
	for(size_t i=0;i<n;i++)
		pt[i]=(float)grid_8bit[i];
	delete(grid_8bit);
	cout << "MRC file read in 8-bit mode: grid " << header.nx << "x" << header.ny << "x" << header.nz << endl;
	return ierr;
}


/** Read the density data of a 32-bit MRC file */
int mrc::read_32_data(float *pt)
{
	int needswap,ierr;
	if (seek_to_data() != 0)
		return 1;

	size_t n=header.nx*header.ny*header.nz; // size of the grid
	ierr=read_grid(pt,sizeof(float), n);
	if(ierr==1)
		return ierr; // There is an error
	// Check for the necessity of changing the endian
	needswap = 0;
	for(size_t i=0;i<n;i++)
   	    if (pt[i] > 1e10)
		{
			needswap = 1; // Really big values usually result if the endian is not correct
			break;
		}
	/* Change endian if necessary */
	if (needswap==1)
	{
	    unsigned char *ch = (unsigned char *)pt;
    	byte_swap(ch, n);
	}
	cout << "MRC file read in 32-bit mode: grid " << header.nx << "x" << header.ny << "x" << header.nz << endl;
	return ierr;
}

/* Read the actual MRC grid data from a file 
	pt - pointer to store the grid
	size - size of the data to read
	n - size of the grid (nx * ny * nz)
*/
int mrc::read_grid(void *pt,size_t size,size_t n)
{
	fs.read((char *)pt,size*n);
	size_t val = fs.gcount();
	if (val != size*n) // If the values read are not the amount requested
		return 1; // There is an error
	else
	    return 0; // Good
}


/** Position the file pointer at the start of the MRC data section */
int mrc::seek_to_data(void)
{
	int ierr=0;
	fs.seekg(sizeof(mrc_header)+header.nsymbt,ios::beg );
	if(fs.fail())
	{
		cout << "mrc::seek_to_data. Cannot find MRC data in file " << filename << endl;
		ierr=1;
	}

    return ierr;
}


// Read header of a MRC file and stores it into the proper class */ 
int mrc::read_header(void)
{
	// Read header
	fs.read((char *) &header,sizeof(mrc_header));
	if(fs.gcount()!=sizeof(mrc_header))
		return 1;
	// Check for endian
	unsigned char *ch = (unsigned char *) &header;
	if ((ch[0] == 0 && ch[1] == 0) + is_bigendian() == 1)
	{
	    int machinestamp = header.machinestamp;
	    byte_swap(ch, 56);
	    header.machinestamp = machinestamp;
  	}
	if (header.mapc != 1 || header.mapr != 2 || header.maps != 3)
	{
	    cout << "mrc::read_header. Non-standard MRC file: column, row, section indices"
                " are not (1,2,3) but are (" << header.mapc << "," << header.mapr << "," << header.maps << ")."
				" Resulting density data may be incorrectly oriented." << endl;
  	}
  return 0;
}



// Write a MRC file
int mrc::write(char *fn,float *pt)
{
	fstream f_out;
	f_out.open(fn,fstream::out | fstream::binary);
	if(!f_out.fail())
	{
		// Write header
		if(write_header(&f_out)==1)
		{
			cout << "mrc::write. Error writing MRC header to file " << fn << endl;
			return 1;
		}
		// Write values
		if(write_data(&f_out,pt)==1)
		{
			cout << "mrc::write. Error writing MRC data to file " << fn << endl;
		    return 1;
		}
	}
	f_out.close();
	return 0;
}




/* Writes the MRC header to a file */
int mrc::write_header(fstream *f_out)
{
	header.alpha = header.beta = header.gamma = 90.0;
	header.mapc = 1; header.mapr = 2; header.maps = 3;
	memcpy(header.map, "MAP ", 4);
	header.machinestamp = get_machine_stamp();
	header.nlabl = 1;
	header.mode = 2;

	f_out->write((char *) &header,sizeof(mrc_header));
	if(f_out->bad())
	{
		cout << "mrc::write_header. Error writing MRC header to file" << endl;
		return 1;
	}

	return 0;
}

/* Writes the grid of values of an EM map to a MRC file */
int mrc::write_data(fstream *f_out,float *pt)
{
	size_t n = header.nx * header.ny * header.nz;
	f_out->write((char *)pt,sizeof(float)*n);
	if(f_out->bad())
	{
		cout << "mrc::write_header. Error writing MRC data to file" << endl;
		return 1;
	}
	cout << "MRC file written: grid " << header.nx << "x" << header.ny << "x" << header.nz << endl;
	return 0;
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



/** Returns true if this machine is big endian */
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

