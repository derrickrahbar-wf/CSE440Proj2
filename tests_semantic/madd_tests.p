program test;

class testFunctionCallArgArray

BEGIN
   
   VAR
      dummyArray : ARRAY[0..9] OF integer;
      retval	 : integer;
      retval	 : real;

FUNCTION setCompilerWorks(value	: integer ): integer;
BEGIN
   PRINT value[6]
END   
  

END
.

