# Lab 2. Generating Cosine and Sine Waves

## Aim of the experiment

The aim of this experiment is to

* explore the design tradeoffs in signal quality vs. runtime complexity in computing amplitude values of sinusoidal signals.

Sinusoidal waveforms will be computed in discrete time, plotted, and played as audio signals.

## Reading assignment

* Digital Signal Processing using Arm Cortex-M based Microcontrollers by Cem Ünsalan, M. Erkin Yücel, H. Deniz Gürhan.
    * Chapter 2, sections 1-5
    * Chapter 3, sections 1-7

* Software Receiver Design by C. Richard Johnson, Jr., William A. Sethares and Andrew Klein
	* Chapter 1
    
* Course reader
    * Lecture 1 slides on [Generating Sinusoidal Signals][1]
    
## Lab 2 instructions: Week 1

In these exercises, we want to generate the continuous signal $x(t) = \sin(2 \pi f_0 t)$ by sending the discrete signal $x[n] = \sin \left( 2 \pi \frac{f_0}{f_s} n\right)$ to the DAC.

### Sinusoidal generation using math library and phase accumulation

The first method we will use to generate a sinusoid is a math library call.

1. Set the sampling rate to $\left(f_s = 16 \text{ kHz} \right)$ in lab.h.

2. Determine the discrete time frequency $\omega_0$ (in radians per sample) corresponding to $f_0 = 1000$ Hz.

3. In lab.c, create a variable corresponding to the current phase. Use the type float32_t.

4. The math library will compute floating point values of the sinusoid in the range $[-1,1]$. However, the ADC and DAC are configured to use values in the range [-3200, 3200]. The `INPUT_SCALE_FACTOR` and `OUTPUT_SCALE_FACTOR` constants are provided to map int16_t data in this range to floating point data with the range [-1,1].

5. In process_left_sample, replace the current talkthrough behavior with the sinusoidal generation using the math library and phase accumulation:

    * Instead of setting `output_sample = input_sample` , call the math library function:  `output_sample = OUTPUT_SCALE_FACTOR * arm_sin_f32(phase)`
    
    * Increment the phase by $\omega_0$
    
6. Using the phase accumulation method, the phase will eventually become too large and cause numerical errors. Add an if statement that prevents the phase from exceeding $2 \pi $.

7. Display the output signal on the oscilloscope and verify the frequency.

8. Approximate the number of clock cycles that elapsed when calling the math library function by placing it between calls of the provided `tic()` and `toc()` functions. The easiest way to do this is by putting a breakpoint *after* the line `elapsed_cycles = toc()`, setting a watch expression for `elapsed_cycles`, and running the program in debug mode until it reaches the breakpoint. The `tic()` and `toc()` functions estimate the number of clock cycles by saving the value of the systick register and taking a difference. Occasionally, you will observe a very high measurement (something like 4 million) caused by overflow. If this happens, you can easily repeat the measurement by continuing the program execution until it reaches the breakpoint again.

**In your lab report, please include:**

* the modified process_left_sample function
* the value of $\omega_0$ you computed
* the clock cycle measurement for the math library call

### Sinusoidal generation using difference equation

In this exercise, we will apply an impulse to a causal discrete-time linear time-invariant system governed by the difference equation

$$ y[n] = (2 \cos \omega_0) y[n-1] - y[n-2] + (\sin \omega_0) x[n-1] $$

and whose initial conditions are all zero as a necessary condition for it to be LTI.

The impulse response of this system is a causal sinusoid with discrete time frequency $\omega_0$.

1.  In lab.c, initialize variables corresponding to the state variables of the system. For example:
    ```c
    float32_t x[3] = {1,0,0};
    float32_t y[3] = {0,0,0};
    ```
    If using an array of values like this example, think about which element of the array should correspond to each state variable. A good convention is that `x[0]` maps to $x[n]$, `x[1]` maps to $x[n-1]$, and so on.
    
