(define (func a b)
  (lambda (x)
    (+ a b x)))

(write ((func 4 5) 10))

