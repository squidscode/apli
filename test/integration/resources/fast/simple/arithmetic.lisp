(defun triple (X)
  "Compute three times X."  
  (* 3 X))                  


(defun negate (X)
  "Negate the value of X."  
  (- X))

(defun write-n (num)
  (progn 
    (write num)
    (terpri)))

(write-n (negate 10))
(write-n (triple 50))
(write-n (+ 10 (* 1) (/ 21 3)))