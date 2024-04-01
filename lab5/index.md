# Lab 5. Digital Data Transmission by Baseband Pulse Amplitude Modulation (PAM)

## Aim of the experiment

In this experiment, you will learn the basics concepts of digital communications like pulse shaping filters, Nyquist criterion, eye diagram, inter-symbol interference and clock recovery. You will learn digital data transmission using baseband pulse amplitude modulation.

Before you start with the experiment, it is necessary that you have an understanding (at some level) of the following concepts:

* Baseband PAM
* Nyquist criterion
* Inter-symbol Interference (ISI)
* Pulse shaping (raised cosine baseband filtering)
* Eye diagrams
* Interpolation filter bank
* Symbol clock recovery

## Reading assignment

* Digital Signal Processing using Arm Cortex-M based Microcontrollers by Cem Ünsalan, M. Erkin Yücel, H. Deniz Gürhan.
    * Chapter 9, sections 1-6
* Software Receiver Design by C. Richard Johnson, Jr., William A. Sethares and Andrew Klein
    * Sections 2.6-2.16
    * Sections 9.1-9.4
    * Sections 10.1-10.4
    * Sections 11.1-11.6
* Course reader
    * Lecture 7: Interpolation and Pulse Shaping
    * Lecture 13: Digital Pulse Amplitude Modulation (PAM transmitter)
    * Lecture 12: Channel Impairments (communication channel)
    * Lecture 14: Matched Filtering (PAM receiver)
* Common Microcontroller Software Interface Standard (CMSIS) Reference
    * [FIR Interpolating Filter][1]
    
## Lab 5 instructions: week 1

In this exercise, we will transmit the scrambled tree image from the STM board to a PC running a receiver to recover the image.

### Simulation in MATLAB

1. Run the following code which simulates the generation of the transmitted signal. Please download the [file containing the PN sequence from lab four][2], and the [tree image][3]. Ensure that both are accessible on the MATLAB path.

    ```
    f0 = 12000; fs = 48000; sps = 16; fsym = fs/sps;
    pulse_shaping_filter = rcosdesign(0.8,4,sps,'normal');
    carrier = @(n,theta) cos(2*pi*(f0/fs)*n + theta);
    header = str2num(fliplr(dec2bin(2524737185))');
    serialized_data = reshape(imread('tree.png'),[1,128^2])';
    pn16k = readmatrix('pn_16384.txt')'; pn16k = pn16k(:);
    pn16k = fliplr(dec2bin(pn16k))'; pn16k = str2num(pn16k(:));
    scrambled = mod(serialized_data + pn16k,2);
    upsampled_data = upsample([header; scrambled]*2-1,16);
    sequence_of_pulses = conv(upsampled_data,pulse_shaping_filter,'same');
    n = 1:length(sequence_of_pulses);
    modulated = sequence_of_pulses' .* carrier(n,0);
    ```

2. Create an eye diagram using the first 1600 samples of the simulated signal. You can either use the 'eyediagram' function in the communications toolbox or reshape the samples into a matrix and use 'plot'. **Include the eye diagram in your lab report.**

    ```
    eyediagram(sequence_of_pulses(1:1600),16);
    ```

    ```
    figure; plot(reshape(sequence_of_pulses(1:1600),[16,100]));
    ```

### Setting up the transmitter on the STM32 board

1. In lab.c, create an array containing the data stream to transmit. The data stream will have 32 bits of header followed by 16384 bits for the (scrambled) tree image. If we store it as an array of 32-bit words, then we will need an array of length 513.

    ```
    uint32_t data_stream[513] = {0};
    ```
    
    For the header sequence, we will use the output of a maximal-length 5-bit LFSR. Rather than repeat the steps from lab 4 to generate the sequence, you can simply copy the provided sequence which has been computed for you.
    
    ```
    data_stream[0] = 0x967c6ea1;
    ```
    
    The tree data and scrambler are the same as lab four. Populate the remainder of the `data_stream` array with the scrambled tree (`tree[i]^PN[i]`).
    
    ```
    for (i_word = 0; i_word < 512; i_word+=1)
	{
	    data_stream[i_word+1] = tree[i_word]^PN[i_word];
	}
    ```

2. Prepare the pulse-shaping filter according the specifications below. Since the interpolating filterbank requires a filter whose length is a multiple of the upsampling factor (16), remove the last coefficient (which should be equal to zero and has no effect on the filter)

    * Filter type: raised cosine
    * Samples per symbol: 16
    * Sampling Frequency $F_s= 44.1 \text{ kHz}$
    * Rolloff factor: 0.8

    In MATLAB, use the 'rcosdesign' function and copy the coefficients into lab.c

    ```
    pulse_shaping_coeffs = rcosdesign(0.8, 4, 16, 'normal');
    sprintf('%f,',pulse_shaping_coeffs(1:end-1))
    ```

    ```
    float32_t pulse_shaping_coeffs[64] = {<coefficients>};
    ```

