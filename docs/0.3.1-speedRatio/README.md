# speedRatio

The power ratio is the $RMS$ voltage divided by the maximum $RMS$ voltage, which is $220V$.
However, the motor cannot spin if the power ratio is not more than $\approx36\%$, and the spin is maximum when $P_r \approx98.7\%$

The Speed Ratio ($S_r$) is just the Power Ratio, but normalized so that $P_r= 35\%$ when $S_r= 0\%$ and $P_r = 98\%$ when $S_r = 100\%$.

$S_r$ is linear when compared to $P_r$.

This version considers the user input "Power Ratio" not as $P_r$, but as $S_r$. Then it is converted to the corresponding $P_r$ and applied to the load.