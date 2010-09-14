(define (repeat-write times sym)
    (if (> times 0)
        (begin
            (write sym)
            (repeat-write (- times 1) sym))))

(repeat-write 3 'hello)
