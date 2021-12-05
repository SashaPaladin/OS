#include <iostream>

extern "C" int PrimeCount(int A, int B);
extern "C" float SinIntegral(float A, float B, float e);

int main(){
    int command;
    while((std::cout << "Enter command: ") && (std::cin >> command)){
        if(command == 1){
            std::cout << "Enter A and B: ";
            int a, b;
            std::cin >> a >> b;
            std::cout << "PrimeCount in [a; b] " << PrimeCount(a, b) << std::endl;
        }
        else if(command == 2){
            float A, B, e;
            std::cout << "Enter A, B, e: ";
            std::cin >> A >> B >> e;
            std::cout << "Integral value " << SinIntegral(A, B, e) << std::endl;
        }
    }
}