; Abstraction of a binary tree. Each tree is recursively defined as a list
; with the entry (data), left subtree and right subtree. Left and right can
; be null.
;
(define (entry tree) (car tree))
(define (left-branch tree) (cadr tree))
(define (right-branch tree) (caddr tree))
(define (make-tree entry left right)
    (list entry left right))

(define (make-new-set)
    '())

(define (element-of-set? x set)
    (cond
        ((null? set) #f)
        ((= x (entry set)) #t)
        ((< x (entry set)) (element-of-set? x (left-branch set)))
        ((> x (entry set)) (element-of-set? x (right-branch set)))))

(define (adjoin-set x set)
    (cond 
        ((null? set) (make-tree x '() '()))
        ((= x (entry set)) set)
        ((< x (entry set))
            (make-tree  (entry set)
                        (adjoin-set x (left-branch set))
                        (right-branch set)))
        ((> x (entry set))
            (make-tree  (entry set)
                        (left-branch set)
                        (adjoin-set x (right-branch set))))))


(define myset 
    (adjoin-set 25 
        (adjoin-set 13 
            (adjoin-set 72
                (adjoin-set 4 (make-new-set))))))

(write (element-of-set? 4 myset))
(write (element-of-set? 5 myset))
(write (element-of-set? 26 myset))
(write (element-of-set? 25 myset))
