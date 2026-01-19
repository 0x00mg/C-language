Float - Double

Float and double are data types used to store decimal numbers in C.

Declare a float variable:
```c
float price = 19.99f;
```
Notice the 'f' suffix, which tells C this is a float value.

Declare a double variable:
```c
double pi = 3.14159265359;
```
The main differences between float and double:

  1.Precision: Double has higher precision than float
        Float: ~7 decimal digits
        Double: ~15 decimal digits
  2.Size:
        Float: 4 bytes
        Double: 8 bytes
  3.Range:
        Float: 1.2E-38 to 3.4E+38
        Double: 2.3E-308 to 1.7E+308

Print a float value:
```c
float temperature = 98.6f;
printf("Temperature is %f degrees\n", temperature);
```
Print with specific decimal places:
```c
printf("Temperature is %.1f degrees\n", temperature);
```
This will show: “Temperature is 98.6 degrees”

Challenge
Easy

Write a program that:

  1. Declares a float variable named ```celsius```
  2. Declares a double variable named ```fahrenheit```
  3. Assigns the value 25.0 to ```celsius```
  4. Converts celsius to fahrenheit using the formula: ```fahrenheit = (celsius * 9.0/5.0) + 32.0```
  5. Prints the result as
   ```25.0 degrees Celsius is equal to 77.0 degrees Fahrenheit```

Make sure to display precisely one decimal place in your output.

To print multiple variables in a single ```printf``` use the following format:

```printf("Value1: %.1f and Value2: %.1f", variable1, variable2);```

```c
#include <stdio.h>

int main() {
    // Write your code here
    float celsius = 25.0f;
    double fahrenheit = (celsius*9.0/5.0)+32.0;
    printf("%.1f degrees Celsius is equal to %.1f degrees Fahrenheit", celsius, fahrenheit);
    return 0;
```
