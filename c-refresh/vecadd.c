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


void vecadd_kernel(const float *a, const float *b, float *c,
     size_t n, size_t block_idx, size_t block_dim, size_t thread_idx){
            
        size_t gi = block_dim * block_idx + thread_idx;

        #ifndef NO_GUARD 
        if (gi >= n){

            return;

            }
        #endif

        c[gi] = a[gi] + b[gi];
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
        return EXIT_FAILURE;
    }


    ptra = malloc(n * sizeof(float));
    ptrb = malloc(n * sizeof(float));
    ptrc = malloc(n * sizeof(float));

   
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
        return EXIT_FAILURE;
    }

    blocks = (n+ blocksize -1)/blocksize;
    printf("No. of blocks needee = %ld\nThe size of the float is: %zu\nThe total bytes is:%zu\n ", blocks, sizeof(float), 3*n*sizeof(float) );


    for (size_t i = 0; i < blocks; i++){
        for (size_t k = 0; k  < blocksize; k++){
            vecadd_kernel(ptra, ptrb, ptrc, n, i, blocksize, k);
        }
    }

    for (size_t l = 0; l < n; l++){

        if (ptrc[l] != ptrb[l] + ptra[l]){
            fprintf(stderr, "error: mismatch at index %zu: got %f, expected %f\n",
                    l, ptrc[l], ptra[l] + ptrb[l]);
            free(ptra);
            free(ptrb);
            free(ptrc);

            return EXIT_FAILURE;
        }

    }

    printf("PASS\n");

    free(ptra);
    free(ptrb);
    free(ptrc);


    return 0;   
}