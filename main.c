#include "utils.h"

int32_t taskN;
int32_t main(int32_t argc, char* argv[]) {
    char* cmd = calloc(S1, sizeof(char));
    if (argc >= 3) {
        snprintf(cmd, S1, "./main < %s > %s", argv[1], argv[2]);
        int8_t sign = system(cmd);
        free(cmd);
        exit(0);
    }
    
    static int32_t C = 0, Q = 0;
    int8_t sign = scanf("%d%d", &Q, &C);

    parser(Q, 2 * C);
    return 0;
}