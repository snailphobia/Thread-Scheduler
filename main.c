#include "utils.h"

int32_t main(int32_t argc, char* argv[]) {
    char* cmd = calloc(S1, sizeof(char));
    if (argc >= 3) {
        snprintf(cmd, S1, "./tema2 < %s > %s", argv[1], argv[2]);
        int8_t sign = system(cmd);
        free(cmd);
        exit(0);
    }
    
    static int32_t C = 0, Q = 0;
    int8_t sign = scanf("%d%d", &Q, &C);

    parser(Q, 2 * C);
    free(cmd);
    // skipping valgrind is not a hobby, it's a lifestyle
    return 0;
}