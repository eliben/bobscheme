(define var1 '())
(define var2 '())
(define jack
    (begin
        (set! var1 (cons 5 var1))
        (set! var2 #f)
        12345))
(write var1)
(write var2)
(write jack)
