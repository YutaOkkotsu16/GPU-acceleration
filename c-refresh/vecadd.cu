
#include <cuda_device_runtime_api.h>
#include <cuda_runtime_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h> 
#include <stdint.h>

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


__global__ void vecadd_kernel(const float *a, const float *b, float *c,
     size_t n){
            
        size_t gi = blockDim.x * (size_t)blockIdx.x + threadIdx.x;

        #ifndef NO_GUARD 
        if (gi >= n){

            return;

            }
        #endif

        c[gi] = a[gi] + b[gi];
        }

void check_error(cudaError_t err, const char *step){

    if (err != cudaSuccess) {
    fprintf(stderr, "error: %s, step: %s\n", cudaGetErrorString(err), step);
    exit(EXIT_FAILURE);
}
}


int main(int argc, char *argv[]) {

    if (argc !=3){
        fprintf(stderr, "Usage: %s <N> <blocksize>\n", argv[0]);

        return EXIT_FAILURE;
    }

    size_t n, blocksize;

    if (!parse_positive(argv[1], &n)) {
        fprintf(stderr, "error: N must be a positive whole number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (!parse_positive(argv[2], &blocksize)) {
        fprintf(stderr, "error: blocksize must be a positive whole number, got '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }

    printf("N: %zu, blocksize: %zu\n", n, blocksize);

    float *ptra = NULL;
    float *ptrb = NULL;
    float *ptrc = NULL;

    if (n > SIZE_MAX/(3* sizeof(float))){
        fprintf(stderr, "error: N is too large to allocate three float arrays\n");
        free(ptra);
        free(ptrb);
        free(ptrc);
        return EXIT_FAILURE;
    }


    ptra = (float *)malloc(n * sizeof(float));
    ptrb = (float *)malloc(n * sizeof(float));
    ptrc = (float *)malloc(n * sizeof(float));

   
    if ((ptra == NULL) || (ptrb == NULL) || (ptrc == NULL)){

        fprintf(stderr, "Out of memory error\n");
        free(ptra);
        free(ptrb);
        free(ptrc);
        return EXIT_FAILURE;
        

    } 

    for (size_t i = 0; i<n; i++){
        ptra[i] = i;
        ptrb[i] = i;

    }

    

    size_t blocks;

    if ((n - 1 > SIZE_MAX - blocksize)){
        fprintf(stderr, "error: N + blocksize is too large to count blocks\n");
        free(ptra);
        free(ptrb);
        free(ptrc);
        return EXIT_FAILURE;
    }

    blocks = (n+ blocksize -1)/blocksize;
    printf("No. of blocks needee = %ld\nThe size of the float is: %zu\nThe total bytes is:%zu\n ", blocks, sizeof(float), 3*n*sizeof(float) );

    float *d_a = NULL;
    check_error(cudaMalloc(&d_a, n * sizeof(float)),"alloc d_a");

    float *d_b = NULL;
    check_error(cudaMalloc(&d_b, n * sizeof(float)),"alloc d_b");

    float *d_c = NULL;
    check_error(cudaMalloc(&d_c, n * sizeof(float)),"alloc d_c");    

    check_error(cudaMemcpy(d_a, ptra, n * sizeof(float), cudaMemcpyHostToDevice), "copy d_a");
    check_error(cudaMemcpy(d_b, ptrb, n * sizeof(float), cudaMemcpyHostToDevice), "copy d_b");

    vecadd_kernel<<<blocks,blocksize>>>(d_a, d_b, d_c, n);
    check_error(cudaGetLastError(), "kernel_launch_error\n");

    check_error(cudaMemcpy(ptrc, d_c, n * sizeof(float), cudaMemcpyDeviceToHost), "copy d_c");


    for (size_t l = 0; l < n; l++){

        if (ptrc[l] != ptrb[l] + ptra[l]){
            fprintf(stderr, "error: mismatch at index %zu: got %f, expected %f\n",
                    l, ptrc[l], ptra[l] + ptrb[l]);
            free(ptra);
            free(ptrb);
            free(ptrc);
            cudaFree(d_a);
            cudaFree(d_b);
            cudaFree(d_c);

            return EXIT_FAILURE;
        }

    }

    printf("PASS\n");

    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);


    free(ptra);
    free(ptrb);
    free(ptrc);


    return 0;   
}
