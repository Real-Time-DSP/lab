# Troubleshooting / Frequently Asked Questions

This page addresses bugs and issues that are commonly encountered when completing the lab exercises.

## Left and right channels

If the operation of your program is not matching the expected behavior on the signal generator/oscilloscope, try swapping the left and right channels of the cables.

## Managing multiple projects in the STM IDE

When importing the starter code for each lab, a new project is created with a new lab.c. By closing all inactive projects (right click -> close project), you can ensure that the correct source files will be used.

## Desynchronized breakpoints

On rare occasions, the STM IDE will desynchronize certain breakpoints in debug mode. This can cause your code to stop at a particular line even after the breakpoint is removed. If this occurs, the safest way to resynchronize the debugger is by creating a new project.