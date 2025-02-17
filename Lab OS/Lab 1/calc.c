#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <string.h>
#include <stdlib.h>

float ANS=0;

void solve(char s[100]) {
    int cong = 0, tru = 0, nhan = 0, chia = 0, pow10x = 1, pow10y = 1, cntx = 0, cnty = 0, checkANSX = 0, checkANSY = 0;
    float x = 0, y = 0;

    for(int i = 0; i < (int)strlen(s); i ++) {
        if(s[i] == 'A' || s[i] == 'N' || s[i] == 'S') {
            x = ANS;
            checkANSX = 1;
        } else if(s[i] == '+') {
            cong = 1;
        } else if(s[i] == '-') {
            tru = 1;
        } else if(s[i] == '*') {
            nhan = 1;
        } else if(s[i] == '/') {
            chia = 1;
        } else if(s[i] != ' ') {
            x *= 10;
            x += s[i] - '0';
            if(cntx == 1) pow10x *= 10;
        } else if(s[i] == '.') {
            cntx = 1;
        }

        if(cong + tru + nhan + chia == 1) {
            i++;
            for(; i < (int)strlen(s); i ++) {
                if(s[i] == 'A' || s[i] == 'N' || s[i] == 'S') {
                    y = ANS;
                    checkANSY = 1;
                } else if(s[i] >= '0' && s[i] <= '9') {
                    y *= 10;
                    y += s[i] - '0';
                    if(cnty == 1) pow10y *= 10;
                } else if(s[i] == '.') {
                    cnty = 1;                    
                }
            }

            break;
        }
    }

    if(cong + tru + nhan + chia == 0) {
        printf("SYNTAX ERROR\n");
        return;
    }

    if(y == 0) {
        printf("MATH ERROR\n");
        return;
    }

    if(checkANSX != 1) x /= pow10x;
    if(checkANSY != 1) y /= pow10y;

    if(cong) {
        ANS = x + y;
        printf("%.2f", ANS);
    } else if(tru) {
        ANS = x - y;
        printf("%.2f", ANS);
    } else if(nhan) {
        ANS = x * y;
        printf("%.2f", ANS);
    } else if(chia) {
        ANS = x / y;
        printf("%.2f", ANS);
    }
    printf("\n");
}

int main() {
    while(1 == 1) {
        printf(">> ");
        char a[100];
        fgets(a, 100, stdin);
        if(a[0] == 'E') break;
        solve(a);
    }

    return 0;
}