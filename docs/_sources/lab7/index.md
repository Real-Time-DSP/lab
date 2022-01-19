# Lab 7. Vocoder and Guitar Effects

## Aim of the experiment

This experiment demonstrates audio effects commonly applied to harmonic instruments like the guitar.

## Reading assignment

* Lab 7 primer
    
## Lab 7 instructions

### Vocoder training in MATLAB

In this exercise, we will encode a voice recording into a matrix of IIR filter coefficients.

1. Record or generate a short segment of speech and save it to a file with 48kHz sampling rate.

    ```
    NET.addAssembly('System.Speech');
    speech_synth = System.Speech.Synthesis.SpeechSynthesizer;
    SetOutputToWaveFile(speech_synth,'example.wav');
    Speak(speech_synth,'do re mi fa so la tee');
    Dispose(speech_synth);
    [y,fs] = audioread('example.wav'); y = resample(y,320,147); fs = 48e3;
    ```
    
    optionally, remove silence from the beginning and end of the clip
    
    ```
    y = flipud(y);
    ind = find(abs(y) > 5e-3);
    y(1:ind(1)) = [];
    y = flipud(y);
    ind = find(abs(y) > 5e-3);
    y(1:ind(1)) = [];
    ```
    
2. Break the signal into frames of length $L = \frac{\text{sample rate}}{\text{vocoder parameter update rate}}$

    ```
    L = 1024; N = floor(length(y)/L);
    y(N*L+1:end) = [];
    y = reshape(y,L,N);
    ```

3. Learn the filter corresponding to each frame of data

    ```
    order = 16;
    a = zeros(N,order);
    b = zeros(N,1);
    for i_frame = 1:N
        [r,lg] = xcorr(y(:,i_frame),'biased'); r(lg<0) = [];
        a(i_frame,:) = levinson(r,order-1);
        b(i_frame) = 1./freqz(1,a(i_frame,:),1);
    end
    a(isnan(a)) = 0;
    b(isnan(b)) = 0;
    ```
    
4. Format the coefficients for C

    ```
    num_coeffs = sprintf('%f,',b)
    den_coeffs = sprintf( ['{', repmat('%f,',[1,order]) '},\n'], a')
    ```    
    
### Vocoder effect in C

In this exercise, you will apply the learned vocal filter to input data in real time.

1. Define `L`, `N`, and `ORDER` based on the vocoder filters from MATLAB

    ```
    #define L 1024
    #define N 68
    #define ORDER 16
    ```

2. Initialize a matrix with the filter coefficients computed in MATLAB.

    ```
    float32_t B[N] = {<exported numerator coefficients>};
    float32_t A[N][ORDER] = {<exported denominator coefficients>};
    ```
    
3. Create variables to track the current filter index

    ```
    uint32_t i_sample = 0;
    uint32_t i_filter = 0;
    ```
    
4. Create an array to hold the previous output values

    ```
    float32_t y[ORDER] = {0};
    ```
    

5. In process_left_sample, apply the all-pole IIR filter

    ```
    y[0] = B[i_filter]*INPUT_SCALE_FACTOR*input_sample;
	for (uint32_t delay = 1; delay < ORDER; delay+=1)
	{
		y[0] -= y[delay]*A[i_filter][delay];
	}

	output_sample = OUTPUT_SCALE_FACTOR*y[0];

	for (uint32_t delay = ORDER-1; delay > 0; delay-=1)
	{
		y[delay] = y[delay-1];
	}
    ```
    
6. Increment the variables which track the current filter index

    ```
    i_sample += 1;
    if (i_sample == L)
    {
        i_sample = 0;
        i_filter += 1;
        if (i_filter == N){i_filter = 0;}
    }
    ```
    
7. In process_right_sample, set the output to be identical to the left channel

    ```
    output_sample = OUTPUT_SCALE_FACTOR*y[0];
    ```

8. Using a separate phone/laptop, provide any input which contains harmonically rich instrument(s), like a guitar, violin, or synthesizer. Verify that the output shares characteristics of the original speech signal.

    One option is to use a [synthesizer app in your browser controlled by the top row (QWERTY...) of your keyboard](https://www.errozero.co.uk/stuff/poly/).

   
### Flanger

In this exercise, you will implement the flanger using the feedforward form of the comb filter.

$$y[n] = x[n] + \alpha x\left[n-K[n]\right]$$

$$K[n] = \frac{R}{2} \left( \cos(\omega_{\text{flanger}}) + 1 \right)$$


1. Define the parameters of the flanger. For this example, the flanger has frequency of 0.5 Hz and a maximum delay of 10 ms so $\omega_{\text{flanger}} = 2 \pi \frac{0.5}{48000} = 6.5 \times 10^{-05} \frac{\text{radians}}{\text{sample}}$ and $R = 48000 \text{ Hz} \times 10 \text{ ms} = 480 \text{ samples}$

    ```
    #define alpha 0.75
    #define R 480
    #define BUFFER_LENGTH 481
    #define OMEGA 0.0000654498469497874
    #define FLANGER_PERIOD 96000
    ```

2. Create two circular buffers (one for each channel) to store previous input values. Also create a counter to track the oscillation.

    ```
    int16_t x_circ[2][BUFFER_LENGTH] = {0};
    int32_t i1 = 0;
    int32_t i2 = 0;
    int32_t n = 0;
    ```
    
3. In process_left_sample, update the indices of the circular buffer and add the most recent sample.

    ```
    i2 = i1;
    i1 = (i1+1) % BUFFER_LENGTH;
    x_circ[0][i2] = input_sample;
    ```
    
4. In process_right_sample, add the newest input sample but leave the indices unchanged

    ```
    x_circ[1][i2] = input_sample;
    ```
    
5. Implement the flanger

    ```
    float32_t x_current;
    float32_t x_delayed;
    size_t Kn;
    size_t ind;
    
    x_current = INPUT_SCALE_FACTOR*x_circ[<channel>][i2];
    
    Kn = 0.5*R*(arm_cos_f32(OMEGA*n) + 1.0);

    if (Kn > i2)
    {
        ind = (i2 + BUFFER_LENGTH) - Kn;
    } else
    {
        ind = i2 - Kn;
    }
    
    x_delayed = INPUT_SCALE_FACTOR*x_circ[<channel>][ind];

    output_sample = OUTPUT_SCALE_FACTOR*(alpha*x_delayed + x_current);
    ```

6. In process_right_sample only, increment the counter

    ```
    n = (n+1) % FLANGER_PERIOD;
    ```

7. Using a separate phone/laptop, play a recording which contains harmonically rich instrument(s), like a guitar, violin, or synthesizer, and listen to the output.

### Distortion

In this exercise, you will implement harmonic distortion by clipping.

1. Add a nonlinearity to the input signal to increase the energy at higher harmonics. Whenever the absolute value of the signal exceeds some threshold $L$, set the value equal to $\text{sgn}(x[n]) L$
       
    ```
    #define CLIP 0.1
    ```

    ```
    float32_t x;
    float32_t y;
    x = INPUT_SCALE_FACTOR*input_sample;
    if (x < -CLIP)
    {
        y = -CLIP;
    }
    else if ( x > CLIP )
    {
        y = CLIP;
    }
    else
    {
        y = x;
    }
    output_sample = OUTPUT_SCALE_FACTOR*y;
    ```
        
     Ensure that the same effect is active on both channels.

2. Using a separate phone/laptop, play a recording which contains harmonically rich instrument(s), like a guitar, violin, or synthesizer, and listen to the output.

## Lab report contents

Good luck on exam two!