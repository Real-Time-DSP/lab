# Lab 6. Quadrature Amplitude Modulation (QAM)

## Aim of the experiment

This experiment primarily concerns the design and implementation of a Quadrature Amplitude Modulation (QAM) transmitter. QAM is a widely used method of transmitting digital data over bandpass channels. QAM is a popular choice because of its bandwidth efficiency and its ability to compensate for linear channel distortion.

Before you start with the experiment, it is necessary that you have an understanding at some level of the following concepts:

* Transmitter design
  * Conversion of a bit stream to a discrete-time baseband signal
  * Conversion of a discrete-time basedband signal to a continuous-time signal for transmission
* Receiver design
  * Conversion of a continuous-time received signal to a discrete-time basedband signal
  * Conversion of a discrete-time baseband signal to a bit stream

## Reading assignment

* Software Receiver Design by C. Richard Johnson, Jr., William A. Sethares and Andrew Klein
  * Sections 12.1-12.4
  * Sections 13.1-13.3
  * Chapter 16
* Course reader
* Lecture 15 slides on QAM Transmission
* Lecture 16 slides on QAM Reception
* Appendix H Modulation Example Handout
* Appendix I Modulation Summary Handout
* Appendix M Symbol Recovery Handout
* Appendix P Communication Performance of PAM vs. QAM Handout
    
## Lab 6 instructions

In this exercise, we will use QAM to transmit the tree image.


### Setting up the transmitter on the STM32 board

1. In lab.c, create an array containing the data stream to transmit. The data stream will have 32 bits of header followed by 16384 bits for the (scrambled) tree image. If we store it as an array of 32-bit words, then we will need an array of length 513.

    ```
    uint32_t data_stream[513] = {0};
    ```
    
    For the header sequence, we will use the output of a maximal-length 5-bit LFSR. Rather than repeat the steps from lab 4 to generate the sequence, you can simply copy the provided sequence which has been computed for you.
    
    ```
    data_stream[0] = 0x967c6ea1;
    ```
    
    The tree data is the same as labs four and five. Populate the remainder of the `data_stream` array with the tree.
    
    ```
    for (i_word = 0; i_word < 512; i_word+=1)
	{
	    data_stream[i_word+1] = tree[i_word];
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

3. In lab.c prepare the variables necessary to use the CMSIS library function for the interpolating filters. Take a moment to read the [description of the FIR interpolator function.][1]

    ```
    arm_fir_interpolate_instance_f32 filter_instance[2];
    uint8_t upsampling_factor = 16;
    uint16_t num_taps = 64;
    float32_t state[2][(FRAME_SIZE/4) + 4 - 1] = {0};
    float32_t filter_in[2][FRAME_SIZE/64] = {0};
    float32_t filter_out[2][FRAME_SIZE/4] = {0};
    float32_t cos_lut[4] = {1,0,-1,0};
    float32_t sin_lut[4] = {0,1,0,-1};
    uint32_t i_lut = 0;
    ```
    
    Also create variables to keep track of our current word and bit position in the data stream (just as you did in lab 4). You can reuse the variables you used for the PN sequence as long as you reset them.
    
    ```
    uint32_t i_word = 0;
    uint32_t i_bit = 0;
    ```
    
4. Initialize the interpolating filters in the lab_init function:

    ```
    arm_fir_interpolate_init_f32 (&filter_instance[0], upsampling_factor, num_taps, pulse_shaping_coeffs, state[0], FRAME_SIZE/4);
    arm_fir_interpolate_init_f32 (&filter_instance[1], upsampling_factor, num_taps, pulse_shaping_coeffs, state[1], FRAME_SIZE/4);
    ```
    
5. In process_input_buffer, construct the buffer of symbols that we will send to the arm_fir_interpolate_f32 function. 

    ```
    for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/64; i_sample+=1)
    {
        for (uint32_t i_comp = 0; i_comp < 2; i_comp +=1)
        {
            filter_in[i_comp][i_sample] = (data_stream[i_word] & (1<<i_bit)) >> i_bit;
            filter_in[i_comp][i_sample] = (filter_in[i_comp][i_sample]*2) - 1;

            i_bit += 1;
            if (i_bit == 32)
            {
                i_word += 1;
                i_bit = 0;
                if (i_word > 512){i_word = 0;}
            }
        }
    }
    ```

6. Perform the filtering and copy the result to the output buffer

    ```
    arm_fir_interpolate_f32 (&filter_instance[0], &filter_in[0], &filter_out[0], FRAME_SIZE/64);
    arm_fir_interpolate_f32 (&filter_instance[1], &filter_in[1], &filter_out[1], FRAME_SIZE/64);
    
    for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
    {
            input_buffer[i_sample] = OUTPUT_SCALE_FACTOR*filter_out[0][i_sample/2]*cos_lut[i_lut];
            input_buffer[i_sample] += OUTPUT_SCALE_FACTOR*filter_out[1][i_sample/2]*sin_lut[i_lut];
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
    QAM = getaudiodata(recorder)';
    save QAM QAM;
    ```
    
3. Create a spectrogram for the recorded transmission. **Compare this to your spectrogram from the PAM transmission. By how much did the bandwidth change?**

    ```
    load QAM;
    figure; spectrogram(QAM,2^10,0,2^10,48000,'yaxis');

4. Run the [QAM receiver demo][2] in MATLAB using the transmission that your recorded. Try to adjust the parameters to recover the tree image. **Show your recovered tree image to the TA.**

## Lab report contents

Enjoy Thanksgiving break!

[1]:https://arm-software.github.io/CMSIS_5/DSP/html/group__FIR__Interpolate.html
[2]:https://github.com/Real-Time-DSP/lab/raw/main/starter_code/QAM_receiver_demo.m
