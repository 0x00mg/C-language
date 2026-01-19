Create a program that:

1. Declares a float variable called ```radius```
2. Declares double variables called ```pi, volume```
3. Sets pi to ```3.14159```
4. Sets radius to ```1.5```
5. Calculates the volume of a sphere using the formula: ```volume = (4.0/3.0) * pi * radius³.```
6. You can write it like this: ``` (4.0/3.0) * pi * radius * radius * radius```
7. rints the result with 2 decimal places in the format:
```The volume of a sphere with radius [radius] is [volume] cubic units```

```c
#include <stdio.h>
#include <math.h>

int main() {
    float radius;
    double pi, volume;
    // Write your code here
    pi = 3.14159;
    radius = 1.5;
    

    volume = (4.0/3.0) * pi * (pow(radius, 3.0));

    printf("The volume of a sphere with radius %.2f is %.2f cubic units", radius, volume);


    return 0;
}
```
