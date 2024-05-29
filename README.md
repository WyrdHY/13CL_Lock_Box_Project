# Laser Lock Box Project
_Software and documentation developed by Alexander Nazeeri&Zhaozhong Cao (alexandernazeeri@ucsb.edu&zhaozhongcao@umail.ucsb.edu)_  
_Hardware made by Hongrui Yan(hongrui_yan@ucsb.edu_  


This repository contains all the software that is needed to operate the laser lock box based on TEENSY 4.1. In this document a complete overview is given of how to use the software and how the backend software is structured.

<div style="border: 5px solid red; padding: 5px;">
    <span style="color: red;"><b>NOTE:</b></span> This setup involves a CW laser operating at 976nm. Though working at invisible band, please still be careful with your eyes.
</div>
<br>

This code is developed in Arduino IDE and in the language of C++. The code is summarized into three parts: 1) Use ADC to read the error signal, 2) Apply PID to the error signal, 3) Use DAC to output the control signal


## How to setup the software
One has to realize that this software is not a complete, one click compiled program. I recommend to run and compile on the TEENSYduino, Version 1.59. This is an add-on to the Arduino IDE and the detailed installation can be found at https://www.pjrc.com/teensy/td_download.html



<details>  
  <summary><b>Step 1: clone repo</b></summary>

In order to run the code on a new computer, first clone the GitHub repository. If you do not have git installed, please download it here: https://git-scm.com/downloads 
</details>

<br>

<details>  
  <summary><b>Step 2: setup the environment</b></summary>
    under construction


</details>
<br>
<details>  
  <summary><b>Step 3: run program</b></summary>

Under Construction

</details>

<div style="page-break-after: always;"></div>

## Break Down 

Code will be explained here

### DAC 


<details>
  <summary><b>Hardware Selection</b></summary>

To achieve a good cavity locking, we require a precise output of the control signal.  We are using the evaluation EVAL-AD5791SDZ as the external DAC module. The central chip is AD5791, a powerful chip performing 1 ppm, 20-Bit, ±1 LSB INL, Voltage Output DAC.
<br>
The communication between the EVAL-AD5791SDZ and TEENSY 4.1 is through SPI Mode_1. 

</details>

<br>

<details>
  <summary><b>Step 1: Setup the power supply connection</b></summary>

  <div style="border: 5px solid red; padding: 5px;">
    <span style="color: red;"><b>WARNING:</b></span> Please, do not, do not, do not use your computer USB to power this board! Please use an external voltage supply to drive it. 
</div>
<br>
The evaluation board offers three ways for power supply: 1)ADP 5070 with LDOS 2) ADP 5070 3) Bench Supply. In order to select the way of power supply, there are multiple physical switches on the board that must be inserted or removed. Since I am using ADP 5070 with LDOs, I will put my connection here: 
<br>

![Schematic Power Connection](Miscellaneous/Sch_power.png)

![Schematic Power Connection](Miscellaneous/sc)

![Schematic Power Connection](Miscellaneous/power_choice.png)

</details>

<br>

<details>
<summary><b>Step 2: Important Pins before communication</b></summary>
    
- CLR: Active when it is LOW. This will set the output of the DAC to a default value. This pin must be set to HIGH to avoid constantly clearing the output.
- RESET: Active when it is LOW. This will reset the DAC back to the power-on state. This pin must be set to HIGH to avoid constantly reseting.
- SDIN: Serial Data Input. Connect it to MOSI for SPI.
- SYNC: Don't be confused by the name. It serves as the function of CS. This must be set to LOW for the DAC board to start communication and set to HIGH when the communication is over. At HIGH, the DAC board will refuse to communicate through SPI.
- SCLK: Clock for SPI
- SDO: Serial Data Output. Connect it to MISO for SPI.
- LDAC: Active when it is LOW. When you write to the DAC register, the previous value will not be erased. Only when the LDAC is active, the old value will be erased and the new-written value starts to become the true output. To perform a real time update, LDAC must be set to LOW.


</details>

<br>

<details>
  <summary><b>Step 3: Setup the Control Register</b></summary>
Below is a summary of the 24-bit register settings for the DAC:

