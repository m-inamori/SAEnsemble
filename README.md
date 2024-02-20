# Project Name
SAEnsemble
## Overview
A new regression method that allows for interaction. Select the variables to be used in multiple regression using simulated annealing, and perform this many times to create an ensemble.
## Installation
### Python library
python setup.py install
### C++
make
## How to Use Test Data
python scripts/predict.py test_data/configs/test1.config<br>
python scripts/predict_CV.py test_data/configs/test2.config<br>
../predict_SAE test_data/configs/test1_cpp.config<br>
../predict_SAE test_data/configs/test2_cpp.config
## License