2. In lab.c replace the output value of process_left_sample with the output $y[n]$ of this system. Use the same value of $\omega_0$ as before.

3. In lab.c, add statements to the process_left_sample function that simulate the state values moving through the delay line. Looking at the block diagram should help you determine how to update your variables.

4. Run the program and view the output on the oscilloscope to verify the behavior.

**Include the modified process_left_sample function in your lab report.**

## Lab 2 instructions: Week 2

### Sinusoidal generation using lookup table

In this exercise, we will examine the lookup table method for generating a sinusoid.

#### Examining periodicity in MATLAB.

This section may help you complete assignment question #6.

The lookup table method requires minimal computation compared the math library call or difference equation, but the memory required depends on the desired frequency and the sampling rate. This MATLAB exercise will demonstrate this relationship.

1. For a sine wave with desired frequency $f_0 = \text{1 kHz}$ and a sampling rate of $f_s = 16 \text{ kHz}$, determine the appropriate discrete-time frequency $\omega_0$ in radians per sample. 

2. In MATLAB, plot the first 100 samples of the discrete-time signal in step one using the `stem` function. Notice that the signal repeats every 16 samples.

3. Repeat step two for a sine wave with desired frequency $f_0 = \text{3 kHz}$ and a sampling rate of $f_s = 16 \text{ kHz}$. Notice that this signal also repeats every 16 samples.

3. Repeat step two for a sine wave with desired frequency $\Omega_0 = 1000 \text{ rad/sec}$ and a sampling rate of $f_s = 8 \text{ kHz}$. Notice that this signal is *aperiodic*.

#### Lookup table: sample-by-sample

1. For a sine wave with desired frequency $f_0 = \text{1 kHz}$ and a sampling rate of $f_s = 16 \text{ kHz}$, determine the appropriate discrete-time frequency $\omega_0$ in radians per sample. Notice that this discrete-time signal is periodic with $L=16$ samples.

2. In lab.h, ensure that the sampling rate is set to 16 kHz.

3. In lab.c, declare an array corresponding to a lookup table that you will build for this sinusoid. Also create a variable corresponding to the current position in the lookup table. For example:

    ```c
    int16_t table[16];
    uint32_t i_table = 0;
    ```

4. In lab.c, add code to the lab_init function to populate your lookup table. For example:

    ```c
    float32_t amplitude;
    for (uint32_t n = 0; n < 16; n+=1)
    {
        amplitude = arm_sin_f32(n * omega0);
        table[n] = OUTPUT_SCALE_FACTOR * amplitude;
    }
    ```

5. In lab.c, modify the process_left_sample function to use the values from your lookup table. For example

    ```c
    output_sample = table[i_table];
    i_table += 1;
    if (i_table == 16) {i_table = 0;}
    return output_sample;
    ```

6. Run the program and view the output on the oscilloscope to verify the behavior. **Include an oscilloscope screenshot in your lab report. Make sure that either the frequency or period of the sinusoid are visible in your screenshot.**

7. Change $\omega$ so that it corresponds to a desired frequency $f_0 = \text{15 kHz}$ and a sampling rate of $f_s = 16 \text{ kHz}$ (This should be a red flag!). Run the program and view the output on the oscilloscope. **Include an oscilloscope screenshot in your lab report. Make sure that either the frequency or period of the sinusoid are visible in your screenshot.**

#### Lookup table: direct memory access (DMA)

The starter code is configured to use the DMA controller to repeatedly transfer a buffer (corresponding to the output signal) to the DAC. Typically, this buffer is updated as new inputs are received. Since we want to generate a periodic signal, we can implement our program more efficiently by populating this buffer with our sinusoid once and then leaving it untouched.

