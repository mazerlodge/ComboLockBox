S1 ReadMe - ComboBox

Combo box is an Arduino program that operates as an electronic lock.

The electronics:
------------------
- Arduino Uno board
- 6 LED lights (colors optional)
	- 4 combo digit lights (red)
	- 1 high bank indicator light (yellow)
	- 1 low bank indicator light (green)
	
- 4 push buttons (spring loaded momentary on) 
- 1 analog dial (potentiometer 0-1024 analog input range)
- Resistors 
	- 6 Yellow-Violet-Brown-Gold (470 Ohm, 5% tolerance), used with LEDs
	- 4 Brown-Black-Orange-Gold (10k Ohm, 5% tolerance), used with pushbuttons (pull-down)


How it works:
--------------
- The default combo is 42

- Enter a combo in binary
	-- Green light indicates "Low Bank" (numbers 1, 2, 4, 8)
	-- Yellow light indicates "High Bank" (numbers 16, 32, 64, 128)
	
- Turn the analog input to:
 	-- Full right, check combo 
 	-- Full left, enter mode to set combination
	-- to the right to activate the low bank
	-- to the left to activate the high bank
	

