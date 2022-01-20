# [Real-time DSP lab manual](https://real-time-dsp.github.io/lab/)

## Organization

The source materials consist of MyST markdown files (.md) which are built into a webpage using [Jupyter Book](https://jupyterbook.org/intro.html)

## Contribute

If you would like to be added as a contributor to this repository, contact [Dan Jacobellis](https://danjacobellis.net)

## Building and hosting

The [build.py](build.py) script will create two versions of the webpage.

`python build.py`

The first version is placed in the /docs folder where github pages automatically hosts the webpage. For links to other course materials in this version, the full ece.utexas.edu URL is used. This version will be automatically published to https://real-time-dsp.github.io/lab/ whenever changes are made.

The build.sh script creates a second version of the HTML files in stm32h735gdk.zip. This version uses relative links to reference any files in the hierarchy higher than 'courses/realtime/lectures/laboratory/stm32h735gdk' (such as lecture slides, matlab info, etc).