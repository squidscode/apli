(defun recursive-fn (num)
    (if (= num 0)
        "do nothing!"
        (progn 
            (write num)
            (terpri)
            (recursive-fn (- num 1)))))

(recursive-fn 20)