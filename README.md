# parallelmeshsimplification
Implements Parallel Implementation of 3D mesh simplification using Quadric Error Metrics and Edge Length


## Quick update (2021-02-21)

After 5 years of being done with this project, I've revisited this repository and thought of some improvements I could do to make the code more interesting.

When I worked on this project (for my MSc thesis), our best computer was a 64GB Intel 16-core i5 (maybe?), and we had some relatively powerful GPUs. The GPUs could barely hold triangular meshes with a few million points. I believe some of the models used in the experiments were too large to be loaded in the GPU. But technological improvement is not all that has happened since then. I sometimes wonder how much I could have made this project more interesting and usable had I had the experience I acquired in these 5 years.

That being said, here is a list of simple improvements:

Python wrapper: Wrap these methods in Python 3. Should only take a few. Might even work better.
Object viewer: Since we'll be using Python, create an object viewer that can show loaded objects in real-time, and show the simplification intermediate steps as if in a video or animation.
GPU: Using GPU with Python has become quite simpler these days. PyCuda and scikit-cuda are very helpful.
