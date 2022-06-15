#include "RSA.h"
#include <string.h>
#include <cmath>
#include <iostream>


CRSA::CRSA(char* pMsg, int length)
{
    // 서로 같으면 안됨
    p = 11;
    q = 241;

    msg = pMsg;
    //for (i = 0; msg[i] != '\0'; i++)
        //m[i] = msg[i];
    for (i = 4; i < length; i++)
        m[i] = msg[i];
    n = p * q;
    t = (p - 1) * (q - 1);

    Ce();
}

int CRSA::Prime(int pr)
{
    int i;
    j = sqrt(pr);
    for (i = 2; i <= j; i++)
    {
        if (pr % i == 0)
            return 0;
    }
    return 1;
}

void CRSA::Ce()
{
    int k;
    k = 0;
    for (i = 2; i < t; i++)
    {
        if (t % i == 0)
            continue;
        flag = Prime(i);
        if (flag == 1 && i != p && i != q)
        {
            e[k] = i;
            flag = Cd(e[k]);
            if (flag > 0)
            {
                d[k] = flag;
                k++;
            }
            if (k == 99)
                break;
        }
    }
}
int CRSA::Cd(int x)
{
    int k = 1;
    while (1)
    {
        k = k + t;
        if (k % x == 0)
            return (k / x);
    }
}
void CRSA::Encrypt()
{
    int pt, ct, key = e[0], k, len;
    i = 0;
    len = strlen(msg);
    while (i != len)
    {
        pt = m[i];
        pt = pt - 96;
        k = 1;
        for (j = 0; j < key; j++)
        {
            k = k * pt;
            k = k % n;
        }
        temp[i] = k;
        ct = k + 96;
        en[i] = ct;
        i++;
    }
    en[i] = -1;

 ///*   
    for (i = 0; en[i] != -1; i++)
        printf("%c", en[i]);
    puts("");
    //*/
}
void CRSA::Decrypt()
{
    int pt, ct, key = d[0], k;
    i = 0;
    while (en[i] != -1)
    {
        ct = temp[i];
        k = 1;
        for (j = 0; j < key; j++)
        {
            k = k * ct;
            k = k % n;
        }
        pt = k + 96;
        m[i] = pt;
        i++;
    }
    m[i] = -1;

    for (i = 0; m[i] != -1; i++)
        printf("%c", m[i]);
}