#include"server.h"
#include"server.c"

int tests()
{
    char * tests[6];
    tests[0] = "US Dollar";
    tests[1] = "Canadian Dollar";
    tests[2] = "Euro";
    tests[3] = "British Pound";
    tests[4] = "Japanese Yen";
    tests[5] = "Swiss Franc";
   
    char* passwords[6]; 
    passwords[0] = "uCh781fY";
    passwords[1] = "Cfw61RqV";
    passwords[2] = "Pd82bG57";
    passwords[3] = "Crc51RqV";
    passwords[4] = "wD82bV67";
    passwords[5] = "G6M7p8az";

   for (int i = 0; i < 6; ++i)
   {
       printf("Testing: %s Index: %i\n",tests[i],checkCurrency(tests[i]));
   }

   char* passTests;

   for (int i = 0; i < 6; ++i)
   {
       passTests = checkPassword(checkCurrency(tests[i]),passwords[i]);
       printf("Testing: %i, %s\n",checkCurrency(tests[i]),passwords[i]);
       printf("Result: %s\n", passTests);
       free(passTests);
   }

   return 0;
}

int main(int argc, char** argv)
{
    tests();
    return EXIT_SUCCESS;
}
