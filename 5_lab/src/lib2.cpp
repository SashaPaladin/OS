#include <vector>
#include <cmath>

extern "C" int PrimeCount(int A, int B);
extern "C" float SinIntegral(float A, float B, float e);

int PrimeCount(int A, int B){
    if (B < 2)
        return 0;
    if(A < 2)
        A = 2;
    int n = B;
    std::vector<char> prime(n + 1, true);
    prime[0] = prime[1] = false;
    for(int i = 2; i <= n; ++i){
        if(prime[i]){
            if(i * i <= n){
                for(int j = i * i; j <= n; j += i){
                    prime[j] = false;
                }
            }
        }
    }
    int count = 0;
    for(int i = A; i <= B; ++i)
        count += prime[i];
    return count;
}

float SinIntegral(float A, float B, float e) {
    float trapezoidal_integral = 0;
    for(float step = A; step + e < B; step+= e) 
    {
        float x1 = step;
        float x2 = (step + e < B)?step+e:B;
        trapezoidal_integral += (x2-x1)*sin(x1) + 0.5*(x2-x1) * (sin(x2) - sin(x1));
    }

    return trapezoidal_integral;
}