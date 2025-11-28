/*
 * powerToAngle.h
 *
 * Header file for power triggering angle calculations.
 */

#ifndef POWERTOANGLE_H
#define POWERTOANGLE_H

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes
float getTrigger(float powerRatio);
float newtonMethod(float triggerAngle, float powerRatio);
float function(float x, float Pr);
float functionSlope(float x);
int angleToCCR(float angle, int arr);

#ifdef __cplusplus
}
#endif

#endif // POWERTOANGLE_H
