#include <cmath>

extern "C" int PrimeCount(int A, int B);
extern "C" float SinIntegral(float A, float B, float e);

int PrimeCount(int A, int B) {
    int count = 0;
    if (B < 2)
        return 0;
    if (A < 3) {
        A = 3;
        ++count;
    }
    for (int number = A; number <= B; ++number) {
        for (int divider = 2; divider < number; ++divider) {
            if (number % divider == 0)
                break;
            if (divider == number - 1)
                ++count;
        }
    }
    return count;
}


float SinIntegral(float A, float B, float e) {
    float rectangle_integral = 0;
    for(float step = A; step + e < B; step+= e) 
    {
        float x1 = step;
        float x2 = (step + e < B)?step+e:B;
        rectangle_integral += 0.5*(x2-x1)*(sin(x1) + sin(x2));
    }

    return rectangle_integral;
}
