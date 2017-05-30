/**
  Converts an ADDA scatter description file (geom) into the netCDF representation 

  assumes: c++11
  
  http://www.nrel.colostate.edu/projects/irc/public/Documents/Software/netCDF/cpp4/html/

  G Ritter, London, Sept 2016
 */

#include <omp.h>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <ios>
#include <list>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>

using std::cout;
using std::endl;
using std::cerr;

#include <grutils.hpp> 

#include <netcdf>
using namespace netCDF;
using namespace netCDF::exceptions;


// Return this in event of a problem.
static const int NC_ERR = 2;

int main( int argc, char*argv[] )
{
	//std::ios_base::sync_with_stdio(false); 

	if( argc < 2)
	{
		cerr << "Usage: " << argv[0] << " filename " << endl; 
		cerr << "       reads filename and (over)writes filname.nc" << endl;
		return 1;
	} 

std::ifstream infile( argv[1] ); 

	if( !infile.is_open() )
	{
		cerr << "Could not open " << argv[1] << endl; 
		return 1;
	} 
	std::string ifilename, ofilename; 
		ifilename = ofilename = argv[1];

std::size_t dotpos = ofilename.find_last_of('.');
	if( dotpos != std::string::npos)
	{
		ofilename = ofilename.substr(0, dotpos);
	} 

	ofilename = ofilename + ".nc";

std::chrono::time_point<std::chrono::system_clock> start, end; 
std::chrono::duration<double> elapsed_seconds;
 
	start = std::chrono::system_clock::now(); 

std::size_t xMax, yMax, zMax, NmatMax; 
	xMax = yMax = zMax = NmatMax = std::numeric_limits<std::size_t>::min(); 

	// find min max, check file header, otherwise search through file 
std::string versionstring;

std::string line; 
long long int headerlinecounter=1;  // first line is line 1 

int fd = open(argv[1], O_RDONLY);
           if (fd == -1)
	   {
               handle_error("open"); 
	   } 

struct stat sb; 
	if (fstat(fd, &sb) == -1) 
	{         
               handle_error("fstat"); 
	} 

size_t filesize = sb.st_size; 

	cout << "file size: " << filesize <<" or " << filesize /1024/1024 <<"MB"<< endl;

off_t pa_offset=0; 
char*addr =  (char*)  mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, pa_offset) ; 
	if (addr == MAP_FAILED)
	{
		handle_error("mmap");
	} 
	// cout << "file mapped" << endl; 
	
const char* startaddr = addr; 

bool insideheader;

	if( *addr == '#' )
	{
		insideheader=true;	
	} 
	else
	{
		cerr << "error reading line 1" << endl; 
		return EXIT_FAILURE;
	}
	
	const char*p = addr; 
	while( insideheader )
	{
		//cout << "'" <<*p1 << "'" << endl;
		if(*p == '#')
		{
		const int LINEBUFSIZE = 4096;
		char buf[LINEBUFSIZE]; 
	        int c=0; 
			while( c<LINEBUFSIZE && *p !='\n')
			{
				buf[c]=*p; 
				c++; 
				p++; 

				if( c==LINEBUFSIZE )
				{
					cerr << "line too long, we ran out of buffer" << endl; 
					return EXIT_FAILURE;
				}
			} 
			buf[c]=0; 
			p++; // skip \n 

			cout <<"header: " << "'"<< buf  << "'"<< endl; 
			headerlinecounter++;

			std::string line(buf);


		} 
		else
		{
			insideheader = false;
		}
	} 
	const char*ppostheader = p;

	cout << "done with header" << endl; 
	cout << "after header char is: " << *ppostheader << endl;

 	start = std::chrono::system_clock::now(); 

// omp_set_num_threads(std::thread::hardware_concurrency());

std::vector<size_t> parlines; 
std::vector<const char*> segmentstarts; 

	parlines.resize( omp_get_max_threads()+1 ); 
	segmentstarts.resize( omp_get_max_threads()+1 ); 

