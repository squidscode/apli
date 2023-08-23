(defun fact (x)
    (if (= x 1) 1
    (* (fact (- x 1)) x)))

(write (fact 15))
(terpri)