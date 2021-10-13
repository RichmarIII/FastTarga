%include "stdint.i"
%include "std_vector.i"

%include <carrays.i>
%include "arrays_csharp.i"
%array_class(unsigned char, ByteArray);


%module FastTarga
%{
#include "TargaImage.h"
%}

%include "TargaImage.h"

%template(Vector_uint8_t) std::vector<uint8_t>;