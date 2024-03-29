// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

#ifdef RCPP_USE_GLOBAL_ROSTREAM
Rcpp::Rostream<true>&  Rcpp::Rcout = Rcpp::Rcpp_cout_get();
Rcpp::Rostream<false>& Rcpp::Rcerr = Rcpp::Rcpp_cerr_get();
#endif

// make_model
List make_model(NumericMatrix X_, NumericVector y_, int num_main, int num_second, int max_time, int max_interval, int num_iterations, int seed, int num_threads);
RcppExport SEXP _SAEnsemble_make_model(SEXP X_SEXP, SEXP y_SEXP, SEXP num_mainSEXP, SEXP num_secondSEXP, SEXP max_timeSEXP, SEXP max_intervalSEXP, SEXP num_iterationsSEXP, SEXP seedSEXP, SEXP num_threadsSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< NumericMatrix >::type X_(X_SEXP);
    Rcpp::traits::input_parameter< NumericVector >::type y_(y_SEXP);
    Rcpp::traits::input_parameter< int >::type num_main(num_mainSEXP);
    Rcpp::traits::input_parameter< int >::type num_second(num_secondSEXP);
    Rcpp::traits::input_parameter< int >::type max_time(max_timeSEXP);
    Rcpp::traits::input_parameter< int >::type max_interval(max_intervalSEXP);
    Rcpp::traits::input_parameter< int >::type num_iterations(num_iterationsSEXP);
    Rcpp::traits::input_parameter< int >::type seed(seedSEXP);
    Rcpp::traits::input_parameter< int >::type num_threads(num_threadsSEXP);
    rcpp_result_gen = Rcpp::wrap(make_model(X_, y_, num_main, num_second, max_time, max_interval, num_iterations, seed, num_threads));
    return rcpp_result_gen;
END_RCPP
}
// predict
NumericVector predict(List model, NumericMatrix X_, std::string weight_type, int num_learners, double weight_exp);
RcppExport SEXP _SAEnsemble_predict(SEXP modelSEXP, SEXP X_SEXP, SEXP weight_typeSEXP, SEXP num_learnersSEXP, SEXP weight_expSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< List >::type model(modelSEXP);
    Rcpp::traits::input_parameter< NumericMatrix >::type X_(X_SEXP);
    Rcpp::traits::input_parameter< std::string >::type weight_type(weight_typeSEXP);
    Rcpp::traits::input_parameter< int >::type num_learners(num_learnersSEXP);
    Rcpp::traits::input_parameter< double >::type weight_exp(weight_expSEXP);
    rcpp_result_gen = Rcpp::wrap(predict(model, X_, weight_type, num_learners, weight_exp));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_SAEnsemble_make_model", (DL_FUNC) &_SAEnsemble_make_model, 9},
    {"_SAEnsemble_predict", (DL_FUNC) &_SAEnsemble_predict, 5},
    {NULL, NULL, 0}
};

RcppExport void R_init_SAEnsemble(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
