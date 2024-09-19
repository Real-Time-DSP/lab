# Lab 3. Digital Filters

## Aim of the experiment

A goal of this experiment is to design discrete-time digital filters and implement them in real time.

In this experiment, you will design and implement finite impulse response (FIR) filters and infinite impulse response (IIR) filters in Matlab. You will measure the response of the filters using a variety of input test signals and quantify the change in magnitude response due to an increase in filter order.

## Reading assignment

* Digital Signal Processing using Arm Cortex-M based Microcontrollers by Cem Ünsalan, M. Erkin Yücel, H. Deniz Gürhan.
    * Chapter 4, sections 1-8
    * Chapter 7, sections 1-4
    * Chapter 8, sections 1-6

* Software Receiver Design by C. Richard Johnson, Jr., William A. Sethares and Andrew Klein
	* Chapter 3, sections 1-4
    * Chapter 4
    * Chapter 6, sections 1-2
    * Chapter 7
    
* Course reader
    * Lecture 3 slides on [Signals and Systems][1]
    * Lecture 5 slides on [Finite Impulse Response Filters][2]
    * Lecture 6 slides on [Infinite Impulse Response Filters][3]
    
## Lab 3 instructions: Week 1

### FIR filter design in MATLAB

We want to design a FIR bandpass filter using the following parameters:
* Design method: Parks-McClellan (a.k.a. equiripple)
* Filter order $N=30$ (the filter will have $N+1=31$ coefficients)
* Sampling Frequency $F_s= 48 \text{ kHz}$
* $F_{\text{stop1}}=4\text{ kHz}$
* $F_{\text{pass1}}=6\text{ kHz}$
* $F_{\text{pass2}}=14\text{ kHz}$
* $F_{\text{stop2}}=16\text{ kHz}$
* $W_{\text{stop1}}=W_{\text{stop2}}=60W_{\text{pass}}$

![](../img/bandpass.svg)

1. Open the the MATLAB Filter Designer by calling the `filterDesigner` function. Enter the parameters and design the filter.

2. Export the coefficients (File → Export → coefficients to workspace). The default variable name is `Num`, corresponding to the numerator of the transfer function.

   Later, we will experimentally measure the frequency response at 1000 Hz, 2000 Hz, $\ldots$, 19000 Hz, and 20000 Hz. In order to compare with our measurements, we need to first tabulate the theoretical response at these frequencies. Steps 3-5 describe this process.

3. Use the `freqz` function to get the response of the filter at 10 Hz intervals:

    ```
    [h,f] = freqz(Num,1,2400,48000);
    ```
4. Define a function to convert the frequency response values returned by freqz from complex values to magnitude values in dB:

    ```
    dB = @(x) 20*log10(abs(x));
    ```
5. Tabulate the theoretical magnitude response of this filter at 1000 Hz increments up to 20 kHz:

    ```
    freq = f(101:100:2001);
    resp_dB = dB(h(101:100:2001));
    table(freq,resp_dB)
    ```
    
    You may find it helpful to export the values to a spreadsheet so that you can record the measured values in a new column later. You can use the `xlswrite` function to export the data to a spreadsheet
    
    ```
    xlswrite('theoretical_response.xlsx',[freq,resp_dB]);
    ```
    
    **Include the tabulated values of the theoretical magnitude response in your lab report**     
    
6. We will need to define an array in C containing these coefficients. You can use the `sprintf` command in MATLAB to generate a comma separated string which you can copy into your C code:

    ```
    sprintf('%f,',Num)
    ```

### FIR filter implementation in C

1. In lab.c, define an array containing your filter coefficients.

    ```
    float32_t b[31] = {<coefficients>};
    ```
    
2. In lab.c, define variables corresponding to the state values of the filter. An good convention is to create an array `x` where `x[0]` corresponds with $x[n]$, `x[1]` corresponds with $x[n-1]$, and so on. Initialize your variables so that all initial conditions are zero. An easy way to do this is with the 'universal zero initializer' `{0}`, e.g. 

    ```
    float32_t x[31] = {0};
    float32_t y = 0;
    ```
    
