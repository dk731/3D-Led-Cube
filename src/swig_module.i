%module ledcd 
%{
#define SWIG_FILE_WITH_INIT
#include "CubeDrawer.h"
%}
%define VIRTUAL_RENDER
%enddef
%include "CubeDrawer.h"