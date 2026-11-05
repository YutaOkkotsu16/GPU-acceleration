#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h> 
#include <stdint.h> 
#include <string.h>

bool parse_float(const char *text, float *out){
        char *end;
        errno = 0;
        float value = strtof(text, &end);

        if (end == text) return false; 
        if (*end != '\0') return false;

        if (errno == ERANGE) return false;

        *out = value;
        
        return true;
    }
void f_to_b(const float fl, uint16_t *bl){
    uint32_t new;
    uint16_t convb;
    memcpy(&new, &fl, sizeof new);
    convb = new >> 16;
    printf("converted bf16: %04x\n", convb);

    *bl = convb;

}

void b_to_f(const uint16_t convb, float *f_fin){
    uint32_t fl;
    fl = ((uint32_t)convb << 16);
    float fl1;
    memcpy(&fl1, &fl, sizeof fl1);
    printf("converted back to float: %.9g\n", fl1);

    *f_fin = fl1;



}
int main(int argc, char *argv[]) {

    if (argc !=2){
        fprintf(stderr, "Usage: %s <Float no.> \n", argv[0]);

        return EXIT_FAILURE;
    }


    float n;
    uint16_t my_b16;
    uint32_t my_int;
    float new_n;

    if (!parse_float(argv[1], &n)) {
        fprintf(stderr, "error: N must be a number, got '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    memcpy(&my_int, &n, sizeof my_int);
    printf("Number in hex: %08X\n", my_int);

    printf("Sign:  %u\n", my_int >> 31);

    printf("Mantissa:  %06X\n", (my_int & ((1u <<23) -1)));    
    printf("Stored Exponent %d\n", (my_int >> 23) & ((1u <<8) -1));
    printf("Exponent:  %d\n", (int32_t)((my_int>> 23) & ((1u <<8) -1)) - 127);  
    printf("Original Float: %.9g\n",n);
    f_to_b(n, &my_b16);
    b_to_f(my_b16, &new_n);
    printf("Error: %.9g\n", n-new_n);

    
    

}
