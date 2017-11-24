# imcdf-lib-c
C library code to read and write INTERMAGNET CDF (IMCDF) data files

imag_cdf_test.c is a piece of test code that creates a CDF data file containing synthetic data. It also shows how to use the library code. 

Brief documentation on using the code is in the header of imcdf.c

This code depends on NASA's CDF library: http://cdf.gsfc.nasa.gov/html/sw_and_docs.html

Simon Flower
6th Mar 2015


This code was last updated for ImagCDF version 1.1, but will also work correctly with ImagCDF version 1.2 data files. The main changes between the two versions is to do with naming of the variables used for time stamps in the data file and use of the variable attribute DEPEND_0 - the C code for ImagCDF version 1.1 is already compatible with these changes.

Simon Flower
24th Nov 2017
