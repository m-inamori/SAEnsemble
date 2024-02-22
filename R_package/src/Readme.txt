
#################### 7/23 ####################

http://sweettips.take-uma.net/r/r%20on%20ubuntu-%20--devtools--

sudo apt install libcrypto++ libssl-dev 

> install.packages("devtools")

やっぱりうまくいかない


#################### 7/10 ####################

cd -
R
library(devtools)
document()
build()
install.packages("../SAForest_0.0.0.9000.tar.gz", repos=NULL, type="source")
q()
n
cd -

results : [ result ] (list)
  result : list
    predictions : 
    coefficients :
    observations :
    variables : IntegerMatrix (num_vars x 3)
    correlations : 


#################### 7/9 ####################

> library(devtools)
> document()
> build()
   checking for file ‘/mnt/c/Users/PXU02/Documents/work/新予測手法/R_package/DE✔  checking for file ‘/mnt/c/Users/PXU02/Documents/work/新予測手法/R_package/DESCRIPTION’ (888ms)
─  preparing ‘SAForest’: (352ms)
✔  checking DESCRIPTION meta-information ...
─  cleaning src
   Warning: /tmp/Rtmp2d6dxx/Rbuilde272b86b00/SAForest/man/exported_funcs.Rd:10: unexpected section header '\examples'
   Warning: /tmp/Rtmp2d6dxx/Rbuilde272b86b00/SAForest/man/exported_funcs.Rd:18: unexpected END_OF_INPUT '}
   '
─  checking for LF line-endings in source and make files and shell scripts
─  checking for empty or unneeded directories
─  building ‘SAForest_0.0.0.9000.tar.gz’

[1] "/mnt/c/Users/PXU02/Documents/work/新予測手法/SAForest_0.0.0.9000.tar.gz"
> q()
Save workspace image? [y/n/c]: n


#################### 7/8 ####################

一つずつコンパイルして試す

> Rcpp::sourceCpp("SimulatedAnnealing.cpp")
Warning message:
In Rcpp::sourceCpp("SimulatedAnnealing.cpp") :
  No Rcpp::export attributes or RCPP_MODULE declarations found in source
