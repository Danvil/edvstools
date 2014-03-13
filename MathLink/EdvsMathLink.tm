
int addtwo P((int, int));

:Begin:
:Function:       addtwo
:Pattern:        AddTwo[i_Integer, j_Integer]
:Arguments:      { i, j }
:ArgumentTypes:  { Integer, Integer }
:ReturnType:     Integer
:End:
:Evaluate: AddTwo::usage = "AddTwo[x, y] gives the sum of two machine integers x and y."

int edvsOpen P((const char*));

:Begin:
:Function:       edvsOpen
:Pattern:        EdvsOpen[opts_String]
:Arguments:      { opts }
:ArgumentTypes:  { String }
:ReturnType:     Integer
:End:
:Evaluate: EdvsOpen::usage = "Opens the connection to an EDVS sensor. Example: opt='/dev/ttyUSB0?baudrate=4000000'"

int edvsClose P(());

:Begin:
:Function:       edvsClose
:Pattern:        EdvsClose[]
:Arguments:      { }
:ArgumentTypes:  { }
:ReturnType:     Integer
:End:
:Evaluate: EdvsClose::usage = "Closes the connection to an EDVS sensor."

void edvsGet P(());

:Begin:
:Function:       edvsGet
:Pattern:        EdvsGet[]
:Arguments:      { }
:ArgumentTypes:  { }
:ReturnType:     Manual
:End:
:Evaluate: EdvsGet::usage = "Reads events. Returns the number of events."
