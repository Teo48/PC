#lang racket
(define (stream-zip-with f s1 s2)
  (if (or (stream-empty? s1) (stream-empty? s2))
      empty-stream
      (stream-cons (f (stream-first s1) (stream-first s2))
                   (stream-zip-with f (stream-rest s1) (stream-rest s2)))))
(define my-stream
  (stream-cons
   0
   (stream-cons
    1
    (stream-zip-with +
                my-stream
                (stream-rest
                 my-stream)))))

(stream-take (stream-take (stream-map (lambda (x) (/ 1 x))
                         (stream-rest my-stream)) 5) 5)