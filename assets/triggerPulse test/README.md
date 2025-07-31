# triggerPulse

In the softstarter, a pulse should occur in sync with the zeroDetector, but with phase/delay.

This simulates a zero detector signal through TIM10's PWM channel, which triggers TIM1's output compare to start.

The initial CCR2 value of TIM1 determines the phase between the zero detector and the trigger pulse, thus controling the average voltage. In this example, it's set to 1.43ms (10000 ticks) after zero detector's rising edge.

The difference between the second and initial CCR2 value sets the trigger pulse width. In this example, it's set to 1ms (7000 ticks).

## Ports

![Circuit Diagram](circuit.png)

### PB8
TIM10 PWM channel with 120Hz, 6% duty cycle (500us HIGH). This is a simulation of the zero detector signal.

### PA8
TIM1 trigger input channel. When a rising edge event happens, TIM1's output capture starts (one pulse mode - CNT counts up to ARR then stops, waiting for another trigger event).

### PA9
TIM1 output compare channel. When TIM1->CNT (which increases at 7MHz) matches TIM1->CCR2, the channel (PA9) toggles.

## Output capture interruption
When the OC channel toggles, an interruption happens, changing CCR2 to 17000. This generates a pulse of 7000 CNT ticks (17000-10000), or 1ms at 7MHz. CCR2 is reset to 10000 if it's already 17000. This reloads the pulse generation.