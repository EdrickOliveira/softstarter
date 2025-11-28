#include <math.h>
#include "powerToAngle.h"

#define INITIAL_GUESS (M_PI/2) //guess pi/2 (half-wave) as the triggering angle. This gives the maximum first guess derivative, which helps convergence.

float getTrigger(float powerRatio);
float newtonMethod(float triggerAngle, float powerRatio);
float function(float x, float Pr);
float functionSlope(float x);
int angleToCCR(float angle, int arr);

////////////////////////////////////////////////////////////////////////////////////////

/*
This function uses Newton's method to find the angle of triggering based on the power ratio.
It can't be found with algebra (transcendental equation), so we use an iterative method to find the root of the function.
*/
float getTrigger(float powerRatio){
    if (powerRatio == 1) return 0; //if power ratio is 1, the trigger angle is 0

    float triggerAngle = INITIAL_GUESS;

    do {
        triggerAngle = newtonMethod(triggerAngle, powerRatio);
        if (fabs(function(triggerAngle, powerRatio)) < 0.001) {
            break;
        }
    } while (1);

    return triggerAngle;
}

float newtonMethod(float triggerAngle, float powerRatio){
    float derivative = functionSlope(triggerAngle);
    return triggerAngle - function(triggerAngle, powerRatio) / derivative;
}

/*
Function whose root is the trigger angle.
The function is defined as:
sin(2*x) - 2*x + C = 0. C is defined in "triggerAngle calculation"

This is the equation we want to solve for x,
where Pr is the power ratio.

The function returns the value of the function at x.
*/
float function(float x, float Pr){
    double C;

    C = 22*M_PI/9;
    C -= 2*M_PI*pow(Pr, 2);
    C -= sin(4*M_PI/9);

    return (float)(sin(2*x) -2*x + C); //return the value of the function at x
}


/*
Function's derivative, which is used in Newton's method.
The derivative is defined as:
f'(x) = 2*cos(2*x) - 2

This is the slope of the function at x.
*/
float functionSlope(float x){
    return 2*cos(2*x)-2;    //return the derivative of the function at x
}

/*
Take the trigger angle in radians and convert it to the CCR of TIM1,
so its Output Compare Channel pulses with the desired delay (angle phase) from zeroDetector

The relationship between the angle and the CCR is linear.
With CCR=0 the phase between the zeroDetector and the Output Compare Channel is 0 degrees.
With CCR=ARR the pulse takes a whole period to occur (8.33ms).
For a phase 'x' between zeroDetector and the trigger pulse, CCR/ARR has to equal x/pi
*/
int angleToCCR(float angle, int arr){
    float ccr;

    ccr = angle/M_PI;
    ccr *= arr;
    ccr = round(ccr);
    if(ccr>arr)	ccr=arr;

    return ccr;
}
