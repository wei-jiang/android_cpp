/* File : cpp_net.i */
%module cpp_net_wrapper

/* Anything in the following section is added verbatim to the .cxx wrapper file */ 
%{
#include "cpp_net.h"
%}

/* This will allow us to iterate through arrays defined by STL containers */
%include "std_string.i"
%include "std_vector.i"

/* Create a template for std::vector< std::string > and std::vector< int > in Java */
namespace std {
   %template(IntVector) vector<int>;
   %template(StringVector) vector<string>;
}

/* This is the list of headers to be wrapped */
/* For Java, it seems we need the file of interest and all files up the inheritance tree */
%include "cpp_net.h"
