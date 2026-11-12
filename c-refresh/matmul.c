#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h> 
#include <stdint.h> 
#include <time.h>
#include <math.h>

bool parse_positive(const char *text, size_t *out){
        char *end;
        errno = 0;
        long value = strtol(text, &end, 10);

        if (end == text) return false; 
        if (*end != '\0') return false;

        if (errno == ERANGE) return false;
        if (value <= 0) return false;

        *out = (size_t)value;
        //fprintf(stderr, "Parsed value: %ld\n", value);
        return true;
    }

    // a, b and c must not overlap


void my_matmul(const float *restrict a, const float *restrict b, float *restrict c, const size_t R, const size_t F, const size_t K){

    for (size_t i=0; i<R; i++){
        for (size_t j=0; j<K; j++){
            for (size_t k=0; k<F; k++){
                c[i * K + j] += a[i * F + k] * b[k * K + j];

            }

        }
    }

}

void ref_matmul(const float *restrict a, const float *restrict b, float *restrict c, const size_t R, const size_t F, const size_t K){

    double sum = 0;

    for (size_t i=0; i<R; i++){
        for (size_t j=0; j<K; j++){
            for (size_t k=0; k<F; k++){
                sum += (double)a[i * F + k] * (double)b[k * K + j];
            }
            c[i * K + j] = (float)sum;
            sum = 0;

        }
    }

}



int main(int argc, char *argv[]) {

    if (argc !=4){
        fprintf(stderr, "Usage: %s <N> <blocksize>\n", argv[0]);

        return EXIT_FAILURE;
    }

    size_t R, F, K;

    struct timespec start, end;

    if (!parse_positive(argv[1], &R)){
        fprintf(stderr, "error: columns must be a positive whole number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;

    }

    if (!parse_positive(argv[2], &F)){
        fprintf(stderr, "error: columns must be a positive whole number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;

    }

    if (!parse_positive(argv[3], &K)){
        fprintf(stderr, "error: columns must be a positive whole number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;

    }

const float test_a[]        = {1, 2, 3, 4, 5, 6};
const float test_b[]        = {7, 8, 9, 10, 11, 12};
const float test_expected[] = {58, 64, 139, 154};
float test_c[4] = {0};

    my_matmul(test_a, test_b, test_c, 2, 3, 2);

for (size_t i = 0; i < 4; i++){
    if (test_c[i] != test_expected[i]){
        fprintf(stderr, "error: known-answer test failed at index %zu: got %f, expected %f\n",
                i, test_c[i], test_expected[i]);
        return EXIT_FAILURE;
    }
}



    float *ptrA = NULL;
    float *ptrB = NULL;
    float *ptrC = NULL;
    float *ptrRef = NULL;

    if (F > SIZE_MAX/sizeof(float)){
        fprintf(stderr, "error: Too large to allocate \n");
        return EXIT_FAILURE;
    }

    if (R > SIZE_MAX/sizeof(float)){
        fprintf(stderr, "error: Too large to allocate \n");
        return EXIT_FAILURE;
    }

    if (K > SIZE_MAX/sizeof(float)){
        fprintf(stderr, "error: Too large to allocate \n");
        return EXIT_FAILURE;
    }

    if (R > SIZE_MAX/((sizeof(float) *F))){
        fprintf(stderr, "error: R is too large to allocate \n");
        return EXIT_FAILURE;
    }

    if (F > SIZE_MAX/(sizeof(float) *K)){
        fprintf(stderr, "error: F is too large to allocate \n");
        return EXIT_FAILURE;
    }

    if (K > SIZE_MAX/(sizeof(float) *R)){
        fprintf(stderr, "error: K is too large to allocate \n");
        return EXIT_FAILURE;
    }

    ptrA = malloc(R*F * sizeof(float));
    ptrB = malloc(F*K * sizeof(float));
    ptrC = malloc(R*K * sizeof(float));
    ptrRef = malloc(R*K * sizeof(float));

    if ((ptrA == NULL) || (ptrB == NULL) || (ptrC == NULL) || (ptrRef ==NULL)){

        fprintf(stderr, "Out of memory error\n");
        free(ptrA);
        free(ptrB);
        free(ptrC);
        free(ptrRef);
        return EXIT_FAILURE;
        

    } 

    memset(ptrC, 0 , R * K * sizeof(float));

    for (size_t i=0; i < R; i ++){
        for (size_t j =0; j < F; j++){

        ptrA[i * F + j] = i + j + 1;

    }
}

    for (size_t i=0; i < F; i ++){
        for (size_t j =0; j < K; j++){

        ptrB[i * K + j] = i  + j + 7;

    }
}

clock_gettime(CLOCK_MONOTONIC, &start);

my_matmul(ptrA, ptrB, ptrC, R, F, K);

clock_gettime(CLOCK_MONOTONIC, &end);

ref_matmul(ptrA, ptrB, ptrRef, R, F, K);

long count = 0; 
for (size_t i=0; i<R; i++){
        for (size_t j=0; j<K; j++){
            if (!(fabsf((ptrC[i * K + j] - ptrRef[i * K + j])) <= 1e-6 + 2e-6 * fabsf((ptrRef[i * K + j])))){
                printf("Calculated: %lf, Expected:%lf , index i: %ld, index j: %ld", ptrC[i * K + j],ptrRef[i * K + j], i, j);
                free(ptrA);
                free(ptrB);
                free(ptrC);
                free(ptrRef);

                return EXIT_FAILURE;
                
            }
        }
    }

printf("No. of same entries:%ld", count);

double ms_1 = (end.tv_sec - start.tv_sec) * 1e3
          + (end.tv_nsec - start.tv_nsec) / 1e6;

printf("time taken:%.9g ms", ms_1);

    

free(ptrA);
free(ptrB);
free(ptrC);
free(ptrRef);



return 0;



}