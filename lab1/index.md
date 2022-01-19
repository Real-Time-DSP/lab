# Lab 1. Overview of Hardware and Software Tools

## Aim of the experiment

The aim of the experiment is to familiarize you with the following:

1. STM32H735G Discovery Kit
2. STM32CubeIDE
3. MATLAB

## Reading assignment

* [Lab Primer][1]

* Digital Signal Processing using Arm Cortex-M based Microcontrollers by Cem Ünsalan, M. Erkin Yücel, H. Deniz Gürhan.
    * Chapter 1, sections 1-3
    * Chapter 5, sections 1-5
    * Chapter 10, sections 1-3

* Software Receiver Design by C. Richard Johnson, Jr., William A. Sethares and Andrew Klein
	* Chapter 1
	
* Course reader
	* Lecture 0 slides on [Introduction][2]
    
## Supplemental reading

* [Common signals in MATLAB][3]

## Lab 1 instructions

Complete the following exercises during your lab section. Include your results in the lab report.

If you are unfamiliar with a particular MATLAB function, type `doc <function>`  or `help <function>` in the MATLAB command window.

### DSP in MATLAB

Since these exercises are intended to teach the basics of MATLAB, some possible solutions are provided.

1.  Create a vector `t` representing the time variable with range of one second and sampling period of of $T_s = 0.001$ sec. One way to create a vector of evenly spaced numbers in MATLAB is with the `:` operator. Alternatively, you can use the `linspace` function.

    *Solution 1:* 
    ```matlab
    t = 0:0.001:1;
    ```
    *Solution 2:* 
    ```matlab
    t = linspace(0,1,1001);
    ```

2.  Create a plot that approximates the continuous time signal $x(t)=sin(2 \pi t)$ for $0 \leq t \leq 1$, using the `plot` function.

    *Solution 1:*
    ```matlab
    xt = sin(2*pi*t);
    plot(t,xt);
    ```
    *Explanation:* Many MATLAB functions that operate on scalars (including `sin`) are automatically ['mapped'][4] to arrays. Since `t` is an vector, the expression `sin(2*pi*t)` applies the scalar function $x(t) = sin( 2\pi t)$ to each element of `t`.
    
    *Solution 2:*
    ```matlab
    x = @(t) sin(2*pi*t);
    plot(t,x(t));
    ```
    *Explanation:* The line `x = @(t) sin(2*pi*t)` defines a new function `x`. This line can be read as '`x` is a function of `t` that returns `sin(2*pi*t)`'. Defining a function is useful to simplify sequences of operations. For example, if we wanted to shift $x(t)$ in time by five seconds, we could use the expression `x(t-5)`.
    
3.  Label the plot using the `xlabel`, `ylabel`, and `title` functions.

    *Solution:*
    ```matlab
    xlabel('Time [seconds]');
    ylabel('Signal Amplitude x(t)');
    title('x(t)=sin(t)');
    ```

4.  Obtain the value of the signal at time t = 0.32 seconds.

    *Solution 1:*
    ```matlab
    [~, ind] = min(abs(t-0.32));
    xt(ind)
    ```
    *Explanation:* The expression `abs(t-0.32)` measures the distance of each element in `t` to the desired value of 0.32. Combining this expression with the `min` function allows us to find the element of t which is closest to 0.32. The `min` function returns two values: the value of the minimum (which we discard by assigning the special name `~`) and the index `ind` which tells us the position in the array that is closest to 0.32.

    *Solution 2:*
    ```matlab
    ind = 1 + 0.32/0.001
    xt(ind)
    ```
    *Explanation:* Since `t` is sampled every 0.001 seconds, we can directly calculate the index corresponding to $t=0.32$ as long as we consider that array indexing in MATLAB is one-based.

5.  Sample the signal at every 0.008-sec.

    *Solution 1:* 
    ```matlab
    xn = xt(1:8:end);
    ```
    
    *Solution 2:* 
    ```matlab
    xn = downsample(xt,8);
    ```
    
    *Warning: the 'downsample' function requires the signal processing toolbox.*

6.  Overlay the sampled signal on your continuous plot using the `stem` function

    *Solution:*
    ```matlab
    tn = t(1:8:end);
    hold all;
    stem(tn,xn);
    ```
    
**Include the code for steps 1-6 in your lab report along with the figure you generated.**
    
### Using the STM32H735G discovery kit and starter code

In order for you to get started programming the board, we have created a sample project which passes a signal from the input jack to the output jack (this is known as a talkthrough.) Additionally, the spectrum of the input signal is visualized on the LCD.

1. Follow the [instructions in the setup guide][5] to import the starter code and program the board..

2. Connect the blue input jack on the board to the signal generator set to a 10kHz sine wave.

3. Connect the green output jack on the board to the oscilloscope.

4. Configure the oscilloscope and verify the that the signal appears as expected.

5. Locate the `process_left_sample` function in `lab.c`. Change the behavior from a talkthrough $f(x[n]) = x[n]$ to a squaring function $f(x[n]) = {(x[n])}^2$ by modifying the line `output_sample = input_sample;` Rerun the program and observe the result on the oscilloscope.

6. At the end of the `process_left_sample` function in `lab.c`, locate the line `return output_sample;`. Put a breakpoint at this line.

7. Allow the program to continue until it stops at the breakpoint. Add a watch expression for the variable `elapsed_cycles` and record the value. **Include this measurement in your lab report.**

## Lab report contents

Be sure to include everything listed in this section when you submit your lab report.

### I. Results from lab exercise

1. DSP in MATLAB

2. Using the STM32H735G discovery Kit and starter code

### II. Assignment questions

1. What is aliasing? How do you manage aliasing in DSP applications?

2. Draw a block diagram of a generic DSP system and a talk-through system.

3. How are 32-bit floating-point results saved on the ARM Cortex-M7 processor? Explain briefly the IEEE single-precision floating-point format.

4. Give an example (such as one or more lines of C code) that would result in the processor using IEEE single-precision floating point for an operation.

5. How can you estimate the number of clock cycles required to execute a section of code on the STM32 board?

[1]:../primer.md
[2]:http://users.ece.utexas.edu/~bevans/courses/realtime/lectures/00_Introduction/lecture0.pptx
[3]:http://users.ece.utexas.edu/~bevans/courses/realtime/handouts/CommonSignalsInMatlab.pptx
[4]:https://en.wikipedia.org/wiki/Map_(parallel_pattern)
[5]:../stm32h735g.md