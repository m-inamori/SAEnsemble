# Generated by using Rcpp::compileAttributes() -> do not edit by hand
# Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

make_model <- function(X_, y_, num_main, num_second, max_time, max_interval, num_iterations, seed, num_threads) {
    .Call(`_SAEnsemble_make_model`, X_, y_, num_main, num_second, max_time, max_interval, num_iterations, seed, num_threads)
}

predict <- function(model, X_, weight_type, num_learners, weight_exp) {
    .Call(`_SAEnsemble_predict`, model, X_, weight_type, num_learners, weight_exp)
}

