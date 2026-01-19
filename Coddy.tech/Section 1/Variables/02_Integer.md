Integer

In C, integers are whole numbers without any decimal points. They are one of the most common data types you'll work with.

Declaring, initializing, and modifying integers:
```c
int age; // Declaring
int score = 100; // Declaring and initializing
score = 90; // Modifying
```
Print an integer using printf:
```c
printf("%d", score);
```
The int type stores whole numbers.

Challenge
Easy

Create a program that:

1. Declares an integer variable named number
2. Assigns the value 50 to this variable
3. Prints the value of the variable using printf

Your output should look like this:

```The value is: 50```

```c
#include <stdio.h>

int main() {
    // Declare an integer variable named 'number'
    int number;
    // Assign the value 50 to the variable
    number = 50;
    // Print the value using printf
    printf("The value is: %d\n", number);
    return 0;
}
```
