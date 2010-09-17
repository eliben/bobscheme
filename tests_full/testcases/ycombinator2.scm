; The applicative-order Y combinator
;
(define Y
 (lambda (X)
  ((lambda (procedure)
     (X (lambda (arg) ((procedure procedure) arg))))
   (lambda (procedure)
     (X (lambda (arg) ((procedure procedure) arg)))))))

(define F*
 (lambda (func-arg)
  (lambda (n)
    (if (zero? n)
        1
        (* n (func-arg (- n 1)))))))

(define fact (Y F*))
(write (fact 8))