1. In lab.h, disable updates to the ouput buffer by uncommenting the statement

    ```c
    # define PERIODIC_LOOKUP_TABLE
    ```
    
    and commenting the statements for the processing functions
    
    ```c
    //   #define PROCESS_LEFT_CHANNEL
    //   #define PROCESS_RIGHT_CHANNEL
    //   #define PROCESS_INPUT_BUFFER
    //   #define PROCESS_OUTPUT_BUFFER
    ```
    
2. In lab.c, modify the construction of your lookup table so that the values are stored in the output buffer. Recall that the left and right channels are interleaved in the output buffer. To use the lookup table on the left channel, you might add

    ```c
    for (uint32_t i_sample = 0; i_sample < FRAME_SIZE; i_sample+=1)
    {
        i_table = (i_sample/2) % 16;
        output_buffer[i_sample] = table[i_table]; //left
        i_sample += 1;
        output_buffer[i_sample] = 0; //right
    }
    ```

3. Run the program and view the output on the oscilloscope to verify the behavior.

4. Notice the frame size (8192) is an integer multiple of our signal's period $L=16$. Recalculate $\omega_0$ and $L$ corresponding to a $f_0 = \text{440 Hz}$ and update your lookup table accordingly. Run the program and observe the output on the oscilloscope. You should notice abrupt changes in your output every $\frac{8192}{16000} \approx 0.5$ seconds.

5. In lab.h, change the frame size to 8000 and run the program. You should notice that the abrupt changes in your output no longer occur.

## Lab report contents

Be sure to include everything listed in this section when you submit your lab report. The goal of the report is to help cement what you learned in memory. For sections I, II, and IV, imagine your audience is a student who is your peer but who has not yet completed the lab.

### I. Introduction

Briefly explain the theory and algorithms behind the programs that you wrote. The slides and reading material might help you in writing this section.

### II. Methods

Describe the steps you took to implement the algorithms in your own words.

### III. Results from lab exercise

Present the results you obtain for each task on the assignment sheet. This section should include illustrative oscilloscope screenshots of the DSP algorithms in action. Also include any code that you wrote or modified. Please do not include all of the boilerplate code from the textbook.

1. Sinusoidal generation using math library and phase accumulation

    * process_left sample function modified for math library method 
    * value of $\omega_0$ used to generate 1 kHz sinusoid

2. Sinusoidal generation using difference equation

    * process_left sample function modified for difference equation method 

3. Sinusoidal generation using lookup table

    * oscilloscope screenshot corresponding to desired frequency $f_0 = \text{1 kHz}$
    * oscilloscope screenshot corresponding to desired frequency $f_0 = \text{15 kHz}$
    
### IV. Discussion

In this section, discuss the takeaway from each lab. You can mention any intuition that you developed. Also mention any problems that you faced and how you rectified them.

### V. Assignment questions

Please answer the following questions.

1. Explain (mathematically) what happened when the 15 kHz sine wave was generated for a sample rate of 16 kHz.

2. Why is the scaling factor necessary? What would happen if the scaling factor was left out?

3. When generating a sinusoid with a sampling rate of 16 kHz, how many cycles would occur between samples? Assume that the processor is running at 500 MHz. 

4. Repeat the previous question for a sampling rate of 192 kHz.

5. Look at the measurement you obtained for the number of clock cycles required to execute the math library sin function. Compare this number to the answers obtained in the previous two question. What do these numbers tell you about the feasibility of the math library method for generating sinusoids as the sampling rate increases?

6. The lookup table method for generating a sinusoid requires almost no computation, but does require memory. For which of the following conditions would the lookup table method be appropriate?

    a. $f_0 = \text{1 kHz}$, $f_s = 16 \text{ kHz}$
    
    b. $f_0 = \text{440 Hz}$ (middle c), $f_s = 44.1 \text{ kHz}$ (CD audio)
    
    c. $\Omega_0 = 1000 \text{ rad/sec}$, $f_s = 16 \text{ kHz}$.
    
[1]:http://users.ece.utexas.edu/~bevans/courses/realtime/lectures/01_Sinusoids/lecture1.ppt