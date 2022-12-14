#include <mbed.h>
#include "MyLCD.h"
#include <string>
#include <charconv>
#include "Dht11.h"
int main()
{
    MyLCD myLCD;
    myLCD.run(); //RUN the program indefinitly

    return 0;
}
