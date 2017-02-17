Parallel Map Algebra,
Jesús Carabaño Bravo <jcaraban@abo.fi>

Map Algebra is a mathematical formalism for the processing and analysis of raster geographical data (Tomlin 90). Relevant operations cover from convolutions, reductions and element-wise functions to more complex iterative algorithms like hydrological and least cost analysis. Map Algebra becomes a powerful spatial modeling framework when embedded into a scripting language and augmented with branching, looping and callable functions.

For my PhD I design a Parallel Map Algebra implementation that runs efficiently on OpenCL devices. Users write sequential single-source Python scripts and the framework generates and executes parallel code automatically. Compiler techniques are at the core of system, from dependency analysis to loop fusion. They key challenge is minimizing memory movements, since they pose the major bottleneck to performance.
