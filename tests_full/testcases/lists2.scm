; Some list modifying with set-car! and set-cdr!
;

(define myp (cons 1 2))
(write myp)
(set-car! myp 5)
(write myp)
(set-cdr! myp (cons 4 (cons 6 '())))
(write myp)

