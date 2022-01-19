# Lab Primer

This page provides background that should help you contextualize the labs and contains a additional information about how the labs are organized. 

## Background: real-time computing

Most embedded computing tasks have a [deadline][1], but the consequences of missing this deadline vary depending on the application. This leads to a natural classification of systems based on the severity consequence for missing the deadline.

**Hard** – Missing the deadline is a total system failure. Example: deployment of a car airbag.

**Firm** – Missing the deadline makes the system useless, but there is an opportunity to retry or correct the result. Example: on a printer, missing a deadline of depositing ink may completely ruin the result, but the print can be restarted or corrected by hand.

**Soft** – Missing the deadline makes the system less useful, but it is still better than having no result at all. Example: frame update in a video game.

Systems with a deadline are said to be [real-time][2]. Systems with a soft deadline can be run on a smartphone, PC, or single-board computer running a general purpose operating system. Systems with hard or firm deadlines are typically executed on [bare metal][3]. The presence of an operating system influences the type of processor that can be used, since most operating systems require a [memory management unit][4].

| Processor Type	| Example application	| Memory Management Unit	| Approximate Power (Watts) | Approximate performance (MIPS)|
| :---        		| :----:   				| :----:   					| :----:   					| :----:						|
| ARM Cortex-M   	| Automotive			| No						| 0.1	  					| 100							|
| ARM Cortex-A   	| Smartphone			| Yes   					| 1							| 1000							|
| TI C6000     		| Medical Imaging		| No	  					| 3		   					| 5000							|
| Intel Core i5		| Desktop PC			| Yes	 					| 50	   					| 100000						|

## Background: digital signal processing

Computing tasks involving digital signal processing generally have unique demands. Applications such as speech processing, telecommunications, or video processing often require high [throughput][5] of data. Often, just moving the data between different areas of memory in the system is as expensive (if not more expensive) than the computations that need to be performed.
 
Peripherals that collect or transmit data, (ADC, DAC, camera, etc) often make use of a [direct memory access controller][6] to move data without explicitly using the processor’s load and store operations. Many high throughput DSP applications would simply not be possible without the increased efficiency of DMA.

DSP computations often require many cheap or low-precision operations that are highly parallelizable, which provides advantage to [RISC architectures][7] and specialized [coprocessors][8]. Additionally, [Single instruction, multiple data (SIMD)][9] instructions are supported on many processors which exploit parallelism.

In the lab, we will use the microcontroller’s DMA to move data between the ADC, main memory, and DAC, and we will utilize SIMD operations supported by the Cortex-M7.

## Goals of the lab

The goal of the lab component of the course is threefold:

**Implementing the algorithms** – We will implement DSP algorithms for tasks such as sinusoidal generation and digital filters which are building blocks to many different systems. We will use these building blocks to implement progressively advanced systems, culminating in the implementation of a [QAM][10] [modem][11], which is the same technology underpinning modern wireless systems like Wi-Fi.

**Understanding the theory** – The DSP theory covered elsewhere in the course can be incredibly dense. Completing the design flow by implementing the algorithms provides another angle for comprehension.

**Understanding the nuances** – We make assumptions in developing the theory that are often violated in practice. For example, floating point arithmetic introduces [numerical errors][12] which must be accounted for. The lab exercises give us an opportunity to test our assumptions and correct for them when necessary.

## Reading assignments and quizzes

The reading assignments are intended to help you get up to speed on background and terminology so that you can communicate more efficiently with your lab partner and the TA. The quiz questions will be drawn from topics covered in the reading assignment.

## Lab reports and lab partners

Each of the seven labs will be worth 40 points. Labs 2-6 follow a similar report structure. Lab 7 has an oral report only. Lab 1 is structured like a homework assignment with a set of questions to answer and turn in. Labs 1 and 7 should be completed individually. Labs 2-6 will be completed in pairs.

For Labs 2-6 each team should turn in one report consisting of the following sections:
1.	**Introduction**: Briefly explain the theory and algorithms behind the programs that you wrote. The slides and reading material might help you in writing this section.
2.	**Methods**: Describe the steps you took to implement the algorithms in your own words. 
3.	**Results**: Present the results you obtain for each task on the assignment sheet. This section should include illustrative oscilloscope screenshots of the DSP algorithms in action. Also include any code that you wrote or modified. Please do not include all of the boilerplate code from the textbook.
4.	**Discussion**: In this section, discuss the takeaway from each lab. You can mention any intuition that you developed. Also mention any problems that you faced and how you rectified them.
5.	**Assignment questions**: Please answer the questions asked in the assignment.

The goal of the report is to help cement what you learned in memory. For sections 1, 2, and 4, imagine your audience is a student who is your peer but who has not yet completed the lab.

## Instructions

Each lab is accompanied by a set of instructions for tasks you should complete. For labs that extend beyond one week, the instructions are divided accordingly. The instructions are designed to be completed within the three hours allocated for lab each week. 

Any code, plots, oscilloscope screenshots, or timing measurements you make while following the instructions should be included section 3 of the report.

The instructions may also include questions on the last slide. Answer these questions in section 5 of the report.

## Hardware and software
Before the start of each lab, check out one development board for each team for labs 2-6 and one board for each individual for labs 1 and 7.

The development board is the STM32H735G Discovery kit. The user manual can be [downloaded from the ST website][13]. Make sure that the board you check out is accompanied by a micro USB cable to connect to your laptop and two BNC to TS cables to connect with the oscilloscope.

See the next section for instructions on programming the board and installing the software.

We will also use MATLAB throughout the course. Please see the [instructions for MATLAB installation][14].

[1]:https://en.wikipedia.org/wiki/Time_limit
[2]:https://en.wikipedia.org/wiki/Real-time_computing
[3]:https://en.wikipedia.org/wiki/Bare_machine
[4]:https://en.wikipedia.org/wiki/Memory_management_unit
[5]:https://en.wikipedia.org/wiki/Throughput
[6]:https://en.wikipedia.org/wiki/Direct_memory_access
[7]:https://en.wikipedia.org/wiki/Reduced_instruction_set_computer
[8]:https://en.wikipedia.org/wiki/Coprocessor
[9]:https://en.wikipedia.org/wiki/SIMD
[10]:https://en.wikipedia.org/wiki/Quadrature_amplitude_modulation
[11]:https://en.wikipedia.org/wiki/Modem
[12]:https://en.wikipedia.org/wiki/Numerical_error
[13]:https://www.st.com/resource/en/user_manual/dm00682073-discovery-kit-with-stm32h735ig-mcu-stmicroelectronics.pdf
[14]:http://users.ece.utexas.edu/~bevans/courses/realtime/homework/matlab.html