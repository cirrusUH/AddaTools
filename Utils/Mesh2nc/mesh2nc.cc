/**
   Packs and unpacks any file into a netcdf variable and back, 
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
		cerr << "       reads filename and (over)writes a filname.nc in netCDF format." << endl; 
		return EXIT_FAILURE;
	} 

	std::ifstream infile; 
	infile.open( argv[1], std::ios::in | std::ios::binary);

	if( !infile.is_open() )
	{
		cerr << "Error opening " << argv[1] << endl; 
		return EXIT_FAILURE;
	} 

std::string ifilename, ofilename; 
		ifilename = ofilename = argv[1]; 

std::size_t dotpos = ofilename.find_last_of('.');
	if( dotpos != std::string::npos)
	{
		ofilename = ofilename.substr(0, dotpos);
	} 
	ofilename = ofilename + ".nc";

	// read file
	std::vector<int> buffer((std::istreambuf_iterator<char>(infile)), 
				 (std::istreambuf_iterator<char>())); 

	size_t nBytes = buffer.size(); 


	// package into netcdf var
	try
	{  
	NcFile dataFile( ofilename.c_str(), NcFile::replace ); 

		dataFile.putAtt("history",  getNowTimestring_iso8061() + " file created from " + ifilename ); 



		// Create netCDF dimensions 
	NcDim nbytes_Dim = dataFile.addDim("nBytes", nBytes); 
		// NcDim nbytes_Dim = dataFile.addDim("nBytes"); 

	std::vector<NcDim> dims; 
		dims.push_back(nbytes_Dim); 
		// dims.push_back(material_domain_Dim); 

	NcVar meshdata = dataFile.addVar("meshdata", ncByte, dims); 
		meshdata.setCompression( false, true, 9 );

	std::vector<size_t> startp = {0}; 
	std::vector<size_t> countp = {nBytes}; 
		meshdata.putVar( startp, countp, buffer.data() ); 

		meshdata.putAtt("units", "bytes"); 
		meshdata.putAtt("filename", ifilename); 
		meshdata.putAtt("comment", "Original mesh data file, binary 1:1 wrapped into here.");

	}
	catch(NcException& e)
	{
		e.what(); 
		return NC_ERR;
	} 


	cout << "Successfully written " << ofilename << endl;
	return EXIT_SUCCESS ;
}

