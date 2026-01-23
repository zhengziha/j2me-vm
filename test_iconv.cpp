#include <iostream>
#include <iconv.h>
#include <vector>
#include <cstring>

int main() {
    iconv_t cd = iconv_open("UTF-16LE", "GBK");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return 1;
    }

    // "中" in GBK: D6 D0
    char input[] = "\xD6\xD0";
    size_t inbytesleft = 2;
    char* inbuf = input;

    char output[10];
    size_t outbytesleft = 10;
    char* outbuf = output;

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)-1) {
        perror("iconv");
        return 1;
    }

    uint16_t* res = (uint16_t*)output;
    printf("Result: %04X\n", *res); // Should be 4E2D for "中"

    iconv_close(cd);
    return 0;
}