omp_set_num_threads( omp_get_max_threads() / 2 );
#pragma omp parallel 
{
size_t N = omp_get_num_threads(); 
int id = omp_get_thread_num(); 

	if( id==0 )
	{
		std::cout << "Threads: " << N << endl; 

		// partition problem 
		// find the nearest line starts 
	int headersize = int(ppostheader-startaddr); 
		cout << "headersize " << headersize << endl;cout.flush();

		segmentstarts.at(0) = ppostheader; // with header skipped 

		for( size_t i=1; i<N; i++)
		{
		
		size_t dipolesonlysize = filesize - headersize; 
			//	cout << "dipolesonlysize=" << dipolesonlysize <<endl;

		const char *pp = ppostheader + ( i * dipolesonlysize )/N; 

			// skip to the start of next line 
		while( pp < ppostheader + ( (i+1) * dipolesonlysize )/N)
			{
				if( *pp == '\n' )
				{
					segmentstarts.at(i)= pp; 
					break; 
				} 
				pp++;
			} 
		} 
		segmentstarts.at(N) = startaddr + filesize; 

		//cout << "file distance " << segmentstarts.at(N) - 	segmentstarts.at(0) << endl;

		//cout << "startaddr [" << id << "] " << (void*)startaddr << endl;cout.flush();

		for(const char* v : segmentstarts)
		{
			if( v !=0 )
			{
				//cout << "segments [" << id << "] " << (void*)v << ", " << int(v-segmentstarts.at(0)) <<" has ' " << std::hex << (*v) <<"'"<< std::dec << endl;cout.flush();
			}
		} 
	} 

	#pragma omp barrier // vector needs to be initialized

	const char*p = segmentstarts.at(id); 
	size_t  linecounter=0; 

		// cout << "["<<id<<"] *p="<< *p << endl; cout.flush(); 

	long long int occupieddipoles=0;
	unsigned long ul[3]; 
	int pos=0; 

	while( p < segmentstarts.at(id+1)-1 )
	{
	char*endptr;
		ul[pos] = strtoul (p, &endptr, 0); 
		if (errno == ERANGE) // || p == endptr) 
		{
			cerr << "range error in line " << linecounter; 
			throw EXIT_FAILURE; // gcc will wrap the parallel section with a try catch.
 		} 
		p = endptr; 
		pos++; 

		if(pos==3)
		{
			//cout << "["<<id<<"] " << ul[0] << " "<< ul[1] << " "<< ul[2] << endl;  cout.flush();
			pos=0; 

			occupieddipoles++; 
			linecounter++; 
		} 
	} 

	parlines.at( id ) = occupieddipoles; 
} 


long long int sumoccdipoles=0;
for(auto v : parlines)
{
	if(v!=0)
	{
		cout << "occupied dipoles found: " << v << endl; 
	}
	sumoccdipoles += v;
} 

cout << "total dipoles "<< sumoccdipoles << endl;

#if 0

