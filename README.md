# AddaTools, Scripts and Examples
These contain various useful utilities and tools and solutions to tasks commonlyfrequently encountered when employing Adda. File format converters, plotting scripts, script to generate or maniputate Adda input and output.

# Utils

## addascatterer2netcdf and netcdf2addascatterer
Standalone converters to and from the netCDF format for the scatterer geometries. You also can use the adda binary itself. Files need to have attributes added in a post processing step. 


## nc2mesh and mesh2nc 

The two commands allow to pack and unpack an external triangulated mesh files used to derive a scatterer geometry file  (using for example  `misc/pip/` from the Adda distribution) as a netCDF variable. The file is stored as a 1:1 binary copy of the file, so this approach is not limited to mesh data. This is useful for appending the triangulated mesh data to the scatterer definition in order to keep the original with the scatterer representation. This allows later re-griding to a different dipoles/lambda grid size, use in other numerical methods requiring triangulated description of particles, as well as consistent distribution of files. 

Code uses the netCDF C++ interface (netcdf-cxx4): https://github.com/Unidata/netCDF-cxx4

 
```
usage: mesh2nc filename 
   Output: filename.nc
```
Converts `filename` into a netCDF variable `meshdata` and writes output to `filename.nc`.

```
usage: nc2mesh filename 
```
Converts the content of the netcdf variable `meshdata` into a file with the filename given in the nc attribute `filename`. 

To append the mesh data to an already existing scatterer geometry use:

```
ncks -A spheremesh.nc sphere.nc 
```

from the netCDF Operators (NCO: http://nco.sourceforge.net, `apt-get install nco`).


