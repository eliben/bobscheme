; In Scheme everything except #f is true
(define var1 
    (if #f
        4
        5))
(define var2
    (if '(kwa)
        10
        20))

(write var1)
(write var2)
