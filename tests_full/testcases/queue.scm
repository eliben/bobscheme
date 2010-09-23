; queue data structure example from SICP 3.3.2
;
(define (front-ptr queue) (car queue))
(define (rear-ptr queue) (cdr queue))
(define (set-front-ptr! queue item) (set-car! queue item))
(define (set-rear-ptr! queue item) (set-cdr! queue item))

(define (empty-queue? queue) (null? (front-ptr queue)))


(define (make-queue) (cons '() '()))

(define (front-queue queue)
  (if (empty-queue? queue)
      (write 'ERROR)
      (car (front-ptr queue))))

(define (insert-queue! queue item)
  (let ((new-pair (cons item '())))
    (cond ((empty-queue? queue)
           (set-front-ptr! queue new-pair)
           (set-rear-ptr! queue new-pair)
           queue)
          (else
           (set-cdr! (rear-ptr queue) new-pair)
           (set-rear-ptr! queue new-pair)
           queue)))) 

(define (delete-queue! queue)
  (cond ((empty-queue? queue)
         (write 'ERROR)) 
        (else
         (set-front-ptr! queue (cdr (front-ptr queue)))
         queue))) 

; Note: the output here will expose the internal representation of the queue
; as a cons of the front pointer and rear pointer. The contents of the queue
; are visible in the front pointer.
;
(define q (make-queue))
(write (insert-queue! q 'a))
(write (insert-queue! q 'b))
(write (delete-queue! q))
(write (insert-queue! q 'c))
(write (insert-queue! q 'd))
(write (delete-queue! q))

