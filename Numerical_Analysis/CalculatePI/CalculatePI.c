#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* the circle with radius r = 1*/
float circle(float x) {
    return sqrt(1-x*x);
}

/* generator a point (x, y) and return 1 when it's in circle, otherwise return 0*/
int PointGenerator() {
    float x = (float)rand()/(float)RAND_MAX; // between 0 and 1
    float y = (float)rand()/(float)RAND_MAX;
    return (x*x+y*y) < 1;
}

float montecarlo();
float romberg(float (*func)(float x), float a, float b, int iter);
float gaussain(float (*func)(float x), float a, float b);

int main(int argc, char* args[])
{
    char* method = args[1];
    float pi;
    if( strcmp(method, "romberg") == 0)
    {
        int iter = 20;
        pi = romberg(circle, 0, 1, iter)*4;
        printf("Romberg gives %f\n", pi);
    }
    else if(strcmp(method, "monte") == 0)
    {
        pi = montecarlo()*4;
        printf("Monte carlo gives %f\n", pi);
    }
    else if(strcmp(method, "gaussian") == 0)
    {
        pi = gaussain(circle, 0, 1)*4;
        printf("Guassian Quadrature gives %f\n", pi);
    }
    return 0;
}

float montecarlo() {
    srand(time(NULL));
    int sum = 0, total = 9999;
    for(int i=0; i<total; i++) {
        if(PointGenerator())
            sum++;
    }
    return (float)sum/total;
}

float gaussain(float (*func)(float x), float a, float b)
{
    float w1 = 5.0 / 9.0;
    float w2 = 8.0 / 9.0;
    float w3 = 5.0 / 9.0;
    float z1 = -0.7746;   
    float z2 = 0;
    float z3 = 0.7746;
    float x1 = (b - a) * z1 / 2.0 + (b + a ) / 2.0;
    float x2 = (b - a) * z2 / 2.0 + (b + a ) / 2.0;
    float x3 = (b - a) * z3 / 2.0 + (b + a ) / 2.0;
    float y1 = func(x1);
    float y2 = func(x2);
    float y3 = func(x3);
    float integral = ((b - a) / 2.0) * (w1 * y1 + w2 * y2 + w3 * y3);
    return integral;
}

float romberg(float (*func)(float x), float a, float b, int iter) {
    float* R = (float*)malloc(sizeof(float)*iter*iter);
    float h = b-a;
    R[0] = h*(func(a)+func(b))/2;
    for(int i=1; i<iter; i++) {
        h /= 2;
        float sum =0;
        for(int j=0; j<pow(2, i-1); j++) {
            sum += h*func(a+(2*j-1)*h);
        }
        R[i*iter] = R[(i-1)*iter]/2 + sum;
        for(int k=1; k<=i; k++) {
            R[i*iter+k] = (pow(4, k)*R[i*iter+k-1] - R[(i-1)*iter+k-1])/(pow(4, k)-1);
        }
    }
    return R[iter*iter-1];
}