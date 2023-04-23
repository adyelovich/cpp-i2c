#include <iostream>

#include <cstdio>
#include "driver.h"
#include "PCA9685.h"

using namespace std;

int main() {
    PCA9685 pwm_hat = PCA9685();
    int pwm;

    do {
        printf("enter pwm: ");
        cin >> pwm;
        pwm_hat.write_pwm(pwm, PCA9685::PWM_LED[0]);
    } while (true);
    
    return 0;
}