3. In lab.c, modify the process_left_sample function to implement the filter as a tapped delay line.

    ![](../img/tapped_delay.svg)
    
    1. Each time the process_left_sample function is called, we are given a new input sample from the ADC which is a 16-bit signed integer. Convert this value to floating point, apply the appropriate scaling, and store it into the array location corresponding to $x[n]$
    
    ```
    x[0] = ((float32_t) input_sample) * INPUT_SCALE_FACTOR;
    ```

    2. Each time the process_left_sample function is called, we need to calculate the value of $y[n]$ from a summation. Assign `y` a value of zero to 'reset' the summation.
    
    3. Create a for loop to perform the summation for the output of the tapped delay line
    
        $$y[n] = \sum_{i=0}^{N}{b_ix[n-i]}$$
    
    4. After computing $y[n]$ we need to move the variables down the delay line. Create another for loop that reassigns the state variables to prepare them for the next output when process_left_sample is called again. (Hint: use a down counting for loop.)
    
4. Connect the signal generator (set to 10 kHz) as the input and the oscilloscope as the output. Verify the operation of the program. (Since 10kHz is in the passband, you should see the signal on the output).

5. Measure the frequency response of your filter at 1 kHz increments up to 20 kHz by changing the frequency on the signal generator and measuring the amplitude of the output signal. **Include the measured values in your lab report.**

6. Convert the amplitude values you measured to decibels. Since your measurement is a voltage and decibels are a ratio of power, the conversion is:

    $$\left| H \right|_{\text{dB}} = 10\log_{10}\left(\frac{P_{\text{out}}}{P_{\text{in}}}\right)= 20\log_{10}\left(\frac{V_{\text{out}}}{V_{\text{in}}}\right)$$
    
    The value for $V_{\text{out}}$ is your measurement. Check the setting your signal generator to determine $V_{\text{in}}$.
    
7. Plot the values of your measured response on top of or next to the values of the theoretical response. **Include this plot in your lab report.**

8. Measure the number of elapsed cycles of your FIR filter. 

### FIR filter with circular buffer.

1. Replace the linear buffer in your direct form FIR implementation with a circular buffer. **Hint:** See the example in lab #0.

2. Measure the number of elapsed cycles for the circular buffer implementation, and compare this with the direct form.

### Frame-based FIR filter

The starter code uses the DMA controller to move blocks of data (also called frames) between the ADC, main memory and DAC. In this exercise, we will implement the same FIR bandpass filter, but apply it to entire frames of data rather than sample-by-sample. To achieve this, we will put our filter in the process_input_buffer function rather than the process_left_sample function.
  
1. We will use the ARM CMSIS DSP library to perform filtering on a frame of data. Take a moment to read the [description of the FIR filter functions][4] in this library.

2. Starting from the original talk through starer project, in lab.c, initialize the variables necessary to use the `arm_fir_f32` function

    1. Create buffers for the input and output of the filter.
    
        ```
        float32_t filter_in[FRAME_SIZE/4] = {0};
        float32_t filter_out[FRAME_SIZE/4] = {0};
        ```
        
        *(The FRAME_SIZE constant specified in lab.h corresponds to the length of the entire circular buffer for both left and right channels. When one half of the circular buffer is accumulating data, we will work with the other half. Additionally, we are only working with one of the two channels. This is why our arrays are of length FRAME_LENGTH/4).*
        
    2. Create a buffer for the filter state
    
        ```
        float32_t state[31+(FRAME_SIZE/4)-1] = {0};
        ```
        
        *(The arm_fir_f32 function uses a state buffer of length num_taps+block_size-1 to implement the filter more efficiently.)*
        
    3. Create a filter instance struct.
    
        ```
        arm_fir_instance_f32 filter_instance;
        ```
        
3. In lab.c, initialize the filter instance in the lab_init function

    ```
    arm_fir_init_f32(&filter_instance, 31, b, state, FRAME_SIZE/4);
    ```

5. In lab.c, modify the process_input_buffer function to perform the filtering

    1. Fill your filter_in buffer with the samples from the left channel.
    
        ```
        for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=2)
        {
            filter_in[i_sample/2] = ((float32_t)input_buffer[i_sample])*INPUT_SCALE_FACTOR;
        }
        ```
    2. Perform the filtering.
        
        ```
        arm_fir_f32(&filter_instance, filter_in, filter_out, FRAME_SIZE/4);
        ```
        
    3. Copy the filter output buffer back to the array used by the DMA controller.
    
        ```
        for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
        {
             input_buffer[i_sample] = OUTPUT_SCALE_FACTOR*filter_out[i_sample/2];
             i_sample+=1;
             input_buffer[i_sample] = 0;
        }
        ```
        