| Bit(s) | Name       | Description                                                     | Setting | Function                                                      |
|--------|------------|-----------------------------------------------------------------|---------|---------------------------------------------------------------|
| 23      | RBUF       | Output amplifier configuration control.                         | 0       | Internal amplifier powered up.                                |
|        |            |                                                                 | 1       | Internal amplifier powered down (default).                    |
| 22 | OPGND      | Output ground clamp control.                                    | 0       | DAC output clamp to ground removed, DAC in normal mode.       |
|        |            |                                                                 | 1       | DAC output clamped to ground, DAC in tristate mode (default). |
| 21    | DACTRI     | DAC tristate control.                                           | 0       | DAC in normal operating mode.                                 |
|        |            |                                                                 | 1       | DAC in tristate mode. Tri state means that the output can exists in three different states:high.low, and high impedance  (default).                               |
| 20      | BIN/2sC    | DAC register coding selection.                                  | 0       | Uses twos complement coding (default).                        |
|        |            |                                                                 | 1       | Uses offset binary coding.                                    |
| 19      | SDODIS     | SDO pin enable/disable control.                                 | 0       | SDO pin enabled (default).                                    |
|        |            |                                                                 | 1       | SDO pin disabled (tristate).                                  |
| 15 to 18| LIN COMP   | Linearity error compensation for varying reference input spans. | 0000    | Reference input span up to 10 V (default).                    |
|        |            |                                                                 | 1001    | Reference input span between 10 V and 12 V.                   |
|        |            |                                                                 | 1010    | Reference input span between 12 V and 16 V.                   |
|        |            |                                                                 | 1011    | Reference input span between 16 V and 19 V.                   |
|        |            |                                                                 | 1100    | Reference input span between 19 V and 20 V.                   |
| 1     | R/overline{W} | Read/write select bit.                                       | 0       | Addressed for a write operation.                              |
|        |            |     |1 | Addressed for a read operation |        
|2 to 4 | Select which register to talk to| |001 | DAC|
| || |010 | Control register|
|Other bits||||They are useless and must be set to 0|

<br>

</details>

<br>
<details>
  <summary><b>Step 4: Setup the DAC Register</b></summary>
  <p>The transfer function for this board is:</p>

  $$
  V_{out}(x) = \frac{VrefP-VrefN}{2^{20} - 1} \cdot x + VrefN = \frac{10+10}{2^{20} - 1} \cdot x + 10
  $$

  <p>The DAC register configuration is straightforward:</p>

  - **bit 1**: Write or read.
  - **bits 2 to 4**: Select which register to communicate with.
  - **bits 5 to 24**: These 20 bits are the binary number for \( x \).

</details>

## Code Structure

<details>
  <summary><b>Overview</b></summary>

Performing cavity locking on a DSP board suffers from a variety of limitations. The most crucial one is handling multiple tasks simultaneously. In short, cavity locking can be summarized as:
1. Scan
2. Read error signal
3. Apply PID on error signal
4. Output the control signal

This is achieved by using multiple interrupts:

### CPU Timer 1
- **Function**: Triggers an interrupt regularly every 10 µs.
- **Purpose**: Used for scanning to produce a triangular wave. The frequency of the triangular wave is tuned by the clock frequency, and the amplitude is adjusted by changing the voltage level written to the DAC board.

### ADC ISR Interrupt
- **Function**: Triggers an interrupt when the ADC sampling is ready.
- **Purpose**: Used to correct the control signal and also serves as a long-term measurement of the voltage variation during locking.

### Serial Interrupt
- **Function**: Mimics the effect of a physical interrupt.
- **Purpose**: During locking, the user typically needs to communicate with the Teensy to set parameters such as the set point, PID coefficients, and commands to start/stop locking and scanning. A custom instruction set is sent through serial port communication with the Teensy.

### CPU Timer 2
- **Function**: Triggers an interrupt regularly every 1000 µs (1 ms), tunable down to 1 µs.
- **Purpose**: Handles the PID control. Within this ISR, it stores the current error for integration and outputs the control signal.


### CPU Timer 3(under construction)
- **Fuction**: This interrupt is for tunning the PID during the locking process
-**Purpose**: When locking to different peaks, a different set of PID parameters is needed. However, there is not way to theoretically calculate the prcise value for PID before you experiment it. Right now user have to relaunch the TEENSY to update the PID coefficients but for effectively tunning, it is good to have another interrupt to handle this. 
</details>


<details>
  <summary><b>Instruction Set</b></summary>
  Current instruction sets are compatible with: 

  Toggle:x; where x is 1 or 0. This can turns on/off a triangular wave

  Set:x; where x is a voltage level and for safety reason, if the user enters a value with abs(x)>0, this will force the voltage to set back to 0V. 

  PID:P,I,D; this command will tell the TEENSY to begin locking to: Kp*e + Kd*de/dt+Ki*int e(t) dt. Notice that if you do not previously set a level, this will use 0 as a setpoint. In most of the cases, the setpoint depends on the voltage level of your peak, and they are not zero. To find the voltage level of the setpoint, Johnson has wrote a python script that takes a screenshot of the oscilloscope with channel 1 as the transmission signal and the channel 2 as the scanning triangular wave. 
</details>


