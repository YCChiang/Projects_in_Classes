#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void printvec(float *vec, int n)
{
    for (int i = 0; i < n; i++)
        printf("%f\n", vec[i]);
}
void printmat(float *mat, int m, int n)
{
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            printf("%f ", mat[i * n + j]);
        printf("\n");
    }
}
float dotproduct(float *x, float *y, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += x[i] * y[i];
    }
    return sum;
}
void matvec(float *A, float *x, float *y, int n)
{
    for (int i = 0; i < n; i++)
    {
        y[i] = 0;
        for (int j = 0; j < n; j++)
            y[i] += A[i * n + j] * x[j];
    }
}

void matmat(float *C, float *A, float *B, int m, int s, int n)
{
    for(int i=0; i<m; i++) {
        for(int j=0; j<n; j++) {
            float sum=0;
            for(int k=0; k<s; k++) {
                sum += A[i*m+k]*B[k*s+j];
            }
            C[i*m+j] = sum;
        }
    }
}

float vec2norm(float *x, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum += x[i] * x[i];
    return sqrt(sum);
}

void transpose(float *AT, float *A, int m, int n)
{
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            AT[j * m + i] = A[i * n + j];
}

void matmat_transA(float *C, float *AT, float *B, int m, int k, int n)
{
    memset(C, 0, sizeof(float) * m * n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            for (int kk = 0; kk < k; kk++)
                C[i * n + j] += AT[kk * m + i] * B[kk * n + j];
}

void matmat_transB(float *C, float *A, float *BT, int m, int k, int n)
{
    memset(C, 0, sizeof(float) * m * n);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            for (int kk = 0; kk < k; kk++)
                C[i * n + j] += A[i * k + kk] * BT[j * k + kk];
}

void gramschmidt_2017011294(float* V, float *X, int m, int n) 
{
    float *sum = (float*)(malloc(sizeof(float)*n));
    for(int i=0; i<n; i++) 
    {
        float *vi = &V[i*n];
        float *xi = &X[i*n];
        memset(sum, 0, sizeof(float)*n);
        for(int j=0; j< i; j++) 
        {
            float *vj = &V[j*n];
            float dp1 = dotproduct(xi, vj, n);
            float dp2 = dotproduct(vj, vj, n);

            for(int k=0; k<n; k++)
                sum[k] += (dp1/dp2) * vj[k];
        }

        for(int k=0; k<n; k++)
            vi[k] = xi[k] - sum[k];
    }
    free(sum);
}

void qr_2017011294(float *Q, float *R, float *A, int m, int n)
{
    gramschmidt_2017011294(Q, A, m, n);

    for (int i = 0; i < m; i++)
    {
        float *qi = &Q[i * n];
        float norm = vec2norm(qi, n);
        for (int j = 0; j < n; j++)
            qi[j] /= norm;
    }

    float *QT = (float *)malloc(sizeof(float) * m * n);
    transpose(QT, Q, m, n);
    matmat_transA(R, QT, A, n, m, n);
    free(QT);
}

void qreigensolver_2017011294(float *eigenvalue, float *eigenvtectors, float *A, int n, int maxiter, float threshold)
{
    float *Ai = (float *)malloc(sizeof(float) * n * n);
    float *Ai_new = (float *)malloc(sizeof(float) * n * n);
    float *Q = (float *)malloc(sizeof(float) * n * n);
    float *R = (float *)malloc(sizeof(float) * n * n);
    float *error = (float *)malloc(sizeof(float) * n);

    float *Qp = (float *)malloc(sizeof(float) * n * n);
    memset(Qp, 0, sizeof(float) * n * n);
    for (int i = 0; i < n; i++)
        Qp[i * n + i] = 1;

    memcpy(Ai, A, sizeof(float) * n * n);
    int iter = 0;
    float errornorm = 0;
    do
    {
        qr_2017011294(Q, R, Ai, n, n);

        matmat_transB(eigenvtectors, Qp, Q, n, n, n);
        memcpy(Qp, eigenvtectors, sizeof(float) * n * n);

        matmat_transB(Ai_new, R, Q, n, n, n);
        for (int i = 0; i < n; i++)
        {
            error[i] = Ai_new[i * n + i] - Ai[i * n + i];
        }
        errornorm = vec2norm(error, n);
        memcpy(Ai, Ai_new, sizeof(float) * n * n);
        iter++;
        printf("iter = %i, errornorm = %f\n", iter, errornorm);
    } while (iter < maxiter && errornorm > threshold);
    for (int i = 0; i < n; i++)
        eigenvalue[i] = Ai_new[i * n + i];
    free(Ai);
    free(Ai_new);
    free(Qp);
}

void tridiagonalization(float *A, float *T, float *P, int n)
{
    float *p1 = (float *)malloc(sizeof(float) * n);
    float *p2 = (float *)malloc(sizeof(float) * n);
    float *w = (float *)malloc(sizeof(float) * n);
    float *wp = (float *)malloc(sizeof(float) * n);

    memset(T, 0, sizeof(float) * n * n);
    memset(p1, 0, sizeof(float) * n);
    p1[0] = 1.0;

    matvec(A, p1, wp, n);
    float alpha = dotproduct(wp, p1, n);
    T[0 * n + 0] = alpha;
    for (int i = 0; i < n; i++)
    {
        w[i] = wp[i] - alpha * p1[i];
    }
    for (int i = 0; i < n; i++)
        P[i * n + 0] = p1[i];

    for (int j = 1; j < n; j++)
    {
        float beta = vec2norm(w, n);
        T[(j - 1) * n + j] = beta;
        T[j * n + (j - 1)] = beta;

        for (int i = 0; i < n; i++)
            p2[i] = w[i] / beta;
        matvec(A, p2, wp, n);
        float alpha = dotproduct(wp, p2, n);
        T[j * n + j] = alpha;
        for (int i = 0; i < n; i++)
            w[i] = wp[i] - alpha * p2[i] - beta * p1[i];
        for (int i = 0; i < n; i++)
            P[i * n + j] = p2[i];
        memcpy(p1, p2, sizeof(float) * n);
    }

    free(p1);
    free(p2);
    free(w);
    free(wp);
}

// int IsSymmtric(float *A, int n)
// {
//     for(int i=0; i<n; i++)
//     {
//         for(int j=0; j<i; j++)
//         {
//             if(fabs(A[i*n+j]-A[j*n+i]) > 1e-6)
//             {
//                 return 0;
//             }
//         }
//     }
// }

// // for non-symmetric matrix
// void SVD(float *image, float *image_compressed, int m, int n)
// {
//     float *A = (float *)malloc(sizeof(float) * n * m);
//     matmat_transB(A, image, image, m, n, m);

//     if(IsSymmtric(A, n))
//     {
//         float *P = (float *)malloc(sizeof(float) * n * n);
//         float *T = (float *)malloc(sizeof(float) * n * n);
//         tridiagonalization(A, T, P, n);
//         float *VT = (float *)malloc(sizeof(float) * n * n);
//         float *S = (float *)malloc(sizeof(float) * n * n);
//         qreigensolver_2017011294(S, VT, T, n, 2000, 0.00001);
        
//         float *temp = (float *)malloc(sizeof(float) * m * n);
//         matmat_transB(temp, image, VT, m, n, n);
//         matmat(image_compressed, temp, S, m, n, n);
//     }
//     else
//     {
//         float *V = (float *)malloc(sizeof(float) * n * n);
//         float *S = (float *)malloc(sizeof(float) * n * n);
//         qreigensolver_2017011294(S, VT, T, n, 2000, 0.00001);
//         float *temp = (float *)malloc(sizeof(float) * m * n);
//         matmat_transB(temp, image, VT, m, n, n);
//         matmat(image_compressed, temp, S, m, n, n);
//     }
    
// }

void lanczos_2017011294(float *image, float *image_compressed, int n)
{
    float *P = (float *)malloc(sizeof(float) * n * n);
    float *T = (float *)malloc(sizeof(float) * n * n);
    tridiagonalization(image, T, P, n);
    float *U = (float *)malloc(sizeof(float) * n * n);
    float *S = (float *)malloc(sizeof(float) * n * n);
    qreigensolver_2017011294(S, U, T, n, 2000, 0.00001);
    float *UT = (float *)malloc(sizeof(float) * n * n);

    transpose(UT, U, n, n);
    float *PT = (float *)malloc(sizeof(float) * n * n);
    transpose(PT, P, n, n);
    
    matmat(image_compressed, P, U, n, n, n);
    matmat(P, image_compressed, S, n, n, n);
    matmat(image_compressed, P, UT, n, n, n);
    matmat(P, image_compressed, PT, n, n, n);
    memcpy(image_compressed, P, sizeof(float) * n * n);

    printf("\n Image_compressed = \n");
    //printmat(image_compressed, n, n);

    free(P);
    free(T);
    free(U);
    free(S);
    free(UT);
    free(PT);
}

int main(int argc, char **argv)
{
    FILE *file;
    char *filename = argv[1];
    file = fopen(filename, "r+");
    char *method = argv[1];
    int n;
    fscanf(file, "%d", &n);
    float *image = (float *)malloc(sizeof(float) * n * n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            fscanf(file, "%f", image + i * n + j);
    fclose(file);

    float *image_compressed = (float *)malloc(sizeof(float) * n * n);

    lanczos_2017011294(image, image_compressed, n);
 
    file = fopen("file.txt" , "w");
    fwrite(image_compressed, sizeof(char), sizeof(image_compressed), file);
 
    fclose(file);
    free(image);
    free(image_compressed);
    return 0;
}