# overcurrentProtection

The solution found for the issue reported in 0.5.3 was to use a debouncing technique, as if the alert was a physical button.

The overcurrent protection works as requested. If the motor current becomes more than twice its nominal current, it is imediatelly turned off. During startup, if the current is more than $1.5$ times the nominal current, the trigger angle (and effectively the Power Ratio) regresses until it's not anymore, so it'll never surpass this threshold during the rising ramp.

When a falling edge of an alert is detected, samples of the signal input start to be measured continuously. If any of them show that the alert is not active anymore, that alert is ignored. Only if, during a certain amount of time (determined by the number in the $if(stopAlertCounter>100000)$ condition), all samples result in an "alert active" state, will it be considered true and handled in the code.

It's important to note that the overcurrent protection only works during the ramps and between them. In $Power Ratio$ setting mode there is no overcurrent protection.