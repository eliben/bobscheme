(write (eqv? #f #t))
(write (eqv? #f #f))
(write (eqv? '() '()))
(write (eqv? 5 (+ 1 4)))
(write (eqv? 6 #f))

(define zara 'zara)
(write (eqv? zara 'zara))
(write (eqv? 'zara 'zara))

(define joe '(1 2 3))
(write (eqv? joe '(1 2 3)))
(write (eqv? joe joe))
