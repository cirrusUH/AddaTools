/*
  allows to read in 3vectors from the commandline using tclap.
  specify with either quoting:
  -v "1.1 2.3 1.2" or use commas 1.1,2.2,3.3

  This is a modified version of example test14.cpp
  from the tclap distribution.
  
  G Ritter, London Sept 2016
 */

#ifndef __GR_TCLAP3DVECTOR_H
#define __GR_TCLAP3DVECTOR_H

#include <iterator>
#include <algorithm>

// Define a simple 3D vector type
template<typename T, size_t LEN>
struct Vect : public TCLAP::StringLikeTrait
{
	//typedef TCLAP::StringLike ValueCategory;
T v[LEN]; 

	std::string trim(std::string& str)
	{
        size_t first = str.find_first_not_of(' '); 
	size_t last = str.find_last_not_of(' '); 
		return str.substr(first, (last-first+1));
	} 


	// operator= will be used to assign to the vector 
	Vect& operator=(std::string str)
	{
		// process the string first, allow [v0 v1 v2], [v0,v1,v2],"v0,v1,v2"
		std::replace(str.begin(),str.end(), '[', ' '); 
		std::replace(str.begin(),str.end(), ']', ' '); 
		std::replace(str.begin(),str.end(), '(', ' '); 
		std::replace(str.begin(),str.end(), ')', ' '); 
		std::replace(str.begin(),str.end(), '{', ' '); 
		std::replace(str.begin(),str.end(), '}', ' '); 
		std::replace(str.begin(),str.end(), ',', ' '); 
		str = trim(str);
		
            std::istringstream iss(str);
            for (size_t n = 0; n < LEN; n++) {
                if (!(iss >> v[n])) {
                        std::ostringstream oss;
                        oss <<" is not a vector of size " << LEN << " maybe there is a space inbetween numbers and you need quotes \"1.1 2.2 3.3\", or alternatively use commas 1.1,2.2,3.3 and no spaces.";
                    throw TCLAP::ArgParseException(str + oss.str());
                }
            }

            if (!iss.eof()) {
                std::ostringstream oss;
                oss << " is not a vector of size " << LEN;
                throw TCLAP::ArgParseException(str + oss.str());
            }

            return *this;
        }

    std::ostream& print(std::ostream &os) const
{
            std::copy(v, v + LEN, std::ostream_iterator<T>(os, " "));
            return os;
        }

};
#endif
