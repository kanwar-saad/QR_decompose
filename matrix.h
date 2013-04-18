#define MAT_TYPE ac_fixed<DATA_BITS,DATA_INTEGER_BITS,true, AC_TRN, AC_SAT_SYM>
 
#define mexPrintf printf

#define DEBUG 1

#define debug_print(fmt, ...) do{ if (DEBUG) mexPrintf(fmt, __VA_ARGS__); }while(0)
    
    
class Matrix
{
    public:
    int M;  // Rows
    int N;  // Columns
    MAT_TYPE **data;
    
    Matrix(int m, int n)
    {
        int i=0, j=0;

        M = m;
        N = n;
        data = new MAT_TYPE* [m];
        for (i=0; i<m; i++)
        {
            data[i] = new MAT_TYPE[n];
        }
    
        for (i=0; i<m; i++)
        {
            for (j=0; j<n; j++)
            {
                data[i][j] = 0;
            }
        }
    };
    
    Matrix(int m, int n, double *in)
    {
        int i, j;
        M = m;
        N = n;
        
        //data = new MAT_TYPE[m][n];
        
        data = new MAT_TYPE* [m];
        for (i=0; i<m; i++)
        {
            data[i] = new MAT_TYPE[n];
        } 
        
        debug_print("Initializing matrix of size %d x %d\n", m, n);
        for (i=0; i<m; i++)
        {
            //debug_print("%s", "\n");
            for (j=0; j<n; j++)
            {
                data[i][j] = *(in + ((i*n) + j));
                //debug_print("%f ", data[i][j].to_double());
            }

        }

        //debug_print("%s", "\n");
    };

    // Default destructor

    ~Matrix()
    {
        int i=0;
        //debug_print("Deleting matrix of size %d x %d\n", M, N);
        for (i=0; i<M; i++)
        {       
            delete [] data[i];
        }
        delete [] data;

    };

    double get_double(int m, int n)
    {
        if ((m >= M) || (n >= N))
        {
            debug_print("%s","Index out of Range\n");
            return 0;
        }
        else
        {
            return data[m][n].to_double();
        }
    };

    void set_double(int m, int n, double val)
    {
        if ((m >= M) || (n >= N))
        {
            debug_print("%s", "Index out of Range\n");
            return;
        }
        else
        {
            data[m][n] = val;
        }
    };

    void copy_to(Matrix *in)
    {
        int i=0, j=0;
        //debug_print("%s, %d, %d\n", "Input", in->M, in->N);
        if ((M > in->M) || (N > in->N))
        {
            debug_print("%s","Matrix dimensions don't match for copying\n");
            return;
        }
        
        for (i=0;i<M;i++)
        {
            for (j=0; j<N; j++)
            {
                in->data[i][j] = data[i][j];
            }
        }
    };

    void copy_to(double *in, int m, int n)
    {
        int i=0, j=0;
        //debug_print("%s, %d, %d\n", "Input", in->M, in->N);
        if ((M != m) || (N != n))
        {
            debug_print("%s","Matrix dimensions don't match for copying\n");
            return;
        }
        
        for (i=0;i<(M);i++)
        {
            for (j=0; j<N; j++)
            {
                *(in + ((i*n) + j)) = data[i][j].to_double();
            }
        }

        //printf("returning from copy_to\n");
    };


    void print()
    {
        int i=0, j=0;
        for (i=0;i<M;i++)
        {
            debug_print("%s","\n");
            for (j=0; j<N; j++)
            {
                debug_print("%-+020.10f ", data[i][j].to_double());
            }
        }
        debug_print("%s", "\n");
    };

    Matrix* transpose(Matrix *out)
    {
        int i=0, j=0, temp=0;
        MAT_TYPE val;
        
        if ((M != out->N) || (N != out->M))
        {
            debug_print("%s","Matrix dimensions don't match for transpose\n");
            return out;
        }
        
        for (i=0;i<M;i++)
        {
            for (j=0; j<N; j++)
            {
                out->data[j][i] = data[i][j];
            }
        }
        return out; 
    };
  
    void f_sqrt(MAT_TYPE in, MAT_TYPE *out)
    {
        int i=0, j=0;
        MAT_TYPE result = 1;
        int iterations = 15;
        
        for (i=0; i<iterations; i++)
        {
            result = (result + (in/result))/2;
        }
        *out = result; 
    };
 
    void column_norm(int col_number, MAT_TYPE *out)
    {
        int i=0, j=0;
        MAT_TYPE sum=0, result=0;

        for (i=0; i<M; i++)
        {
            //printf("%f ", (float)data[i][col_number].to_double());
            sum = sum + (data[i][col_number] * data[i][col_number]);
        }
        //printf("\n");
        f_sqrt(sum, out);
        
    };

    void row_norm(int row_number, MAT_TYPE *out)
    {
        int i=0, j=0;
        MAT_TYPE sum=0;

        for (i=0; i<N; i++)
        {
            sum = sum + (data[row_number][i] * data[row_number][i]);
        }

        f_sqrt(sum, out);


    };

    void matrix_multiply(Matrix *right_matrix, Matrix *dst)
    {
        MAT_TYPE sum = 0;
        int i=0, j=0, k=0;

        if (N != right_matrix->M)
        {
            debug_print("%s", "Matrix dimesions don't agree for multiplication\n");
            return;
        }

        // Check dimensions of resulting matrix
        
        if ((M != dst->M) || (right_matrix->N != dst->N))
        {
            debug_print("%s", "Resulting matrix has invalid dimesions for multiplication result.\n");
            return;
            
        }

        for(i=0;i<dst->M;i++){ //row of first matrix
            for(j=0;j<dst->N;j++){  //column of second matrix
                sum=0;
                for(k=0;k<N;k++)
                    sum = sum + (data[i][k] * right_matrix->data[k][j]);

                dst->data[i][j] = sum;
            }
        }

    }
 
};
