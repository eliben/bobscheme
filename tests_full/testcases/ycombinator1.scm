; This isn't yet the real Y combinator, but an example of what it's needed
; for. A function is passed to itself to implement recursion without explicit
; naming
;
(write
  (((lambda (procedure)
      (lambda (n)
        (if (zero? n)
            1
            (* n ((procedure procedure) (- n 1))))))
    (lambda (procedure)
      (lambda (n)
        (if (zero? n)
            1
            (* n ((procedure procedure) (- n 1)))))))
   5))

