
(define (make-withdraw balance)
    (lambda (amount)
        (if (>= balance amount)
            (begin (set! balance (- balance amount))
                    balance)
            'no-funds)))

(define W1 (make-withdraw 100))
(define W2 (make-withdraw 500))
(write (W1 20))
(write (W2 30))
(write (W1 80))
(write (W2 100))
