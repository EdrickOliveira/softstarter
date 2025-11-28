# setPowerRatio

This version implements the possibility of changing the trigger angle (which controls the power ratio) mid-execution (only once - loop has to be implemented)

The UI has also started to be built.

It still hasn't been thought about protection in case the user types inputs without being prompted.

Yet, a menu is displayed on the console, prompting the user to pick an option. Only the "set power ratio" option is availiable. The user types the Power Ratio (in percentage), and then 'enter' (except if it is 100% - then they just need to type "100"). The corresponding trigger angle is calculated, converter to CCR and applied to the timer.

After testing multiple $P_r$ values, the scope analysis showed that it works just fine.