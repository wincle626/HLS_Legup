#pragma map generate_hw 1
unsigned long int sqrt(unsigned long int a)
{
        unsigned long int x, r, h, x2;
        x = a;
        r = ((2147483647L * 2UL + 1UL) >> 2) + 1;
        h = ((2147483647L * 2UL + 1UL) >> 3) + 1;
        x2 = ((2147483647L * 2UL + 1UL) >> 2) + 1;
        for(; h != 0; h>>=2)
        {
                if( x >= x2 )
                {
                        x2 = x2 + r + (h >> 1);
                        r = (r+h) >> 1;
                }
                else
                {
                        x2 = x2 - r + (h >> 1);
                        r = (r-h) >> 1;
                }
        }
        if( x <= (x2-r) )
                r--;
        else if( x > (x2+r) )
                r++;
        return r;
}