6. Connect the signal generator as the input and the oscilloscope as the output. Verify the operation of the program by sweeping through different frequencies and observing the response.

## Lab 3 instructions: Week 2

### IIR filter design in MATLAB

We want to design an IIR bandpass fitler using the following parameters:
* Design method: Elliptic
* Filter order $N=6$
* Sampling Frequency $F_s= 48 \text{ kHz}$
* $F_{\text{pass1}}=5\text{ kHz}$
* $F_{\text{pass2}}=15\text{ kHz}$
* $A_{\text{stop}}=60 \text{dB}$
* $A_{\text{pass}}=2 \text{dB}$

![](../img/bandpass_IIR.svg)

1. Open the the MATLAB Filter Designer by calling the `filterDesigner` function. Enter the parameters and design the filter.

    Notice that once the filter is designed, the filter structure is changed to `Direct Form II, Second-Order Sections` in the 'Current Filter Information' panel.
    
2. Choose Edit → convet to single section, and export the coefficients (File → Export → coefficients to workspace). Two variables will be created corresponding to the numerator and denominator coefficients of the transfer function.

3. Use the `freqz` function to get the response of the filter at 10 Hz intervals:

    ```
    [h,f] = freqz(Num,Den,2400,48000);
    ```
4. Define a function to convert the frequency response values returned by freqz from complex values to magnitude values in dB:

    ```
    dB = @(x) 20*log10(abs(x));
    ```
5. Tabulate the theoretical magnitude response of this filter at 1000 Hz increments up to 20 kHz:

    ```
    freq = f(101:100:2001);
    resp_dB = dB(h(101:100:2001));
    table(freq,resp_dB)
    ```
    
    You may find it helpful to export the values to a spreadsheet so that you can record the measured values in a new column later. You can use the `xlswrite` function to export the data to a spreadsheet
    
    ```
    xlswrite('theoretical_response_iir.xlsx',[freq,resp_dB]);
    ```
    
    **Include the tabulated values of the theoretical magnitude response in your lab report**     
    
6. We will need to define an array in C containing these coefficients. You can use the `sprintf` command in MATLAB to generate a comma separated string which you can copy into your C code:

    ```
    sprintf('%f,',Num)
    sprintf('%f,',Den)
    ```
    
7. In the filter designer, change the structure back to the default (Direct-Form II, Second-Order Sections) and export the coefficients. Two variables, 'SOS' and 'G' will be created.

8. The first three columns of the SOS matrix that is created corresponds to the numerator coefficients of three cascaded second-order filters, and the last three columns correspond to the denominator coefficients. The G variable corresponds to the gain that is applied between sections. Later, we will use these coefficients in our C Code. You can use sprintf in MATLAB to format the coefficients for C.

    Numerator coefficients:
    ```
    sprintf('{%f,%f,%f},\n',SOS(1,1:3),SOS(2,1:3),SOS(3,1:3))
    ```
    
    Denominator coefficients:
    ```
    sprintf('{%f,%f,%f},\n',SOS(1,4:6),SOS(2,4:6),SOS(3,4:6))
    ```
    
    Gain:
    ```
    sprintf('%f,',G)
    ```

### IIR filter implementation: direct form

In this exercise, we will implement the IIR filter in direct form.

![](../img/iir_df_I.svg)

1. In lab.c, initialize variables for your filter coefficients and state variables. Notice that MATLAB has assigned a value of one where the nonexistent $a_0$ coefficient would be.

2. In lab.c, modify the process_left_sample function to implement the summation for the filter output. The diagram above should help you.

    $$y[n] = \sum_{k=0}^{N}{b_k x[n-k]} - \sum_{m=1}^{M}{a_m y[n-m]}$$
    
    Recall that `input_sample` is a 16 bit integer, so you will need to scale it when converting to floating point.
    
3. In lab.c modify the process_left_sample function to simulate the movement of data through the delay blocks (Hint: use a down counting for loop). **Include the C code for your implementation of the filter in the lab report.** 

