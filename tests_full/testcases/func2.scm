(define (map proc lst)
    (if (null? lst)
        '()
        (cons (proc (car lst)) (map proc (cdr lst)))))

(define (filter proc lst)
    (cond 
        ((null? lst) '())
        ((proc (car lst)) (cons (car lst) (filter proc (cdr lst))))
        (else (filter proc (cdr lst)))))

(write (map (lambda (x) (* x x)) '(1 2 3 4)))
(write (filter (lambda (x) (> x 2)) '(1 2 3 4)))
