(defun fib (x)
    (if (= x 0) 0
    (if (= x 1) 1
    (+ (fib (- x 1)) (fib (- x 2))))))

(defun my-loop (low high call-back)
    (if (= low high) 
        (funcall call-back low)
        (progn 
            (funcall call-back low)
            (my-loop (+ low 1) high call-back))))

(my-loop 1 10
    (lambda (x)
        (write x)
        (write-string "    ")
        (if (<= x 9) (write-string " ") 1)
        (write (fib x))
        (terpri)))