4. Connect the signal generator as the input and the oscilloscope as the output. Measure the frequency response of your filter at 1 kHz increments up to 20 kHz by changing the frequency on the signal generator and measuring the amplitude of the output signal. Convert the amplitude values you measured to decibels. **Include the measured values in your lab report.**
    
5. Plot the values of your measured response on top of or next to the values of the theoretical response. **Include this plot in your lab report.**

## Lab 3 instructions: Week 3

### IIR filter with circular buffer.

1. Replace the linear buffer in your direct form IIR implementation from week #2 with a circular buffer. **Hint:** See the example in lab #0.

2. Measure the number of elapsed cycles for the circular buffer implementation, and compare this with the direct form.

### IIR filter implementation: second order sections

In this exercise, we will implement the same IIR filter but as a cascade of second order filters (biquads).

![](../img/cascade_biquads.svg)

1. In lab.c, initialize variables for your biquad coefficients state variables, and gain. A convenient way organize the coefficients and state variables in C is with two dimensional arrays:

    ```
    float32_t B[3][3] = {
        <first three columns of SOS matrix>
    };
    float32_t A[3][3] = {
        <last three columns of SOS matrix>
    };
    float32_t Y[3][3] = {0};
    float32_t X[3][3] = {0};
    ```
    
2. In lab.c, add a for loop to process_left_sample that iterates over the three biquads to implement the filter.

    1. The input to the first biquad should come from `input_sample`. Otherwise, the input should come from the output of the the previous biquad.
    
    2. Apply the appropriate gain value to the input of each biquad.
    
    3. For each biquad, implement the second order filter using direct form (see the diagrams above).
    
    The process_left_sample function should return the output of the last biquad (after appropriate scaling and conversion back to a 16 bit integer). **Include the C code for your implementation of the filter in the lab report.** 
    
3. Run the program and verify that it produces the expected bandpass response.

## Lab report contents

Be sure to include everything listed in this section when you submit your lab report. The goal of the report is to help cement what you learned in memory. For sections I, II, and IV, imagine your audience is a student who is your peer but who has not yet completed the lab.

### I. Introduction

Briefly explain the theory and algorithms behind the programs that you wrote. The slides and reading material might help you in writing this section.

### II. Methods

Describe the steps you took to implement the algorithms in your own words.

### III. Results from lab exercise

Present the results you obtain for each task on the assignment sheet. This section should include illustrative oscilloscope screenshots of the DSP algorithms in action. Also include any code that you wrote or modified. Please do not include all of the boilerplate code from the textbook.

In addition to the code you modified, make sure to include.

1. Theoretical magnitude response values (tabulated).
    * For FIR bandpass filter (week one)
    * For IIR bandpass filter (week two)

2. Measured magnitude response values of (tabulated).
    * For FIR bandpass filter (week one)
    * For IIR bandpass filter (week two)

3. Plot of theoretical response on top of or next to a plot of your measured response.
    * For FIR bandpass filter (week one)
    * For IIR bandpass filter (week two)
    
4. C code for filter implementations
    * FIR filter using circular buffer
    * IIR filter in direct form for both linear and circular buffers
    * IIR filter using cascade of second order sections
    
### IV. Discussion

In this section, discuss the takeaway from each lab. You can mention any intuition that you developed. Also mention any problems that you faced and how you rectified them.

### V. Assignment questions

Please answer the following questions.

1. Why use circular buffers?

2. What happens if you do not multiply the gain for each biquad in an IIR filter implementation using a cascade of second-order sections (SOS)?

3. Explain the difference between the theoratical magnitude response and the measured magnitude response in the FIR case.  

4. List the advantages and disadvantages of FIR vs. IIR filters.

[1]:http://users.ece.utexas.edu/~bevans/courses/realtime/lectures/03_Signals_Systems/lecture3.pptx
[2]:http://users.ece.utexas.edu/~bevans/courses/realtime/lectures/05_FIR_Filters/lecture5.pptx
[3]:http://users.ece.utexas.edu/~bevans/courses/realtime/lectures/06_IIR_Filters/lecture6.ppt
[4]:https://www.keil.com/pack/doc/CMSIS/DSP/html/group__FIR.html