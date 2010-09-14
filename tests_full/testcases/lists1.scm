(define (list-length lst)
    (if (null? lst)
        0
        (+ 1 (list-length (cdr lst)))))

(define (append lst1 lst2)
    (if (null? lst1)
        lst2
        (cons (car lst1) (append (cdr lst1) lst2))))

(define (list-reverse lst)
    (if (null? lst)
        '()
        (append (list-reverse (cdr lst)) (list (car lst)) )))

; Takes a list and returns a list of (cons elem elem) for each elem in the 
; given list.
;
(define (pairify lst)
    (if (null? lst)
        '()
        (cons 
            (cons (car lst) (car lst)) 
            (pairify (cdr lst)))))

(write (list-length (list 1 2 3 4 5)))
(write (append '(1 2 3 4) (list 9)))
(write (list-reverse (list 1 2 3 4 5)))
(write (pairify (list 1 2 3 4)))
