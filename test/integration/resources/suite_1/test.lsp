(defun x (y) (+ y 1))
(print "Hello World!\n")

(defun fib (x)
    (if (= x 0) 0
    (if (= x 1) 1
    (+ (fib (- x 1)) (fib (- x 2))))))

(defun loop (low high cbf)
    (if (= low high)
        (cbf low)
        (progn 
            (cbf low) 
            (loop (+ low 1) high cbf))))

(loop 1 10
    (lambda (i) 
        (progn
            (print (fib i))
            (print "\n"))))