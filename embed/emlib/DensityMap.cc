#include "DensityMap.h"


int DensityMap::ReadXplor(istream & XPLORstream,int _indOrder)
{

  //  indOrder = Parameters::getInt("XYZ_ORDER");
  indOrder = _indOrder;
  ReadHeader(XPLORstream);

  AllocateData(extent);

  ReadMap(XPLORstream);
  return 0;
}


int DensityMap::ReadHeader(istream & XPLORstream)
{
 
  char line[300];
  int lineWidht = 300;

  // read the configuration values:
  for (int i=0;i<4;i++)
    {
      XPLORstream.getline(line,lineWidht);
    }
  char* lineBreaker;
  
  XPLORstream.getline(line,lineWidht);

  int div;
  int mod;
  int result;
  lineBreaker = strtok(line," ");
  for (int j=0;j<9;j++)
    {
      result = atoi(lineBreaker);
      div = (int)(floor(j/3.0));
      mod = j - 3 * div;
      if (mod == 0)
	grid[div] = result;
      else if (mod == 1)
	{
	  orig[div]=result;
	}
      else // mod == 2
	top[div]=result;
      lineBreaker = strtok (NULL," ");
    }
  for(int j=0;j<3;j++)
    {
      extent[j] = top[j]-orig[j]+1;
    }

  ////////////////////
  // read the unit cell dimensions
  ////////////////////
  //   cout << " a3 " << endl;
  XPLORstream.getline(line,lineWidht);

  lineBreaker = strtok(line," ");

  float val;
  for (int j=0;j<6;j++)
    {
      val = atof(lineBreaker);
      div = (int)(floor(j/3.0));
      mod = j - 3 * div;
      if (div == 0)
	{
	  cellSize[mod] = val;
	}
      else // div == 1
	{
	  cellAngle[mod] = val;
	}
   
      lineBreaker = strtok (NULL," ");
    }
  /////////////
  // calculate the spacing
  ////////////

  for(int j=0;j<3;j++)
    spacing[j] = cellSize[j]/grid[j];

  translateGrid.updateX(orig[0]*spacing[0]);
  translateGrid.updateY(orig[1]*spacing[1]);
  translateGrid.updateZ(orig[2]*spacing[2]);
  //////////////
  // read the grid order
  //  we do not use this data. We use the user input for the grid order.
  //////////////
  XPLORstream.getline(line,lineWidht);
    
  return 0;
}


int  DensityMap::ReadMap(istream &XPLORstream)
{
  //reading the map according to the grid order.
  char line[300];
  int lineWidht = 300;
  int x,y,z =0; //the step size in the three directions on the grid.
  x=0;y=0;z=0;
  bool status = false;

  // update orig and top according to the values in the map
  for(int i=0;i<3;i++)
    {
      orig[i] = 9999;
      top[i]  =-9999;
    }

  while (!XPLORstream.eof())
    {
      XPLORstream.getline(line,lineWidht);
      //  cout << x << "   " << y << "    " << z << endl << line << endl;
      if (!status) // status = false means that we moved a section
       	{
       	  status = true;
	  //  cout <<"REMARK  " <<  line << endl;  	  
      	}
      else
	{
	  // each line have strlen/12 density values.
	  // We iterater through the line
	    
	  int counter = 0;
	  bool keep = true;
	  char dens[12];
   
	  int densNum = strlen(line)/12;
	  float density;
	  while ((counter <densNum) && keep)
	    {
	      strncpy(dens,line+(counter*12),12);
	      counter ++;
	      density = atof(dens);   
	      data[x][y][z] = density;

	      //	      if (density>0.0)
	      //{
		  if (x<orig[0])
		    orig[0]=x;
		  if (y<orig[1])
		    orig[1]=y;
		  if (z<orig[2])
		    orig[2]=z;
		  if (x>top[0])
		    top[0]=x;
		  if (y>top[1])
		    top[1]=y;
		  if (z>top[2])
		    top[2]=z;
		  //}

	      if (indOrder ==  XYZ_ORDER)
		{
		  z++;
		  if (z >= extent[2] )
		    {
		      z=0;
		      y++;
		      if ( y>=extent[1])
			keep = false;
		    }
		}
	      else if (indOrder == ZYX_ORDER)
		{

		  x++;
		  if (x >= extent[0] )
		    {
		      x=0;
		      y++;
		      if ( y>= extent[1])
			keep = false;
		      }
		  }
		else if (indOrder == YXZ_ORDER) // The grid order is  YXZ 
		  {
		    z++;
		    if (z >= extent[2] )
		      {
			z=0;
			x++;
			if ( x>= extent[0])
			  {
			    keep = false;
			  }
		      }
		  } // indOrder=2
		else if (indOrder == ZXY_ORDER) // z in slow y fast
		  {
		    y++;
		    if (y>=extent[1])
		      {
			y =0;
			x++;
			if ( x>= extent[0])
			  {
			    keep = false;
			  }	
		      }
		  }
	      } // while counter < densNum
	    if (indOrder == XYZ_ORDER ) // The grid order is XYZ
	      {
		if (y >= extent[1])
		  {
		    z=0;
		    y=0;
		    x++;
		  
		    status=false;
		  }
	      } // end of indOrder == 0
	    else if (indOrder == ZYX_ORDER ) // The grid order is ZYX
	      {
	  
		if (y >= extent[1])
		  {
		    x=0;
		    y=0;
		    z++;

		    status=false;
		  }
	      } // end of indOrder == 1
	    else if (indOrder ==YXZ_ORDER ) // The grid order is YXZ
	      {
		if (x >= extent[0])
		  {
		    //cout << " now y is  : " << y << " ---- " << x << endl;
		    z=0;
		    x=0;
		    y++;
		    status=false;
		      }
	      } // end of indOrder ==2
	    else if (indOrder ==ZXY_ORDER)
	      {
		if (x >= extent[0])
		  {
		    //	cout << " now y is  : " << y << " ---- " << " x is " << x << "and z is " << z << endl;
		    y=0;
		    x=0;
		    z++;
		    status=false;
		      }
	      } // end of indOrder ==3
	  } //else
    }
  return 0;
}





