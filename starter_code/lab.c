/*
This file contains the functions you will modify in order to complete the labs.
These functions allow you to perform DSP on a per-frame or per-sample basis.
When processing on a per-sample basis, there are separate functions for left and right channels.
When processing on a per-frame basis, the left and right channels are interleaved.
The sample rate and frame size can be modified in lab.h.
You can also configure which of the four functions are active in lab.h
*/

#include "main.h"
#include "lab.h"

//These functions allow estimation of the number of elapsed clock cycles
extern void tic(void);
extern uint32_t toc(void);

//Function to display a binary image
void display_image(uint32_t* binary_image, uint32_t width, uint32_t height);

//variables used for the spectrum visualization
extern arm_rfft_fast_instance_f32 fft_inst;
extern float32_t fft_in[FRAME_SIZE/4];
extern float32_t fft_out[FRAME_SIZE/4];
extern float32_t fft_mag[FRAME_SIZE/8];

//declare variables local to this file
uint32_t elapsed_cycles;

/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table or run code that does not require I/O
*/
void lab_init(int16_t* output_buffer)
{
	return;
}

/*
This function will be called each time a complete frame of data is recorded.
Modify this function as needed.
Default behavior:
	1. Deinterleave the left and right channels
	2. Combine the two channels (by addition) into one signal
	3. Save the result to the fft_in buffer which will be used for the display
	4. The original audio buffer is left unchanged (passthrough)
*/
void process_input_buffer(int16_t* input_buffer)
{
	int16_t left_sample;
	int16_t right_sample;
	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
	{
		left_sample = input_buffer[i_sample];
		i_sample +=1;
		right_sample = input_buffer[i_sample];
		fft_in[i_sample/2] =  (((float32_t) left_sample) + ((float32_t) right_sample))/2;
	}
	arm_rfft_fast_f32(&fft_inst, fft_in, fft_out, 0);
	arm_cmplx_mag_f32(fft_out, fft_mag, FRAME_SIZE/8);
	return;
}

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the left channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_left_sample(int16_t input_sample)
{
	tic();
	int16_t output_sample;
	output_sample = input_sample;
	elapsed_cycles = toc();
	return output_sample;
}

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the right channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_right_sample(int16_t input_sample)
{
	tic();
	int16_t output_sample;
	output_sample = input_sample;
	elapsed_cycles = toc();
	return output_sample;
}

/*
This function provides another opportunity to access the frame of data
The default behavior is to leave the buffer unchanged (passthrough)
The buffer you see here will have any changes that occurred to the signal due to:
	1. the process_input_buffer function
	2. the process_left_sample and process_right_sample functions
*/
void process_output_buffer(int16_t* output_buffer)
{
	return;
}