{


		// update min/max search 

		if( x > xMax ) xMax = x; 
		if( y > yMax ) yMax = y; 
		if( z > zMax ) zMax = z; 
		if( mat > NmatMax ) NmatMax = mat; 

		if( !(linecounter%10000)) 
		{
			cout << "line " << linecounter << "\r"; 
		}
 
		linecounter++;
	} 

	cout << "Found maximums in number of elements: " << xMax << ", "<< yMax << ", "<< zMax << endl; 
	cout << "Materials found: " << NmatMax << endl; 

	if( NmatMax > 15 )
	{
		cout << "Warning, number of materials found " << NmatMax << " exceeds the default allowed maximum (15) of materials"
		     << "setting in Adda. See section 6.3 of the Adda manual"
		     << "you might need to recompile Adda "<<endl;
	}

		end = std::chrono::system_clock::now(); 
	std::chrono::duration<double> elapsed_seconds1 = end-start; 

		cout << "elapsed time: " << elapsed_seconds1.count() << "s\n"; 

	std::size_t Nx = xMax; // indices in the adda file start at 0 - so 0,0,0 is the corner voxel
        std::size_t Ny = yMax; 
	std::size_t Nz = zMax; 

	size_t mem =  (Nx * Ny * Nz ); 
		cout << "now reserve some memory ("<< double(mem)/(1024*1024*1024) <<" GB) and read the data file" << endl; 

	unsigned char*vol; 
		try {	
			vol = new unsigned char [ mem ]; 
		} catch(...) {
			cerr << "Out of memory, we would have needed: " << mem/1024 <<"MBs"<< endl; 
			return EXIT_FALIURE;
		} 
		
		std::fill(vol, vol + mem, 0); 

		cout << "second pass, construct volume"<<endl; 
		
		// go through the file again and populate the volume 
			infile.clear(); 
			infile.seekg(0, std::ios::beg); 

		while ( infile.good() )
		{
			getline(infile, line) ; 

		std::istringstream iss(line); 
		std::size_t x, y, z; 
		std::string s; 
			
		if( line.size() == 0  || line.at(0) == '#')
		{
			cout << "skipped: " << line << endl;
			continue;
		} 

		if( NmatMax > 1 )
		{
			if( line.at(0) == 'N')
			{
				cout << "skipped: " << line << endl; 
				continue;
			}	
		} 
		

		if( (iss >> x >> y >> z) ) 
		{
		       linecounter++; 
		     
		       if( !(linecounter%10000)) 
		       {
			       cout << "line " << linecounter << "\r"; 
		       } 
		       //cout << x<< " "<< y<< " "<<z<< " "<< endl; 

		       if( (foundBoxsizeinheader == true) && ( x > Nx || y > Ny || z > Nz) )
		       {
			       cout << "Warning: Index larger than box size found in header" << endl; 
				       
		       }

		       vol[ tolinear(x, y, z, Nx, Ny, Nz) ] = 1;
		} 
		else
		{
			cerr << "Error3 reading line " << linecounter << ", '" <<line<< "'"<<endl;
		} 
	} 

		/*
		  Dump netcdf here
		*/ 



    try
    {  
    NcFile dataFile( ofilename.c_str(), NcFile::replace); 

	    if( versionstring.length() == 0)
	    {
		    versionstring = "v.1.3b6";
	    } 

	    dataFile.putAtt("version",  versionstring ); 

	    // ISO 8061 timestring
/*
	    std::time_t t = std::time(nullptr); 
	    std::tm tm = *std::localtime(&t); 

	    std::stringstream oss; 
	    	oss << std::put_time(&tm, "%FT%T"); 
		dataFile.putAtt("history", oss.str() + " file created from " + ifilename ); 
*/ 
	    // swap in this old version if your compiler 
	    // does not support it, gcc > 5.x does 
	    
		   time_t now; 
			   time(&now); 
		   char buf[sizeof "2011-10-08T07:07:09Z"]; 
		   strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now)); 
		   dataFile.putAtt("history",  std::string(buf) + " file created from " + ifilename ); 

	    
		  

      // Create netCDF dimensions
      NcDim xDim = dataFile.addDim("x", Nx);
      NcDim yDim = dataFile.addDim("y", Ny); 
      NcDim zDim = dataFile.addDim("z", Nz); 

      std::vector<NcDim> dims; 
      	dims.push_back(xDim); 
	dims.push_back(yDim); 
	dims.push_back(zDim);
      NcVar data = dataFile.addVar("geom", ncByte, dims); 
	      data.setCompression( false, true, 9 ); 

	      data.putAtt("Nmat", NcType::nc_INT, int(NmatMax) ); 
	      data.putAtt("N_occupied_voxels", NcType::nc_INT, int(Noccvoxels) ); 

	      data.putVar(vol); 

	      cout << "Successfully written file: " << ofilename << endl;
      return 0; 
    }
    catch(NcException& e)
    {e.what();
      return NC_ERR;
} 

#endif 


	elapsed_seconds = std::chrono::system_clock::now()-start; 

	cout << "Done. written " << ofilename << endl 
	     << "Elapsed time: " << elapsed_seconds.count() << "s  "       
	     << (double(filesize)/1024/1024)/elapsed_seconds.count() << "MB/s" << endl; 
	
	return 0;
}
