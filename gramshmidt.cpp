#include "stdio.h"
#include "stdlib.h"
#include "ac_fixed.h"

#define DATA_BITS 32 
#define DATA_INTEGER_BITS 28 // = DATA_BITS - DATA_FRACT_BITS

#define AC_FIXED ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM>

#include "matrix.h"


void Gramshmidt(Matrix *H, Matrix *Q, Matrix *R);

#define PRINT(num) printf ("%0-+f\n", num.to_double());


int main (void)
{
    int x=0, i=0, j=0, tmp=0;
    double *arr = new double[24];
    double *arr2 = new double [15];

    for (i=0; i<24; i++)
    {
        *(arr + i) = i+1;
    }

    Matrix H(6, 4, arr);
    Matrix Q(H.M, H.M);
    Matrix R(H.M, H.N);
    
    H.print();
    Gramshmidt(&H, &Q, &R);

    printf("Q = \n");
    Q.print();
    printf("R = \n");
    R.print();
    
    
    Matrix tr_result(4, 6);
    Matrix mul_result(H.M, H.N);
    
    Q.matrix_multiply(&R, &mul_result);
    printf("Multiplication result = \n");

    mul_result.print();
    ////////////////////////////////////////////
    /*
    Matrix b(10, 5, arr);

    //b.print();
    //b.set(3, 4, 100);
    //printf("sum = %f, %d\n", (float)b.get(2, 3));
    
    //b.print();
    //c.print();
    //printf("b printed\n");
    //c.print();
    //printf("a printed\n");
   
     b.copy_to(&a);
    a.transpose(&tr);
    tr.print();

    tr.copy_to(arr2, 10, 5);
    printf("arr2 printing \n ");
    for (i=0; i<50; i++)
    {
        printf("%f ",(float)*(arr + i));
    }

    printf("\n ");
    


    AC_FIXED num = 167;
    AC_FIXED norm;
    tr.row_norm(1, &norm);
    printf("norm = %f\n",(float)norm.to_double() );
    printf("Program ENd\n");
    delete [] arr;
    */
    delete [] arr;
    delete [] arr2;
    return 0;
}   

void skinny_decompose(Matrix *H, Matrix *Q, Matrix *R)
{
    int i=0, j=0, k=0, x=0, y=0;
    AC_FIXED temp = 0;

    H->copy_to(Q);

    for (k=0; k<H->N; k++)
    {
        //printf("----\n");
        Q->column_norm(k, &temp);
        R->data[k][k] = temp;

        Matrix temp4(H->M, 1);
        for (x=0; x<H->M; x++)
        {
            temp4.data[x][1] = Q->data[x][k];
        }
        for (x=0; x<H->M; x++)
        {
            Q->data[x][k] = temp4.data[x][1]/temp;
        }
        
        for (j=k+1; j<H->N; j++)
        {
            AC_FIXED temp2 = 0;
            for (x=0; x<H->M; x++)
            {
                temp2 = temp2 + (H->data[x][j] * Q->data[x][k]);
            }
            R->data[k][j] = temp2;

            Matrix temp3(H->M, 1);
            for (x=0; x<H->M; x++)
            {
                temp3.data[x][1] = (R->data[k][j] * Q->data[x][k]); 
            }
            for (x=0; x<H->M; x++)
            {
                Q->data[x][j] = Q->data[x][j] - temp3.data[x][1];
                //printf("%+-0 .9f ", Q->data[x][j].to_double());
            }
            
        }
     
    }
}




void fat_decompose(Matrix *H, Matrix *Q, Matrix *R)
{


}



void Gramshmidt(Matrix *H, Matrix *Q, Matrix *R)
{
    if (H->M > H->N)
        skinny_decompose(H, Q, R); 
    else
        fat_decompose(H, Q, R); 
}
