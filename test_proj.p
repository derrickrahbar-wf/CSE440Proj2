program test;

class tes
BEGIN   
FUNCTION tes(value : integer): integer;
BEGIN
    aa := bb + cc;
    bb := 10;
    gg := dd + cc + ee;
    kk := dd - cc * ( aa + bb );
    if aa + bb > cc THEN
        BEGIN 
            aa := 1;
            bb := cc * dd
        END
    ELSE
        aa := bb;
    WHILE cc < 7 DO
        BEGIN
        cc := cc + 1;
        dd := aa + dd
        END

END

END
.
