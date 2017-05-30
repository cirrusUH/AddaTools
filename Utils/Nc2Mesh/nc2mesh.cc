/**
   Unpacks any file into a netcdf variable and back, 
   adhering CF conventions. Meant to allow inclusion of geometry mesh 
   data files into CF conform descriptions.

   Required attributes (institution, etc.) expected to be added 
   externally using ncks, see workflow documentation.

   assumes: c++11, netcdf-cxx4-4.3.0
   G Ritter, London, Sept 2016
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <string>
#include <cerrno>

using std::cout;
using std::endl;
using std::cerr;

#include <grutils.hpp>

#include <netcdf>
#include <netcdf.h>
using namespace netCDF;
using namespace netCDF::exceptions;

// Return this in event of a problem.
static const int NC_ERR = 2;

#include <fstream>
#include <streambuf>
#include <string>
#include <cerrno>


int main( int argc, char*argv[] )
{
	if(argc<2)
	{
		cerr << "Usage: " << argv[0] << " filename " << endl; 
		cerr << "       unpacks the variable 'meshdata' in filename into the standalone file." << endl; 
		return EXIT_FAILURE;
	} 

	std::ifstream infile; 
	infile.open( argv[1], std::ios::in | std::ios::binary);

	if( !infile.is_open() )
	{
		cerr << "Error opening " << argv[1] << endl; 
		return EXIT_FAILURE;
	} 

	if( !isHDF5(argv[1]) )
	{
		cerr  << argv[1] << "is not a netcdf file."<< endl; 
		return EXIT_FAILURE;	
	} 


	std::string ifilename,ofilename; 
		ifilename = argv[1]; 

	try
	{  
	NcFile ncFile( ifilename.c_str(), NcFile::read); 

	NcDim nBytes_dim = ncFile.getDim("nBytes"); 
	size_t nBytes = nBytes_dim.getSize(); 

	char*buffer; 
		try 
		{
			buffer  = new char [nBytes];
		} 
		catch(std::bad_alloc& ba)
		{
			cerr << "Out of memory, we would have needed ~" << (nBytes)/1024 <<"KBs"<< endl; 
			return EXIT_FAILURE;
		} 

	NcVar meshdata = ncFile.getVar("meshdata"); 

		// meshdata.getVar(buffer); 
		// the interface getVar throws an error when tyring to read back the data
		nc_get_var( meshdata.getParentGroup().getId(), meshdata.getId(), buffer); 

		meshdata.getAtt("filename").getValues(ofilename); 

       std::ofstream ofile( ofilename, std::ios::binary); 
       		if( !ofile.is_open() )
		{
			cerr << "Error creating " << ofilename << endl; 
			return EXIT_FAILURE;
		} 
 
       		ofile.write( buffer, nBytes); 

		ofile.close();

	}
	catch(NcException& e)
	{
		cerr << e.what() <<endl; 
		return NC_ERR;
	} 


	cout << "Successfully written " << ofilename << endl;
	return EXIT_SUCCESS ;
}

