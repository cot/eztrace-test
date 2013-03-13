# There are deliberately trailing whitespace everywhere. This
   # (<-- see?) is for stressing the script parser.
# Sorry for the inconvenience.
 BEGIN_MODULE
NAME example_lib
DESC "module for the example library"
LANGUAGE C
ID 99

BEGIN_INCLUDE
#include <stdio.h>
END_INCLUDE

int example_do_event(    int n)
BEGIN
 EVENT("Do new function")
END


double example_function1(double* array, int array_size)

int example_function2(int* array, int array_size)
BEGIN
  RECORD_STATE("running example_function2")
END

int example_fcall(   int* array,
        int array_size )
BEGIN
 CALL_FUNC
END

int example_push (int* array,int array_size)
BEGIN
 PUSH_STATE("doing function example_push")
END


int example_event(int* array, int array_size)
BEGIN
 EVENT("example_event called")
END

int example_set_var(int* array, int array_size)
BEGIN
 SET_VAR("variable name", 5)
END

int example_add_var(int* array,    int array_size)
BEGIN
 ADD_VAR("variable name", 1)
END

 int example_sub_var (    int* array,    int array_size)
BEGIN
 SUB_VAR("variable name", 4)
END


int example_set_var2 (    int* array,    int array_size)
BEGIN
 SET_VAR("another variable name", 21)
END

int example_set_var3(    int* array,    int array_size)
BEGIN
 ADD_VAR("another variable name", 10)
END


 int example_set_var4(    int* array,    int array_size)
BEGIN
 SUB_VAR("another variable name", 3)
END


 int example_set_var5(    int* array,    int array_size)
BEGIN
 SET_VAR("another variable name", 13)
END

int example_set_var6(    int* array,    int array_size)
BEGIN
 ADD_VAR("variable name", 2)
 CALL_FUNC
 SUB_VAR("variable name", 1)
END

 int example_many_args(
      int arg1,
      int arg2,
      int arg3,
      int arg4,
      int arg5,
      int arg6,
      int arg7)
BEGIN
 ADD_VAR("variable name", 2)
 CALL_FUNC
 SUB_VAR("variable name", 1)
END


END_MODULE
