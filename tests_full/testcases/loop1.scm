; Iterative prime-checking, the stupidest kind possible (dividing
; by numbers until num - 1)
;
(define (divides k n)
    (= (modulo n k) 0))

(define (primecheck num)
    (define (auxprimecheck divisor)
        (cond 
            ((= divisor num) #t)
            ((divides divisor num) #f)
            (else (auxprimecheck (+ 1 divisor)))))
    (auxprimecheck 2))

(write (primecheck 17))
(write (primecheck 49))
(write (primecheck 51))
(write (primecheck 71))
(write (primecheck 101))
(write (primecheck 102))
