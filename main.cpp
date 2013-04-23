/************************************************/
/* QRD modified Gram-Schmidt                    */
/* for Communication Signal Processing course 	*/
/*	1/2013				 						*/
/*	Janne Janhunen		         				*/		
/*                     		         			*/
/************************************************/	
//#include "mex.h"            /* Matlab MEX interface */
#include <math.h>  /* Math operations */
#include <stdio.h> /* Standard IO */
#include "ac_fixed.h" /* Change path */

#define DATA_BITS 32 
#define DATA_INTEGER_BITS 16 // = DATA_BITS - DATA_FRACT_BITS

#define mexPrintf printf

/* Examples of fraction accuracy */
// 2^-18 => 0,000003814697265625 
// 2^-16 => 0,0000152587890625
/*****************************/
/*   Modified Gram-Schmidt   */
/*****************************/



/////////////////////////////////
// /* Initializing zero matrix */ 
/////////////////////////////////

void zeros(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *matrix, int rows, int cols)
{
    int i = 0;
    
    for (i=0; i<(rows*cols); i++)
        *(matrix + i) = (i+1) ;
}

////////////////////////////////
// /* square root of a number */
///////////////////////////////

void sq_rt(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *sq_in, ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *sq_out )
{
    int j=0;
    int iterations = 15;
    *sq_out = 1;
    
    for(j=1; j<=iterations; j++)
    {
        *sq_out	 = (*sq_out + (*sq_in / *sq_out))/2;                                                  
    }
}

///////////////////////////////////
// /* setting values in an array */
//////////////////////////////////

void set_arr_el(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *ptr, int row_vect_size, int x, int y, double val)
{
   
    *(ptr + (y*row_vect_size) + x) = val;
}


/////////////////////////////////////////////
// /* getting value from an array location */
////////////////////////////////////////////

void get_arr_el(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *ptr, ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *get_out, int row_size, int x, int y)
{ 
    //get_out = 0;
    int offset = ((x*row_size) + y);
    *get_out = *(ptr + offset);
    //mexPrintf("get_out[%d]=%lf\n", get_out->to_double());
}

///////////////////////////////
/* norm calculating *////////
//////////////////////////////

void norm(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *matrix, ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *out, int column, int rows, int cols)
{

    ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> temp_val = 0;
    ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> sum =0;
    int i=0;
    for (i=0;i<rows;i++)
    {
        get_arr_el(matrix, &temp_val, cols, i, column);
        mexPrintf("temp_val[%d]=%lf\n", temp_val.to_double());
        sum += (temp_val * temp_val);
    }
    mexPrintf("sum = %f\n", sum.to_double());
    sq_rt(&sum,out);
    mexPrintf("out=%f\n", out->to_double());
}



///////////////////////////////
// /* Printing Matrtix *//////
//////////////////////////////

void print_matrix(ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> *mat_p, int M, int N)
{
    int i=0, j=0;
    ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> tvtp=0;
    for (i=0;i<(M); i++)
    {
        mexPrintf("\n");
        for (j=0;j<N;j++)
        {
            get_arr_el(mat_p, &tvtp, N, i, j);
            mexPrintf("%f ", tvtp.to_double());
        }
       
    }
    mexPrintf("\n");
}





void GRAMSCHMIDT(double *Q_io, double *R_io, double *H, int m, int n)
{
  
  int i,j,k,h;    
  int counter = 0;

  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> opt;
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> out;
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> r_temp;
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> inp = 81;
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> get_out;

  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> sq_in;;
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> sq_out;

  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> tmp_U[m];
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> temp[m];   
  
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> r[m*n];   
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> Q[m*m];   
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> R[m*n];
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> tmp[m*n];
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> invR[m*n];   
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> invH[m*n];   
  ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM> H_mtx[m*n];
  
  
//  
//   sq_rt(&val,&sum);
//   mexPrintf("sum[%d]=%lf\n", sum.to_double());
//   
//  
 for(i=0;i<m;i++)
 {
     for(h=0;h<n;h++)
     {
         H_mtx[i*n+h] = *(H + (i+h*n));    
         tmp[i*n+h] = *(H + (i+h*n));
     }
 }

// // 
// //   
// // /* one possible way to debug your code is to print vectors in double format */
// //   for(k=0; k<m*n; k++)
// //   {	
// // 	mexPrintf("H_mtx[%d]=%lf\n",k, H_mtx[k].to_double());
// //   }
// // 
// // // 
  

  //sq_rt(&inp,&opt);
  //mexPrintf("opt[%d]=%lf\n", opt.to_double());
  
  //mexPrintf("out[%d]=%lf\n", out.to_double());
  zeros(R, m, n);
  zeros(Q, n, n);
  print_matrix(R, m, n);
  get_arr_el(R, &get_out, n, 2, 2);
  printf("outside val = %f\n", get_out.to_double());
  norm(R, &get_out, 1, 4, 3);
  printf("outside val = %f\n", get_out.to_double());
  
  
  set_arr_el(R, n, 1, 1, 23);
  //norm(Q, out, 1, m, n);
  //print_matrix(R, m, n);

//   if (m>n){

      
      
//       mexPrintf("tvtp[%d]=%ld\n", tvtp.to_double());

      
//       print_matrix(Q, m, n);
//       print_matrix(R, m, n);
      
//       set_arr_el(double *ptr, , int x, int y, double val)
//       
//       for (i=0; i<(m*n) ; i++)
//           
//       
//   
//   }
//   
//   
//   
//   
//   else 
  
  
// // 	
// // 
// // 	
// // 
// //     
// // /* MATRIX MULTIPLICATION */
// // /* Computes upper triangular matrix R = Q' * A */
// // 
// // 
// // 
// // 
// // 
/* Output R and Q */  

for(i=0; i<m; i++)
{
    for(h=0; h<n; h++)
    {
        *(Q_io + (i*n+h)) = Q[i+h*n].to_double(); 
        *(R_io + (i*n+h)) = R[i+h*n].to_double();
    }
}
  
// 
// 
}


int main (void)
{
    double H[4*3];
    double Q[4*4];
    double R[4*3];
    int i=0, j=0;
    for (i=0; i< 4; i++)
    {
        for (j=0; j<3; j++)
        {
            H[(i*3)+ j] = (i*3)+j;
        }
    }

    GRAMSCHMIDT(Q, R, H,4, 3);
    return 0; 

}





