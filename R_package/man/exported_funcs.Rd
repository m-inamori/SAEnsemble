\name{make_model}
\alias{make_model}
\alias{predict}
\title{Package functions}
\usage{
make_model(X, y)

predict(model, X)
}
\examples{
X <- matrix((1:6)+0.5, rows=2)
y <- c(1.2, 2.3)
model <- make_model(X, y)
X1 <- matrix((1:6)-0.2, rows=2)
y1 <- predict(model, X1)
print(y1)
}
