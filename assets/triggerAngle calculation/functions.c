#include <stdio.h>
#include <math.h>

#define INITIAL_GUESS (M_PI/2) //guess pi/2 (half-wave) as the triggering angle. This gives the maximum first guess derivative, which helps convergence.

float getTrigger(float powerRatio, float guess);
float function(float x, float Pr);
float functionSlope(float x);

int main(void) {
    float powerRatio = 1;
    
    while(powerRatio>0){
        scanf("%f", &powerRatio);
        printf("Power ratio = %f\nTrigger = %f\n\n", powerRatio, getTrigger(powerRatio, INITIAL_GUESS));
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

/*
This function uses Newton's method to find the angle of triggering based on the power ratio.
It can't be found with algebra (transcendental equation), so we use an iterative method to find the root of the function.

Original equation: sin(2*trigger)-2*trigger = 2pi(powerRatio^2-1)
*/
float getTrigger(float powerRatio, float guess){
    float x0, x1, derivative;
    
    if(powerRatio==1)   return 0; //if power ratio is 1, the trigger angle is 0

    x0 = guess;
    derivative = functionSlope(x0); //calculate P'(x0)
    x1 = x0-function(x0, powerRatio)/derivative; //calculate the next guess using Newton's method

    if(fabs(function(x1, powerRatio))<0.001) { //check if this guess is close enough to the root
        return x1; //if so, return this guess
    }
    else {
        return getTrigger(powerRatio, x1); //if not, retry using x1 as the initial guess
    }
}

float function(float x, float Pr){
    float C;
    
    C = -2*M_PI*(pow(Pr, 2)-1);

    return sin(2*x) -2*x + C; //return the value of the function at x
}

float functionSlope(float x){
    return 2*cos(2*x)-2;    //return the derivative of the function at x
}
