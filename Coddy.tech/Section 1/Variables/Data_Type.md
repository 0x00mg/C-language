Characters

In C, the char data type is used to store a single character. Characters in C are enclosed in single quotes.

Declare a character variable:
```char letter;```

Assign a character to the variable:
```letter = 'A';```

You can also declare and initialize in one line:
```char grade = 'B';```

A character occupies 1 byte of memory and is actually stored as an integer representing its ASCII value. For example, 'A' is stored as 65, 'B' as 66, and so on.

You can print a character using the %c format specifier:

```printf("The letter is: %c\n", letter);```

Challenge
Easy

Create a program that:

    Declares a character variable named symbol
    Assigns the character '@' to this variable
    Prints the message "The symbol is: @" where @ is the value of your variable
```c
#include <stdio.h>

int main() {
    // Write your code here
    char symbol = '@';
    printf("The symbol is: %c", symbol);



    return 0;
}
```
