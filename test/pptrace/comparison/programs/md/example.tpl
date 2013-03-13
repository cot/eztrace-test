BEGIN_MODULE
NAME example_lib
DESC "module for md"
LANGUAGE C
ID 98

int  compute_ (int*  np,int*  nd,int*  pos,int*  vel,int*  mass,int*  f,int*  pot,int*  kin )
int  dist_ (int*  nd,int*  r1,int*  r2,int*  dr,int*  d )
int  initialize_ (int*  np,int*  nd,int*  box,int*  seed,int*  pos,int*  vel,int*  acc )
int  timestamp_ ( )
int  update_ (int*  np,int*  nd,int*  pos,int*  vel,int*  f,int*  acc,int*  mass,int*  dt )

END_MODULE
