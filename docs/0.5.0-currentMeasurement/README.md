# currentMeasurement

In order to build the overcurrent protection circuit, the first step was to measure the current through the motor. The method used was with a shunt resistor:

![alt text](diagram.png)

## Results

The $V_{AN}$ was measured for different $S_r$ values:

### $S_r=10\%$

![alt text](Sr_10.JPG)

### $S_r=20\%$

![alt text](Sr_20.JPG)

### $S_r=30\%$

![alt text](Sr_30.JPG)

### $S_r=50\%$

![alt text](Sr_50.JPG)

### $S_r=70\%$

![alt text](Sr_70.JPG)

### $S_r=100\%$

![alt text](Sr_100.JPG)

### $S_r=100\%; I=1.5I_{typ}$

![alt text](1.5I.JPG)
$V_{shunt_{peak}}=360mV$

### $S_r=100\%; I=2I_{typ}$

![alt text](2I.JPG)
$V_{shunt_{peak}}=280mV$

## Discussion

If we can measure the current discretely frequently enough, we can just check if it's greater than a desired threshold to take action.

The typical motor current is $500mA$. The system has to shut down if it's more than $1A$, or pull the trigger back if more than $750mA$ ($2\cdot I_{typ}$ ; $1.5\cdot I_{typ}$ - project criteria).