#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <errno.h>

bool parse_positive(const char *text, size_t *out){
        char *end;
        errno = 0;
        long value = strtol(text, &end, 10);

        if (end == text) return false; 
        if (*end != '\0') return false;

        if (errno == ERANGE) return false;
        if (value <= 0) return false;

        *out = (size_t)value;

        return true;
    }

void transpose(const float *tmem_in, float *tmem_out, size_t rows, size_t cols){

    for (size_t r = 0; r <rows; r++){
        for (size_t c=0; c <cols; c++){
            tmem_out[c* rows + r] = tmem_in[r * cols + c];

        }
    }

}

void print_matrix(const float *tmem, size_t rows, size_t cols){ 

    for (size_t r = 0; r <rows; r++){
        for (size_t c=0; c <cols; c++){
            printf("%d ", (int)tmem[r * cols + c]);

        }
        printf("\n");
    }



}

bool check_matrix(const float *tmem_in, const float *tmem_out, size_t rows, size_t cols){
    for (size_t r = 0; r <rows; r++){
        for (size_t c=0; c <cols; c++){
            if (tmem_out[c* rows + r] != tmem_in[r * cols + c]){
                return false;
            }
            }

        }
        return true;
    }
    
    


int main(int argc, char *argv[]) {


     if (argc !=3){
        fprintf(stderr, "Usage: %s <rows> <cols>\n", argv[0]);

        return EXIT_FAILURE;
    }

    size_t rows, cols;

    if (!parse_positive(argv[1], &rows)) {
        fprintf(stderr, "error: row must be a positive whole number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (!parse_positive(argv[2], &cols)) {
        fprintf(stderr, "error: column must be a positive whole number, got '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }

    if (cols > SIZE_MAX/(sizeof(float))){
        fprintf(stderr, "Too big, mission abort!!\n");
        return EXIT_FAILURE;
    }
    if (rows > SIZE_MAX/(cols*sizeof(float))){
        fprintf(stderr, "Too big, mission abort!!\n");
        return EXIT_FAILURE;
    }

    float *tmem_in = NULL;
    float *tmem_out = NULL;

    tmem_in = malloc(rows*cols*sizeof(float));
    tmem_out = malloc(rows*cols*sizeof(float));

    if ((tmem_in == NULL) || (tmem_out == NULL)){
        fprintf(stderr, "Out of memory error\n");
        free(tmem_in);
        free(tmem_out);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i <rows; i++){
        for (size_t j=0; j <cols; j++){
            tmem_in[i*cols +j] = i * cols + j;

        }
    }

    transpose(tmem_in, tmem_out, rows, cols);
    print_matrix(tmem_in, rows, cols);
    print_matrix(tmem_out, cols, rows);

    if (!check_matrix(tmem_in, tmem_out, rows, cols)){
        fprintf(stderr, "fail\n");
        free(tmem_in);
        free(tmem_out);
        return  EXIT_FAILURE;
        
    }else{
        printf("PASS\n");
    }
    

    printf("rows_in: %zu, columns_in: %zu\n", rows, cols);
    printf("Size of in: %zu\n", sizeof(float) * rows * cols);

    free(tmem_in);
    free(tmem_out);

    return 0;


}