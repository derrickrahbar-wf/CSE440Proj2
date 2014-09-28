program test;

class testFunctionCallArgArray

BEGIN
   
   VAR
      dummyArray : ARRAY[0..9] OF integer;
      retval	 : integer;

FUNCTION tester(value	: integer): integer;
VAR
    poop : integer;
BEGIN
   PRINT value[6]
END   
  

END
.

