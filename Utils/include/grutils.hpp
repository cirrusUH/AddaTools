/*
  Collection of helpers
  
  inline size_t tolinear( size_t x, size_t y, size_t z, size_t sizeX, size_t sizeY, size_t sizeZ )
  inline size_t tolinear( size_t x, size_t y, size_t sizeX, size_t sizeY)
  size_t getFilesize( const char*filename )
  bool isHDF5( const char*filename )
  std::string getNowTimestring_iso8061()

  G Ritter, London 2016
 */
#ifndef __GR_GRUTILS_HPP
#define __GR_GRUTILS_HPP

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <string.h>

using std::cerr;
using std::endl;

bool isHDF5( const char*filename )
{

// https://support.hdfgroup.org/HDF5/doc/H5.format.html
const unsigned char HDF5signature[] = {0x89,0x48,0x44,0x46,0x0d,0x0a,0x1a,0x0a};

int fd = open(filename, O_RDONLY); 
	if (fd == -1)
	{
		cerr << "Error opening " << filename << endl; 
		return EXIT_FAILURE;  
	} 

char buf[8]; 
int n = read( fd, buf, 8);
 
	if( n!=8 )
	{
		cerr << "Error reading " << filename << endl; 
		return EXIT_FAILURE;  	
	}

	close(fd); 

	if( strncmp( buf, (const char*)HDF5signature, 8) !=0 )
	{
		return false; 
	} 

	return true;
}

size_t getFilesize( const char*filename )
{
	// file size 
int fd = open(filename, O_RDONLY); 
	if (fd == -1)
	{
		cerr << "Error opening " << filename << endl; 
		return EXIT_FAILURE;  
	} 

struct stat sb; 
	if (fstat(fd, &sb) == -1) 
	{        
		cerr << "Error on fstat " << filename << endl; 
		return EXIT_FAILURE;  
	} 

	if( close(fd) !=0 )
	{
		cerr << "Error closing " << filename << endl; 
		return EXIT_FAILURE;  
	}

size_t filesize = sb.st_size; 
	return filesize;
}

std::string getNowTimestring_iso8061()
{
	// ISO 8061 timestring
	/*
	  std::time_t t = std::time(nullptr); 
	  std::tm tm = *std::localtime(&t); 
	  
	  std::stringstream oss; 
	  oss << std::put_time(&tm, "%FT%T"); 
	*/

	// swap in this old version if your compiler 
	//  does not support it, gcc > 5.x does
	
time_t now; 
	time(&now); 

char buf[sizeof "2011-10-08T07:07:09Z"]; 
	strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));

	return std::string(buf);
}

inline size_t tolinearC3D( size_t i, size_t j,  size_t k, size_t nx, size_t ny, size_t nz)
{
	// ok and tested
	return (k + j*nz + i*ny*nz);
}


inline size_t  tolinearC2D( size_t i, size_t j,  size_t nx,  size_t ny )
{
	// ok and tested
	return  (j + i*ny);
}

#endif // __GR_GRUTILS_HPP
