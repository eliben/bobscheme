(define (make-multiplier bynum)
    (lambda (x)
        (* x bynum)))

(define mult4 (make-multiplier 4))
(define mult2 (make-multiplier 2))

(write (mult4 5))
(write (mult2 5))