3. In lab.c prepare the variables necessary to use the CMSIS library function for the interpolating filter. Take a moment to read the [description of the FIR interpolator function.][1]

    ```
    arm_fir_interpolate_instance_f32 filter_instance; 
    uint8_t upsampling_factor = 16;
    uint16_t num_taps = 64;
    float32_t state[(FRAME_SIZE/4) + 4 - 1] = {0};
    float32_t filter_in[FRAME_SIZE/64] = {0};
    float32_t filter_out[FRAME_SIZE/4] = {0};
    float32_t cos_lut[4] = {1,0,-1,0};
    uint32_t i_lut = 0;
    ```
    
    Also create variables to keep track of our current word and bit position in the data stream (just as you did in lab 4). You can reuse the variables you used for the PN sequence as long as you reset them.
    
    ```
    uint32_t i_word = 0;
    uint32_t i_bit = 0;
    ```
    
4. Initialize the interpolating filter in the lab_init function:

    ```
    arm_fir_interpolate_init_f32 (&filter_instance, upsampling_factor, num_taps, pulse_shaping_coeffs, state, FRAME_SIZE/4);
    ```
    
5. In process_input_buffer, construct the buffer of symbols that we will send to the arm_fir_interpolate_f32 function. 

    ```
    for (uint32_t i_sample = 0; i_sample<FRAME_SIZE/64; i_sample+=1)
    {
        filter_in[i_sample] = ( data_stream[i_word] & (1<<i_bit) ) >> i_bit;
        filter_in[i_sample] = (filter_in[i_sample]*2) - 1;
    
        i_bit += 1;
        if (i_bit == 32)
        {
            i_word += 1;
            i_bit = 0;
            if (i_word > 512){i_word = 0;}
        }
    }
    ```

6. Perform the filtering and copy the result to the output buffer

    ```
    arm_fir_interpolate_f32 (&filter_instance, filter_in, filter_out, FRAME_SIZE/64);
    
    for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
    {
         input_buffer[i_sample] = OUTPUT_SCALE_FACTOR*filter_out[i_sample/2]*cos_lut[i_lut];
         i_lut = (i_lut + 1) % 4;
         i_sample+=1;
         input_buffer[i_sample] = 0;
    }
    ```


### Examining the transmission in MATLAB

1. With the transmitter running, record 12 seconds of data using MATLAB and save it to a file.

    ```
    fs = 48000;
    recorder = audiorecorder(fs,16,1);
    recordblocking(recorder, 12);
    scrambled = getaudiodata(recorder)';
    save scrambled scrambled;
    ```
    
2. Remove the scrambler by replacing the line `data_stream[i_word+1] = tree[i_word]^PN[i_word];` with `data_stream[i_word+1] = tree[i_word]`. Repeat the recording, but ensure that you choose a new file name.

    ```
    fs = 48000;
    recorder = audiorecorder(fs,16,1);
    recordblocking(recorder, 12);
    not_scrambled = getaudiodata(recorder)';
    save not_scrambled not_scrambled;
    ```
    
3. Create a spectrogram for both recorded transmissions. **Include both spectrograms in your lab report.**

    ```
    load scrambled;
    figure; spectrogram(scrambled,2^10,0,2^10,48000,'yaxis');

    load not_scrambled;
    figure; spectrogram(not_scrambled,2^10,0,2^10,48000,'yaxis');
    ```

4. Run the [receiver demo][4] in MATLAB using the transmission that was not scrambled. Try to adjust the parameters to recover the tree image. **Include the recovered tree image in your lab report using the best parameters you find.**

## Lab 5 instructions: week 2

