Create a program that:
    Declares a float variable called ```radius```
    Declares double variables called ```pi, volume```
    Sets pi to ```3.14159```
    Sets radius to ```1.5```
    Calculates the volume of a sphere using the formula: ```volume = (4.0/3.0) * pi * radius³.```
    You can write it like this: ``` (4.0/3.0) * pi * radius * radius * radius```
    Prints the result with 2 decimal places in the format:
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

    ```
