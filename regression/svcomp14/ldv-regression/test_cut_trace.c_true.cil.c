/* Generated by CIL v. 1.3.7 */
/* print_CIL_Input is true */

#line 2 "./assert.h"
void __blast_assert(void) 
{ 

  {
  ERROR: 
#line 4
  assert(0);
}
}
#line 6 "files/test_cut_trace.c"
int main(void) 
{ int z ;
  int a ;

  {
#line 8
  z = 0;
#line 9
  if (z == 0) {

  } else {
    {
#line 9
    __blast_assert();
    }
  }
#line 10
  a = z;
#line 11
  if (a == 0) {

  } else {
    {
#line 11
    __blast_assert();
    }
  }
#line 12
  return (0);
}
}