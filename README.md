add CUDA support to the IMP build system

See ChangeLog.md for more info.
close #418

Experiment with building CUDA with IMP. So far just in scratch. You can stick `.cu` files into `modules/scratch/cuda` and modify `modules/scratch/cuda/CMakeLists.txt` and everything should just work.