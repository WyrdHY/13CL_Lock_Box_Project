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
![Schematic Power Connection](13CL_Lock_Box_Project/Miscellaneous
/Sch_power.png
)
</details>

<br>

<details>
<summary><b>Step 2: Important Pins before communication</b></summary>



</details>

<br>

<details>
  <summary><b>Step 3: Setup the Control Register</b></summary>


<br>

</details>

<br>
<details>
  <summary><b>Step 4: Setup the DAC Register</b></summary>

</details>

