#ifndef INC_LAB_H_
   #define INC_LAB_H_

// These statements enable the corresponding functions in lab.c.
// If one or more of the statements are commented out, the function will no longer be called.
// Use this to disable any functions you are not using to reduce overhead.
   #define PROCESS_LEFT_CHANNEL
   #define PROCESS_RIGHT_CHANNEL
   #define PROCESS_INPUT_BUFFER
   #define PROCESS_OUTPUT_BUFFER

// When PERIODIC_LOOKUP_TABLE is defined, the output signal will not be derived from the input.
// The process_left_channel, process_right_channel, and process_input_buffer functions are unused.
// Instead, whatever values are contained in the output buffer will repeatedly be sent to the DAC.
//   #define PERIODIC_LOOKUP_TABLE

// When ENABLE_VISIALIZATION is defined, the LCD will show the spectrum of the input signal.
// Disable this before modifying the process_input_buffer function in lab.c.
// See the display_spectrum function in main.c.
   #define ENABLE_VISUALIZATION

// FRAME_SIZE specifies the length of the circular buffer that is used for audio I/O.
// While data is collected on one half of the buffer, the user can process the other half.
// The spectrum visualization expects a frame size of 8192.
// Disable visualization before changing the frame size.
   #define FRAME_SIZE 8192U

// SAMPLE_RATE applies to to both the input and output.
// Values of 44,100 and 16,000 have been tested extensively.
// Other values may require additional clock configuration
   #define SAMPLE_RATE 44100U

#endif /* INC_LAB_H_ */
