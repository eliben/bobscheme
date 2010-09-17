
; Taken directly from chapter 6.4 of PAIP
;
(define (append lst1 lst2)
    (if (null? lst1)
        lst2
        (cons (car lst1) (append (cdr lst1) lst2))))

(define (binary-tree-next x)
    (list (* 2 x) (+ 1 (* 2 x))))

(define (tree-search states goal-p successors combiner)
    (write states)
    (cond
        ((null? states) #f)
        ((goal-p (car states)) (car states))
        (else (tree-search 
                (combiner (successors (car states)) (cdr states))
                goal-p successors combiner))))

(define (dfs start goal-p successors)
    (tree-search (list start) goal-p successors append))

; Prints the states in tree-search on the path to the goal
; Careful: DFS is infinite and for other goal numbers can
; run forever.
;
(dfs 1 (lambda (x) (= 8 x)) binary-tree-next)
