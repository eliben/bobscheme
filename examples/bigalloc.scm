(define bowl (cons '() '()))

(define (attach ni)
    (if (= ni 0)
        0
        (begin
            (set! bowl (cons (cons ni ni) bowl))
            (attach (- ni 1)))))

(attach 700)
(set! bowl 40)
(define joe 2)


    
