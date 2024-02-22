# SAEnsemble_CV.R

library(SAEnsemble)


#################### functions ####################

strip <- function(s) {
	l <- nchar(s)
	if(l == 0) {
		return(s)
	}
	else if(substr(s, 1, 1) == " ") {
		return(strip(substr(s, 2, l)))
	}
	else if(substr(s, l, l) == " ") {
		return(strip(substr(s, 1, l-1)))
	}
	else {
		return(s)
	}
}

read.config <- function(path) {
	lines <- scan(path, what=character(), sep="\n", blank.lines.skip=T)
	config <- list()
	for(line in lines) {
		if(substring(line, 1, 1) == "#") {
			next
		}
		v <- strsplit(line, "=")
		if(length(v[[1]]) != 2) {
			next
		}
		key   <- strip(v[[1]][1])
		value <- strip(v[[1]][2])
		config[key] = value
	}
	return(config)
}

weight.type <- function(config) {
	if("WEIGHT_TYPE" %in% names(config)) {
		return(config$WEIGHT_TYPE)
	}
	else {
		return("rsq")
	}
}

num.learners <- function(config) {
	if("NUM_LEARNERS" %in% names(config)) {
		return(as.integer(config$NUM_LEARNERS))
	}
	else {
		return(0)
	}
}

weight.exponent <- function(config) {
	if("WEIGHT_EXP" %in% names(config)) {
		return(as.numeric(config$WEIGHT_EXP))
	}
	else {
		return(1.0)
	}
}

read.geno <- function(path) {
	geno1 <- read.csv(path)
	geno2 <- as.integer(as.matrix(geno1[,-1]))
	geno3 <- matrix(geno2, ncol=dim(geno1)[2]-1)
	return(t(geno3))
}

read.pheno <- function(path, trait) {
	pheno <- read.csv(path)
	return(pheno[,colnames(pheno)==trait])
}

read.samples <- function(path, trait) {
	pheno <- read.csv(path)
	return(pheno[, 1])
}

predict.by.SAEnsemble <- function(i) {
	p <- rep(0.0, num.strains)
	for(g in 1:num.div) {
		test <- CV[, i] == g
		x_train <- geno[!test, ]
		y_train <- pheno[!test]
		x_test  <- geno[test, ]
		seed <- (as.integer(config$NUM_ITERATIONS) * (num.div * (i-1) + g-1)
											+ as.integer(config$RANDOM_SEED))
		model <- make_model(X=x_train, y=y_train,
					num_main=as.integer(config$NUM_MAIN_EFFECT_VARIABLES),
					num_second=as.integer(config$NUM_SECOND_CAUSAL_VARIABLES),
					max_time=as.integer(config$MAX_TIME),
					max_interval=as.integer(config$MAX_INVARIANT_TIME),
					num_iterations=as.integer(config$NUM_ITERATIONS),
					seed=seed,
					num_threads=as.integer(config$NUM_THREADS))
		v <- predict(model=model,
						X=x_test,
						weight_type=weight.type(config),
						num_learners=num.learners(config),
						weight_exp=weight.exponent(config))
		p[test] = v
		diff_t = Sys.time() - start.time
		print(sprintf("%d %d %.1fs", i, g, as.numeric(diff_t, units="secs")),
																	quote=F)
	}
	return(p)
}

summarize.results <- function(table) {
	correlations = c()
	for(i in 1:num.repeat) {
		c1 = cor(pheno, table[, i+1])
		correlations = c(correlations, c1)
		print(c1)
	}
	
	print("---------", quote=F)
	print(mean(correlations))
}


#################### main ####################

start.time <- Sys.time()
args <- commandArgs(trailingOnly=TRUE)
path.config <- args[1]
config <- read.config(path.config)

geno <- read.geno(config$GENOTYPE)
pheno <- read.pheno(config$PHENOTYPE, config$TRAIT)
samples <- read.samples(config$PHENOTYPE)
CV <- read.csv(config$PATH_CV, row.names=1)
num.div <- max(CV)
num.repeat <- dim(CV)[2]

num.strains <- length(pheno)
table <- rep(0.0, num.repeat);
for(i in 1:num.repeat) {
	p <- predict.by.SAEnsemble(i)
	table = cbind(table, p)
	print(cor(pheno, p))
	print("---------", quote=F)
}
summarize.results(table)
table[, 1] <- pheno
colnames(table) <- c("y", paste0("p", 1:num.repeat))
rownames(table) <- samples
write.csv(table, config$OUTPUT)
