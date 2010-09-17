(define (even? n)
    (if (= n 0)
        #t
        (odd? (- n 1))))
; comment
(define (odd? n)
    (if (= n 0)
        #f
        (even? (- n 1))))

(write (even? 5))
(write (odd? 5))
(write (even? 52))
(write (odd? 52))
