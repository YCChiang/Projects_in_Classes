#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define zero 1e-6

/* coordinate format */
struct COO {
    int row;
    int col;
    float val;
};

/* compressed sparse row format */
typedef struct CSR {
    int col;
    float val;
} CSR;

/* compressed sparse column format */
typedef struct CSC {
    int row;
    float val;
} CSC;

/* convert sparse matrix to CSR format*/
int formatCSR(float *A, int n, int m, int *rowptr, CSR **csr) 
{
    rowptr = (int *)malloc(sizeof(int)*(n+1));
    csr = (CSR **)malloc(sizeof(CSR*)*m);
    int k = 0, s = m;
    rowptr[0] = 0;
    for(int i=0; i<n; i++) {
        for(int j=0; j<m; j++) {
            if(fabs(A[i*m+j])>zero) {
                if(k >= s) {
                    s += m;
                    realloc(csr, sizeof(CSR*)*s);
                }
                csr[k] = (CSR*)malloc(sizeof(CSR));
                csr[k]->col = j;
                csr[k]->val = A[i*m+j];
                k++;
            }
        }
        rowptr[i+1] = k;
    }
    return k;
}

void matvec_multi(float *x, float *A, float *y, int n, int m) 
{    
    int *rowptr, num;
    CSR **csr;
    num = formatCSR(A, n, m, rowptr, csr);
    
    float *y = (float *)malloc(sizeof(float)*n);
    memset(y, 0, sizeof(float)*n);

    // sparse matrix-vector multiplication
    for(int i=0; i<n; i++) {
        for(int j = rowptr[i]; j = rowptr[i+1]; j++) {
            int col = csr[j]->col;
            y[i] += x[col]*csr[j]->val;
        }
    }

    free(rowptr);
    for(int i=0; i<num; i++)
        free(csr[i]);
    free(csr);
}

int main(int argc, char* args[]) 
{
    return 0;
}