In this exercise we will transmit the tree image (either from MATLAB or your lab partner's board) and receive it on the STM board.

### Costas loop for carrier recovery

1. In lab.c, create variables necessary for the costas loop. Assume that the lowpass filter in the costas loop is the same pulse shaping filter as before with 64 coefficients.

    ```
    float32_t Ux[64] = {0};
    float32_t Lx[64] = {0};
    float32_t Uy = 0;
    float32_t Ly = 0;
    float32_t wc = 1.5707963267949;
    float32_t theta = 0;
    float32_t mu = 0.01;
    float32_t baseband[FRAME_SIZE/4] = {0};
    uint32_t i_baseband = 0;
    ```

2. Add code to process_left_sample which computes the newest values for the upper and lower branches.

    ```
    float32_t r = input_sample*INPUT_SCALE_FACTOR;
    
    Ux[0] = r*arm_cos_f32(theta);
    Lx[0] = r*arm_sin_f32(theta);
    
    Uy = 0;
    Ly = 0;
    
    for (uint32_t i_sample = 0; i_sample < 64; i_sample +=1)
    {
        Uy += Ux[i_sample]*pulse_shaping_coeffs[i_sample];
        Ly += Lx[i_sample]*pulse_shaping_coeffs[i_sample];
    }
    
    baseband[i_baseband] = Uy;
    i_baseband += 1;
    if (i_baseband > FRAME_SIZE/4){i_baseband = 0;}
    
    for (i_sample = 63; i_sample > 0; i_sample -=1)
    {
        Ux[i_sample] = Ux[i_sample-1];
        Lx[i_sample] = Lx[i_sample-1];
    }
    ```
    
3. Update the estimate of $\theta$.

    ```
    theta += wc - mu*Uy*Ly;
    if (theta > 6.283185){theta -= 6.283185;}
    ```
4. Connect the input jack on the board to a transmitting source (either the matlab simulation via soundsc or a second board running the previous week's lab. Send the output of the costas loop (`Uy`) to the DAC. The pulse shaping filter applies a gain of about 3.005, so you may want to use a smaller scale factor.

    ```
    return OUTPUT_SCALE_FACTOR*Uy*0.3;
    ```
    
5.  Display the output on the oscilloscope and verify its behavior. For the scrambled signal, the output of the Costas loop should closely resemble the baseband signal before modulation. **Include an oscilloscope screenshot in your lab report.**

#### Symbol recovery by downsampling and quantization

1. Create a variable $k$ that acts as a counter for the downsampler. Since every 16th sample corresponds to a new symbol, let $k$ count up to 15 before saving the next value and resetting the counter.

    ```
    int32_t k = 0;
    ```

    ```
    if (k == 15)
    {
        k = 0;
        .
        .
        .
        // save the current value
        .
        .
        .
    }
    else
    {
        k +=1;
    }
    ```
    
2. Create an array to store 16384 bits (128 by 128 binary image) which can be displayed on the screen. Also create variables (or reset existing variables) to keep track of the current word and current bit. Quantize the output of the downsampler by comparing its value to zero, then store it in the array.

    ```
    uint32_t data[512] = {0};
    uint32_t i_word = 0;
    uint32_t i_bit = 0;
    ```

    ```
    if (Uy>0)
    {
        data[i_word] |= (1<<i_bit);
    }

    i_bit += 1;
    if (i_bit == 32)
    {
        i_word += 1;
        i_bit = 0;
    }

    ```
    
3. Create an if statement to detect if the array is full. When it is, display it as an image.

    ```
    if (i_word>511){
        display_image(data,128,128);
    }
    ```
    
4. Setup the transmitter as the input (either with your lab partner running the week 1 code or the provided MATLAB simulation) *without* scrambling the data. Run the receiver with a breakpoint immediately after image is displayed. You should see a noisy and possibly inverted tree image. Since no symbol timing recovery algorithm is in place, the quality of the recovered image could change when you repeat the experiment. **Include the recovered image in your lab report.** (You can either use a photo or [export the array][5] and plot it in MATLAB using the `image` function).

## Lab 5 instructions: week 3

In this exercise we will add a symbol clock recovery subsystem and implement the cross-correlation to match the PN header.

### Cross-correlation for header detection

1. Create the variables necessary to implement the header detection as an FIR filter including a semaphore variable to track when the header has been detected.

    ```
    int8_t xcorr[32] = {0};
    int8_t header[32] = {1,-1,-1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,1,-1,1,1,1,-1,1,-1,1,-1,-1,-1,-1,1};
    int8_t R = 0;
    int8_t header_matched = 0;
    ```

2. Each time a new symbol is processed (i.e. inside the `if (k==15)` block), map the symbol to either +1 or -1, and store it in the 'xcorr' array.

    ```
    if (Uy > 0){xcorr[0] = 1;}
    else{xcorr[0] = -1;}
    ```

3. Perform the correlation as an FIR filter (the header variable as defined above has already been flipped).

    ```
    R = 0;
    for (uint32_t i_hdr = 0; i_hdr < 32; i_hdr +=1)
    {
        R += xcorr[i_hdr]*header[i_hdr];
    }
    for (uint32_t i_hdr = 31; i_hdr > 0; i_hdr -=1)
    {
        xcorr[i_hdr] = xcorr[i_hdr - 1];
    }
    ```

4. If the cross correlation reaches the maximum possible value, update the semaphore.

    ```
    if (R > 30) {header_matched = 1;}
    ```

5. Move the data collection code block into an if statement so that it only occurs after the header is detected

    ```
    if (header_matched)
    {
        if (Uy>0){ data[i_word] |= (1<<i_bit); }
        .
        .
        .
    }

6. Repeat the transmission and put a breakpoint after the image is displayed. The image should appear with proper alignment. **Include the recovered tree image in your lab report** You can either take a photo or perform a memory export and plot it in MATLAB.

### Symbol timing recovery

1. Design two second order IIR bandpass filters in the MATLAB filter designer with the following parameters

    **BPF 1**
    * Design method: Elliptic
    * Filter order $N=2$
    * Sampling Frequency $F_s= 48 \text{ kHz}$
    * $F_{\text{pass1}}=1.45\text{ kHz}$
    * $F_{\text{pass2}}=1.55\text{ kHz}$
    * $A_{\text{stop}}=80 \text{dB}$
    * $A_{\text{pass}}=1 \text{dB}$

    **BPF 2**
    * Design method: Elliptic
    * Filter order $N=2$
    * Sampling Frequency $F_s= 48 \text{ kHz}$
    * $F_{\text{pass1}}=2.9\text{ kHz}$
    * $F_{\text{pass2}}=3.1\text{ kHz}$
    * $A_{\text{stop}}=80 \text{dB}$
    * $A_{\text{pass}}=1 \text{dB}$

2. Apply the first bandpass filter to the output of the Costas loop `Uy`.

3. Square the output of the first bandpass filter.

4. Apply the second bandpass filter after the squaring block.

5. Send the output to the DAC and display it on the oscilloscope. The peaks of this clock signal indicate when to sample each symbol. **Include an oscilloscope screenshot of the recovered clock signal in your lab report**.

### Symbol error probability

1. In MATLAB, plot the theoretical symbol error probability as a function of signal to noise ratio for $M = 2, 4, 8.$ Put the x-axis SNR variable in dB. 

    $$ P_e = \frac{2(M-1)}{M}\times Q\left( \sqrt{\text{SNR}\times \frac{3}{M^2 -1}} \right)$$
    
    The [$Q-$function][6] is related the the [Gaussian cumulative distribution function $\Phi(x)$][7] by $Q(x) = 1-\Phi(x)$. If you have the communications toolbox installed, you can simply use 'qfunc' in MATLAB. Otherwise, you can use the closely related [error function][8] 'erf'.
    
    $$ \Phi(x) = \frac{1}{2} \left[ 1 + \text{erf}\left( \frac{x}{\sqrt 2} \right) \right] $$
    
    ** Include the error probability vs SNR plot in your lab report**

## Lab report contents

Be sure to include everything listed in this section when you submit your lab report. The goal of the report is to help cement what you learned in memory. For sections I, II, and IV, imagine your audience is a student who is your peer but who has not yet completed the lab.

### I. Introduction

Briefly explain the theory and algorithms behind the programs that you wrote. The slides and reading material might help you in writing this section.

### II. Methods

Describe the steps you took to implement the algorithms in your own words.

### III. Results from lab exercise

Present the results you obtain for each task on the assignment sheet. This section should include illustrative oscilloscope screenshots of the DSP algorithms in action. Also include any code that you wrote or modified. Please do not include all of the boilerplate code from the textbook.

#### Week one

* Eye diagram of simulated transmission(What exactly is the diagram showing?)
* Spectrograms of PAM transmissions
* Recovered tree image

#### Week two
* Recovered tree image

#### Week three
* Recovered tree image
* Symbol error probability vs SNR (Please use "semilogy" instead of "plot")
    
### IV. Discussion

In this section, discuss the takeaway from each lab. You can mention any intuition that you developed. Also mention any problems that you faced and how you rectified them.

### V. Assignment questions

Please answer the following questions.

* Why we had only 4 elements for cos array while modulating. (Week 1)

* Why we had the stripes when using the receiver demo. (Week 1)

* Explain how does the Costas loop compensate for frequency offset at the receiver.(Week 2)

* What are the trade-offs of increasing the number of symbols M. (Week 3)

[1]:https://arm-software.github.io/CMSIS_5/DSP/html/group__FIR__Interpolate.html
[2]:https://real-time-dsp.github.io/lab/_sources/lab5/pn_16384.md
[3]:https://real-time-dsp.github.io/lab/_images/tree.png
[4]:https://github.com/Real-Time-DSP/lab/raw/main/starter_code/receiver_demo.m
[5]:../data.md
[6]:https://en.wikipedia.org/wiki/Q-function
[7]:https://en.wikipedia.org/wiki/Normal_distribution#Cumulative_distribution_function
[8]:https://en.wikipedia.org/wiki/Error_function
