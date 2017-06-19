## Note
The standalone spb programs in the `bin` directory (described in the main README file) employ helper functions to facilitate the use of 
various IMP classes for representation, scoring, and sampling. These helper functions are in the `src` directory:
 
- `spb_parser.cpp`: this file contains helper functions to parse all the input files needed by the standalone C++ programs;
- `spb_representation.cpp`: this file contains all the functions needed to represent the components of the SPB core at different resolution;
- `spb_ISD_particles.cpp`: this file contains the definition of all the variables used in our Bayesian approach to quantify the level of noise in the data and unknown structural parameters, such as the dimension of the SPB unit cell and the thickness of the CP layer; 
- `spb_assemble_restraints.cpp`: this file contains the list of all restraints used in the SPB modeling;
- `spb_restraint.cpp`: this file includes the definition of all the restraints used in the SPB modeling;
- `spb_sampler.cpp`: this file contains the definition of the MC movers for all the particles used to represent the SPB components as well as the ISD particles defined above and the parameters of the MC sampler.
